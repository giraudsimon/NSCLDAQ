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


//TODO: Factor the getters so that we just need to hand them the offset within the
//      channel and the factored code will hand us the 6 values.

#include <config.h>
#include "CV6533.h"
#include <string>
#include "CControlModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"	// for the AM codes.

#include <TCLInterpreter.h>
#include <TCLObject.h>

#include <stdint.h>
#include <stdio.h>
#include <tcl.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <memory>

using namespace std;

// Register offset:'

#define Const(name) static const uint32_t name =

// Board parameters (common to all channels):

Const(BoardVmax)     0x50;
Const(BoardImax)     0x54;
Const(BoardStatus)   0x58;
Const (Firmware)      0x5c;

// Each Channels has a common format that is described later:

Const(Channels[6])   {
  0x080, 0x100, 0x180, 0x200, 0x280, 0x300
};

// Board configuration parameters:

Const(ChannelCount)   0x8100;
Const(Description)    0x8102;
Const(Model)          0x8116;
Const(SerialNo)       0x811e;
Const(VmeFirmware)    0x8120;

//  Below are the register offsets within each of the Channels array elements:

Const(Vset)      0x00;
Const(Iset)      0x04;
Const(VMon)      0x08;
Const(IMon)      0x0c;
Const(PW)        0x10;
Const(ChStatus)  0x14;
Const(TripTime)  0x18;
Const(SVMax)     0x1c;
Const(RampDown)  0x20;
Const(RampUp)    0x24;
Const(PwDown)    0x28;
Const(Polarity)  0x2c;
Const(Temp)      0x30;

// Global status register bits:

Const(Chan0Alarm)    0x0001;
Const(Chan1Alarm)    0x0002;
Const(Chan2Alarm)    0x0004;
Const(Chan3Alarm)    0x0008;
Const(Chan4Alarm)    0x0010;
Const(PwrFail)       0x0080;
Const(OverPwr)       0x0100;
Const(MaxVUncal)     0x0200;
Const(MaxIUncal)     0x0400;

// Individual channel status register bits.

Const(On)           0x0001;
Const(RampingUp)    0x0002;
Const(RampingDown)  0x0004;
Const(OverCurrent)  0x0008;
Const(OverVoltage)  0x0010;
Const(UnderVoltage) 0x0020;
Const(MaxV)         0x0040;
Const(MaxI)         0x0080;
Const(Trip)         0x0100;
Const(OverPower)    0x0200;
Const(Disabled)     0x0400;
Const(Interlocked)  0x0800;
Const(Uncalibrated) 0x1000;


//  Address modifier to use when accessing the board:

static const uint8_t amod = CVMUSBReadoutList::a32PrivData;

/*------------------------------------- Canonical methods */

/**
 * Construct the object.  For now we won't do anything
 * with the monitored registers.  We will just let those
 * populate as the update/monitor list runs.
 */
CV6533::CV6533() :
  CControlHardware(),
  m_pConfiguration(0)
{
}
/**
 * Copy construction; clone already does what we need
 * just invoke it:
 */
CV6533::CV6533(const CV6533& rhs) :
  CControlHardware(rhs)
{
  copy(rhs);
}
/**
 * Destruction is not really needed since we are not going to be
 * destroyed for the life of the program.
 */
CV6533::~CV6533()
{
}
/**
 *  Assignment is also taken care of by clone for the most part.
 * Just need to be sure we don't do anything on a self assign.
 * @param rhs - The object being assigned to us.
 * @return CV6533
 * @retval *this
 */
CV6533&
CV6533::operator=(const CV6533& rhs)
{
  if (this != &rhs) {
    copy(rhs);
    CControlHardware::operator=(rhs);
  }
  return *this;
}
/**
 * Equal configuration implies equality.
 * @param rhs  - The object to check with.
 */
int
CV6533::operator==(const CV6533& rhs) const
{
  return CControlHardware::operator==(rhs);
}
/**
 ** Inequality as usual is the inverse of equality:
 ** @param rhs - the object to compare with *this
 */
int
CV6533::operator!=(const CV6533& rhs) const
{
  return !(*this == rhs);
}
/*------------------ Operations for running the module -----*/


