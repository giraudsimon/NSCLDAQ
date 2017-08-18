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

#include <config.h>
#include "DriverTemplate.h"
#include "CControlModule.h"
#include "CCCUSB.h"
#include "CCCUSBReadoutList.h"	// for the AM codes.

#include <stdint.h>
#include <stdio.h>
#include <tcl.h>

using namespace std;

// Register offsets:

#define Const(name) static const uint32_t name = 



/*!
   construct the beast.. The shadow registers will all get set to zero
*/
CDriverTemplate::CDriverTemplate() :
  CControlHardware()
{
  // Construct and initialize any data you need here.
}

/*!

  Copy construction:
*/
CDriverTemplate::CDriverTemplate(const CDriverTemplate& rhs) :
  CControlHardware(rhs)
{
}
/*!
  While destruction could leak I seem to recall problems if I destroy
  the configuration..
*/
CDriverTemplate::~CDriverTemplate()
{
}

/*!
  Assignment is a clone:
*/
CDriverTemplate&
CDriverTemplate::operator=(const CDriverTemplate& rhs)
{
  if(this != &rhs) {
    // honor your parents
    CControlHardware::operator=(rhs);
  }
  return *this;
}

/*!
  Same configuration implies equality:
*/
int 
CDriverTemplate::operator==(const CDriverTemplate& rhs) const
{
  return CControlHardware::operator==(rhs);
}
/*
   != is the logical inverse of ==
*/
int
CDriverTemplate::operator!=(const CDriverTemplate& rhs) const
{
  return !(*this == rhs);
}

///////////////////////////////////////////////////////////////////////////

/*!
   Attachment : We need to define our parameters which are:
   The template defines -slot for the CAMAC slot number.

 \param configure - Encapsulates the configuration for this module.

*/
void
CDriverTemplate::onAttach(CControlModule& configuration)
{
  m_pConfig = &configuration;
  configuration.addParameter("-slot", XXUSB::CConfigurableObject::isInteger, NULL, string("0"));


}
////////////////////////////////////////////////////////////////////////////
/*!
    Initialize the module bringing it to a known state.


   \param CCCUSB& crate
     Controller that hooks us to the CC-USB.
*/
void
CDriverTemplate::Initialize(CCCUSB& crate)
{

}

/*!
  Update the device from the shadow configuration.

  \param crate  - CRATE controller object.
  \return string
  \retval "OK"  - Everything worked.
  \retval "ERROR -...."  Describes why things did not work.

*/
string
CDriverTemplate::Update(CCCUSB& crate)
{
  uint32_t slotAddress = getSlot();

  std::unique_ptr<CCCUSBReadoutList> list(crate.createReadoutList());

  //  Fill in the list with the operations needed to load the device
  // from any internal state we keep

  // This executes the list and reports any errors.

  uint32_t dummy;
  size_t   dummysize;

  int status = crate.executeList(*list, &dummy, sizeof(dummy), &dummysize);

  if (status != 0) {
    return string("ERROR - Could not execute update list in CC-USB");
  }
  else {
    return string("OK");
  }
  
}
///////////////////////////////////////////////////////////////////////////////////
/*!
   Set a value in the device.

  The parameter must be an integer.  For the inhibits it must be between
  0-0xffff, for the majority threshold it must be between 1 and 20.  For
  the other parameters in the range 0-0xff.

  \param crate        - The CC-USB controller object.
  \param parameter  - Name of the parameter to set.
  \param value      - Value to which the parameter will be set.

\note  There are also restrictions on the parameter name and the channel number of
      the parameter name.

   \return string
   \retval "OK"    - The parameter was set (the shadow value will also be set).
   \retval "ERROR -..."  An error was detected, the remaining string describes the error.
*/
string
CDriverTemplate::Set(CCCUSB& crate, string parameter, string value)
{

  // Perform the CCUSB operations needed to set parameter to value.
  // return "OK" on success and "ERROR" on falure 

  return string("OK");
}
/////////////////////////////////////////////////////////////////////////////////////////
/*!
   Get a value from the device.
   Since the actual device is read-only we will return a value from the shadow state
   of the device.  This should be accurate as long as the crate has not been power-cycled...
   and since a power cycle would invalidate the VM_USB controller object that's a good enough 
  condition.
   \param crate  - CC-USB object.
   \param parameter - The name of the desired parameter.
 
  \return string
  \retval stringified-integer   - Things went ok.
  \retval "ERROR - ..."         - An error occured described by the remainder of the string.
*/
string
CDriverTemplate::Get(CCCUSB& crate, string parameter)
{

  // Perform the CC usb operations to get and return the value of the parameter.
  // return ERROR if there's any sort of error:

  return string("1234");	// dummy value.
}

///////////////////////////////////////////////////////////////////////////////////////
/*!
  At present, cloning is a no-op.
*/
CControlHardware*
CDriverTemplate::clone() const
{
  return (new CDriverTemplate(*this));
}

//////////////////////////////////////////////////////////////////////////////////
///////////////// private utilities //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

/*
   Return the slot address of the device:

*/
uint32_t 
CDriverTemplate::getSlot()
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
