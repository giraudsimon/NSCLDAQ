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
#include "C894.h"
#include "CControlModule.h"
#include "CCCUSB.h"
#include "CCCUSBReadoutList.h"	// for the AM codes.

#include <stdint.h>
#include <stdio.h>
#include <tcl.h>
#include <string.h>
#include <unistd.h>

using namespace std;

// Register offsets:

#define Const(name) static const uint32_t name = 



/*!
   construct the beast.. The shadow registers will all get set to zero
*/
C894::C894() :
  CControlHardware()
 {

  // Construct and initialize any data you need here.

	for (int i=0; i<16;i++)
	{
		m_thresholds[i] = 0;
	}

	for (int i=0; i<2;i++)
	{
		m_widths[i] = 0;
	}

	m_majority = 1;
	m_inhibits =0xffff;

}

/*!

  Copy construction:
*/
C894::C894(const C894& rhs) :
  CControlHardware(rhs)
{
}

/*!
  While destruction could leak I seem to recall problems if I destroy
  the configuration..
*/
C894::~C894()
{
}

/*!
  Assignment is a clone:
*/
C894&
C894::operator=(const C894& rhs)
{
  if(this != &rhs) {
    CControlHardware::operator=(rhs);
  }
  return *this;
}

/*!
  Same configuration implies equality:
*/
int 
C894::operator==(const C894& rhs) const
{
  return CControlHardware::operator==(rhs);
}
/*
   != is the logical inverse of ==
*/
int
C894::operator!=(const C894& rhs) const
{
  return !(*this == rhs);
}

///////////////////////////////////////////////////////////////////////////

/*!
   Attachment : We need to define our parameters which are:
   The template defines -slot for the CAMAC slot number.
   We also define  a -file to hold the discriminator settings file that
   is loaded into the module on startup:

 \param configure - Encapsulates the configuration for this module.

*/
void
C894::onAttach(CControlModule& configuration)
{
  m_pConfig = &configuration;
  configuration.addParameter("-slot", XXUSB::CConfigurableObject::isInteger, NULL, string("0"));
  configuration.addParameter("-file", (typeChecker)NULL, NULL, string(""));


}
////////////////////////////////////////////////////////////////////////////
/*!
    Initialize the module bringing it to a known state.


   \param CCCUSB& camac
     Controller that hooks us to the CC-USB.
*/
void
C894::Initialize(CCCUSB& camac)
{
  configFileToShadow();		// load the local config from file
  Update(camac);		// update the module to match it.
}

