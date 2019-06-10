/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifdef SWIG
#ifndef _FLATTEN_NESTED_CLASSES
#define _FLATTEN_NESTED_CLASSES
#endif
%module CVMUSBReadoutList
%{
#define _FLATTEN_NESTED_CLASSES
#include <CVMUSBReadoutList.h>
%}
#endif

#ifndef __CVMUSBREADOUTLIST_H
#define __CVMUSBREADOUTLIST_H



#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __CRT_SYS_TYPES_H
#include <sys/types.h>
#ifndef __CRT_SYS_TYPES_H
#define __CRT_SYS_TYPES_H
#endif
#endif


#ifndef __CPP_IOSTREAM
#include <iostream>
#ifndef __CPP_IOSTREAM
#define __CPP_IOSTREAM
#endif
#endif

/*!
   The best way to use the VM-USB involves building lists of VME
   operations, called \em stacks.  These stacks can either be submitted
   for immediate execution or stored inside the VM-USB for triggered
   execution.  In this way, several events will be autonomously handled
   by the VM-USB with no computer intervention.

   This class allows application programs to painlessly build a stack.
   Stacks are built up by creating an instance of this class, 
   invoking menber functions  to add list elements, and then 
   passing the list to either CVMUSB::loadList or CVMUSB::executeList

   There is nothing sacred about a list with respect to copy construction,
   assignment, or comparison.  Those operations are simply delegated to 
   member data.

   This class is also wrapped by SWIG.  To make the wrapping easier to use
   there are a bunch of inlined methods that use more generic types.
   These also have a // SWIG comment near them.

   \note Not all VMUSB list operations are supported by this class.

*/

class CVMUSBReadoutList
{
private:
  std::vector<uint32_t> m_list; // Stack lines are all 32 bits wide.
public:
  CVMUSBReadoutList();
  CVMUSBReadoutList(std::vector<uint32_t>& list);
  CVMUSBReadoutList(const CVMUSBReadoutList& rhs);

  virtual ~CVMUSBReadoutList();
  
  CVMUSBReadoutList& operator=(const CVMUSBReadoutList& rhs);
  int operator==(const CVMUSBReadoutList& rhs) const;
  int operator!=(const CVMUSBReadoutList& rhs) const;
  
  
  // Operations on the list as a whole:
  
  void                  clear();
  size_t                size() const;
  std::vector<uint32_t> get()  const;
  
  // Register operations 
  
public:
  void addRegisterRead(unsigned int address);
  void addRegisterWrite(unsigned int address, uint32_t data);
  void addRegisterWrite(unsigned int address, int data) { // SWIG
    addRegisterWrite(address, (uint32_t)data);
  }

    // Single shot VME operations.  Note that these are only supported
    // in natural alignments, as otherwise it is not so easy to let the
    // application know how to marshall the multiple transers appropriately.
    
public:
  // Writes:

  void addWrite32(uint32_t address, uint8_t amod, uint32_t datum);
  void addWrite16(uint32_t address, uint8_t amod, uint16_t datum);
  void addWrite8(uint32_t address,  uint8_t amod, uint8_t datum);
  void addWrite32(int address, int amod, int data) { // SWIG
    addWrite32((uint32_t)address, (uint8_t)amod, (uint32_t)data);
  }
  void addWrite16(int address, int amod, int data) { // SWIG
    addWrite16((uint32_t)address, (uint8_t)amod, (uint16_t)data);
  }
  void addWrite8(int address, int amod, int data) { // SWIG
    addWrite8((uint32_t)address, (uint8_t)amod, (uint8_t)data);
  }



  // Reads:

  void addRead32(uint32_t address, uint8_t amod);
  void addRead16(uint32_t address, uint8_t amod);
  void addRead8(uint32_t address, uint8_t amod);
  void addRead32(int address, int amod) { // SWIG
    addRead32((uint32_t)address, (uint8_t)amod);
  }
  void addRead16(int address, int amod) { // SWIG
    addRead16((uint32_t)address, (uint8_t)amod);
  }  
  void addRead8(int address, int amod) { // SWIG
    addRead8((uint32_t)address, (uint8_t)amod);
  }


