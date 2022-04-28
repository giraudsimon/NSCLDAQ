/*
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321

Driver for CAEN C894 - CAEN 16 Channel LED

*/

#include <config.h>
#include "CPH7106.h"
#include "CControlModule.h"
#include "CCCUSB.h"
#include "CControlModule.h"
#include "CCCUSB.h"
#include "CCCUSBReadoutList.h"	// for the AM codes.

#include <stdint.h>
#include <stdio.h>
#include <tcl.h>
#include <string.h>

using namespace std;




/*!
  Construct hte beast. 
  @param - Name of the module.
*/
CPH7106::CPH7106() :
  CControlHardware()
{}

/*!
 *
 */
CPH7106::CPH7106(const CPH7106& rhs) 
 : CControlHardware(rhs)
{
}

/*!
   Copy construction
   @param rhs - source for the copy.
*/
CPH7106::~CPH7106()
{
}

/*!
  Assignment simply clones:
  @param rhs - the rhs of the assignment.
*/
CPH7106& 
CPH7106::operator=(const CPH7106& rhs)
{
  if (this != &rhs) {
    CControlHardware::operator=(rhs);
  }
  return *this;
}

/*!
   Use base class equality.
   @param rhs - The right hand side of the comparison.
   @return int
   @retval 0 - unequal 1 - equal.
*/
int
CPH7106::operator==(const CPH7106& rhs) const
{
  return CControlHardware::operator==(rhs);
}
/*!
  inequality compare
   @param rhs - The right hand side of the comparison.
   @return int
   @retval 1 - unequal 0 - equal.
*/
int
CPH7106::operator!=(const CPH7106& rhs) const
{
  return !(*this == rhs);
}
/////////////////////////////////////////////////////

/*!
  Attaching the configuration to us.  Here is where we provide the
  configuration parameter, which in our case is just the slot. number.
  @param configuration - References the module configuration object.
*/
void
CPH7106::onAttach(CControlModule& configuration)
{
  m_pConfig = &configuration;
  configuration.addParameter("-slot",
			     XXUSB::CConfigurableObject::isInteger, NULL,  string("0"));

}

/*!
  Initialization just forces the module into the camac state if necessary
  @param camac - The Crate controller object.
*/
void
CPH7106::Initialize(CCCUSB& camac)
{
  forceCamacMode(camac);
}

/*!
  Update is a noop for us as the state is readable.
*/
string
CPH7106::Update(CCCUSB& camac)
{
  return string("OK");
}

/*!
  Set a value.. determines which value to set and calls the appropriate
  set function.  If the value is bad, an error message is produced.
  @param camac - The camac interface object
  @param parameter - Which parameter to set: must e either threshold or mask.
  @param value     - value to which the parameter is set.. must be an integer.
  @return string
  @retval OK  - Success.
  @retval ERROR... - An error occured, the remainder of the string explains matters.

*/
string
CPH7106::Set(CCCUSB& camac, string parameter, string value)
{
  unsigned int parameterValue;
  if (sscanf(value.c_str(), "%i", &parameterValue) < 0) {
    return string("ERROR - Value is not an integer and must be");
  }
  if (parameter == "threshold") {
    return setThreshold(camac, parameterValue);
  }
  else if (parameter == "mask") {
    return setMask(camac, parameterValue);
  }
  return string("ERROR Invalid paramter name for CPH7106");

}

/*!
  Gets a value from the discriminator.
  @param camac - the controller.
  @param parameter - name of the parameter to get. This is one of:
                             "threshold", "mask", "mode"
  @return string
  @retval stringified parameter - success.
  @retval ERROR reaason  N aerror occured.

*/
string
CPH7106::Get(CCCUSB& camac, string parameter)
{
  if (parameter == "threshold") {
    return getThreshold(camac);
  }
  else if (parameter == "mask") {
    return getMask(camac);
  }
  else if (parameter == "mode") {
    return getMode(camac);
  }
  else {
    return string ("ERROR - invalid parameter name");
  }
}
/*!
  clone - virtual copy constructor.
*/

CControlHardware*
CPH7106::clone() const
{
  return (new CPH7106(*this));
}
//////////////////////////////////////////////////////////////////////
// Private utilities.
//

//
// Return the integer value of the -slot parameter.
//
uint32_t
CPH7106::getSlot()
{
  if (m_pConfig) {
    string strSlot = m_pConfig->cget("-slot");
    unsigned int slot;
    sscanf(strSlot.c_str(), "%i", &slot);
    return static_cast<uint32_t>(slot);
  }
  else {
    return static_cast<uint32_t>(0);
  }  
}
//
//  Turn an integer to a string
//
string
CPH7106::iToS(int value)
{
  char sValue[100];;
  sprintf(sValue, "%d", value);
  return string(sValue);
}
///
// Set the threshold value
//
string
CPH7106::setThreshold(CCCUSB& camac, uint16_t value)
{
  uint32_t slot = getSlot();
  uint16_t qx;
  camac.simpleWrite16(slot, 0, 17, value, qx);
  return string("OK");
}
//
// Retrieve the threshold value.
//
string
CPH7106::getThreshold(CCCUSB& camac)
{
  uint32_t slot = getSlot();
  uint16_t value;
  uint16_t qx;

  camac.simpleRead16(slot, 0, 1, value, qx);
  return iToS(value & 0x3ff);
}
//
//  Set the mask register.
//
string
CPH7106::setMask(CCCUSB& camac, uint16_t value)
{
  uint32_t slot = getSlot();
  uint16_t qx;
  camac.simpleWrite16(slot, 0, 16, value, qx);
  return string("OK");
}
//
// Get the mask register value
//
string
CPH7106::getMask(CCCUSB& camac)
{
  uint32_t slot = getSlot();
  uint16_t value;
  uint16_t qx;
  camac.simpleRead16(slot, 0, 0, value, qx);
  return iToS(value);
}
//
// get Mode - return the mode (local or camac).
//
string
CPH7106::getMode(CCCUSB& camac)
{
  uint32_t slot = getSlot();
  uint16_t mode;
  uint16_t qx;

  camac.simpleRead16(slot,  1, 1, mode, qx);
  if (mode & 0x8000) {
    return string("local");
  }
  else {
    return string("camac");
  }
}
// Attempt to put the module in camac mode.. This can fail if the switch
// is stubbornly set to local
//
void
CPH7106::forceCamacMode(CCCUSB& camac)
{
  uint16_t qx;
  camac.simpleControl(getSlot(), 0, 26, qx);
}
