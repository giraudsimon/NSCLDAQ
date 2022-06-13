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

/*
  \class CVMEptr
  \file CVMEptr.h

  Encapsulates a CVMEptr object. CVMEptrs are the lowest level of the
  vme module memory mapping hierarchy. They perform the actual mapping
  via the CVMEInterface class. They directly manipulate the registers
  and scale the pointer offsets.

  Author:
     Jason Venema
     NSCL
     Michigan State University
     East Lansing, MI 48824-1321
     mailto: venemaja@msu.edu
*/

#ifndef CVMEPTR_H
#define CVMEPTR_H

#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <daqdatatypes.h>
#include <RangeError.h>
#include <CVMEInterface.h>
#include <string>
#include <MmapError.h>
#include <iostream>
#include <stdint.h>
#include <stdexcept>

template<class T>
class CVMEptr
{
  uint32_t             m_nLength;  // the length of the mmap
  volatile Address_t m_pStart;   // the starting address of the mmap
  void*              m_pHandle;	 // Handle to VME interface device.
  uint32_t              m_nOffset;
  uint32_t             m_nSpace;   // The address space of the mapping
  uint32_t             m_nBase;    // The base address in the crate

 public:
  typedef enum _Space {
    a16 = 0,
    a24 = 1,
    a32 = 3,
    geo = 4
  } Space;

 public:
  // Default constructor and Copy Constructor 
  CVMEptr<T>(uint32_t space, uint32_t base, uint32_t length, uint32_t crate=0);
  CVMEptr<T>(Space space, uint32_t base, uint32_t length, uint32_t crate=0);
  CVMEptr<T>(const CVMEptr& aCVMEptr);
  CVMEptr<T>();

  // operator= Assignment operator
  CVMEptr<T>& operator= (const CVMEptr& aCVMEptr);

  // Operator== equality operator
  int operator== (const CVMEptr<T>& aCVMEptr)
    {
      return ((m_nOffset == aCVMEptr.m_nOffset) &&
	      (m_nLength == aCVMEptr.m_nLength) &&
	      (m_pStart  == aCVMEptr.m_pStart));
    }

  // Destructor
  ~CVMEptr<T>();

 protected:

  // Protected function member:
  // This does the actual mmapping of the VME module
  void CreateMap(uint32_t space, uint32_t base, uint32_t length,
		 uint32_t crate = 0);

  // Public accessor functions to data members
 public:

  uint32_t getOffset() const {
    return m_nOffset;
  }
  uint32_t getLength() const {
    return m_nLength;
  }
  Address_t getStart() const {
    return m_pStart;
  }
  void* getHandle() const {
    return m_pHandle;
  }
  uint32_t getSpace() const {
    return m_nSpace;
  }
  uint32_t getBase() const {
    return m_nBase;
  }
  Address_t getgenptr(uint32_t nOffset);
  Address_t getcurrptr();

  // Protected mutator functions to data members
 public:
  void setOffset(uint32_t am_nOffset) {
    m_nOffset = am_nOffset;
  }
  void setLength(uint32_t am_nLength) {
    m_nLength = am_nLength;
  }
  void setStart(Address_t pStart) {
    m_pStart = pStart;
  }
  void setHandle(void* am_pHandle) {
    m_pHandle = am_pHandle;
  }
  void setSpace(uint32_t am_nSpace) {
    m_nSpace = am_nSpace;
  }
  void setBase(uint32_t am_nBase) {
    m_nBase = am_nBase;
  }

  // Public member functions
 public:
  // Pointer dereference
  T& operator*();
  T* operator->();

  // Indexing
  T& operator[] (uint32_t nOffset);
  
  // Pointer addition/subtraction
  CVMEptr<T> operator+(uint32_t nOffset);
  CVMEptr<T> operator-(uint32_t nOffset);
  CVMEptr<T>& operator+=(uint32_t nOffset);
  CVMEptr<T>& operator-=(uint32_t nOffset);

  // Pointer pre-increment/decrement
  CVMEptr<T>& operator++();
  CVMEptr<T>& operator--();

  // Pointer post-increment/decrement
  CVMEptr<T> operator++(Int_t);
  CVMEptr<T> operator--(Int_t);
};

#endif

/*
  \fn CVMEptr(VmeSpace space, uint32_t base, uint32_t length)

  Operation Type:
     Construction

  \param  VmeSpace space - enumeration indicating which vme address space 
                           to use
          uint32_t base - the base (offset) into the mapping
	  uint32_t length - the length of the mapping
*/