/**
 * This function is called when the object is attached
 * to its configuration.  We must save the configuration
 * and register our parameters.  At present, for HV
 * units we are not going to save any configuration.
 * that would be the responsibility of the GUI the user
 * interacts with
 * @param configuration - Configuration object that will
 *                        maintain our object's configuration.
 */
void
CV6533::onAttach(CControlModule& configuration)
{
  m_pConfiguration = &configuration;
  configuration.addParameter("-base", \
			     XXUSB::CConfigurableObject::isInteger,
			     NULL, std::string("0"));
}
/**
 * Called to initalize the module.  For us initialization is
 * playing it safe: All the voltages are set to zero.
 * Each channel is turned off.
 * @param vme   - Object that proxies for the VM-USB 
 *                controller module.
 */
void
CV6533::Initialize(CVMUSB& vme)
{


  unique_ptr<CVMUSBReadoutList> pList(vme.createReadoutList());	// List of configuration ops.
  CVMUSBReadoutList& list = *pList;
  for (unsigned int i =0; i < 6; i++) {
    turnOff(list, i);
    setRequestVoltage(list, i, 0.0);
  }
  // Execute the list.
  size_t buffer;		// No actual reads..
  int status = vme.executeList(list,
			       &buffer, sizeof(buffer),
			       &buffer);

}
/**
 * For devices with write-only registers, this function
 * would set the device to a configuration that matches
 * state held in the driver.  This function serves
 * no purpose for the CAEN V6533 and thus is a no-op.
 * @param vme - VM-USB proxy object.
 * @return string
 * @retval OK
 */
string
CV6533::Update(CVMUSB& vme)
{
  return "OK";
}
/**
 *  Processes set operations for the device.  See
 * the header for a description of the supported 
 * parameters.
 * @param vme         - Proxy object for the VM-USB controller.
 * @param parameter   - Name of the parameter to set.
 *                      in the parmeter set described in the
 *                      header the n trailing each paramter name
 *                      is the channel number (0-5).
 * @param value       - Value of the parameter.
 * @return string
 * @retval "OK"   - The set worked correctly.
 * @retval "ERROR - ..." Some error occured. The remainder
 *                       of the string describes
 *                       the error
 */
string
CV6533::Set(CVMUSB& vme, string parameter, string value)
{
  // The meaning of the value depends on the actual parameter
  // type.  What this function does is determine the
  // root parameter name and the channel number (if
  // applicable) and dispatch to the appropriate function.

  unsigned channelNumber;
  CVMUSBReadoutList list;
 
  try {

    if (sscanf(parameter.c_str(), "v%u", &channelNumber) == 1) {
      setRequestVoltage(list, channelNumber, atof(value.c_str()));
    }
    else if (sscanf(parameter.c_str(), "i%u",
		    &channelNumber) == 1) {
      setRequestCurrent(list, channelNumber, atof(value.c_str()));
    }
    else if(sscanf(parameter.c_str(), "on%u", &channelNumber) == 1) {
      setChannelOnOff(list, channelNumber, strToBool(value));
    }
    else if (sscanf(parameter.c_str(), "ttrip%u", &channelNumber) == 1) {
      setTripTime(list, channelNumber, atof(value.c_str()));
    }
    else if (sscanf(parameter.c_str(), "svmax%u", &channelNumber) == 1) {
      setMaxVoltage(list, channelNumber, atof(value.c_str()));
    }
    else if (sscanf(parameter.c_str(), "rdown%u", &channelNumber) == 1) {
      setRampDownRate(list, channelNumber, atof(value.c_str()));
    } 
    else if (sscanf(parameter.c_str(), "rup%u", &channelNumber) == 1) {
      setRampUpRate(list, channelNumber, atof(value.c_str()));
    }
    else if (sscanf(parameter.c_str(), "pdownmode%u", &channelNumber) ==1 ) {
      setPowerDownMode(list, channelNumber, value);
    }
    else {
      throw string("Unrecognized parameter");
    }

    // If there's a non zero length list, execute it.
    
    if (list.size() > 0) {
      size_t buffer;
      int status= vme.executeList(list,
				  &buffer, sizeof(buffer),
				  &buffer);
      if (status < 0) {
	throw string("VME list execution failed");
      }
    }
  }
  catch(string msg) {
    string error = "ERROR - ";
    error += msg;
    return error;
  }
  
  return string("OK");
}
/**
 * Get - this function retrieves the value of a parameter from the device.
 *       note that if a parameter is in the monitor set it will still be read from the device
 *       though the monitored data will be updated by the read that results.
 *       For readings that are per channel, we are going to save some time by 
 *       reading all values from all channels.. When data taking is in progress, this will
 *       reduce the number of times the Get needs to be invoked and hence the number
 *       of times the DAQ needs to be stopped.
 *  @param vme        - Object that stands in for the VM-USB controller.
 *  @param parameter  - Name of the paramter to get.. in the case of the per channel parameters
 *                      leave off the channel number as all channesl will be read and returned
 *                      as a Tcl List.
 * @return string
 * @retval "ERROR -..." An error occured, the remainder of the text string is a human readable
 *                      error message.
 * @retval "OK xxx"   Everything worked.  If xxx represents a single paramter (e.g. globalmaxv
 *                    this will just be a number.  If the requested parameter was a per channel value
 *                    xxx will be a correctly formatted Tcl list that contains the per channel
 *                    values of the parameter.
 */
