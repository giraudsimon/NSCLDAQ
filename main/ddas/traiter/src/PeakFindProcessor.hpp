// PeakFindProcessor.h
//
// Author : Jeromy Tompkins
// Date   : 8/14/2013


#ifndef PEAKFINDPROCESSOR_H
#define PEAKFINDPROCESSOR_H
    
#include "Trace.hpp"
#include "AlgoIterator.hpp"
#include "TObject.h"

namespace TrAnal 
{

/// Result of FindPeak algorithm
template<class T>
class PeakFindProcResult 
{
    public:
    double max; ///< the value of the maximum found 
    double umax; ///< the uncertainty 
    
    TrIterator<T> index;  ///< points to the peak value

    // ROOT dictionary generation 
    /// \cond
    ClassDef(PeakFindProcResult,1);
    /// \endcond

}; // end of PeakFindProcResult class

/// Finds the local maximum on range defined as [begin,end)
/**
 * @param begin points to the first valid value within the range
 * @param end points to the first value outside of the range
 * @return a struct containing the value and location of the maximum 
 */
template<class T>
PeakFindProcResult<T> FindPeak(const TrIterator<T>& begin, const TrIterator<T>& end)
{
    
    double max=0;
    double current=0;
    TrIterator<T> index=begin; 

    TrIterator<T> it=begin;
    while ( it<end ) {
        current = *it;
        if (current>max) {
            max = current;
            index = it;
        }
        ++it;
    }

    // We successfully completed the algorithm...store the result
    // and pass it to the caller
    PeakFindProcResult<T> res;
    res.max = max;
    res.umax = 0;
    res.index = index;

    return res;
} // end of FindPeak

/// Finds maximum value of AlgoIterator between begin and end
/**
 * @param begin starting location of the AlgoIterator (this will be iterated)
 * @param end points to the first value outside of the range
 * @return a struct containing the value and location of maximum, which is min_extent of AlgoIterator
 */
template<class T>
PeakFindProcResult<T> FindPeak(AlgoIterator<T>& begin, const TrIterator<T>& end)
{
    
    double max=0;
    double current=0;
    TrIterator<T> index=begin.min_extent(); 

    AlgoIterator<T>& it=begin;
    while ( it<end ) {
        current = *it;
        if (current>max) {
            max = current;
            index = it.min_extent();
        }
        ++it;
    }

    // We successfully completed the algorithm...store the result
    // and pass it to the caller
    PeakFindProcResult<T> res;
    res.max = max;
    res.umax = 0;
    res.index = index;

    return res;
} // end of FindPeak

} // end namespace
#endif
