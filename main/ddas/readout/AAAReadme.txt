While the DDAS Readout has been simplified greatly, it's still a complex beast
and worthy of some documentation.  The goals of the rewrite were as follows:

-   Make the logic clearer.
-   Eliminate data copying for bulk data.
-   Eliminate, where possible, dynamic memory management.

The first of these goals promotes maintainability while the last promotes
performance, as profiling of other NSCLDAQ code (specifically eventlog)
suggested that performance can be drastically improved by minimizing those
actions.


Logic clarification was done by dividing the actual acquisition code into three
classes:

ModuleReader - Responsible for getting data from a digitizer and parsing it
               into 'hits'
CHitManager  - Responsible for maintaining time ordered hits, indicating
               when a hit can be emitted and providing that hit.

Zero-copy and reduction of dynamic memory allocation were improved by
the following classes:

ReferenceCountedBuffer - is storage that can keep track of the references
                         to it by external objects.
BufferArena            - Is a class that supports re-use of
                         ReferenceCountedBufers.
ZeroCopyHit            - Is a DDAS Hit whose data are located in a
                         ReferenceCountedBuffer that came from a BufferArena.

Finally note that ZeroCopyHit is derived from RawHit which understands the
structure of a  hit but can have either locally allocated or client-provided
storage.

Finally CExperiment, the caller of the CMyEventSegment instance has been added
to allow the read code to indicate it has more events to provide prior to
entering the trigger loop again.

Let's take a high level look at how CMyEventSegment::read operates and then
drill down into how the pieces it uses function:

Read can be called for two reasons:
1.  It asked to be called because, after emitting a hit, it has more hits to
    give.
2.  CMyTrigger indicated that at least one module had data in the fifo that
    exceeded the FiFo threshold.

The first case has priority.  We want to emit as many events as possible before
reading more data;

The event segment has a CHitManager (m_sorter).
*  It asks the sorter if it has any hits that can be emitted.
*  If there are emittable hits, it calls its own emitHit method to emit the
   least recently acquired hit to the event buffer.
*  If there are still hits that can be emitted, it tells the experiment that it
   has more events it can without awaiting a new trigger.
*  Regardless, if a hit was emitted, its work is done and it returns the size
   of that hit.

If the hit manager says there were no hits to emit, we must have been called in
response to a trigger by CMyEventTrigger.  We reset the trigger (a hold over
from prior code).

The trigger maintains an array of the number of words it saw in each module
FIFO.  This makes it unecessary to ask each module again how many words it has
(there's a sanity check that can do that however by defining the preprocessor
symbol SANITY_CHECKING).

Each module with data (non zero FIFO words) is read by its module reader
(m_readers array).  The hits are then added to the hit manager which maintains
the time ordered hist read so far.  Note that each hit is a pair consisting of
a pointer to the module it came from and a pointer to  zero copy hit.  This
allows this stuff to be passed around without bulkdata copy and for the non
malloc/free storage management to be done at a module level for both the hit
objects and the buffers they came from.

The hits are handed off to the hit manager which sorts them into the set of
hits already accumulated.  If this results in emittable hits, logic identical
to the code at entry is invoked (emit the hit and ask to be called again if
there are still more hits).

Finally if the hit manager still says there are no hits to emit, we invoke the
base class reject() method which results in the event not producing a ring item.

A bit about the data structures used.  The hits are accumluated into a
std::vector<std::deque<DDASReadout::ModuleReader::HitInfo>>

Each element of the vector is a deque of hits that were read from one module.
Why not put them in a single deque?  Keeping the data from each module
separated makes the sort faster.  Good sort algorithms (e.g. quicksort) run on
order O(n*log2(n)) where log2 is the base to logarithm.  Suppose we have 5 modules
each with n hits.  Sorting them all at once gives performance O((5n)log2(5n)) =
O(5n(log2(n) + log2(5)).
).
Sorting them individually gives performance O(5nlog2(n)), where that extra term
is no longer present.  Since we do a _lot_ of sorting and merging it's worth
it.

We'll say more about the sorting of hits when we describe the hit manager.

Another point:  There's some code that might appear to be a bit strange:

	    std::deque<DDASReadout::ModuleReader::HitInfo> tmp;
            moduleHits.push_back(tmp);
            auto& hits(moduleHits.back());
...
            m_readers[i]->read(hits, words[i]);

