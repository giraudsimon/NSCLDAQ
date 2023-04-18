//  TrIterator.hpp
//
//  Author : Jeromy Tompkins
//  Date   : 8/14/2013

#ifndef TRITERATOR_H
#define TRITERATOR_H

#include <cstddef>
#include "TObject.h"

namespace TrAnal
{

/// TrIterator template class
/**
 * The bedrock component of the Traiter package. It is a basic iterator that provides 
 * the full functionality of an input iterator. However, it also supports random access
 * at a minimum level for convenience. TrIterators DO NOT alter the trace elements pointed
 * to. They can be copied, assigned to, subtracted from one another, and compared with one
 * another.  
 */
template<class T>
class TrIterator // functions as though it inherits from std::iterator< ::std::input_iterator_tag,T>
{
    public:
    typedef T value_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef T& reference;
    typedef std::input_iterator_tag iterator_category;

    private:
    const T* m_iter;
    
    public:
    /// Default constructor
    /**
     * Default to a null iterator when the argument is absent. Dereferencing a 
     * null iterator is undefined and will result in unpredictable behavior. It 
     * is the user's responsibility to avoid this. 
     * @param iter is a pointer to an object of type T
     */
    TrIterator(const T* iter=0) : m_iter(iter) {}
    
    /// Copy constructor
    /**
    *   Creates a new iterator that points at the same object as the argument.
    *   @param iter the iterator to copy
    */   
    TrIterator(const TrIterator& iter) : m_iter(iter.m_iter) {}

    /// Assignment operator
    /**
    *   Copies the internal pointer
    *   @param iter the TrIterator whose state will be copied
    *   @return reference to this
    */
    TrIterator& operator=(const TrIterator& iter) {
        if (this != &iter) {
            m_iter = iter.m_iter;
        }
        return *this;
    }

    // support boolean comparison
    /// Equality operator
    bool operator==(const TrIterator& iter) const { return m_iter==iter.m_iter;} 
    /// Inequality operator
    bool operator!=(const TrIterator& iter) const { return m_iter!=iter.m_iter;} 

    /// Less than operator
    bool operator<(const TrIterator& iter) const { return m_iter<iter.m_iter;}
    /// Less than or equal to operator
    bool operator<=(const TrIterator& iter) const { return m_iter<=iter.m_iter;}
    /// Greater than operator
    bool operator>(const TrIterator& iter) const { return m_iter>iter.m_iter;}
    /// Greater than or equal to operator
    bool operator>=(const TrIterator& iter) const { return m_iter>=iter.m_iter;}

    // Incrementation support
    /// ++iter type incrementation 
    TrIterator& operator++() { ++m_iter; return *this;}
    /// iter++ type incremenation 
    TrIterator operator++(int) { TrIterator tmp(*this); operator++(); return tmp;}

    // Support for decrementing
    /// --iter type decrementation 
    TrIterator& operator--() { --m_iter; return *this;}
    /// iter-- type decremenation 
    TrIterator operator--(int) { TrIterator tmp(*this); operator--(); return tmp;}

    // Support basic random access 
    /// iter+n
    TrIterator operator+(int n) const { TrIterator tmp(m_iter+n); return tmp;}
    /// iter += n
    TrIterator& operator+=(int n) { m_iter=m_iter+n; return *this;}
    /// iter-n
    TrIterator operator-(int n) const { TrIterator tmp(m_iter-n); return tmp;}
    /// iter -= n
    TrIterator& operator-=(int n) { m_iter=m_iter-n; return *this;}

    // Subtraction
    /// Distance between two iterators
    /**
    *   @return number of elements between the two iterators
    */
    difference_type operator-(const TrIterator& that) const
    {
        return (m_iter-that.m_iter);
    }    
    
    // support value access
    /// Dereference operator
    /**
    *   @return value currently pointed to by the iterator
    */
    value_type operator*() const { return *m_iter;} 

