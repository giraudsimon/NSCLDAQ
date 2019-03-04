/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
/*!
  \class: TranslatorPtr
  \file:  TranslatorPtr.h
  
  Encapsulates a TranslatorPtr object. A TranslatorPtr holds
  a pointer to a dynamically allocated BufferTranslator. By wrapping 
  the BufferTranslator in the TranslatorPtr, it can be used as
  though it were a normal pointer. Note that the BufferTranslator must
  then be deleted by the caller.

  Author:
    Jason Venema
    NSCL
    Michigan State University
    East Lansing, MI 48824-1321
    mailto:venemaja@msu.edu

Change Log:
   $Log$
   Revision 5.3  2005/07/07 14:19:45  ron-fox
   Add i64 adjustments from 3.0 branch

   Revision 5.1.2.2  2005/07/07 13:26:19  ron-fox
   Fix another casting issue

   Revision 5.1.2.1  2004/12/21 17:51:18  ron-fox
   Port to gcc 3.x compilers.

   Revision 5.1  2004/11/29 16:55:56  ron-fox
   Begin port to 3.x compilers calling this 3.0

   Revision 4.8.6.2  2004/07/08 15:25:52  ron-fox
   Fix some small issues with translator pointer arithmetic I didn't catch at first (operator+ and operator- were wrong).

   Revision 4.8.6.1  2004/07/02 11:31:24  ron-fox
   - Made the offset a byte offset and corrected the code in all members to deal
   with that.

   - Added a new constructor of the form:

       template <class U>
            TranslatorPtr<T>(TranslatorPtr<U>& rhs)

   	 This supports code like:


            TranslatorPtr<UShort_t> p1(.... );
   	        ...
                    TranslatorPtr<ULong_t> pl(p1);

   		    Where now pl will translate longs from the buffer starting at the same offset as
   		    p1.  This simplifies the case where you must pull longword data from the event
   		    buffer.

   Revision 4.8  2003/01/02 17:11:32  venema
   Major version upgrade to 2.0 includes support for arbitrary user coordinate mapping and sticky print options.

   Revision 4.7  2002/08/08 13:16:57  venema
   Fixed counter-intuitive indexing operator to index relative to the current offset, instead of the beginning of the buffer.
   VS: ----------------------------------------------------------------------

   Revision 4.6  2002/08/08 11:48:27  ron-fox
   Add change log.

*/

#ifndef TRANSLATORPTR_H
#define TRANSLATORPTR_H

#include "ByteBuffer.h"
#include "ByteOrder.h"

#include <stdexcept>
#include <iterator>

namespace DAQ
{
  namespace Buffer
  {

    template <class T>
    class BufferPtr
    {

    public:
      typedef std::ptrdiff_t    difference_type;
      typedef T                 value_type;
      typedef T*                pointer;
      typedef T&                reference;
      typedef std::random_access_iterator_tag iterator_category;

    private:
//      ByteBuffer& m_buffer; /*! Reference to the BufferTrans. */
      ByteBuffer::const_iterator       m_iter;           //!< Byte offset into the buffer.
      DAQ::BO::CByteSwapper      m_swapper;

    public:

      // Basic constructor for this receives a reference to a BufferTranslator
      // and defaults m_nOffset to 0
      BufferPtr(ByteBuffer::const_iterator pos = ByteBuffer::const_iterator(),
                     DAQ::BO::CByteSwapper swapper = DAQ::BO::CByteSwapper(false)) :
        m_iter(pos),
        m_swapper(swapper)
      {}

      // Constructor from any other type of translator pointer:

      template <class U>
      BufferPtr<T>(const BufferPtr<U>& RHS) :
        m_iter(RHS.getBaseIterator()),
        m_swapper(RHS.getSwapper())
      {}

      // Copy constructor for this
      BufferPtr<T>(const BufferPtr<T>& RHS) :
        m_iter(RHS.m_iter),
        m_swapper(RHS.m_swapper)
      { }

      // Assignment operator=
      BufferPtr<T> operator=(const BufferPtr<T>& RHS)
      {
        m_iter = RHS.m_iter;
        m_swapper = RHS.m_swapper;
        return *this;
      }

      // Operator= type conversion operator
      template<class U>
      BufferPtr<T>& operator=(const BufferPtr<U>& rhs)
      {
        m_iter = rhs.getBaseIterator();
        m_swapper = rhs.getSwapper();
        return *this;
      }

      // Member functions for class TranslatorPtr
    public:

      /*!
    \fn T operator*() const

    Purpose:
       Dereference this at the current offset i.e. return the value
       stored in m_buffer at the current m_nOffset.
   */
      inline T operator*() const
      {
        return m_swapper.copyAs<T>(m_iter);
      }

      /*!
    \fn T operator[](Int_t nOffset) const

    Purpose:
       Indexing operator returns the value stored at position
       (m_nOffset+nOffset)*sizeof(T) in m_buffer.
   */
      inline T operator[](std::size_t nOffset) const
      {
        return m_swapper.copyAs<T>(m_iter + nOffset*sizeof(T));
      }

      /*!
    \fn TranslatorPtr<T> operator++(Int_t)

    Purpose:
       Post-increment operator++ increments the offset held by this,
       and returns the pre-incremented value of this.

    \param Int_t - dummy argument indicating post-increment form
  */
      inline BufferPtr<T> operator++(int)
      {
        BufferPtr<T> before(*this);
        operator++();
        return before;
      }