string
CV6533::Get(CVMUSB& vme, string parameter)
{
  // error handing in the utilities means tossing exceptions with the error message.
  // the individual utilities will return the correct appropriate output (not the ok)

  string ok = "OK ";
  string result;
  try {
    // Big honking if/else to deal with all the parameters... all oparameters are readable
    // and the utilities will take care of updating the member variables:

    if (parameter == "globalmaxv") {
      result = getGlobalMaxV(vme);
    }
    else if (parameter == "globalmaxI") {
      result = getGlobalMaxI(vme);
    }
    else if (parameter == "v") {
      result = getChannelVoltages(vme);
    }
    else if (parameter == "i") {
      result = getChannelCurrents(vme);
    }
    else if (parameter == "on") {
      result = getOnOffRequests(vme);
    }
    else if (parameter == "vact") {
      result = getActualVoltages(vme);
    }
    else if (parameter == "iact") {
      result = getActualCurrents(vme);
    }
    else if (parameter == "status") {
      result = getChannelStatuses(vme);
    }
    else if (parameter == "ttrip") {
      result = getTripTimes(vme);
    }
    else if (parameter == "svmax") {
      result = getSoftwareVmax(vme);
    }
    else if (parameter == "rdown") {
      result = getRampDownRates(vme);
    }
    else if (parameter == "rup") {
      result = getRampUpRates(vme);
    }
    else if (parameter == "pdownmode") {
      result = getPowerdownModes(vme);
    }
    else if (parameter == "polarity") {
      result = getPolarities(vme);
    }
    else if (parameter == "temp") {
      result = getTemperatures(vme);
    }
    else {
      throw "Invalid parameter";
    }
  }
  catch(string msg) {
    string error = "ERROR - ";
    error       += msg;
    return error;
  }
  ok += result;
  return ok;
}
/**
 * clone another CV6533 into this...this implements a virtual copy constructor.
 *
 * @param rhs - the object to clone into *this
 */
CControlHardware*
CV6533::clone() const
{
  return (new CV6533(*this));
}

void CV6533::copy(const CV6533& rhs)
{
  m_globalStatus   = rhs.m_globalStatus;
  memcpy(m_channelStatus, rhs.m_channelStatus, sizeof(m_channelStatus));
  memcpy(m_voltages,      rhs.m_voltages,      sizeof(m_voltages));
  memcpy(m_currents,      rhs.m_currents,      sizeof(m_currents));
  memcpy(m_temperatures,  rhs.m_temperatures,  sizeof(m_temperatures));
}

/*----------------------------------  Monitoring --------------------------*/

/**
 * Add our list to the monitoring functions.  Unfortunately, the whole
 * layout of the register set and 16 bitedness means we're going to be doing
 * single shot transfers rather than blocks.
 * @param vmeList - VME readout list to which we will be adding data.
 */
void
CV6533::addMonitorList(CVMUSBReadoutList& vmeList)
{
  uint32_t base  = getBase();
  // Global status:

  vmeList.addRead16(base + BoardStatus, amod);
  
  // Channel stuff will be grouped by channel so that looping can be done both here and
  // when unpacking:

  for(int i =0; i < 6; i++) {
    vmeList.addRead16(base + Channels[i] + ChStatus, amod); // Channel status.
    vmeList.addRead16(base + Channels[i] + VMon,     amod); // Actual voltage.
    vmeList.addRead16(base + Channels[i] + IMon,     amod); // Monitored voltage.
    vmeList.addRead16(base + Channels[i] + Temp,     amod); // Channel temperature.
  }

}
/**
 * Unpack the data from the monitor list into our member data.
 * @param pData      - Pointer to where in the buffer to start.
 * @param remaining  - Words of data left in the list.
 * @return void*
 * @retval pointer to the next unused part of pData.
 */