    // ROOT dictionary generation
    /// \cond
    ClassDef(TrIterator,0);
    /// \endcond

}; // end TrIterator class 

/// n + iter support
/**
*  This merely commutes the arguments so that iter+n is computed
*  instead of n+iter.
*/
template<class T>
TrIterator<T> operator+(int n, const TrIterator<T>& iter)
{
    return iter.operator+(n);
}

/// n - iter support
/**
*  This merely commutes the arguments so that iter-n is computed
*  instead of n-iter.
*/
template<class T>
TrIterator<T> operator-(int n, const TrIterator<T>& iter)
{
    return iter.operator-(n);
}

//-------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////
//
// TrRange class
//___________________________________________________________________


/// TrRange class
/**
 *  A convenience class for defining iterator ranges. Most algorithms
 *  operate within a bounded region, and this provides a means for defining
 *  ranges for use in those algorithms. A strength of this class is the
 *  ability to define a range based on a single iterator and integral range 
 *  specification. Ranges are typically used such that begin() is the first 
 *  element of a range of valid elements and end() is the first element beyond
 *  valid elements, i.e., [begin, end) in mathematical notation.
 *
 *  NOTE: This class cannot determine whether or not the two iterators provided
 *        to it are associated with each other. For example, it is possible 
 *        to construct a range in the following way without any errors.
 *
 *        TrRange<type> range( trace0.begin(), trace1.end() );
 */
template<class T>
class TrRange 
{
    private:
    TrIterator<T> m_begin; ///< first element of range
    TrIterator<T> m_end;   ///< first element out of range

    public:
    /// Default constructor 
    /**
     * Constructs a range from two iterators. There is no ability to determine 
     * whether the two arguments are associated with the same trace. If they are
     * not, then problems are likely to ensue. 
     * @param b iterator defining beginning of range
     * @param e iterator defining ending of range
     */
    TrRange(const TrIterator<T>& b=TrIterator<T>(), 
            const TrIterator<T>& e=TrIterator<T>()) 
        : m_begin(b), m_end(e) {}

    /// Range specified constructor
    /** 
    * Construct a range based on a single iterator and an offset from it.
    * If the range is negative, the range will be generated such that ref is 
    * at the end by swapping begin and end. In this way, this constructor will always
    * guarantee that the the beginning and end of the range are in increasing order.
    *
    *   @param ref iterator identifying begining (end) of range if range arg is positive (negative)
    *   @param range width of range, can be either positive or negative
    */
    TrRange(const TrIterator<T>& ref, int range) : m_begin(ref), m_end(ref+range) 
    {
        if (m_begin>m_end) invert();
    } 

    /// Copy constructor
    TrRange(const TrRange& that) : m_begin(that.m_begin), m_end(that.m_end) {}

    /// Assignment operator
    TrRange& operator=(const TrRange& that) 
    {
        if (this != &that) {
            m_begin = that.m_begin; 
            m_end = that.m_end;
        }
        return *this;
    }

    // Member access operators
    /// Get value of begin iterator
    TrIterator<T> begin() const { return m_begin;}
    /// Get value of end iterator
    TrIterator<T> end() const { return m_end;}

    /// ++range style incrementation
    /**
    * Increments both the begin and end iterators together.
    * @return reference to this after incrementing
    */
    TrRange& operator++() 
    {
        m_begin.operator++();
        m_end.operator++(); 

        return *this;
    }    

    /// range++ style incrementation
    /**
    * Returns a copy of the current range while 
    * also incrementing both the begin and end iterators.
    * @return copy of range before incrementing
    */
    TrRange operator++(int) 
    {
        TrIterator<T> tmpb = m_begin++;
        TrIterator<T> tmpe = m_end++; 
        TrRange tmp (tmpb,tmpe);

        return tmp;
    }    
    
    /// --range style decrementation
    /**
    * Decrements both the begin and end iterators together.
    *   @return reference to this range after decrementing
    */
    TrRange& operator--() 
    {
        m_begin.operator--();
        m_end.operator--(); 

        return *this;
    }    

    /// Decrementation range--;
    /**
    * Returns a copy of the current range while 
    * also decrementing both the begin and end iterators.
    *   @return copy of range before decrementing
    */
    TrRange operator--(int) 
    {
        TrIterator<T> tmpb = m_begin--;
        TrIterator<T> tmpe = m_end--; 
        TrRange tmp (tmpb,tmpe);

        return tmp;
    }    