      /*!
    \fn TranslatorPtr<T> operator--(Int_t)

    Purpose:
       Post-decrement operator-- decrements the offset held by this,
       and returns the pre-decremented value of this.

    \param Int_t - dummy argument indicating post-decrement form
   */
      inline BufferPtr<T> operator--(int)
      {
        BufferPtr<T> before(*this);
        operator--();
        return before;
      }

      /*!
    \fn TranslatorPtr<T>& operator++()

    Purpose:
       Pre-increment operator++ increments the offset held by this,
       and returns a reference to this.
  */
      inline BufferPtr<T>& operator++()
      {
        m_iter+=sizeof(T);
        return *this;
      }

      /*!
    \fn TranslatorPtr<T>& operator

    Purpose:
       Pre-decrement operator-- decrements the offset held by this,
       and returns a reference to this.
  */
      inline BufferPtr<T>& operator--()
      {
        m_iter -= sizeof(T);
        return *this;
      }

      /*!
    \fn TranslatorPtr<T> operator+(Int_t Offset)

    Purpose:
       Add an offset to the m_nOffset value of this.

    \param Int_t Offset - an integer to add to m_nOffset
  */
      inline BufferPtr<T> operator+(std::size_t Offset)
      {
        return BufferPtr<T>(m_iter+Offset*sizeof(T), m_swapper);

      }

      /*!
    \fn TranslatorPtr<T> operator-(Int_t Offset)

    Purpose:
       Subtract an offset to the m_nOffset value of this. Throws
       a CRangeError exception if Offset > m_nOffset.

    \param Int_t Offset - an integer value to subtract from m_nOffset
   */
      inline BufferPtr<T> operator-(std::size_t Offset)
      {
        return BufferPtr<T>(m_iter-Offset*sizeof(T), m_swapper);
      }

      inline difference_type operator-(const BufferPtr<T>& rhs)
      {
        return (m_iter - rhs.getBaseIterator())/sizeof(value_type);
      }

      /*!
    \fn CTranslatorPtr<T>& operator+=(Int_t Offset)

    Purpose:
       Add an offset to the m_nOffset value of this.
       Returns a reference to this.

    \param Int_t Offset - an integer value to add to m_nOffset
  */
      inline BufferPtr<T>& operator+=(std::size_t Offset)
      {
        m_iter += Offset*sizeof(T);
        return *this;
      }

      /*!
    \fn CTranslatorPtr<T>& operator-=(Int_t Offset)

    Purpose:
       Subtract an offset from the m_nOffset value of this.
       Returns a reference to this or throws a CRangeError
       exception if Offset > m_nOffset.

    \param Int_t Offset - an integer value to subtract from m_nOffset
  */
      inline BufferPtr<T>& operator-=(std::size_t Offset)
      {
        m_iter -= Offset*sizeof(T);
        return *this;
      }

      /*!
    \fn bool operator<(const CTranslatorPtr<T>& RHS)

    Purpose:
       Less than operator. Compare the m_nOffset value of this with the
       m_nOffset of RHS. Return true if the former is less than the latter.

    \param const CTranslatorPtr<T>& RHS - a reference to a TranslatorPtr
                                             to compare with this
   */
      inline bool operator<(const BufferPtr<T>& RHS)
      {
        return m_iter < RHS.m_iter;
      }

      /*!
    \fn bool operator==(const CTranslatorPtr<T>& RHS)

    Purpose:
       Equality operator. Compare the m_nOffset value of this with the
       m_nOffset of RHS. Return true if the former is equal to the latter.

    \param const CTranslatorPtr<T>& RHS - a reference to a TranslatorPtr
                                             to compare with this
   */
      inline bool operator==(const BufferPtr<T>& RHS)
      {
        return m_iter == RHS.m_iter;
      }

      /*!
    \fn bool operator>(const TranslatorPtr<T>& RHS)

    Purpose:
       Greater than operator. Compare the m_nOffset value of this with the
       m_nOffset of RHS. Return true if the former is greater than the latter.

    \param const TranslatorPtr<T>& RHS - a reference to a TranslatorPtr
                                             to compare with this
   */
      inline bool operator>(const BufferPtr<T>& RHS)
      {
        return (m_iter > RHS.m_iter);
      }

      inline bool operator>=(const BufferPtr<T>& RHS)
      {
        return ((*this>RHS) || (*this==RHS));
      }

      inline bool operator<=(const BufferPtr<T>& RHS)
      {
        return ((*this<RHS) || (*this==RHS));
      }

      /*!
    bool operator!=(const CTranslatorPtr<T>& RHS

    Purpose:
       Inequality operator. Compare the m_nOffset value of this with the
       m_nOffset of RHS. Return true if the former is not equal to the latter.

    \param const CTranslatorPtr<T>& RHS - a reference to a TranslatorPtr
                                             to compare with this
   */
      inline bool operator!=(const BufferPtr<T>& RHS)
      {
        return !(*this == RHS);
      }

      // Accessor functions:
    public:
      ByteBuffer::const_iterator getBaseIterator() const
      {
        return m_iter;
      }

      const DAQ::BO::CByteSwapper& getSwapper() const
      {
        return m_swapper;
      }
      DAQ::BO::CByteSwapper& getSwapper()
      {
        return m_swapper;
      }


    };

  } // end of Buffer
} // end of DAQ

#endif