void*
CV6533::processMonitorList(void* pData, size_t remaining)
{
  // For now don't bother with remaining... TODO: Figure out some sort of
  // error handling strategy if remaining is too small to satisfy our needs.

  uint16_t*   p = reinterpret_cast<uint16_t*>(pData);

  m_globalStatus = *p++;		// The global status register is first.

  for (int i = 0; i < 6; i++) {		// followed by per channel data.
    m_channelStatus[i] = *p++;
    m_voltages[i]      = *p++;
    m_currents[i]      = *p++;
    m_temperatures[i]  = *p++;
  }
  return p;
}  
/**
 * This function is called when a client wants our most
 * recent version of monitored data.
 * We are not allowed todo any VME actions as we are
 * explicitly not synchronized with the 
 * data taking.  We will instead return the 
 * most recent copy of the monitored data.
 * @return string
 * @retval A properly formatted tcl list that consists
 *         of the following elements:
 *         - Most recent value of the global status
 *           register.
 *         - 6 element list of most recent values of
 *           the channel status registers.
 *         - 6 element list of the most recent values of the
 *           actual voltages.
 *         - 6 element list of the most recent values of
 *           the actual channel currents.
 *         - 6 element list of the most recent values of
 *           the channel temperatures.
 */
string
CV6533::getMonitoredData()
{
  CTCLInterpreter interp;	// Captive interpeter..fmt.
  CTCLObject      objResult; 	// Top level list result.
  CTCLObject      statusList;   // List for status vals.

  objResult.Bind(interp);

  // Global status element:

  objResult += m_globalStatus;

  // Channel statuses:

  statusList.Bind(interp);
  for(int i = 0; i < 6; i++) {
    statusList += m_channelStatus[i];
  }
  objResult += statusList;

  //  Channel voltages:

  objResult += scaledIToString(m_voltages, 0.1);

  // Channel Currents:

  objResult += scaledIToString(m_currents, 0.05);
  
  // Temperatures:

  objResult += scaledIToString(m_temperatures, 1.0);

  // Return as string:

  string result = "OK ";
  result       += static_cast<string>(objResult);
  return result;

}

/*----------------------------- Utility functions ----------------------------- */
/**
 * Convert a string to a boolean..throws an exception if the string is not a valid bool.
 * @param value - value to convert.
 * @return bool
 * @retval true - if the string is a valid representation of true (as defined by XXUSB::CConfigurableObject).
 * @retval false - if the string is a valid representatino of false (as defined by XXUSB::CConfigurableObject).
 * @throw string if the string is not a valid bool.
 */
bool
CV6533::strToBool(string value)
{
  if (!XXUSB::CConfigurableObject::isBool("null", value, NULL)) {
    throw string(" Invalid booleanvalue");
  }
  return XXUSB::CConfigurableObject::strToBool(value);
}
/**
 * Get the base address of the module from the configuration.
 * @return uint32_t
 * @retval - Value of the -base configuration value.
 */
uint32_t
CV6533::getBase()
{
  return m_pConfiguration->getUnsignedParameter("-base");
}


/**
 * Convert a floating point value to a string.
 * @param value
 * @return string
 * @retval the string representation of the value.
 */
string
CV6533::fToString(float value)
{
  char cValue[100];
  sprintf(cValue, "%f", value);
  return string(cValue);
}

/**
 * Convert an array of 6 uint16_t values to a properly formatted list of 
 * floats after applying a scale factor.
 * @param values      - Pointer to the array of uint16_t's to convert.
 * @param scaleFactor - Foating point scale factor used to convert the values to floats.
 * @return string
 * @retval Properly formatted Tcl list of scaled floats. 
 */
