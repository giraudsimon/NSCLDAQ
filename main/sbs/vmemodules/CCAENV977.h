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

///////////////////////////////////////////////////////////
//  CCAENV977.h
//  Implementation of the Class CCAENV977
//  Created on:      07-Jun-2005 04:42:54 PM
//  Original author: Ron Fox
///////////////////////////////////////////////////////////

#ifndef CCAENV977_H
#define CCAENV977_H

#include <daqdatatypes.h>
#include <stdint.h>

// Forward definitions.

class CVmeModule;

/*!
 * Low level driver for the CCAENV977 I/O register module.  This class provides
 * member functions and state to interact with a single module.In addition to the
 * member shown, we'll need several static const members representing bits in the
 * control/status registers:
 * - Test Register:
 * - test_Clear    - clear test channel
 * - test_Mask    - Mask the test channel.
 * - test_OrMask - Mask the or of test into the or output.
 * - test_IrqMask - Mask test's generation of interrupts.
 * - test_Read     - The value of the test register.
 * 
 * ControlRegister:
 * 
 * - control_Pattern      - Run module in pattern mode.
 * - control_gateMask - Disable the gate.
 * - control_OrMask     -  Disable or output on front panel.
 * @author Ron Fox
 * @version 1.0
 * @created 07-Jun-2005 04:42:54 PM
 */
class CCAENV977
{
private:
	/**
	 * Device indpedendent way to access a set of VME addresses.
	 */
  CVmeModule& m_Module;


// Constants with class scope:
public:
    // bits in the test control register:
    
    static const uint16_t  test_Clear   = 1;
    static const uint16_t  test_Mask    = 2;
    static const uint16_t  test_OrMask  = 4;
    static const uint16_t  test_IrqMask = 8;
    static const uint16_t  test_Read    = 0x10;
    
    // Bits in the module constrol register:
    
    static const uint16_t control_Pattern  = 1;
    static const uint16_t control_gateMask = 2;
    static const uint16_t control_OrMask   = 4;

public:
    // Constructors and other canonical operations:
    
  CCAENV977(uint32_t lBase, unsigned nCrate = 0);
  virtual ~CCAENV977();
  CCAENV977(const CCAENV977& rhs);
  CCAENV977& operator=(const CCAENV977& rhs);
  int operator==(const CCAENV977& rhs) const;
  int operator!=(const CCAENV977& rhs) const;
  
  // Functions of the object itself:
  
  uint16_t inputSet();
  void     inputSet(uint16_t value);
  uint16_t inputMask();
  void     inputMask(uint16_t mask);
  uint16_t inputRead();
  uint16_t singleHitRead();
  uint16_t multihitRead();
  uint16_t outputSet();
  void     outputSet(uint16_t pattern);
  uint16_t outputMask();
  void     outputMask(uint16_t mask);
  uint16_t interruptMask();
  void     interruptMask(uint16_t mask);
  void     outputClear();
  uint16_t singleHitReadAndClear();
  uint16_t multiHitReadAndClear();
  uint16_t testControlRegister();
  void     testControlRegister(uint16_t mask);
  uint16_t serialNumber();
  uint16_t firmwareLevel();
  void     controlRegister(uint16_t mask);
  uint16_t controlRegister();
  void     Reset();
  

};


#endif 