where an empty deque is pushed in to the vector, a reference to it gotten, then
read into.  If, instead, the code looked liek:

	    std::deque<DDASReadout::ModuleReader::HitInfo> tmp;
            auto& hits(moduleHits.back());
...
            m_readers[i]->read(tmp, words[i]);
            moduleHits.push_back(tmp);

The contents of the dequeue would have to be copy constructed into the new
vector element.  The code as written allows the data to be read into the
element in place.


The ModuleReader code is realtively straight forward;
-  It has a BufferArena from which it gets buffers into which to read data.
-  It has a hit pool from which, if possible, it gets zerocopy hits and to
   which those hits are returned.

The read will allocate a buffer big enough to hold the data.   Since this comes
from its buffer arena, eventually, the buffer arena will have enough
pre-allocated data to hold all of the in-flight hits from this module.  Once
the data are read, they are parsed into a deque of
std::pair<ModuleReader*,ZeroCopyHit*>  objects (this is called HitInfo object).  The ZeroCopyHits refer to storage
in the buffer, so the hits were created without any data copy operations.
Including a pointer to the module that parsed those hits allows them to be
freed back to the right hit pool and right  buffer arena when the underlying
buffers have no more references.

(the static method ModuleReader::freeHit does this).

The most complicated bits of code are in the hit manager (CHitManager).
Specifically in trying to find efficient ways to maintain the hits sorted by
time.  This code is all triggered by calls to the addHits method.  Here's the
sequence of operations:

*  Each of the deques of hits from digitizer modules is sorted.
*  The sorted hit deques are merged using a minheap algorithm.
*  The sorted hit deque is merged into the existing sorted hit queue.

The individual deques are sorted using the std::sort algorithm which uses an
O(nLog(n)) algorithm.  The deques are merged using the minheap algorithm,which
is O(nLog(m)) where m is the number queues.  This leaves two fully sorted
deques to merge.   Merging two sorted lists is an O(n+m) problem where n and m
are the number of elements in each list.  The time windows typically ensure
that the number of existing hits is much larger than the number of new hits.

This final merge considers three cases:

- There are no existing hits.  The sorted new hits become the existing hits.
- The last existing hit (largest timestamp) has a timestamp before the first
- (smallest timestamp) new hit timstamp.   The new hits are appended to the
 existing hits.
-  The  last existing hit's timestamp is smaller than the first new hit's
   timestamp.  In that case a reduced merge/append approach is taken:
   Hits are pulled off the back of the existing hit list onto the front of a
   temporary deque until the timestamp at the back of the existing hit deque is
   less than the timestamp at the front of the new hits deque, or the existing
   hit deque is empty.  At that point, the  new hit deque and temporary deque
   formed above are merged to the back of the existing hit deque.


The algorithm for the last case takes advantage of the fact that while the hits
from the digitizers are not time ordered, the timestamp overall is
increasing.

These merge operations are implemented in the two overloaded CHitManager::merge
methods.

The end of run is one final complication.  At the end of a run, in general, the hit
manager will have a set of unflushed hits.  The DDASReadout program replaces
the "end" command.  The end replacement stops data taking in the Pixie16
modules, and flushes their FIFOs to file.  It then calls
CMyEventSegment::onEnd.   That method puts the hit manager into flush mode.  In
flush mode, haveHit will return true if there are any hits in the hit queue.
If there are hits, CExperiment::ReadEvent is called which will, in turn, call
CMyEventSegment::read as many times as needed to empty that queue.  On return,
the hit manager is taken out of flush mode and each module reader's most recent
timestamp array is zeroed out in preparation for the next run.


One last comment on container choices: std::deque vs std::list.
Both of these containers have suitable access patterns, however the
implementation of std::deque results in fewer dynamic memory allocations.

An std::list is a doubly linked list of nodes.  Each node has a payload
containing the data at that point in the list.  Each list element, therefore
requires that the node be allocated and each list element removal requires that
node be deleted.

An std::deque is implemented as a set of fixed length arrays and a pointer
array to the beginning and end of each array.  Each array contains several
deque nodes.  Therefore memory allocation/free is substantially less
granually.  Memory for a deque is only freed when the deque is destroyed and
only allocated when pushing a new item on the front or back overflows the array
of nodes at the front or back of the deque.  Therefore, in general, deques are
used rather than lists for the 'lists' of hits.
