//  TrapFilter.hpp
//
//  Author : Jeromy Tompkins
//  Date   : 8/14/2013

#ifndef TRAPFILTER_H
#define TRAPFILTER_H

#include <iostream>
#include "AlgoIterator.hpp"
#include "SumIterator.hpp"
#include <TObject.h>

namespace TrAnal 
{

/// A simple fast filter (lead sum - trail sum)
/**
 * A simple trapezoidal filter is implemented that evaluates the 
 * difference between a leading sum and a trailing sum. It is 
 * naturally composed of two SumIterator objects. The filter implements
 * the AlgoIterator interface and functionally is like an input iterator. 
 * When this is dereferenced, the difference between the current values of 
 * the two summing regions is returned. When it is incremented, the two 
 * summing regions are also incremented.
 * 
 */
template<class T>
class TrapFilter : public AlgoIterator<T>
{
    private:
    SumIterator<T> m_trailsum; ///< trailing sum
    SumIterator<T> m_leadsum; ///< leading sum

    private:
    /// Private default constructor
    /** 
     * A default constructor does not make much sense so its use is 
     * prohibited.
     */
    TrapFilter();

    public:
    // Defines the trailing type of sums
    /// \brief Constructs a symmetric trap filter
    /**
     * Given a starting position, the symmetric filter is built from
     * the rise_range and gap_range provided. These make use of the 
     * offset definitions of a TrRange. The trailing sum is defined 
     * as a region [begin, begin+rise_range) and the leading sum is 
     * defined as [begin+rise_range+gap_range, begin+ 2*rise_range+gap_range).
     *
     * @param begin the iterator defining the starting position of the trailing sum
     * @param rise_range the range of the summing region and the length of the filter rise
     * @param gap_range the distance between the max_extent of the trailing sum and min_extent of leading sum
     */
    TrapFilter(const TrIterator<T>& begin, int rise_range, int gap_range)
        : AlgoIterator<T>(), 
        m_trailsum(TrRange<T>(begin,rise_range)), 
        m_leadsum(TrRange<T>(begin+rise_range+gap_range,rise_range))
    {}
    
    /// \brief Constructs an arbitrary trap filter
    /**
     * Construct an arbitrary fast trapezoidal filter by providing the summing ranges
     * 
     * @param trail_range the initial trailing sum range
     * @param lead_range the initial leading sum range
     */
    TrapFilter(const TrRange<T>& trail_range, const TrRange<T>& lead_range)
        : AlgoIterator<T>(), 
        m_trailsum(trail_range), 
        m_leadsum(lead_range)
    {}

    /// \brief Assignment operator
    /**
     *  Copy the two summing regions.
     * @param that the object whose state will be copied
     * @return a reference to this after assignment
     */ 
    TrapFilter& operator=(const TrapFilter& that) 
    {
        if (this != &that) {
            m_trailsum = that.m_trailsum;
            m_leadsum = that.m_leadsum;
        }

        return *this;
    }

    /// Virtual destructor
    /**
     * Does nothing b/c no memory has been allocated dynamically
     */
    virtual ~TrapFilter() {}

    ///  ++filter style increment
    /** 
     * Increments the two summing regions together as ++sum
     *
     * @return a reference to this after incrementing
     */
    virtual TrapFilter& operator++() 
    {
        ++m_trailsum;
        ++m_leadsum;

        return *this;
    }

    ///  filter++ style increment
    /** 
     * Increments the two summing regions together as sum++
     * 
     * @return a copy of filter before incrementing
     */
    TrapFilter operator++(int) 
    {
        SumIterator<T> it = m_trailsum++;
        SumIterator<T> it2= m_leadsum++;
        TrapFilter tmp (TrRange<T>(it.min_extent(),it.max_extent()),
                        TrRange<T>(it2.min_extent(),it2.max_extent()));
        return tmp;
    }
    
    /// \brief Less than comparison to TrIterator
    /**
     *
     * Comparison against a simple iterator (useful for checking whether next dereference
     * will be a valid and safe). Literally compares whether the iterator returned by 
     * max_extent() is less than the argument. In normal situations, this is analogous 
     * to checking whether the max_extent() of the leading sum comes before the argument.
     *
     * @param it the iterator to compare to 
     * @return true if it<max_extent(), false otherwise
     *
     */
    virtual bool operator<(const TrIterator<T>& it) const
    {
        TrIterator<T> max = max_extent();

        return (max<it);
    }

    /// \brief Greater than comparison to TrIterator
    /**
     * 
     * Comparison against a simple iterator. Literally compares whether the iterator returned by 
     * min_extent() is greater than the argument. In normal situations, this is analogous 
     * to checking whether the min_extent() of the trailing sum comes after the argument.
     * 
     * @param it the iterator to compare to 
     * @return true if min_extent() > it, false otherwise
     * 
     */
    bool operator>(const TrIterator<T>& it) const
    {
        TrIterator<T> min = min_extent();

        return (min>it);
    }

    /// \brief Gets the most advanced iterator position of the summing regions
    /**
     * Returns the most advanced iterator between the two summing regions. This most
     * likely points to the first value outside of the leading sum's range.
     * However, no assumption is made about which summing region comes before the other
     * because no logic is in place to ensure proper ordering. 
     *
     * @return iterator pointing to the most advanced iterator of the two summing regions
     */
    virtual TrIterator<T> max_extent() const 
    {
        TrIterator<T> it;
        if (m_trailsum<m_leadsum) {
            it = m_leadsum.max_extent();
        } else {
            it = m_trailsum.max_extent();
        }

        return it;
    }

    /// \brief Gets the minimum iterator position of the summing regions
    /**
     * Returns the minimum iterator position between the two summing regions. This most
     * likely points to the first valid value of the trailing sum's range.
     * However, no assumption is made about which summing region comes before the other
     * because no logic is in place to ensure proper ordering. 
     *
     * @return iterator pointing to the minimum iterator position of the two summing regions
     */
    virtual TrIterator<T> min_extent() const 
    {
        TrIterator<T> it;
        if (m_trailsum>m_leadsum) {
            it = m_leadsum.min_extent();
        } else {
            it = m_trailsum.min_extent();
        }

        return it;
    }

    /// \brief Dereference operator <==> evaluates to current filter value 
    /**
     * @return difference between leading and trailing sums.
     */
    virtual double operator*() const { return *m_leadsum - *m_trailsum;}    


    public:
    // ROOT dictionary generation
    /// \cond
    ClassDef(TrapFilter,0);
    /// \endcond

}; // end class

} // end namespace
#endif 
