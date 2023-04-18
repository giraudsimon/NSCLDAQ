//  Threshold.hpp
//  
//  Author : Jeromy Tompkins
//  Date   : 8/14/2013

#ifndef THRESHOLD_H
#define THRESHOLD_H

#include "Trace.hpp"
#include "AlgoIterator.hpp"

namespace TrAnal {


    /// Linear search for first value to exceed threshold
    /**
     * Searches the range [begin,end). If no 
     * value breaches the threshold value, then the return value
     * is equal to the end value.
     *
     * @param begin points to first value in search range
     * @param end points to first value outside of search range 
     * @param thresh the threshold
     * @return the first position to meet or exceed the value. Returns end if threshold search fails
     */
    template<class T>
        TrIterator<T> Threshold(const TrIterator<T>& begin,
                const TrIterator<T>& end, 
                double thresh)
        {
            TrIterator<T> it=begin;
            while ( it<end && *it<thresh ) ++it;              

            return it;
        }

    /// Linear within an explicit range
    /**
     *   Convenience function for the Threshold(const TrIterator<T>&,const TrIterator<T>&, double) form.
     *   @param range a TrRange<T> object
     *   @param thresh the threshold
     *   @return iterator pointing to first point at or above the threshold
     */
    template<class T>
        TrIterator<T> Threshold(const TrRange<T>& range, double thresh) { return Threshold<T>(range.begin(), range.end(), thresh);}

    /// Linear search for AlgoIterators
    /**
     *   Run an "AlgoIterator" until it passes a threshold or goes out of bounds. Failure to meet threshold returns end
     *
     *   @param begin the AlgoIterator object to be iterated
     *   @param end the end of search range
     *   @param thresh the threshold
     *
     *   @return min_extent of Algoiterator object after reaching or exceeding threshold.
     */
    template<class T>
        TrIterator<T> Threshold(AlgoIterator<T>& begin,
                const TrIterator<T>& end, 
                double thresh)
        {
            // Default return is end
            TrIterator<T> ret_it = end;

            // change the name of begin
            AlgoIterator<T>& it = begin;

            while ( it<end && *it<thresh ) {
                ++it;              
            }

            // Store result if threshold exceeded
            if ( *it >= thresh) ret_it = it.min_extent(); 

            return ret_it;
        }

} // end namespace
#endif
