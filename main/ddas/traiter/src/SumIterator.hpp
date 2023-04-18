//  SumIterator.hpp
//  
//  Author : Jeromy Tompkins
//  Date   : 8/14/2013

#ifndef SUMITERATOR_H
#define SUMITERATOR_H

#include "AlgoIterator.hpp"
#include "TrIterator.hpp"
#include "TObject.h"

namespace TrAnal
{

    /// Summing iterator
    /**
    *   Implements the AlgoIterator interface but extends the 
    *   functionality to provide richer comparisons.
    *   
    *   The summing region is semi-inclusive, i.e. [low,high) .
    *   As a result, out-of-bounds can be reliable checked before
    *   attempting to access an invalid memory location.
    *
    */
    template<class T>
    class SumIterator : public AlgoIterator<T> {
        public:
        typedef T value_type;
        typedef ptrdiff_t difference_type;
        typedef T* pointer;
        typedef T& reference;
        typedef std::input_iterator_tag iterator_category;

        private:
        TrRange<T> m_range;
        double m_sum;

        public:
        // Canonicals
        SumIterator(const TrRange<T>& range)
            : AlgoIterator<T>(), m_range(range), m_sum(T())
        { m_sum = accumulate(range); }

        SumIterator(const SumIterator& that)
            : AlgoIterator<T>(), m_range(that.m_range), m_sum(that.m_sum)
        {}

        virtual ~SumIterator() {}

        SumIterator& operator=(const SumIterator& that) 
        {
            if (this!=&that) {
                m_range = that.m_range;
                m_sum = that.m_sum;
            }
            return *this;
        }

        // Equality operators
        // Note that this will compare both the range
        bool operator==(const SumIterator& that) const
        {
            return (m_range==that.m_range); 
        }
        bool operator!=(const SumIterator& that) const
        {
            return (! operator==(that)); 
        }

        /// Comparison operator
        /**
         *  Compares whether max_extent is less than the 
         * the argument. 
         *
         *  @param it the iterator to compare to 
         */
        virtual bool operator<(const TrIterator<T>& it) const 
        {
            return ( max_extent() < it );
        }
        
        // Comparison operators...note that these do not 
        // check that the iterators even correspond to the same 
        // composite object
        bool operator<(const SumIterator& that) const 
        {
            return (m_range<that.m_range);
        }
        bool operator>(const SumIterator& that) const
        {
            return !(m_range<that.m_range);
        }
        bool operator<=(const SumIterator& that) const
        {
            return (!operator>(that));
        }
        bool operator>=(const SumIterator& that) const
        {
            return !(m_range<that.m_range);
        }

        // Incrementation ++iter;
        virtual SumIterator& operator++() 
        {        
            IncrementalUpdate();
        
            return *this;
        }

        // Incrementation ++iter;
        SumIterator operator++(int) 
        {        
            SumIterator tmp(*this);
            operator++();
            return tmp;
        }
    
        /// Dereference operator
        /**    
        *   @return sum of values pointed to by the range [min_extent,max_extent) 
        */
        virtual double operator*() const { return m_sum;}

        virtual TrIterator<T> min_extent() const 
        { return m_range.begin() < m_range.end() ? m_range.begin() : m_range.end(); }

        virtual TrIterator<T> max_extent() const 
        { return m_range.end() > m_range.begin() ? m_range.end() : m_range.begin();  }

        private:
        void IncrementalUpdate() 
        {
            // Because ranges are semi inclusive { i.e. [beg,end) }, we
            // already have all of the information needed to update the
            // sum. The end is currently "out-of-range" but will become 
            // the newest in-range value once the range in incremented.
            // The value pointed to by begin will not be in the range any
            // more once incremented.
            T old_begin_val = *(m_range.begin());
            T old_end_val = *(m_range.end());

            m_sum -= old_begin_val;
            m_sum += old_end_val;

            ++m_range;
        } 

        // Sum all values within the range 
        T accumulate(const TrRange<T>& range) 
        {
            T sum=T();
            TrIterator<T> it = range.begin(); 
            while (it<range.end()) {
                sum += *it;
                ++it;
            }
            return sum;
        }

        public:
        // ROOT dictionary generation
        /// \cond
        ClassDef(SumIterator,0);
        /// \endcond
    };


} // end TrAnal namespace
#endif
