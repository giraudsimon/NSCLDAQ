
/*! 
  
  \page traiter
  \author Jeromy Tompkins
  \date   8/14/2013

  \section overview Overview

  The Traiter (i.e. TRAce-ITERator) package is meant to analyze traces using iterator type objects 
(TrIterator). Though a generic trace type is defined by the Basic_Trace interface, one need not 
have a waveform stored as a Basic_Trace object. The reason for this is that all of the algorithms 
are based on the TrIterator type, which is constructed from a pointer.

The foundation of the analysis library is, therefore, the TrIterator template class. All algorithms 
built within the library use these objects to access the data of a Trace object. They typically 
operate over a range specified by either a begin and end iterator object, akin to the STL algorithms. 
Because these range specifications are so prevalent, the TrRange class is provided, which is really 
just a class containing a begin and end TrIterator. There is additional functionality though in this
 class to allow the range to iterate itself.

The algorithms are separable into static algorithms and algorithm iterator (AlgoIterator) objects. 
The former are simply functions that perform primitive operations like computing a baseline 
(ComputeBaseline). The latter are objects that iterate and evaluate to a value based on their
 internal state. The simplest example of an AlgoIterator-derived object is the SumIterator. 
It maintains a range definition and sums up all of the values within that range. As it iterates, 
it updates the sum. 

It is important to note that these functions place responsibility on the user to provide valid 
range specifications over which to compute values. For example, the following code would not 
necessarily complain:
    
    \code {.cpp}
    
    // ... code ...

    // define a very primitive trace
    int trace[] = {0,1,2,3,4,5,6,7,8,9};

    // construct an iterator pointer to the first element
    TrIterator<int> begin(trace);

    // construct an iterator that references well beyond the end of the array
    TrIterator<int> end(trace+12);

    // Run the algorithm 
    // On a good day, this would segfault. On a less fortunate day
    // this would run happily and create gibberish.
    BaseLineProcResult res = ComputeBaseLine(begin, end);
    
    // ... more code ..

    \endcode

Finally, to clean up the code, some precompiled typedefs are provided if you include TraceDefs.h.

TraceS = TraceT<uint16_t>

TrRangeS = TrRange<uint16_t>

TrIterS = TrIterator<uint16_t>

SumIterS = SumIterator<uint16_t>

TrFilterS = TrapFilter<uint16_t>

Also, all of the algorithms can be included by including the TraceAlgorithms.h header.

The user can now write code such as:

    \code {.cpp}

    #include "TraceDefs.h"
    #include "TraceAlgorithms.h"

    using namespace TrAnal;

    ddaschannel* dch = new ddaschannel;

    // fill the ddaschannel object with data
    // ...

    TraceS trace(dch->GetTrace());

    TrRangeS range(trace.begin(), trace.end());
    
    TrIterS it = Threshold(range, 20);

    // ... 
    
    \endcode

\section building Building the package

  The package is now included in the unified DDAS softgware package and will
be installed when you install it.

 */
