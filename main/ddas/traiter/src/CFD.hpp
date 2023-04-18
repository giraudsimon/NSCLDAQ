//  CFD.hpp
//  
//  Author : Jeromy Tompkins
//  Date   : 8/14/2013 

#ifndef CFD_H
#define CFD_H

#include <deque>
#include <cmath>
#include "TrapFilter.hpp"
#include "Solver.h"
#include "TObject.h"

namespace TrAnal
{

/// Result for a CFD computation
/**
*   Together the two items in this struct locate the zero crossing.
*   
*   Because the CFD algorithm does not necessarily start its search at 
*   the beginning of the search, it can only identify a location. 
*   Typically, the trig and the fraction will be used in a manner similar
*   to as follows.
*
*       ... code ...
*       
*       // Execute CFD algorithm over first 100 elements of trace
*       CFDResult<uint16_t> res = CFD(  TrRange<uint16_t>(trace.begin(),100), //range to search
*                                       10, // fast filter rise
*                                       2,  // fast filter gap 
*                                       3,  // cfd delay
*                                       0 );  // cfd scale factor
*
*       // Compute distance from start of trace to res.trig and then add fraction
*       double cfd_zero_crossing = (res.trig - trace.begin()) + fraction;
*       
*       ... code ...
*                                                               
*/
template<class T>
struct CFDResult
{
    TrIterator<T> trig; ///< TrIterator pointing to element immediately before zero crossing
    double fraction; ///< fractional distance to the zero crossing beyond trig iterator

    // ROOT dictionary generation 
    /// \cond
    ClassDef(CFDResult,0);
    /// \endcond
};


/// Find zero crossing location with Pixie16 CFD algorithm 
/**
 * \todo This should be broken up so that CFD becomes an AlgoIterator and then this becomes an algorithm to find a zero crossing
 *
 * @param range specifies a contiguous range of valid trace samples
 * @param rise_len is the number of points defining the rise portion of the fast filters
 * @param gap_len is the number of points defining the gap portion of the fast filters
 * @param delay is the number of points defining the delay between the leading and trailing filters 
 * @param scale_factor specifies the scaling of the trailing sum (see above details)  
 * @param solver an implementation of the Solver to compute the zero crossing
 * @return location of zero cross on success or null CFDResult on failure
 *
 *  Given a range, fast filter parameters, cfd delay, cfd scale factors, and 
 *  a Solver to compute a zero crossing, perform a CFD algorithm. The methodology
 *  is based on the Pixie16 manual. It is defined as the difference between two 
 *  fast filters, where one is delayed and scaled. The fast filter, \f$FF\f$, is defined 
 *  as 
 *  
 * \f[
 *      FF[i] = \sum_{k=i-\text{rise_len}}^{i} - \sum_{k=i-2*\text{rise_len}-\text{gap_len}}^{i-\text{rise_len}-\text{gap_len}}.
 * \f] 
 * The CFD algorithm is then,
 *
 *  \f[
 *      CFD[i+\text{delay}] = FF[i+\text{delay}] - \frac{FF[i]}{2^{\text{scale_factor}+1}}
 *  \f]
 *
 * The range specifies a region of valid trace elements. Because the CFD algorithm
 * requires 2*rise_len+gap_len + delay valid elements to compute on value, the CFD
 * algorithm will compute less points than exists within the range. To be exact, the 
 * number of CFD points computed is given by 
 *
 *      int cfd_algo_width = range.range() - (2*rise_len+gap_len+delay);
 * 
 * The algorithm maintains a deque containing the results of the minimum necessary 
 * number of values for the solver to computer the zero crossing point. For the default linear solver, only two points
 * will ever be maintained by the algorithm. Once a zero crossing is found, the deque
 * is passed to the Solver, which then compute the location of the zero crossing with
 * respect to the first element. 
 * 
 *
 */
template<class T> 
CFDResult<T> CFD(const TrRange<T>& range,  
        int rise_len, 
        int gap_len,   
        int delay,     
        int scale_factor,
        const Solver& solver=LinearSolver()  )

{ 
    // TrapFilters are defined from their lower bound. First locate the 
    // the location of the lead filter
    TrIterator<T> lead_start = range.begin()+delay;
    // Verify that it is in range
    if (lead_start>=range.end() ) return CFDResult<T>();

    // Construct the two trap filters
    TrapFilter<T> lead_filter (lead_start, rise_len, gap_len);
    TrapFilter<T> dely_filter (range.begin(), rise_len, gap_len);

    //// THE ALGORITHM /////

    // initialize the queue
    std::deque<double> cfd_res;
    for (unsigned int i=0; i<solver.npoints() && lead_filter<range.end(); ++i) {
        
        // Compute the CFD value
        cfd_res.push_back((*lead_filter) - (*dely_filter)/pow(2.0,scale_factor+1));

        ++lead_filter;
        ++dely_filter;
    }

    // Initialize the counter for number of negative points
    unsigned int nneg=0;
    while (lead_filter<range.end()) {
        // Compute new CFD values
        cfd_res.push_back((*lead_filter) - (*dely_filter)/pow(2.0,scale_factor+1));
    
        // Have we crossed over zero? 
        if (cfd_res.back() < 0) ++nneg;
    
        // Do we have enough points to determine the zero crossing?
        // if so, stop!
        if (nneg>=solver.npoints()-1) break;

        ++lead_filter;
        ++dely_filter;

        cfd_res.pop_front();
    }

    // If not enough data exists to find a zero crossing, return a null result
    if (nneg!=solver.npoints()-1) return CFDResult<T>();

    // This will return the fraction index of the zero crossing with respect to the
    // front of the queue
    double trig = solver(cfd_res);
    int floortrig = static_cast<int>(::floor(trig));

    // trig location happened at ceiltrig from end
    int n_from_end=static_cast<int>(cfd_res.size())-floortrig;
    TrIterator<T> trigit = lead_filter.max_extent()-n_from_end;
    double frac = trig - floortrig; 

    // Construct the result and move return it
    CFDResult<T> res;
    res.trig = trigit;
    res.fraction = frac;

    return res;

} // end CFD function

} // end namespace 

#endif
