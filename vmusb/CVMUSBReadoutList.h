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

   \note Not all VMUSB list operations are supported by this class.

*/

class CVMUSBReadoutList
{
private:
  std::vector<uint32_t> m_list; // Stack lines are all 32 bits wide.
public:
  CVMUSBReadoutList();
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

    // Single shot VME operations.  Note that these are only supported
    // in natural alignments, as otherwise it is not so easy to let the
    // application know how to marshall the multiple transers appropriately.
    
public:
  // Writes:

  void addWrite32(uint32_t address, uint8_t amod, uint32_t datum);
  void addWrite16(uint32_t address, uint8_t amod, uint16_t datum);
  void addWrite8(uint32_t address,  uint8_t amod, uint8_t datum);

  // Reads:

  void addRead32(uint32_t address, uint8_t amod);
  void addRead16(uint32_t address, uint8_t amod);
  void addRead8(uint32_t address, uint8_t amod);

  // Block transfer operations. 
  // These must meet the restrictions of the VMUSB on block transfers.
  //
  void addBlockRead32(uint32_t baseAddress, uint8_t amod, size_t transfers);
  void addFifoRead32(uint32_t  baseAddress, uint8_t amod, size_t transfers);

  // The following constants define address modifiers that are known to
  // VME Rev C.  There are other amods that are legal in newer revs of the
  // standard, since I don't know them, I don't list them :-).

  // Mappings between terms used in VME C table 2-3 and these defs:
  //
  // "extended" = a32 "nonprivileged" = User. "supservisory" = Priv.
  // "short"    = a16
  // "standard" = a24.
  //
  static const uint8_t a32UserData = 0x09;
  static const uint8_t a32UserProgram = 0xa;
  static const uint8_t a32UserBlock = 0x0b;

  static const uint8_t a32PrivData = 0x0d;
  static const uint8_t a32PrivProgram = 0x0e;
  static const uint8_t a32PrivBlock = 0x0f;

  static const uint8_t a16User = 0x29;
  static const uint8_t a16Priv = 0x2d;

  static const uint8_t a24UserData = 0x39;
  static const uint8_t a24UserProgram = 0x3a;
  static const uint8_t a24UserBlock = 0x3b;
  
  static const uint8_t a24PrivData = 0x3d;
  static const uint8_t a24PrivProgram = 0x3e;
  static const uint8_t a24PrivBlock = 0x3f;

  // utility functions:

private:
  uint32_t dataStrobes(uint32_t address);
  void     addBlockRead(uint32_t base, size_t transfers,
			uint32_t startingMode);

};


#endif