  // Block transfer operations. 
  // These must meet the restrictions of the VMUSB on block transfers.
  //
  void addBlockRead32(uint32_t baseAddress, uint8_t amod, size_t transfers);
  void addFifoRead32(uint32_t  baseAddress, uint8_t amod, size_t transfers);
  void addFifoRead16(uint32_t baseAddress, uint8_t amod, size_t transfers);
  void addBlockWrite32(uint32_t baseAddresss, uint8_t amod, void* data, 
		       size_t transfers);
  void addBlockRead32(int base, int amod, int transfers) { // SWIG
    addBlockRead32((uint32_t)base, (uint8_t)amod, (size_t)transfers);
  }
  void addFifoRead32(int base, int amod,int  transfers) { // SWIG
    addFifoRead32((uint32_t)base, (uint8_t)amod, (size_t)transfers);
  }
  void addFifoRead16(int base, int amod, int transfers) { // SWIG
    addFifoRead16((uint32_t)base, (uint8_t)amod, (size_t)transfers);
  }
  // NOTE: addBlockWrite is not supported for SWIG at this time...
  //       need to figure out how I'd want to implement it.



  // VMEUSB supports getting the block readout count from a constant mask and
  // a count read from a VME module.  This is supported in this software by
  // providing functions to set the mask from a list, 
  // to specify the readout of the count (8,16 or 32 bits).
  // and to initiate the block transfer:
  // as with all block reads, only 32 bit transfers are supported.
  //

  void addBlockCountRead8(uint32_t  address, uint32_t mask, uint8_t amod);
  void addBlockCountRead16(uint32_t address, uint32_t mask, uint8_t amod);
  void addBlockCountRead32(uint32_t address, uint32_t mask, uint8_t amod);
  void addMaskedCountBlockRead32(uint32_t address, uint8_t amod);
  void addMaskedCountFifoRead32(uint32_t address, uint8_t amod);
  void addBlockCountRead8(int a, int m, int am) { // SWIG
    addBlockCountRead8((uint32_t)a, uint32_t(m), (uint8_t)a);
  }
  void addBlockCountRead16(int a, int m, int am) { // SWIG
    addBlockCountRead16((uint32_t)a, uint32_t(m), (uint8_t)a);
  }
  void addBlockCountRead32(int a, int m, int am) { // SWIG
    addBlockCountRead32((uint32_t)a, uint32_t(m), (uint8_t)a);
  }
  void addMaskedCountBlockRead32(int a, int am) { // SWIG
    addMaskedCountBlockRead32((uint32_t)a, (uint8_t)am);
  }
  void addMaskedCountFifoRead32(int a, int am) { // SWIG
    addMaskedCountFifoRead32((uint32_t)a, (uint8_t)am);
  }



  // Miscellaneous:

  void addDelay(uint8_t clocks);
  void addDelay(int clocks) {	// SWIG
    addDelay((uint8_t)clocks);
  }


  void addMarker(uint16_t value);
  void addMarker(int value) {	// SWIG
    addMarker((uint16_t) value);
  }


  // Debugging:

  void dump(std::ostream& str);	/* Dump contents of stack. */


  // The following constants define address modifiers that are known to
  // VME Rev C.  There are other amods that are legal in newer revs of the
  // standard, since I don't know them, I don't list them :-).

  // Mappings between terms used in VME C table 2-3 and these defs:
  //
  // "extended" = a32 "nonprivileged" = User. "supservisory" = Priv.
  // "short"    = a16
  // "standard" = a24.
  //
  static const uint8_t a32UserData ;
  static const uint8_t a32UserProgram ;
  static const uint8_t a32UserBlock ;

  static const uint8_t a32PrivData ;
  static const uint8_t a32PrivProgram;
  static const uint8_t a32PrivBlock;

  static const uint8_t a16User ;
  static const uint8_t a16Priv;

  static const uint8_t a24UserData;
  static const uint8_t a24UserProgram;
  static const uint8_t a24UserBlock;
  
  static const uint8_t a24PrivData;
  static const uint8_t a24PrivProgram ;
  static const uint8_t a24PrivBlock ;

  // utility functions:

private:
  uint32_t dataStrobes(uint32_t address);
  void     addBlockRead(uint32_t base, size_t transfers,
			uint32_t startingMode,
			size_t   width = sizeof(uint32_t));
  void     lastTransferIsNumberData(uint32_t mask);

 public:
  // SWIG helper functions.

  static int uint32_vector_size(std::vector<uint32_t> v) {
    return v.size();
  }

  static int uint32_vector_get(std::vector<uint32_t> v, int n) {
    return  v[n];
  }

};


#endif
