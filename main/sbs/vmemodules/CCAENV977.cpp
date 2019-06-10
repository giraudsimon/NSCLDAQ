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
//  CCAENV977.cpp
//  Implementation of the Class CCAENV977
//  Created on:      07-Jun-2005 04:42:54 PM
//  Original author: Ron Fox
///////////////////////////////////////////////////////////

#include <config.h>
#include "CCAENV977.h"
#include "VmeModule.h"
#include <string>

using namespace std;

#define CHECK_ALL
#define _DEBUG
#include <DesignByContract.h>

using namespace DesignByContract;


// The following are word offsets into the device:

static const uint32_t INPUT_SET(0);
static const uint32_t INPUT_MASK(1);
static const uint32_t INPUT_READ(2);
static const uint32_t SINGLEHIT_READ(3);
static const uint32_t MULTIHIT_READ(4);
static const uint32_t OUTPUT_SET(5);
static const uint32_t OUTPUT_MASK(6);
static const uint32_t INTERRUPT_MASK(7);
static const uint32_t CLEAR_OUTPUT(8);
static const uint32_t SINGLEHIT_RDCLEAR(11);
static const uint32_t MULTIHIT_RDCLEAR(12);
static const uint32_t TEST_CONTROL(13);
static const uint32_t IPL(16);
static const uint32_t INTERRUPT_ID(17);
static const uint32_t SERIAL(18);
static const uint32_t FIRMWARE_REV(19);
static const uint32_t CONTROL_REGISTER(20);
static const uint32_t DUMMY_REGISTER(21);
static const uint32_t SOFTWARE_RESET(23);


// Multi bit masks.

static const uint16_t validTestBits =  (CCAENV977::test_Clear   |
					CCAENV977::test_Mask    |
					CCAENV977::test_OrMask  |
					CCAENV977::test_IrqMask |
					CCAENV977::test_Read);

static const uint16_t validControlBits = (CCAENV977::control_Pattern  |
					  CCAENV977::control_gateMask |
					  CCAENV977::control_OrMask);

/*!
   Construct a CAENV977 I/O register.
   @param base
        Base address of the module in the rotary switches
   @param crate
        Crate in which the module is installed (defaults to 0).
*/
CCAENV977::CCAENV977(uint32_t lBase, unsigned nCrate) :
  m_Module(*(new CVmeModule(CVmeModule::a32d32, lBase, 0x100, nCrate)))
{

}


/*!
   The destructor must delete the vme module.
*/
CCAENV977::~CCAENV977()
{
  delete &m_Module;
}
/*!
    Copy construction:
*/
CCAENV977::CCAENV977(const CCAENV977& rhs) :
  m_Module(*(new CVmeModule(rhs.m_Module)))
{
}
/*!
  Assignment:
 */
CCAENV977&
CCAENV977::operator=(const CCAENV977& rhs)
{
  if(this != &rhs) {
    m_Module = rhs.m_Module;
  }
  return *this;
}
/*!
   Equality comparison
*/
int
CCAENV977::operator==(const CCAENV977& rhs) const
{
  return m_Module == rhs.m_Module;
}
/*!
   Inequality 
*/
int
CCAENV977::operator!=(const CCAENV977& rhs) const
{
  return !(*this == rhs);
}

/// 'useful' operations.

/**
 * Read the input set register.
 */
uint16_t 
CCAENV977::inputSet()
{
  return m_Module.peekw(INPUT_SET);
}


/**
 * Write the input set register.
 * @param value    New value for the
 * 
 */
void 
CCAENV977::inputSet(uint16_t value)
{
  m_Module.pokew(value, INPUT_SET);
}


/**
 * Read the input mask register.
 */
uint16_t 
CCAENV977::inputMask()
{
  return m_Module.peekw(INPUT_MASK);
}


/**
 * write the input mask register.
 * @param mask    New input mask value.
 * 
 */
void 
CCAENV977::inputMask(uint16_t mask)
{
  m_Module.pokew(mask, INPUT_MASK);
}


