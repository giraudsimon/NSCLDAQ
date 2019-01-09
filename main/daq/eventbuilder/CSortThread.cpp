/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013-2018
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CSortThread.cpp
# @brief  Implementation of thread to sort together extents of fragment queues
# @author <fox@nscl.msu.edu>
*/

#include "CSortThread.h"
#include "COutputThread.h"


// Timestamp comparison for sorted merge:

static bool
TsCompare(std::pair<time_t, EVB::pFragment>& q1, std::pair<time_t, EVB::pFragment>& q2)
{
  return q1.second->s_header.s_timestamp < q2.second->s_header.s_timestamp ;
}


/**
 * construction
 *   Just need to salt away the fragment handler:
 */
CSortThread::CSortThread() : Thread(*(new std::string("SortThread"))),
m_pHandler(0)
{}

/**
 * destruction
 *    Clear the buffer queue.  The assumption is that no data is coming in
 *    in other threads, so once it's empty we're done.
 */
CSortThread::~CSortThread()
{
    clearBufferQueue();

}

/**
 * run
 *   Thread entry point
 *   Locic is pretty simple:
 *   - get a set of fragments from the buffer queue
 *   - merge those fragments into a single fragment list
 *   - Queue that single fragment list to the output thread
 *   - Release the storage taken by the fragments and their input lists.
 *   - Rinse and repeat.
 */
void
CSortThread::run()
{
   
    while (1) {
        Fragments* newData = dequeueFragments();
        if (!m_pHandler)
            m_pHandler = CFragmentHandler::getInstance(); // We know frag handler construction is done.
        FragmentList* mergedFrags = new FragmentList;  // Deleted by output thread.
        merge(*mergedFrags, *newData);
        
        //m_pHandler->observe(*mergedFrags);
        
        COutputThread* pOutput = m_pHandler->getOutputThread();
        pOutput->queueFragments(mergedFrags);
        releaseFragments(*newData);
    }
}
/**
 * queueFragments
 *    Called by the fragment handler thread to queue fragments for processing
 *    by the sort thread
 *
 * @param frags - References a dynamically allocated Fragments object that
 *     will be processed in this thread.
 */
void
CSortThread::queueFragments(Fragments& frags)
{
    m_fragmentQueue.queue(&frags);
}
/**
 * dequeueFragments
 *    Returns the least recently queued fragment.  Blocks if needed.
 *  @return Fragments*
 */
CSortThread::Fragments*
CSortThread::dequeueFragments()
{
    return m_fragmentQueue.get();
}
/**
 * merge
 *    Given a vector of sorted lists, merges them into a single sorted vector.
 *
 *  @param[out] result - list into which the fragments will be merged
 *                      (could be empty).
 *  @param[in] lists - vector of fragment lists to merge
 */
void
CSortThread::merge(FragmentList& result, Fragments& lists)
{
    for (int i = 0; i < lists.size(); i++) {
        result.merge(*lists[i], TsCompare);
    }
}
/**
 * clearBufferQueue
 *    Empties out the buffer queue and
 *    destroys all data in it.
 */
void
CSortThread::clearBufferQueue()
{
    Fragments* pFrags;
    while(m_fragmentQueue.getnow(pFrags)) {
        releaseFragments(*pFrags);
    }
}
/**
 * releaseFragments
 *    Gets rid of all fragment lists and then the fragments object itself.
 *
 * @parm frags - fragments to get rid of.
 */
void
CSortThread::releaseFragments(Fragments& frags)
{
#ifdef UNDEFINED              // I think the output thread has to delete these?!?
    for (int i =0; i < frags.size(); i++) {
        releaseFragmentList(*frags[i]);   
    }
#endif
    delete &frags;
}
/**
 * releaseFragmentList
 *    Frees all the fragments in a fragments list and the list iself
 *
 *  @param frags -fragment list reference.
 */
void
CSortThread::releaseFragmentList(FragmentList& frags)
{
    while(!frags.empty()) {
        delete frags.front().second;
        frags.pop_front();
    }
    delete &frags;
}