    // Basic random access
    /// range+n style increment
    /**
    * Increments both the begin and end iterators together.
    * @return copy of range after incrementing
    */
    TrRange operator+(int n) const
    {
        TrRange tmp(m_begin.operator+(n),
                m_end.operator+(n));

        return tmp;
    }    

    /// range-n style decrement
    /**
    * Decrements both the begin and end iterators together.
    * @return copy of range after decrementing
    */
    TrRange operator-(int n) const
    {
        TrRange tmp(m_begin.operator-(n),
                m_end.operator-(n)); 

        return tmp;
    }    

    /// range += n support
    /**
    * Increments both the begin and end iterators together.
    * @return reference to this range after incrementing
    */
    TrRange& operator+=(int n) 
    {
        m_begin.operator+=(n);
        m_end.operator+=(n);

        return *this;
    }    

    /// range -=n style decrement
    /**
    * Decrements both the begin and end iterators together.
    * @return reference to this range after decrementing
    */
    TrRange& operator-=(int n) 
    {
        m_begin.operator-=(n);
        m_end.operator-=(n);

        return *this;
    }    

    /// Compute distance between begin and end
    /**
     *  @return number of elements between begin and end
     */
    typename TrIterator<T>::difference_type range() const { return (m_end - m_begin);}

    /// Swap begin and end iterators
    void invert() 
    { 
        TrIterator<T> tmp=m_begin; 
        m_begin=m_end; 
        m_end=tmp; 

    }

    // Equality operators
    /// Equality operator
    /**
    * Equality implies that both begin and end iterators are the same
    */
    bool operator==(const TrRange& that) const { return ((m_begin==that.m_begin)&&(m_end==that.m_end));}
    /// Inequality operator
    bool operator!=(const TrRange& that) const { return (! operator==(that));}

    // Comparison operators
    /// Less than operator
    /**
     * Ensures that both begin and end iterators are less than their counterparts. 
     * This definition allows ranges to overlap.
     *
     * @param that range to compare against
     * @return true if this->begin()<that.begin() && this->end()<that.end()
     */
    bool operator<(const TrRange& that) const { return ((m_begin<that.m_begin) && (m_end<that.m_end));}
    /// Greater than operator
    /**
     * Ensures that both begin and end iterators are greater than their counterparts 
     * This definition allows ranges to overlap.
     * @param that range to compare against
     * @return true if this->begin()>that.begin() && this->end()>that.end()
     */
    bool operator>(const TrRange& that) const { return ((m_begin>that.m_begin) && (m_end>that.m_end));}

    /// Greater than or equal operator
    bool operator<=(const TrRange& that) const { return (operator<(that) || operator==(that)); } 
    /// Less than or equal operator
    bool operator>=(const TrRange& that) const { return (operator>(that) || operator==(that)); } 
    
    /// \cond
    ClassDef(TrRange,0);
    /// \endcond

}; // end of TrRange class

/**
* \fn template<class T> TrRange<T> operator+(int n, const TrRange<T>& range)
* \brief n + range incrementation
*
*   @param n offset value
*   @param range the range to increment from 
*   @return increment range
*
*   Effectively reorders the operation to range+n.
*   Result of this is (begin,end) -> (begin+n,end+n)
*/
template<class T>
TrRange<T> operator+(int n, const TrRange<T>& range)
{
    TrRange<T> tmp = range.operator+(n);
    return tmp;
}


/**
*   \fn template<class T> TrRange<T> operator-(int n, const TrRange<T>& range)
*   \brief n - range incrementation
*
*   @param n offset value
*   @param range the range to increment from
*   @return decremented range
*
*   Effectively reorders the operation to range-n.
*   Result of this is (begin,end) -> (begin-n,end-n)
*
*/
template<class T>
TrRange<T> operator-(int n, const TrRange<T>& range)
{
    TrRange<T> tmp = range.operator-(n);
    return tmp;
}

} // end namespace


#endif