string
CV6533::scaledIToString(uint16_t* values, float scaleFactor)
{
  float           scaledValue;
  CTCLInterpreter interp;
  CTCLObject      objResult;
  string          stringResult;
  
  objResult.Bind(interp);
  for (int i=0; i < 6; i ++) {
    scaledValue = values[i];
    scaledValue *= scaleFactor;
    objResult += scaledValue;
  }
  stringResult = static_cast<string>(objResult);
  return stringResult;

}


/*--------------- Setting the device registers -------------------------------*/


/**
 * Add a command to turn a specified channel off to a VME list.
 * @param list    - CVMUSBReadoutList to which to add the command.
 * @param channel - Channel to which to add the command (not range checked).
 */
void
CV6533::turnOff(CVMUSBReadoutList& list, unsigned int channel)
{
  list.addWrite16(getBase() + Channels[channel] + PW , amod, (uint16_t)0);
}
/**
 * Add a request to set the requested voltage for a channel to a VME operation list.
 * @param list     - CVMUSBReadoutList& to which the request is added.
 * @param channel  - Channel number whose request voltage will be modified.
 * @param value    - Floating point voltage to request.
 *
 * @note Channel and value are not range checked.
 */
void
CV6533::setRequestVoltage(CVMUSBReadoutList& list, unsigned int channel, float value)
{
  uint16_t vrequest = (uint16_t)(value * 10.0); // Request resolution is .1 V  see manual 3.2.2.1

  list.addWrite16(getBase() + Channels[channel] + Vset, amod, vrequest);
}
/**
 * Add a request to set the requested current max for a specified channel to a VME operation list.
 * @param list    - The CVMUSBReadoutList& to add to.
 * @param channel - Number of the channel to modify.
 * @param value   - The micro-amp current limit.
 * 
 * @note neither value nor channel are range checked.
 */
void
CV6533::setRequestCurrent(CVMUSBReadoutList& list, unsigned int channel, float value)
{
  uint16_t irequest = (uint16_t)(value/0.05); // Register value.
  list.addWrite16(getBase() + Channels[channel] + Iset, amod, irequest);
} 
/**
 * Add a request to turn a channel on or off to a readout list.
 * @param list     - CVMUSBReadoutList& to which the request will be added.
 * @param channel  - Number of the channel to set.
 * @param on       - true to turn channel on or false to turn it off.
 *
 * @note The channel number is not range checked.
 */
void
CV6533::setChannelOnOff(CVMUSBReadoutList& list, unsigned int channel, bool value)
{
  list.addWrite16(getBase() + Channels[channel] + PW, amod, 
		  (uint16_t)(value ? 1 : 0)); // Not going to assume anything about true/false representation.
}
/**
 * Set the trip time.  Presumably this is the amount of time
 * an out of condition state is allowed to persist
 * before the channel trips off.
 * @param list   - VMUSB list to which the request will
 *                 be added.
 * @param channel - Which channel should be affected.
 * @param time    - Time in seconds.
 *
 */
void 
CV6533::setTripTime(CVMUSBReadoutList& list, unsigned int channel, float time)
{
  uint16_t timeval = (uint16_t)(time/0.1); // register value.
  list.addWrite16(getBase() + Channels[channel] + TripTime,
		  amod, timeval);
}
/**
 * Set the maximum allowed voltage (software voltage limit)
 * for the channel.  If a voltage setting is made above
 * this limit, the setting is limited to this level.
 * @param list    - VMUSB list to which the request is added
 * @param channel - Channel that should be affected.
 * @param voltage - Voltage limit (in volts).
 */
void
CV6533::setMaxVoltage(CVMUSBReadoutList& list,
		       unsigned int channel, float voltage)
{
  uint16_t voltval = (uint16_t)(voltage/.1);
  list.addWrite16(getBase() + Channels[channel] + SVMax,
		  amod, voltval);
}
/**
 * Set the rate at which a channel ramps down.
 * @param list    - VMUSB list to which to add the request.
 * @param channel - Channel to modify.
 * @param rate    - Rate in Volts/sec
 */
void
CV6533::setRampDownRate(CVMUSBReadoutList& list,
			unsigned int channel, float rate)
{
  uint16_t rateVal = (uint16_t)rate;
  list.addWrite16(getBase() + Channels[channel] + RampDown,
		  amod, rateVal);
}
/**
 * set the rate at which a channel ramps up in voltage.
 * @param list    - VMUSB list to which to add the request.
 * @param channel - Channel number affected.
 * @param rate    - Ramp rate in Volts/sec.
 */
