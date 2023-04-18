// AmplitudeProcessor.h
//
// Author : Jeromy Tompkins
// Date   : 8/14/2013



#ifndef AMPLITUDEPROCESSOR_H
#define AMPLITUDEPROCESSOR_H
    
#include "BaseLineProcessor.hpp"
#include "PeakFindProcessor.hpp"
#include "Exceptions.h"

#include <TObject.h>
namespace TrAnal 
{

/// Result for ComputeAmplitude algorithm
class AmplitudeProcResult 
{
    public:
    double amp; ///< amplitude (i.e. peak - baseline)
    double uamp; ///< uncertainty in amplitude

    double baseline; ///< mean value of trace within baseline search range
    double ubaseline; ///< std. dev. of trace within baseline search range 

    // ROOT dictionary generation
    /// \cond
    ClassDef(AmplitudeProcResult,1);
    /// \endcond
};

/// \brief Determine difference between peak value and baseline value 
/**
 * @param bline_range range of valid contiguous values to compute the baseline from
 * @param pf_range range of valid contiguous values to find the peak value
 * @return a struct containing the amp and baseline values
 * 
 * This calls the ComputeBaseLine function and then subsequently the FindPeak algorithm.
 * The results of those two computations are used to compute the amplitude, which is the
 * difference between the peak and the baseline. 
 *
 * \throw InvalidResultException when peak value is less than baseline value
 */
template<class T>
AmplitudeProcResult ComputeAmplitude( const TrRange<T>& bline_range, const TrRange<T>& pf_range)
{ 
    // Call the primitive processors
    // There results will be deleted automatically when they go out of scope
    BaseLineProcResult bl_res 
        = ComputeBaseLine(bline_range.begin(), bline_range.end());

    PeakFindProcResult<uint16_t> pf_res 
        = FindPeak(pf_range.begin(), pf_range.end());

    if (pf_res.max < bl_res.mean) {
        throw InvalidResultException("amplitude less than baseline");
    }

    // No exceptions were thrown, create the result and fill it with values
    AmplitudeProcResult res;
    res.amp = pf_res.max - bl_res.mean;
    res.uamp = ::sqrt(::pow(bl_res.stdev,2.0) );

    res.baseline = bl_res.mean;
    res.ubaseline = bl_res.stdev;

    return res;
}

} // end namespace
#endif