/*!
  Update the device from the shadow configuration.

  \param camac  - CRATE controller object.
  \return string
  \retval "OK"  - Everything worked.
  \retval "ERROR -...."  Describes why things did not work.

*/
string
C894::Update(CCCUSB& camac)
{
  uint32_t slotAddress = getSlot();

  std::unique_ptr<CCCUSBReadoutList> pList(camac.createReadoutList());;

  //  Fill in the list with the operations needed to load the device
  // from any internal state we keep

  // This executes the list and reports any errors.

  for (int i=0; i < 16; i ++) {
    pList->addWrite16(slotAddress, i, 16, \
		    -m_thresholds[i]); // Thresholds in mV module wants 0-255
  }

  for (int i=0; i < 2; i++) {
    pList->addWrite16(slotAddress, i, 18, m_widths[i]);
  }

  pList->addWrite16(slotAddress, 0, 17, m_inhibits);
  pList->addWrite16(slotAddress, 0, 20, majorityToRegister(m_majority));

  // Execute the list:


  uint32_t dummy;
  size_t   dummysize;

  int status = camac.executeList(*pList, &dummy, sizeof(dummy), &dummysize);

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

  \param camac        - The CC-USB controller object.
  \param parameter  - Name of the parameter to set.
  \param value      - Value to which the parameter will be set.

\note  There are also restrictions on the parameter name and the channel number of
      the parameter name.

   \return string
   \retval "OK"    - The parameter was set (the shadow value will also be set).
   \retval "ERROR -..."  An error was detected, the remaining string describes the error.
*/
string
C894::Set(CCCUSB& camac, string parameter, string value)
{

  // Perform the CCUSB operations needed to set parameter to value.
  // return "OK" on success and "ERROR" on falure 

  unsigned int channelNumber;
  unsigned int parameterValue;

  if (sscanf(value.c_str(), "%i", &parameterValue) < 0) {
    return string("ERROR - Value is not an integer and must be");
  }

  if (sscanf(parameter.c_str(), "threshold%u", &channelNumber) == 1) {
    return setThreshold(camac, channelNumber, parameterValue);
  }
  else if (sscanf(parameter.c_str(), "width%u", &channelNumber) == 1) {
    return setWidth(camac, channelNumber, parameterValue);
  }
  else if (parameter == string("inhibits")) {
    return setInhibits(camac, parameterValue);
  }
  else if (parameter == string("majority")) {
    return setMajority(camac, parameterValue);
  }
  else {
    return string("ERROR - parameter specifier invalid");
  }

}
/////////////////////////////////////////////////////////////////////////////////////////
/*!
   Get a value from the device.
   Since the actual device is read-only we will return a value from the shadow state
   of the device.  This should be accurate as long as the camac has not been power-cycled...
   and since a power cycle would invalidate the CC_USB controller object that's a good enough 
  condition.
   \param camac  - CC-USB object.
   \param parameter - The name of the desired parameter.
 
  \return string
  \retval stringified-integer   - Things went ok.
  \retval "ERROR - ..."         - An error occured described by the remainder of the string.
*/
string
C894::Get(CCCUSB& camac, string parameter)
{

  unsigned int channelNumber;

  if (sscanf(parameter.c_str(), "threshold%u", &channelNumber) == 1) {
    return getThreshold(channelNumber);
  }
  else if (sscanf(parameter.c_str(), "width%u", &channelNumber) == 1) {
    return getWidth(channelNumber);
  }
  else if (parameter == string("inhibits")) {
    return getInhibits();
  }
  else if (parameter == "majority") {
    return getMajority();
  }
  else {
    return string("ERROR -  parameter specifier invalid");
  }

}

///////////////////////////////////////////////////////////////////////////////////////
/*!
  At present, cloning is a no-op.
*/
CControlHardware*
C894::clone() const
{
  return (new C894(*this));
}

//////////////////////////////////////////////////////////////////////////////////
///////////////// private utilities //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

/*
   Return the slot address of the device:

*/
uint32_t 
C894::getSlot()
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


/*
  Set a threshold value:
  Parameters:
    camac            - the CC-USB controller object.
    channel        - Which one to set (error if not in [0, 15]).
    value          - Value to set (error if not in [0,0xff]).

  Returns:
    string("OK")   - Everything worked.
    string("ERROR - ...") There was a problem.  The string describes the problem.
*/

string
C894::setThreshold(CCCUSB& camac, unsigned int channel, uint16_t value)
{

  // Validate parameters:

  if (channel >= 16) {
    return string("ERROR - Invalid channel number [0-15]");
  }
  if (value > 0xff) {
    return string("ERROR - Invalid threshold value [0-255]");
  }

  // set the hardware:

  uint16_t qxmask;
  int status = camac.simpleWrite16(getSlot(), channel, 16, value, qxmask);
  if (status != 0) {
    return string("ERROR - Write to device failed");
  }

  // set the shadow register:

  m_thresholds[channel ] = value;

  return string("OK");
}

/*
Set a width value.
  Parameters:
   camac      - The CC-USB controller.
   selector - Which width to set 0 - 0-7, 1, 8-15.
   value    - New width value. [0,0xff]

  Returns:
    string("OK")   - Everything worked.
    string("ERROR - ...") There was a problem.  The string describes the problem
*/
string
C894::setWidth(CCCUSB& camac, unsigned int selector, uint16_t value)
{
  // validation:

  if (selector > 1) {
    return string("ERROR - Invalid width selector");
  }
  if (value > 0xff) {
    return string("ERROR - Invalid width value [0..255]");
  }

  // set the hardware

  uint16_t qxmask;
  int status = camac.simpleWrite16(getSlot(), selector, 18, value, qxmask);

  if (status != 0) {
    return string("ERROR - Write to device failed");
  }

  // set the shadow register.

  m_widths[selector] = value;

  return string("OK");
}

/*
  Set the majority logic value.  The majority is storead as a majority level that has to
  be converted to a register value.

  Parameters:
  camac      - CC-USB controller object.
  value    - New majority logic value [1..20].


 Returns:
    string("OK")   - Everything worked.
    string("ERROR - ...") There was a problem.  The string describes the problem
*/
string
C894::setMajority(CCCUSB& camac, uint16_t value)
{
  // Validate:

  if ((value < 1)  || (value > 20)) {
    return string("ERROR - invalid majority level [1..20]");
  }
  // Set the hardware

  uint16_t  qxmask;
  int status  = camac.simpleWrite16(getSlot(), 0, 20, majorityToRegister(value), qxmask);

  if(status != 0) {
    return string("ERROR - write to device failed");
  }

  // set the shadow register.

  m_majority = value;

  return string("OK");
}


/*!
   Set the inhibits value

  Parameters:
  camac    - CC-USB controller object.
  value    - New inhibits value [0-255].


 Returns:
    string("OK")   - Everything worked.
    string("ERROR - ...") There was a problem.  The string describes the problem
*/
string
C894::setInhibits(CCCUSB& camac, uint16_t value)
{


  // Set the hardware

  uint16_t qxmask;
  int status = camac.simpleWrite16(getSlot(), 0, 17, value, qxmask);

  if (status != 0) {
    return string("ERROR - write to device failed");
  }

  // set the shadow registers.

  m_inhibits = value;

  return string("OK");
}

/////////////////////////////////////////////////////////////////////////////////

/*
  return a threshold value.

*/
string
C894::getThreshold(unsigned int channel)
{
  // validate:

  if (channel >= 16) {
    return string("ERROR - invalid channel number");
  }

  return iToS(m_thresholds[channel]);
}
/*
  Return a width
*/
string
C894::getWidth(unsigned int selector)
{
  if (selector > 1) {
    return string("ERROR - invalid width selector");
  }
  return iToS(m_widths[selector]);
}

/*!
  Return majority logic level
*/
string
C894::getMajority()
{
  return iToS(m_majority);
}

/*
   Return inhibits (really enables)
*/
string
C894::getInhibits()
{
  return iToS(m_inhibits);
}

///////////////////////////////////////////////////////////////////////

/*
  Turn an integer into a string
*/

string
C894::iToS(int value)
{
  char sValue[100];;
  sprintf(sValue, "%d", value);
  return string(sValue);
}

//////////////////////////////////////////////////////////////////////

/*
  Turn the majority level into a desired register value
*/
uint16_t
C894::majorityToRegister(int value)
{
  uint16_t result = (uint16_t)(((float)value*50.0 -25.0)/4.0 + 0.5);

  return result;
}
/////////////////////////////////////////////////////////////////////

/*
   Configuration file processing:
*/
void
C894::configFileToShadow()
{
  string filename = initializationFile();

  Tcl_Obj* pObj = Tcl_NewStringObj(filename.c_str(), strlen(filename.c_str()));

  Tcl_IncrRefCount(pObj);	// Evidently FSAccess needs this.

  if (Tcl_FSAccess(pObj, R_OK)) {
    Tcl_DecrRefCount(pObj);
    return;			// Not allowed to read the file..or does not exist.
  }
  Tcl_DecrRefCount(pObj);

  // Simplest actually to do this in a non  object manner:

  Tcl_Interp* pInterp = Tcl_CreateInterp();
  int status          = Tcl_EvalFile(pInterp, filename.c_str());

  // Ignore errors as some items may have been set:

  // The threshold array:

  for (int i=0; i < 16; i++) {
    const char* value = Tcl_GetVar2(pInterp, "thresholds", iToS(i).c_str(), TCL_GLOBAL_ONLY);
    if (value) {
      int iValue;
      if (sscanf(value, "%i", &iValue) == 1) {
	m_thresholds[i] = iValue;
      }
    }
  }
  // Widths:

  for (int i =0; i < 2; i++) {
    const char* value = Tcl_GetVar2(pInterp, "widths", iToS(i).c_str(),
				    TCL_GLOBAL_ONLY);
    if(value) {
      int iValue;
      if (sscanf(value, "%i", &iValue) == 1) {
	m_widths[i] = iValue;
      }
    }
  }

  // enables and majority

  const char* value = Tcl_GetVar(pInterp, "enables", TCL_GLOBAL_ONLY);
  if (value) {
    int iValue;
    if (sscanf(value, "%i", &iValue) == 1) {
      m_inhibits = iValue;
    }
  }
  value = Tcl_GetVar(pInterp, "majority", TCL_GLOBAL_ONLY);
  if (value) {
    int iValue;
    if (sscanf(value, "%i", &iValue) == 1) {
      m_majority = iValue;
    }
  }
  Tcl_DeleteInterp(pInterp);

}
/*
  Return the name of the file to process for the initial values of the 
  shadow register set:
*/
string
C894::initializationFile()
{
  if (m_pConfig) {
    return m_pConfig->cget("-file");
  } 
  else {
    return string("");
  }
}