template<class T>
CVMEptr<T>::CVMEptr(uint32_t space, uint32_t base, uint32_t length,
		       uint32_t crate) :
  m_nOffset(0),
  m_nLength(length),
  m_nSpace(space),
  m_nBase(base)
{
  CreateMap(space, base, length, crate);
}
/*
  Same as above, but use Space instead of int:
*/
template<class T>
CVMEptr<T>::CVMEptr(Space space, uint32_t base, uint32_t length,
		       uint32_t crate) :
  m_nOffset(0),
  m_nLength(length),
  m_nSpace(space),
  m_nBase(base)
{
  CreateMap((int)space, base, length, crate);
}


/*
  \fn CVMEptr<T>::CVMEptr<T>()

  Operation Type:
     Constructor

  Purpose:
     Construct an object of this type. Note that no mapping is
     performed, and m_nOffset and m_nLength are both set to 0.
*/
template<class T>
CVMEptr<T>::CVMEptr()
{
  m_nOffset = 0;
  m_nLength = 0;
}

/*
  \fn CVMEptr(const CVMEptr& aCVMEptr)

  Operation Type:
     Copy construction

  \param CVMEptr aCVMEptr - the CVMEptr to copy
*/
template<class T>
CVMEptr<T>::CVMEptr(const CVMEptr& aCVMEptr)
{
  m_nOffset = aCVMEptr.m_nOffset;
  m_nLength = aCVMEptr.m_nLength;
  m_pStart  = aCVMEptr.m_pStart;
}

/*
  \fn CVMEptr& operator= (const CVMEptr& aCVMEptr)

  Operation type:
     operator= Assignment operator

  \param CVMEptr& aCVMEptr - reference to the CVMEptr to assign this to
*/
template<class T>
CVMEptr<T>&
CVMEptr<T>::operator= (const CVMEptr& aCVMEptr)
{
  if(this == &aCVMEptr) return *this;

  m_nOffset = aCVMEptr.m_nOffset;
  m_nLength = aCVMEptr.m_nLength;
  m_pStart  = aCVMEptr.m_pStart;

  return *this;
}

/*
  \fn CVMEptr<T>::~CVMEptr<T>()
  
  Operation type:
     Destructor

  Purpose: Destructor is responsible for munmapping the module
*/
template<class T>
CVMEptr<T>::~CVMEptr<T>()
{

  CVMEInterface::Unmap(m_pHandle, m_pStart, m_nLength);
  CVMEInterface::Close(m_pHandle);
}

/*
  \fn void CVMEptr<T>::CreateMap(uint32_t space, uint32_t base, uint32_t length)

  Operation Type:
     Map

  \param uint32_t space  - indicates which vme device to open 
                         (a16d16, a24d32 or a32d32)
         uint32_t base   - the base address of the module which we're mapping to
	 uint32_t lenght - the length (bytes) of the map
	 uint32_t crate  - Selects the vme crate.
*/
template<class T>
void
CVMEptr<T>::CreateMap(uint32_t space, uint32_t base, uint32_t length, uint32_t crate)
{

  m_nOffset = 0;

  // First open the specified vme device

  try {
    switch(space) {
    case 0:
      m_pHandle = CVMEInterface::Open(CVMEInterface::A16, crate);
      break;
    case 1:			// No difference between d16, d32.
    case 2:
      m_pHandle = CVMEInterface::Open(CVMEInterface::A24, crate);
      break;
    case 3:
      m_pHandle = CVMEInterface::Open(CVMEInterface::A32, crate);
      break;
    case 4:
      m_pHandle = CVMEInterface::Open(CVMEInterface::GEO , crate);
      break;
    default:
      std::cerr << "Default condition in CVMEptr<T> CreateMap switch " << space 
	   << std::endl;
      std::cerr.flush();
      
      break;
    }
  }
  catch(std::string& err) {
    std::string merr = "CVMEptr<T>::CreateMap - Opening connection to vme device";
    merr += '\n';
    merr += err;
    merr += "\n";
    CMmapError me(err.c_str());
    throw me; 
  }


  // Determine the page size and page offset

  m_nLength = length;
  try {
    m_pStart  = CVMEInterface::Map(m_pHandle, base, length);
  }
  catch (std::string& err) {
    std::string merr("CVMEptr<T>::CreateMap - Unable to perform CVMEInterface::Map");
    merr += '\n';
    merr += err;
    merr += "\n";
    throw CMmapError(merr.c_str());

  }

  
}


