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
  \class CVmeModule
  \file CVmeModule.h

  Encapsulates a Vme module object. CVmeModules have access to
  memory which is mapped via the mmap(3) system service using 
  the CVME and CVMEptr objects. CVmeModules can write to and read 
  from these registers using poke and peek operations, respectively.

  \note
        While this module is not final, the data transfer methods are
       not virtual

  Author:
     Jason Venema
     NSCL
     Michigan State University
     East Lansing, MI 48824-1321
     mailto: venemaja@msu.edu
*/

#ifndef VMEMODULE_H
#define VMEMODULE_H

#include <CVME.h>
#include <stdint.h>


// Convenience macros:

#define SOffset(s,field) ((unsigned int)(((uint64_t)&(((s*)0)->field))/sizeof(uint16_t)))
#define LOffset(s,field) ((unsigned int)(((uint64_t)&(((s*)0)->field))/sizeof(uint32_t)))


/**
 * @class CVmeModule
 *   Encapsulates a chunk of VME address space providing an imperative,
 *   rather than data, interface to it.
 */
class CVmeModule
{
 public:
  enum Space {
    a16d16 = 0,
    a24d16 = 1,
    a24d32 = 2,
    a32d32 = 3,
    geo    = 4
  };

private:

  CVME<UShort_t> m_CVME;  // the CVME map to the module


 public:
  // Default constructors
  CVmeModule(Space space, uint32_t base, uint32_t length, int nCrate = 0);
  
  // Copy Constructor
  CVmeModule(const CVmeModule& aCVmeModule);

  // Destructor
  virtual ~CVmeModule() {}

  // Operator= assignment operator
  CVmeModule& operator= (const CVmeModule& aCVmeModule);

  // Operator== equality operator
  int operator== (const CVmeModule& aCVmeModule);

  // Mutator function


 protected:
  void setCVME(CVME<uint16_t> aCVME)
    {
      m_CVME = aCVME;
    }


  CVME<uint16_t>& getCVME()
    {
      return m_CVME;
    }


  // Public member functions
public:
  uint8_t peekb(uint32_t offset=0);
  uint16_t peekw(uint32_t offset=0);
  uint32_t peekl(uint32_t offset=0);
  void pokeb(uint8_t byte, uint32_t nOffset);
  void pokew(uint16_t word, uint32_t nOffset);
  void pokel(uint32_t lword, uint32_t nOffset);
  
  uint32_t readl(void* pBuffer, uint32_t nOffset, size_t longs);
  uint32_t readw(void* pBuffer, uint32_t nOffset, size_t words);
  uint32_t readb(void* pBuffer, uint32_t nOffset, size_t bytes);
  
 
 // Utility:
protected:
   void CopyToMe(const CVmeModule& rModule);
};

#endif