/**
 * Read the input read register (which reflects the value of the instantaneous
 * inputs to the module).
 * 
 */
uint16_t 
CCAENV977::inputRead()
{
  return m_Module.peekw(INPUT_READ);
}


/**
 * Read the single hit read register.  
 */
uint16_t 
CCAENV977::singleHitRead()
{
  return m_Module.peekw(SINGLEHIT_READ);
}


/**
 * Read the multihit read register.
 */
uint16_t 
CCAENV977::multihitRead()
{
  return m_Module.peekw(MULTIHIT_READ);
}


/**
 * Read the output set register.
 */
uint16_t 
CCAENV977::outputSet()
{
  return m_Module.peekw(OUTPUT_SET);
}


/**
 * Write the output set register.
 * @param pattern    New output pattern.
 * 
 */
void 
CCAENV977::outputSet(uint16_t pattern)
{
  m_Module.pokew(pattern, OUTPUT_SET);
}


/**
 * Read the output mask register.
 */
uint16_t 
CCAENV977::outputMask()
{
  return m_Module.peekw(OUTPUT_MASK);
}


/**
 * Write the output mask register.
 * @param mask    New output mask value.
 * 
 */
void 
CCAENV977::outputMask(uint16_t mask)
{
  m_Module.pokew(mask, OUTPUT_MASK);
}


/**
 * Read the interrupt mask register.
 */
uint16_t 
CCAENV977::interruptMask()
{
  return m_Module.peekw(INTERRUPT_MASK);
}


/**
 * Write the interrupt mask register.
 * @param mask    New interrupt mask value.
 * 
 */
void 
CCAENV977::interruptMask(uint16_t mask)
{
  m_Module.pokew(mask, INTERRUPT_MASK);
}


/**
 * Clears all output flipflops.
 */
void 
CCAENV977::outputClear()
{
  m_Module.pokew(0, CLEAR_OUTPUT);
}


/**
 * Read the single hit register and clear it.
 */
uint16_t 
CCAENV977::singleHitReadAndClear()
{
  
  return m_Module.peekw(SINGLEHIT_RDCLEAR);
}


/**
 * Read and clear the multi hit read register.
 */
uint16_t 
CCAENV977::multiHitReadAndClear()
{

  return m_Module.peekw(MULTIHIT_RDCLEAR);

}


/**
 * Read the test control register.
 */

uint16_t 
CCAENV977::testControlRegister()
{

  return (m_Module.peekw(TEST_CONTROL) & validTestBits);
}


/**
 * Write the test control register.
 * @param mask    New mask of bits to write to the Test Control register.  If this
 * has invalid bits set, a contract violation is thrown.
 * 
 */
void 
CCAENV977::testControlRegister(uint16_t mask)
{

  REQUIRE(mask == (mask & validTestBits), "Invalid bits in test mask");
  m_Module.pokew(mask, TEST_CONTROL);

}



/**
 * Read the serial number register.
 */
uint16_t 
CCAENV977::serialNumber()
{

  return m_Module.peekw(SERIAL);

}


/**
 * Read the firmware revision level.
 */
uint16_t 
CCAENV977::firmwareLevel()
{

  return m_Module.peekw(FIRMWARE_REV);

}


/**
 * Write the control register.
 * @param mask    new mask of control register bits.  If extraneous bits are set,
 * a contract violation is thrown.
 * 
 */
void 
CCAENV977::controlRegister(uint16_t mask)
{

  REQUIRE(string("Invalid bits in control Register mask"), (mask & validControlBits) == mask);
  m_Module.pokew(mask, CONTROL_REGISTER);

}


/**
 * Read the control register.
 */
uint16_t 
CCAENV977::controlRegister()
{

  return (m_Module.peekw(CONTROL_REGISTER) & validControlBits);
}


/**
 * Resets the module to default conditions.
 */
void 
CCAENV977::Reset()
{

  m_Module.pokew(0, SOFTWARE_RESET);

}