/*
  \fn T& CVMEptr<T>::operator*()

  Operation Type:
     Pointer dereference

  Purpose:
     Return the value stored at location m_nOffset in the mapped 
     address space. Throws an exception if the value is out of range.
*/
template<class T>
T&
CVMEptr<T>::operator*()
{
  if( (m_nOffset < 0) || (m_nOffset >= m_nLength)) {
    throw CRangeError (0, m_nLength, m_nOffset, 
		       "CVMEptr<T>::operator*() - outside of address window");

  }

  char* pVa = (reinterpret_cast<char*>(m_pStart) + m_nOffset*sizeof(T));

  if ( (intmax_t(pVa) % __alignof__(T) ) != 0) {
    throw std::runtime_error("CVMEptr<T>::operator*() Failed cast to T because misaligned memory");
  }

  T* pReturn = reinterpret_cast<T*>(pVa);

  return *pReturn;
}

/*
  \fn T* CVMEptr<T>::operator->()

  Operation Type:
     Pointer dereference

  Purpose:
     Return a pointer to the value at location m_nOffset
     into the mapped address space. Throws an exception if the value is
     out of range.
*/
template<class T>
T*
CVMEptr<T>::operator->()
{
  if((m_nOffset < 0) || (m_nOffset >= m_nLength)) {
    throw CRangeError(0, m_nLength, m_nOffset, 
		 "CVMEptr<T>::operator->() - outside of address window");
  }

  char* pVa = (reinterpret_cast<char*>(m_pStart) + m_nOffset*sizeof(T) );

  if ( (intmax_t(pVa) % __alignof__(T) ) != 0) {
    throw std::runtime_error("CVMEptr<T>::operator->() Failed cast to T because misaligned memory");
  }

  return reinterpret_cast<T*>(pVa);
}

/*
  \fn T& CVMEptr<T>::operator[] (uint32_t nOffset)

  Operation Type:
     Index read/write

  Purpose:
     Return the value stored at locaton nOffset into the mapped address
     space. Throws an exception if the value is out of range.

  \param uint32_t nOffset - the offset into the address space from which to read.
*/
template <class T>
T&
CVMEptr<T>::operator[] (uint32_t nOffset)
{
  if((m_nOffset < 0) || (m_nOffset >= m_nLength)) {
    throw CRangeError(0, m_nLength, m_nOffset, 
		      "CVMEptr<T>::operator[]() - outside of address window");
  }

  char* pVa = (reinterpret_cast<char*>(m_pStart) + nOffset*sizeof(T));
  if ( (intmax_t(pVa) % __alignof__(T) ) != 0) {
    throw std::runtime_error("CVMEptr<T>::operator*() Failed cast to T because misaligned memory");
  }

  T* pReturn = reinterpret_cast<T*>(pVa);

  return *pReturn;
}

/*
  \fn CVMEptr<T>& CVMEptr<T>::operator+(uint32_t nOffset)

  Operation Type:
     Mutator

  Purpose:
     Add an offset to the current offset. Throws an exception if this
     puts us out of range of the memory map.

  \param uint32_t nOffset - the integer to add to the current offset
*/
template<class T>
CVMEptr<T>
CVMEptr<T>::operator+(uint32_t nOffset)
{
  if(nOffset+m_nOffset <= m_nLength) {  // make sure we're in bounds
    CVMEptr<T> temp = *this;
    temp += nOffset;
    return temp;
  }
  else {
    CRangeError re(0, m_nOffset, nOffset, 
		 "CVMEptr<T>::operator+() - outside of mapped address window");
    throw re;
  }
}

/*
  \fn CVMEptr<T>& CVMEptr<T>::operator-(uint32_t nOffset)

  Operation Type:
     Mutator

  Purpose:
     Subtracts an offset to the current offset. Throws an exception if this
     puts us out of range of the memory map.

  \param uint32_t nOffset - the integer to subtract from the current offset
*/
template<class T>
CVMEptr<T>
CVMEptr<T>::operator-(uint32_t nOffset)
{
  if(nOffset <= m_nOffset) {    // make sure we're in bounds
    CVMEptr<T> temp = *this;
    temp -= nOffset;
    return temp;
  }
  else {
    CRangeError re(0, m_nOffset, nOffset,
	      "CVMEptr<T>::operator-() - outside of mapped addressed window");
    throw re;
  }
}

/*
  \fn CVMEptr<T>& CVMEptr<T>::operator+=(uint32_t nOffset)

  Operation Type:
     Mutator

  Purpose:
     Add an offset to the current offset. Throws an exception if this
     puts us out of range of the memory map.

  \param uint32_t nOffset - the integer to add to the current offset
*/
template<class T>
CVMEptr<T>&
CVMEptr<T>::operator+=(uint32_t nOffset)
{
  if(m_nOffset+nOffset <= m_nLength) {
    m_nOffset += nOffset;
    return *this;
  }
  else {
    CRangeError re(0, m_nOffset, nOffset,
	       "CVMEptr<T>::operator+=() - outside of mapped address window");
    throw re;
  }
}

