//  RiseTimeProcessor.hpp
//
//  Author : Jeromy Tompkins
//  Date   : 8/14/2013

#ifndef RISETIMEPROCESSOR_H
#define RISETIMEPROCESSOR_H
#include <iostream>
#include <TObject.h>
#include "Trace.hpp"
#include "AmplitudeProcessor.hpp"
#include "Threshold.hpp"

namespace TrAnal 
{

/// Result of ComputeRiseTime
template<class T>
class RiseTimeProcResult
{

    public:
    double baseline;  ///< computed value of baseline
    double amp;     ///< computed amplitude

    TrIterator<T> t10_index; ///< points to first value equal to or exceeding 10% of the amplitude
    TrIterator<T> t90_index; ///< points to first value equal to or exceeding 90% of the amplitude

    double t10_amp; ///< value pointed to by t10_index
    double t90_amp; ///< value pointed to by t90_index

    // ROOT dictionary generation
    /// \cond
    ClassDef(RiseTimeProcResult,0);
    /// \endcond
};
 

/// Computes Amplitude then finds locations of T10 and T90
/**
 * @param bline_range is the range over which to compute the base line
 * @param pfind_range is the range over which to search for local maxima 
 * @return a struct containing the computed values
 */
template<class T>
    RiseTimeProcResult<T> ComputeRiseTime(const TrRange<T>& bline_range, const TrRange<T>& pfind_range)
    {
    // Process the amplitude to get the baseline and the peak height

    // Use smart pointer to make this exception safe... we don't want 
    // to have one of the results for the primitive processors to persist
    // if an exception is thrown.

    AmplitudeProcResult amp_res = ComputeAmplitude(bline_range,pfind_range);

    // find the t10 point
    double amp10 = amp_res.baseline + 0.10*amp_res.amp;
    TrIterator<T> thr10_res = Threshold(pfind_range,amp10);

    // find the t90 point
    double amp90 = amp_res.baseline + 0.90*amp_res.amp;
    TrIterator<T> thr90_res = Threshold(TrRange<T>(thr10_res,pfind_range.end()),amp90);

    // if order of the t10 and t90 indices is not 
    // first t10 then t90, then something bizarre happened 
    // This is not a valid result.
    if (thr10_res >= thr90_res)  {
        throw InvalidResultException("t10 comes after t90...nonsensical result");
    }

    if (*thr10_res >= *thr90_res) {
        throw InvalidResultException("t10_amp > t90_amp ... nonsensical result");
    }

    // if we made it here, then there were no exceptions...
    // create the result and fill it with data.
    // Note that this will not be deleted when it goes out of scope... the 
    // ownership of the result will be passed to the caller
    RiseTimeProcResult<T> res;

    res.baseline = amp_res.baseline;
    res.amp = amp_res.amp;

    res.t10_index = thr10_res;
    res.t90_index = thr90_res;

    res.t10_amp = *(res.t10_index);
    res.t90_amp = *(res.t90_index);

    return res;

}

/// Computes amplitude then finds locations of T10 and T90
/**
 * @param begin points to the first value of the base line range
 * @param bline_range specifies the size of the range to compute the base line 
 * @param pfind_range specifies the size of the range to compute the base line 
 * @return a struct containing the computed values 
 *
 * A convenience function to construct the baseline and peak find ranges
 * from offsets. The arguments are used to construct the following ranges.
 * 
 *      baseline_range  [begin              , begin+bline_range             )
 *      peakfind_range  [begin+bline_range  , begin+bline_range+pfind_range )
 * 
 */
template<class T> 
RiseTimeProcResult<T> ComputeRiseTime(const TrIterator<T>& begin, int bline_range, int pfind_range)
{
    return ComputeRiseTime(TrRange<T>(begin,bline_range), TrRange<T>(begin+bline_range, pfind_range));
}

} // end TrAnal namespace

#endif