void
CV6533::setRampUpRate(CVMUSBReadoutList& list,
			unsigned int channel,
			float rate)
{
  uint16_t rateVal = (uint16_t)rate;
  list.addWrite16(getBase() + Channels[channel] + RampUp, amod,
		  rateVal);
}
/**
 * Set the power down mode.
 * @param list   - VMUSB list to which the request is added.
 * @param channel- Number of the channel to modify.
 * @param mode   - Mode to use  This can be one of two values:
 *                 - kill : power off in a hurry.
 *                 - ramp : Ramp down at the ramp down rate.
 */
void
CV6533::setPowerDownMode(CVMUSBReadoutList& list,
			 unsigned int channel, string mode)
{
  uint16_t modeVal;
  if (mode == "kill") {
    modeVal = 0;
  }
  else if (mode == "ramp")  {
    modeVal = 1;
  }
  else {
    throw string("Illegal mode value");
  }
  list.addWrite16(getBase() + Channels[channel] + PwDown,
		  amod, modeVal);

}
/*--------------------------- Getting device register values ----------------------*/

/**
 * Get the value of the global Voltage max regsiter.  This register defines
 * the maximum voltage capability of the module.
 * @param vme  - Object that represents the VME controller.
 * @return string
 * @retval Stringified maximum voltage in volts.
 */
string
CV6533::getGlobalMaxV(CVMUSB& vme)
{
  int status;

  uint16_t value;
  status = vme.vmeRead16(getBase() + BoardVmax,
			 amod, &value);
  if (status != 0) {
    throw string("VME Read of Global max voltage failed");
  }
  return fToString((float)value);
}
/**
 * Return the global maximum current in microamps.
 * @param vme  - Object that represents the VME controller.
 * @return string
 * @retval Stringified maximum current in micro-Amps.
 */
string
CV6533::getGlobalMaxI(CVMUSB& vme)
{
  int status;
  uint16_t value;
  status = vme.vmeRead16(getBase() + BoardImax,
			 amod, &value);
  if (status != 0) {
    throw string("VME Read of Global max current failed");
  }
  return fToString((float)value);
}
/**
 * Retrieve the channel voltage request values. 
 * as well.
 * @param vme   - VM-USB controller object.
 * @return string
 * @retval 6-element Tcl list containing the actual voltages.
 */
string
CV6533::getChannelVoltages(CVMUSB& vme)
{
  uint16_t requests[6];
  int      status;
  size_t   nread;

  CVMUSBReadoutList list;

  for (int i=0; i < 6; i++) {
    list.addRead16(getBase() + Channels[i] + Vset,
		   amod);
  }
  status = vme.executeList(list, requests, sizeof(requests), &nread);
  if (status < 0) {
    throw string("Could not read voltage requests");
  }
  return scaledIToString(requests, 0.1);
}
/**
 *  Retrieve the channel requested currents.
 * @param vme - The VME controller object.
 * @return string
 * @retval A properly formatted Tcl list of the currents in uA.
 */
string
CV6533::getChannelCurrents(CVMUSB& vme)
{
  uint16_t requests[6];
  int     status;
  size_t  nread;

  CVMUSBReadoutList list;
  for (int i=0; i < 6; i++) {
    list.addRead16(getBase() + Channels[i] + Iset, 
		   amod);
  }
  status = vme.executeList(list, requests, sizeof(requests), &nread);
  if (status < 0) {
    throw string("Could not read current requests");
  }

  return scaledIToString(requests, 0.05);
}
/**
 * Get the on off request values.  This is a set of values that determines if a
 * channel is requested to be on or off.  
 * @param vme   - The VME controller object.
 * @return string
 * @retval a properly formatted TCl list of values for each channel. 0 means off is requested
 *         1 means on is requested.
 */
string
CV6533::getOnOffRequests(CVMUSB& vme)
{
  uint16_t requests[6];
  size_t   nRead;
  int      status;
  CVMUSBReadoutList list;

  for(int i=0; i < 6; i++) {
    list.addRead16(getBase() + Channels[i]  + PW,
		   amod);
  }
  status = vme.executeList(list, requests, sizeof(requests), &nRead);

  if (status < 0) {
    throw string("Unable to read channel power on/off requests");
  }
  return scaledIToString(requests, 1.0);
}
/**
 * Get the actual voltages asserted on the channel by the module.
 * This will update the m_channelStatus values as well.
 * @param vme - VMUSB controller object.
 * @return string
 * @retval Tcl formatted list of output voltages in floating point V.
 */