/*
  \fn CVMEptr<T>& CVMEptr<T>::operator-=(uint32_t nOffset)

  Operation Type:
     Mutator

  Purpose:
     Subtract an offset to the current offset. Throws an exception if this
     puts us out of range of the memory map.

  \param uint32_t nOffset - the integer to subtract from the current offset
*/
template<class T>
CVMEptr<T>&
CVMEptr<T>::operator-=(uint32_t nOffset)
{
  if(m_nOffset-nOffset > 0) {
    m_nOffset -= nOffset;
    return *this;
  }
  else {
    CRangeError re(0, m_nOffset, nOffset,
	       "CVMEptr<T>::operator-=() - outside of mapped address window");
  }
}

/*
  \fn CVMEptr<T>& CVMEptr<T>::operator++()
  
  Operation Type:
     Mutator

  Purpose:
     (Pre)Increment the current offset. Throws an exception if this puts
     m_nOffset past the length of the memory map.
*/
template<class T>
CVMEptr<T>&
CVMEptr<T>::operator++()
{
  if(m_nOffset < m_nLength) {
    m_nOffset++;
    return *this;
  }
  else {
    std::string reason = "CVMEptr<T>::operator++() - reference to address which\n";
    reason += "is greater than map size";
    CRangeError re(0, m_nOffset, m_nOffset+1, reason);
    throw re;
  }
}

/*
  \fn CVMEptr<T>& CVMEptr<T>::operator--()
  
  Operation Type:
     Mutator

  Purpose:
     (Pre)Decrement the current offset. Throws an exception if this puts
     m_nOffset less than 0.
*/
template<class T>
CVMEptr<T>&
CVMEptr<T>::operator--()
{
  if(m_nOffset > 0) {
    m_nOffset--;
    return *this;
  }
  else {
    std::string reason = "CVMEptr<T>::operator--() - reference to address which\n";
    reason += "is less than map size";
    CRangeError re(0, m_nOffset, m_nOffset-1, reason);
    throw re;
  }
}

/*
  \fn CVMEptr<T>& CVMEptr<T>::operator++(Int_t)

  Operation type:
     Mutator

  Purpose:
     (Post)Increment the current offset. Throws an exception if this puts
     us past the length of the map.

  \param Int_t - dummy parameter indicates this is a 
                 post-increment operator
*/
template<class T>
CVMEptr<T>
CVMEptr<T>::operator++(Int_t)
{
  if(m_nOffset < m_nLength) {
    CVMEptr<T> p(*this);
    operator++();
    return p;
  }
  else {
    std::string reason = "CVMEptr<T>::operator++() - reference to memory address\n";
    reason += "which is greater than the size of the map";
    CRangeError re(0, m_nOffset, m_nOffset+1, reason);
    throw re;
  }
}

/*
  \fn CVMEptr<T>& CVMEptr<T>::operator--(Int_t)

  Operation type:
     Mutator

  Purpose:
     (Post)Decrement the current offset. Throws an exception if this puts
     us before the beginning of the memory map.

  \param Int_t - dummy parameter indicates this is a 
                 post-decrement operator
*/
template<class T>
CVMEptr<T>
CVMEptr<T>::operator--(Int_t)
{
  if(m_nOffset > 0) {
    CVMEptr<T> p(*this);
    operator--();
    return p;
  }
  else {
    std::string reason = "CVMEptr<T>::operator--() reference to memory address\n";
    reason += "which is less than the start address of the map";
    CRangeError re(0, m_nOffset, m_nOffset-1, reason);
    throw re;
  }
}

/*
  \fn Address_t CVMEptr<T>::getgenptr(uint32_t nOffset)

  Operation type:
     Selector

  Purpose:
     Return a pointer to the current address+offset into the module

  \param uint32_t nOffset - an offset to use to determine the pointer
*/
template<class T>
Address_t
CVMEptr<T>::getgenptr(uint32_t nOffset)
{
  Address_t p = (Address_t)(nOffset*sizeof(T) + (uint64_t)m_pStart);
  if(p) return p;
  else return (Address_t)kpNULL;
}

/*
  \fn Address_t CVMEptr<T>::getcurrptr(uint32_t nOffset)

  Operation Type:
     Selector

  Purpose:
     Return a pointer to the current address+current offset into the module
*/
template<class T>
Address_t
CVMEptr<T>::getcurrptr()
{
  Address_t p = (Address_t)(m_nOffset*sizeof(T) + (uint32_t)m_pStart);
  if(p) return p;
  else return (Address_t)kpNULL;
}
