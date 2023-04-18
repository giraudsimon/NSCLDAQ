//  BaseLineProcessor.hpp
//
//  Author : Jeromy Tompkins
//  Date   : 8/14/2013

#ifndef BASELINEPROCESSOR_H
#define BASELINEPROCESSOR_H
    
#include "Trace.hpp"
#include <cmath>
#include "Exceptions.h"
#include <TObject.h>

namespace TrAnal 
{

/// Result of the ComputeBaseline function
class BaseLineProcResult 
{
    public:
    double mean; ///< mean of values within range
    double stdev; ///< std. dev. of values within range

    // ROOT dict generation
    /// \cond
    ClassDef(BaseLineProcResult,1);
    /// \endcond
};


/// Computes the arithmetic of values within the [begin,end) range
/**
 * @param begin points to the first valid value within range
 * @param end points to the first value outside range
 */
template<class T>
double ComputeMean(const TrIterator<T>& begin, const TrIterator<T>& end)
{
   
    double sum=0;
    double n=0;
    TrIterator<T> it=begin;
    while (it<end) 
    {
        // access element
        sum += *it;
        ++n;

        // increment iterator
        ++it;
    }
    
    return sum/n;
}

/// Computes the average of values within the [begin,end) range
/**
 * @param begin points to the first valid value within range
 * @param end points to the first value outside range
 * @param mean the arithmetic mean of the values within the range 
 *
 * The computation for the set of n points, \f$arr\f$, is defined as:
 *
 *  \f[
 *      \sigma = \sqrt{ \sum_{i=0}^{n} \frac{(arr[i] - mean)^2}{n-1} }
 *  \f]
 */
template<class T>
double ComputeStDev(const TrIterator<T>& begin, const TrIterator<T>& end, double mean)
{

    double stdev2 =0;
    double tmpdiff = 0;

    double n=0;

    TrIterator<T> it=begin;
    while ( it<end ) 
    {
        tmpdiff = (*it-mean);
        stdev2 += ::pow(tmpdiff,2.0); 
        ++n;

        ++it;
    }

    return ::sqrt(stdev2 / (n - 1.0));
}

/// Computes the mean and stddev within a range
/**
 * @param begin points to the first valid value within the range
 * @param end points to the first value outside of the range
 * @return struct containing mean and std dev
 */
template<class T>
BaseLineProcResult ComputeBaseLine(const TrIterator<T>& begin, const TrIterator<T>& end)
{
    double mean = ComputeMean<T>(begin, end);
    double stdev = ComputeStDev<T>(begin, end, mean);

    BaseLineProcResult res;
    res.mean = mean;
    res.stdev = stdev;
    return res;
}

} // end namespace
#endif