string
CV6533::getActualVoltages(CVMUSB& vme)
{
  size_t nRead;
  int    status;
  CVMUSBReadoutList list;

  for (int i=0; i < 6; i++) {
    list.addRead16(getBase() + Channels[i] + VMon,
		   amod);
  }
  status = vme.executeList(list, m_voltages, sizeof(m_voltages), &nRead);
  if (status < 0) {
    throw string("Unable to read channel output voltages monitors");
  }
  return scaledIToString(m_voltages, 0.1);
}
/**
 * Get the current sourced by each channel.  These are actuals not limits.
 * Note that m_currents will be updated as well by this.
 * @param vme   - VMUSB controller object.
 * @return string
 * @retval Tcl formatted list of currents in uAmp.
 */
string
CV6533::getActualCurrents(CVMUSB& vme)
{
  size_t nRead;
  int    status;
  CVMUSBReadoutList list;

  for (int i =0; i < 6; i++) {
    list.addRead16(getBase() + Channels[i] + IMon, amod);
  }
  status = vme.executeList(list, m_currents, sizeof(m_currents), &nRead);
  if (status < 0) {
    throw string("Unable to read the channel output currents");
  }
  return scaledIToString(m_currents, 0.1);
}
/**
 * Get the current values of the channel status registers.
 * This will update the values of m_chanelStatus.
 * @param vme - The CVMUSB controller object.
 * @return string
 * @retval a tcl formatted list of integer encoded strings of the register values.
 *         see the manual 3.2.2.6 for a bit by bit breakdown of the register values.
 */
string
CV6533::getChannelStatuses(CVMUSB& vme)
{
  size_t nRead;
  int    status;
  CVMUSBReadoutList list;

  for (int i =0; i < 6; i++) {
    list.addRead16(getBase() + Channels[i] + ChStatus, amod);
  }
  status = vme.executeList(list, m_channelStatus, sizeof(m_channelStatus), &nRead);
  if (status < 0) {
    throw string("Unable to read the channel status registers");
  }
  // Use a captive interpreter to do the list formatting:

  CTCLInterpreter interp;
  CTCLObject      resultList;
  resultList.Bind(interp);

  for (int i=0; i < 6; i++) {
    resultList += m_channelStatus[i];
  }
  string result = (string)(resultList);
  return result;

  
}
/**
 * Get the trip times for all of the channels. This is the amount of time
 * in seconds a limit condition can be violated on a channel before the channel is
 * turned off.
 * @param vme   - VM-USB controller object.
 * @return string
 * @retval  A properly formatted Tcl lists whose elements are the trip times 
 *          in secondes.
 */
string
CV6533::getTripTimes(CVMUSB& vme)
{
  size_t            nRead;
  int               status;
  CVMUSBReadoutList list;
  uint16_t          tripTimes[6];

  for (int i =0; i < 6; i++) {
    list.addRead16(getBase() + Channels[i] + TripTime,
		   amod);
  }
  status = vme.executeList(list, tripTimes, sizeof(tripTimes), &nRead);
  if (status < 0) {
    throw string("Unable to read trip times");
  }
  return scaledIToString(tripTimes, 0.1);
}
/**
 * Get the software VMax values from all the channels.
 * @param vme   - VM-USB controller object.  
 * @return string
 * @retval Properly formatted Tcl list that contains the 
 *         software voltage limits for each channel.
 *         Note that these values limit the requested voltage not the 
 *         actual voltage.  Any VSET > SVMAX is turned into an SVMAX setting.
 */
string
CV6533::getSoftwareVmax(CVMUSB& vme)
{
  size_t            nRead;
  int               status;
  CVMUSBReadoutList list;
  uint16_t           sVmax[6];
  
  for (int i=0; i < 6; i++) {
    list.addRead16(getBase() +  Channels[i] + SVMax, amod);
  }
  status = vme.executeList(list, sVmax, sizeof(sVmax), &nRead);
  if (status < 0) {
    throw string("Unable to read software VMAX registeres in list");
  }
  return scaledIToString(sVmax, 0.1);
}
/**
 * Get the ramp down rates.
 * @param vme  - VM-USB controller object.
 * @return string
 * @retval Tcl list that contains the ramp down rates in V/seconds for each channel.
 */
string
CV6533::getRampDownRates(CVMUSB& vme)
{
  size_t            nRead;
  int               status;
  CVMUSBReadoutList list;
  uint16_t          rdRates[6];

  for (int i=0; i < 6; i++) {
    list.addRead16(getBase() + Channels[i] + RampDown, amod);
  }
  status = vme.executeList(list, rdRates, sizeof(rdRates), &nRead);
  if (status <  0) {
    throw string("Unable to read ramp down rates via list");
  } 
  return scaledIToString(rdRates,1.0);
}
/**
 * Get the ramp up rates.
 * @param vme   - VM-USB controller object.
 * @return string
 * @retval Tcl list that contains the ramp up rates in V/seconds 
 *         for each channel.
 */
string
CV6533::getRampUpRates(CVMUSB& vme)
{
  size_t            nRead;
  int               status;
  CVMUSBReadoutList list;
  uint16_t          rupRates[6];

  for (int i= 0; i < 6; i++) {
    list.addRead16(getBase() + Channels[i] + RampUp, amod);
  }
  status = vme.executeList(list, rupRates, sizeof(rupRates), &nRead);
  if (status < 0) {
    throw string("Unable to read ramp up rates from list");
  }
  return scaledIToString(rupRates, 1.0);
}
/**
 **  Get the power down modes
 ** @param vme  -  VM-USB controller object.
 ** @return string
 ** @retval properly formatted list of rampdown modes for each channel.
 **         the value for each channel is  either "ranmp" or "kill".
 */
string
CV6533::getPowerdownModes(CVMUSB& vme)
{
  size_t            nRead;
  int               status;
  CVMUSBReadoutList list;
  uint16_t          modes[6];
  CTCLInterpreter   interp;
  CTCLObject        objResult;
  string            result;

  objResult.Bind(interp);         // Needed for list handlnig.

  for (int i=0; i < 6; i++) {
    list.addRead16(getBase() + Channels[i] + PwDown, amod);
  }
  status = vme.executeList(list, modes, sizeof(modes), &nRead);
  if (status < 0) {
    throw string("Unable to read power down mode registers from list");
  }
  for(int i =0; i < 6; i++) {
    objResult += string(modes[i] ? "ramp" : "kill");
  }
  result = static_cast<string>(objResult);

  return result;
}
/**
 * Get the temperatures for each channel.  This will also update the
 * m_tempuratures member so that if there are requests for monitored value
 * between the time we are called and the next monitor list execution,
 * the updated values will be used.
 * @param vme  - The VM-USB controller object.
 * @return string
 * @retval Properly formatted Tcl list of temperatures for each channel.
 */
string
CV6533::getTemperatures(CVMUSB& vme)
{
  size_t             nRead;
  int                status;
  CVMUSBReadoutList  list;
  
  for (int i =0; i < 6; i++) {
    list.addRead16(getBase() + Channels[i] + Temp, amod);
  }
  status = vme.executeList(list, &m_temperatures, sizeof(m_temperatures), &nRead);
  if (status < 0) {
    throw string("Unable to read temperatures in getTemperatures list");
  }
  return scaledIToString(m_temperatures, 1.0);
}

/**
 * Get the channel polarities.
 * @param vme
 * @return string
 * @retval Tcl list of strings "+" for positive "-" for negative.
 */
string
CV6533::getPolarities(CVMUSB& vme)
{
  size_t             nRead;
  int                status;
  CVMUSBReadoutList  list;
  uint16_t           polarityValues[6];
  CTCLInterpreter    interp;	// captive interpreter for formattting.
  CTCLObject         objResult;
  string             stringResult;

  for (int i=0; i < 16; i++) {
    list.addRead16(getBase() + Channels[i] + Polarity, amod);
  }
  status = vme.executeList(list, polarityValues, sizeof(polarityValues), & nRead);
  if (status < 0) {
    throw string("Unable to read the channel polarity values via list");
  }

  objResult.Bind(interp);
  for (int i =0; i < 6; i++) {
    objResult += polarityValues[6] ? string("+") : string("-");
  }
  stringResult = static_cast<string>(objResult);
  return stringResult;
}
