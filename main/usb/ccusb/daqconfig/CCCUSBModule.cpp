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

// Implementation of the CCUSB module support module.

#include <config.h>
#include "CCCUSBModule.h"



#include "CReadoutModule.h"
#include <CCCUSB.h>
#include <CCCUSBReadoutList.h>

#include <tcl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <set>

#include <iostream>

using namespace std;

///////////////////////////////////////////////////////////////////////
// Validat5or information.
///////////////////////////////////////////////////////////////////////

// Enumerator values for gdg sources.. this is a null terminated array:

const char* GdgSourceValues[] = {
  "disabled",  "in1", "in2", "in3", "event", "eventend", "usbtrigger", "pulser",
  NULL
};

// Enumerator values for out1:

const char* Out1Values[] = {
  "busy", "event", "gdga", "gdgb",
  NULL
};
// out2:

const char* Out2Values[] = {
  "acquire", "event", "gdga", "gdgb",
  NULL
};
const char* Out3Values[] = {
  "busyend", "busy", "gdga", "gdgb",
  NULL
};


// Enumerator values for leds:

static const char* redValues[] = {
  "event", "busy", "usboutnotempty", "usbinnotfull", 0
};
static const char* greenValues[] = {
  "acquire", "nimi1", "nimi2", "usbinnotempty", "usbtrigger", 0
};
static const char* yellowValues[] = {
  "nimi2", "nimi3", "busy", "usbinfifonotempty", 0
};

// Enumerator values for scaler counts:

static const char* scalerAInputs[] = {
  "disabled", "nimi1", "nimi2", "nimi3", "event", "carryb", "dgga", "dggb", 0
};
static const char* scalerBInputs[] = {
  "disabled", "nimi1", "nimi2", "nimi3", "event", "carrya", "dgga", "dggb", 0
};

static const char* bulkTransferModeValues[] = {
  "default","nbuffers", "timeout",0
};

static const char* bufferLengthValues[] = {
  "4096","2048","1024","512","256","128","64","singleevent",0
};
// Range of gdg delays/widths


// widths:

static XXUSB::CConfigurableObject::limit minwidth(1);
static XXUSB::CConfigurableObject::limit maxwidth(65535);
static XXUSB::CConfigurableObject::Limits WidthLimits(minwidth, maxwidth);

// gdga delay:

static XXUSB::CConfigurableObject::limit mindelay(0);
static XXUSB::CConfigurableObject::limit maxdelaya(0x7fffffff);
static XXUSB::CConfigurableObject::limit maxdelayb(0x7fffffff);
static XXUSB::CConfigurableObject::Limits DelayA(mindelay, maxdelaya);
static XXUSB::CConfigurableObject::Limits DelayB(mindelay, maxdelayb);

// Limits for number of buffers to transfer 
static XXUSB::CConfigurableObject::limit minbufs2transfer(-1);
static XXUSB::CConfigurableObject::limit maxbufs2transfer(255);
static XXUSB::CConfigurableObject::Limits BulkTransferLimits(minbufs2transfer, 
                                                      maxbufs2transfer);
// uav bulk transfer timeout limits
static XXUSB::CConfigurableObject::limit mintransfertimeout(-1);
static XXUSB::CConfigurableObject::limit maxtransfertimeout(15);
static XXUSB::CConfigurableObject::Limits BulkTransferTimeoutLimits(mintransfertimeout, 
                                                            maxtransfertimeout);

////////////////////////////////////////////////////////////////////////////////////////
// Canonicals:
///////////////////////////////////////////////////////////////////////////////////////

/*!
   Construction:
*/
CCCUSBModule::CCCUSBModule()
{
  m_pConfiguration = 0;
}
/*!
   Copy construction:
   @param rhs the module we are copying as a template for our construction.
*/
CCCUSBModule::CCCUSBModule(const CCCUSBModule& rhs)
{
  m_pConfiguration = 0;
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
}
/*!
   Assignment.. should be illegal really.
   @param rhs - the guy assigning to us.
   @return CCCUSBModule&
   @retval *this
*/
CCCUSBModule&
CCCUSBModule::operator=(const CCCUSBModule& rhs)
{
  return *this;
}

/*!
  Destruction .. leaks configurations I'm afraid otherwise there are other problems.

*/
CCCUSBModule::~CCCUSBModule()
{
}

/////////////////////////////////////////////////////////////////////
//  CReadoutHardware interface implementation
////////////////////////////////////////////////////////////////////

/*!
   Called when the module is attached to its configuration.
   At this time, the configuration parameters and their constraints
   are defined so that the configuration file can be processed.
   @param configuration - Reference to the module's configuration.
*/

void
CCCUSBModule::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration;

  // gdga/bsource:



  configuration.addEnumParameter("-gdgasource", GdgSourceValues);
  configuration.addEnumParameter("-gdgbsource", GdgSourceValues);

  // The gate generator width/delay parameters:

  configuration.addParameter("-gdgawidth",
			     XXUSB::CConfigurableObject::isInteger,
			     &WidthLimits, "1");
  configuration.addParameter("-gdgbwidth",
			     XXUSB::CConfigurableObject::isInteger,
			     &WidthLimits, "1");
  configuration.addParameter("-gdgadelay", 
			     XXUSB::CConfigurableObject::isInteger,
			     &DelayA, "0");
  configuration.addParameter("-gdgbdelay",
			     XXUSB::CConfigurableObject::isInteger,
			     &DelayB, "0");

  // LED parameters:

  configuration.addEnumParameter("-red", redValues, "event");
  configuration.addBooleanParameter("-redlatch", false);
  configuration.addBooleanParameter("-redinvert", false);

  configuration.addEnumParameter("-green", greenValues, "acquire");
  configuration.addBooleanParameter("-greenlatch", false);
  configuration.addBooleanParameter("-greeninvert", false);

  configuration.addEnumParameter("-yellow", yellowValues, "nimi2");
  configuration.addBooleanParameter("-yellowlatch", false);
  configuration.addBooleanParameter("-yellowinvert", false);

  // scaler source parameters:

  configuration.addEnumParameter("-scalera", scalerAInputs, "disabled");
  configuration.addEnumParameter("-scalerb", scalerBInputs, "disabled");

  // Outputs:


  configuration.addEnumParameter("-out1", Out1Values, "busy");
  configuration.addBooleanParameter("-out1latch", false);
  configuration.addBooleanParameter("-out1invert", false);

  configuration.addEnumParameter("-out2", Out2Values, "event");
  configuration.addBooleanParameter("-out2latch", false);
  configuration.addBooleanParameter("-out2invert", false);

  configuration.addEnumParameter("-out3", Out3Values, "busyend");
  configuration.addBooleanParameter("-out3latch", false);
  configuration.addBooleanParameter("-out3invert", false);

  // Control the readout list:

  configuration.addBooleanParameter("-readscalers", false);
  configuration.addBooleanParameter("-incremental", false);


  // Control the bulk transfer
  configuration.addEnumParameter("-bulktransfermode", 
                                  bulkTransferModeValues, "default");
  configuration.addParameter("-nbuffers2transfer", XXUSB::CConfigurableObject::isInteger,
                          &BulkTransferLimits,"-1");
  configuration.addParameter("-bulktransfertimeout", XXUSB::CConfigurableObject::isInteger,
                          &BulkTransferTimeoutLimits,"-1");

  // Control buffer size
  configuration.addEnumParameter("-bufferlength", 
                                  bufferLengthValues, "4096");

  // mixed buffer 
  configuration.addBooleanParameter("-mixedbuffer",false);
  
  // flush scaler buffer immediately after execution
  configuration.addBooleanParameter("-forcescalerdump",false);

  // allow camac bus arbitration... totally broken.
  //  i tried setting this and it make my CC-USB slow down incredibly and then fail.
  configuration.addBooleanParameter("-arbitratebus",false);
                                  
  // in case we want it later... at the moment we don't use it
  configuration.addBooleanParameter("-optionalheader",false);
                                  
  configuration.addBooleanParameter("-triggerlatch",false);

  configuration.addBooleanParameter("-printconfig",false);
}
/*!

  Called just prior to the start of the run.  This is really the point of this module.
  The CCUSB resources are set up in accordance with the current configuration values.
  @param controller - Reference to a controller object that can be used to talk to the module.
*/

void
CCCUSBModule::Initialize(CCCUSB& controller)
{
  // the action is delegated to a series of utility functions:
  uint32_t firm;
  controller.readFirmware(firm);
  cout << "CCUSB firmware=" << hex << firm << dec << endl;

  configureGdg1(controller);
  configureGdg2(controller);
  configureDevices(controller);
  configureOutput(controller);
  configureLED(controller);
  configureBulkTransfer(controller);
  configureBufferLength(controller);
  configureGlobalMode(controller);

  if (m_pConfiguration->getBoolParameter("-printconfig")) {
    controller.dumpConfiguration(cout);
  }
}
 
/*!
  The module reads the scalers if -readscaler is set and clears them if -incremental is set.
  Note that the scalers are disabled for the time the read and optional clear is performed 
  so that there is no time skew in the counts between the scalers.   This is especially 
  important if the scalers are ganged together.  A is read first then B.

   @param list - CVMUSBReadoutList to which we make no contributions:
*/
void
CCCUSBModule::addReadoutList(CCCUSBReadoutList& list)
{

  if (m_pConfiguration->getBoolParameter("-readscalers")) {

    // Read the scalers:
    list.addRead24(25, 11, 0);	// scalerA
    list.addRead24(25, 12, 0);	// scalerB

    if (m_pConfiguration->getBoolParameter("-incremental")) {
      list.addControl(25, 11, 15); // clear both scalerA and scalerB
    }

  }


}

/*!
  Used for virtual copy construction.. clone ourselves into a new object:
  @return CReadoutHardware*
  @retval pointer to a copy of ourself.
*/
CReadoutHardware*
CCCUSBModule::clone() const
{
  return new  CCCUSBModule(*this);
}

////////////////////////////////////////////////////////////////////////////
// Utilities:
///////////////////////////////////////////////////////////////////////////

/**
 * This utilitiy is used to translate an enumerated value into an
 * index into an enumerator table.
 * @param values - The table of enumerator values.  This is null terminated.
 * @param parameter - The configuration parameter value to translate.
 * @return int
 * @retval - The index corresp;onding to parameter.
 * @throw  - String exception if there's no match... This should not happen
 *           as the validator should catch that case.
 */
int
CCCUSBModule::enumIndex(const char** values, string parameter)
{
  for (int i =0; *values; i++, values++) {
    if (parameter == *values) return i;
  }
  throw string("CCCUSBModule::enumIndex - could not find a match!!!!");
}
/**
 * COnfigure the outputs.  This function processes the various -out* parameters,
 * builds up an image of the NIM Output selector and writes the register with it.
 * @param controller - CCCUSB controller object (reference).
 */
void
CCCUSBModule::configureOutput(CCCUSB& controller)
{
  uint32_t registerValue = 0;
  
  // O1
  uint32_t o1Register = m_pConfiguration->getEnumParameter("-out1", Out1Values) <<
    CCCUSB::OutputSourceRegister::nimO1Shift;
  if (getBoolParameter("-out1latch")) {
    o1Register |= CCCUSB::OutputSourceRegister::nimO1Latch;
  }
  if (getBoolParameter("-out1invert")) {
    o1Register |= CCCUSB::OutputSourceRegister::nimO1Invert;
  }
  registerValue |= o1Register;


  // O2

  uint32_t o2Register = m_pConfiguration->getEnumParameter("-out2", Out2Values)
    << CCCUSB::OutputSourceRegister::nimO2Shift;
  if (getBoolParameter("-out2latch")) {
    o2Register |= CCCUSB::OutputSourceRegister::nimO2Latch;
  }
  if (getBoolParameter("-out2invert")) {
    o2Register |= CCCUSB::OutputSourceRegister::nimO2Invert;
  }
  registerValue |= o2Register;
 
  // O3

  uint32_t o3Register = m_pConfiguration->getEnumParameter("-out3", Out3Values) 
    << CCCUSB::OutputSourceRegister::nimO3Shift;
  if (getBoolParameter("-out3latch")) {
    o3Register |= CCCUSB::OutputSourceRegister::nimO3Latch;
  }

  if (getBoolParameter("-out3invert")) {
    o3Register |= CCCUSB::OutputSourceRegister::nimO3Invert;
  }
  registerValue |= o3Register;

  // write the register.

  controller.writeOutputSelector(registerValue);
}
/**
 * Configure Gate and Delay Geneartor A.  Specifically, we configure the gate width and the delay.
 * The source is configure by configureDevices.
 * @param controller - Reference to the controller we will be configuring.
 */
void
CCCUSBModule::configureGdg1(CCCUSB& controller)
{
  uint32_t delay = getIntegerParameter("-gdgadelay") & 0xffff;
  uint32_t width = getIntegerParameter("-gdgawidth");

  uint32_t registerValue = (width << 16) | delay;
  controller.writeDGGA(registerValue);

}
/**
 * Configure the gate and delay for GDGB.. this resource has a 24 bit
 * delay that's spread across two registers.
 * @param controller - Reference to the controller that is being configured.
 */
void
CCCUSBModule::configureGdg2(CCCUSB& controller)
{
  uint32_t delay = getIntegerParameter("-gdgbdelay");
  uint32_t width = getIntegerParameter("-gdgbwidth");
  uint32_t adelay= getIntegerParameter("-gdgadelay");

  uint32_t fineValue = (width << 16) | (delay & 0xffff);
  uint32_t coarseValue    = (delay & 0xffff0000) | ((delay >> 16) & 0xffff);


  controller.writeDGGB(fineValue);
  controller.writeDGGExt(coarseValue);

}
/**
 * Configure the sourcdes for the various devices.  At this point in time
 *
 * @param controller - The CCCUSB controller object.
 */
void
CCCUSBModule::configureDevices(CCCUSB& controller)
{
  uint32_t registerValue = 0;

  // Scaler A:

  registerValue |= (m_pConfiguration->getEnumParameter("-scalera", scalerAInputs)
    << CCCUSB::DeviceSourceSelectorsRegister::scalerAShift);

  // Scaler B:

  registerValue |= (m_pConfiguration->getEnumParameter("-scalerb", scalerBInputs)
      << CCCUSB::DeviceSourceSelectorsRegister::scalerBShift);

  // DGGA:
  
  registerValue |= m_pConfiguration->getEnumParameter("-gdgasource", GdgSourceValues)
    << CCCUSB::DeviceSourceSelectorsRegister::DGGAShift;

  // DGGB:

  registerValue |= m_pConfiguration->getEnumParameter("-gdgbsource", GdgSourceValues)
    << CCCUSB::DeviceSourceSelectorsRegister::DGGBShift;

  // First disable scalers by writing zero to their enable bits and 1 to their freeze bits 
  controller.writeDeviceSourceSelectors( CCCUSB::DeviceSourceSelectorsRegister::scalerAFreeze
                                         | CCCUSB::DeviceSourceSelectorsRegister::scalerBFreeze );
  uint32_t reg;
  controller.readDeviceSourceSelectors(reg);
  controller.writeDeviceSourceSelectors(0);
  controller.readDeviceSourceSelectors(reg);

  // Write the new values to the code bits...
  controller.writeDeviceSourceSelectors(registerValue);
  m_deviceSource = registerValue;
  controller.readDeviceSourceSelectors(reg);

  // Enable and reset the scalers.
  controller.writeDeviceSourceSelectors(CCCUSB::DeviceSourceSelectorsRegister::scalerAEnable
                                         | CCCUSB::DeviceSourceSelectorsRegister::scalerAFreeze);
  controller.readDeviceSourceSelectors(reg);
  controller.writeDeviceSourceSelectors(CCCUSB::DeviceSourceSelectorsRegister::scalerBEnable
                                         | CCCUSB::DeviceSourceSelectorsRegister::scalerBFreeze);

  controller.readDeviceSourceSelectors(reg);

  controller.writeDeviceSourceSelectors( reg 
      | CCCUSB::DeviceSourceSelectorsRegister::scalerAReset
      | CCCUSB::DeviceSourceSelectorsRegister::scalerAFreeze
      | CCCUSB::DeviceSourceSelectorsRegister::scalerBReset
      | CCCUSB::DeviceSourceSelectorsRegister::scalerBFreeze
      );
  controller.readDeviceSourceSelectors(reg);

  uint16_t qx;
  // Clear scalers atomically
  controller.simpleControl(25,12,15, qx);

  controller.readDeviceSourceSelectors(reg);

}
/**
 * configureLED
 *
 * Configure the value of the LED Source register.  This deterimens which LEDS light and when.
 *
 * @param controller - reference to the CC-USB  object that represents the CAMAC controller.
 *
 */
void
CCCUSBModule::configureLED(CCCUSB& controller)
{
  uint32_t registerValue = 0;

  // red:

  registerValue |= m_pConfiguration->getEnumParameter("-red", redValues) 
    << CCCUSB::LedSourceRegister::redShift;
  if (m_pConfiguration->getBoolParameter("-redinvert")) {
    registerValue |= CCCUSB::LedSourceRegister::redInvert;
  }
  if (m_pConfiguration->getBoolParameter("-redlatch")) {
    registerValue |= CCCUSB::LedSourceRegister::redLatch;
  }
  // green:

  registerValue |= m_pConfiguration->getEnumParameter("-green", greenValues)
    << CCCUSB::LedSourceRegister::greenShift;
  if (m_pConfiguration->getBoolParameter("-greeninvert")) {
    registerValue |= CCCUSB::LedSourceRegister::greenInvert;
  }
  if (m_pConfiguration->getBoolParameter("-greenlatch")) {
    registerValue |= CCCUSB::LedSourceRegister::greenLatch;
  }
  // yellow:

  registerValue |= m_pConfiguration->getEnumParameter("-yellow", yellowValues);
  if (m_pConfiguration->getBoolParameter("-yellowinvert")) {
    registerValue |= CCCUSB::LedSourceRegister::yellowInvert;
  }
  if (m_pConfiguration->getBoolParameter("-yellowlatch")) {
    registerValue |= CCCUSB::LedSourceRegister::yellowLatch;
  }

  controller.writeLedSelector(registerValue);
  
} 

/**
 * configureBulkTransfer
 *
 * Configure the value of the USB bulk transfer register.  
 * This determines the criteria for when data is transfered from the USB
 * device to the host.
 *
 * @param controller - reference to the CC-USB  object that represents the CAMAC controller.
 *
 */
void
CCCUSBModule::configureBulkTransfer(CCCUSB& controller)
{
  uint32_t registerValue = 0;

  int mode = m_pConfiguration->getEnumParameter("-bulktransfermode",bulkTransferModeValues);

  // Deal with each mode
  if (mode==0) { // default
    // do nothing b/c the CAcquisitionThread has already set the default values

  } else if (mode==1) { // user defined number of buffers to transfer 
    int nbuffers = m_pConfiguration->getIntegerParameter("-nbuffers2transfer");
    if (nbuffers==-1) {
      std::string errmsg("-nbuffers2transfer is required by -bulktransfermode=nbuffers");
      errmsg += " and has not be set";
      throw errmsg;
    }
    registerValue |= ((nbuffers << CCCUSB::TransferSetupRegister::multiBufferCountShift) 
                        & CCCUSB::TransferSetupRegister::multiBufferCountMask);



  } else if (mode==2) { // timeout mode
    int timeout = m_pConfiguration->getIntegerParameter("-bulktransfertimeout");
    if (timeout==-1) {
      std::string errmsg("-bulktransfertimeout is required by -bulktransfermode=timeout");
      errmsg += " and has not be set";
      throw errmsg;
    }
    registerValue |= ((timeout << CCCUSB::TransferSetupRegister::timeoutShift) 
                        & CCCUSB::TransferSetupRegister::timeoutMask);

  }
  
  // Set the new transfer setup
  controller.writeUSBBulkTransferSetup(registerValue); 
  
} 



/**
 * configureBufferLength
 *
 * Configure the size of the buffers generated by the CCUSB. This can force bulk transfer of data
 * to occur more or less frequently.
 *
 * @param controller - reference to the CC-USB  object that represents the CAMAC controller.
 *
 */
void CCCUSBModule::configureBufferLength(CCCUSB& controller)
{

  uint16_t registerValue = 0;
  uint16_t newValue = 0;

  if (controller.readGlobalMode(registerValue)<0) {
    throw std::string("CCCUSBModule::configureBufferLength(CCCUSB&) Failure reading global mode");
  } 

  int mode = m_pConfiguration->getEnumParameter("-bufferlength",bufferLengthValues);

  switch(mode) {
    case 0: //4096
      newValue = (CCCUSB::GlobalModeRegister::bufferLen4K 
                        << CCCUSB::GlobalModeRegister::bufferLenShift);
      break;
    case 1: // 2048
      newValue = (CCCUSB::GlobalModeRegister::bufferLen2K 
                        << CCCUSB::GlobalModeRegister::bufferLenShift);
      break;
    case 2: // 1024 
      newValue = (CCCUSB::GlobalModeRegister::bufferLen1K 
                        << CCCUSB::GlobalModeRegister::bufferLenShift);
      break;
    case 3: // 512 
      newValue = (CCCUSB::GlobalModeRegister::bufferLen512 
                        << CCCUSB::GlobalModeRegister::bufferLenShift);
      break;
    case 4: // 256 
      newValue = (CCCUSB::GlobalModeRegister::bufferLen256 
                        << CCCUSB::GlobalModeRegister::bufferLenShift);
      break;
    case 5: // 128 
      newValue = (CCCUSB::GlobalModeRegister::bufferLen128
                        << CCCUSB::GlobalModeRegister::bufferLenShift);
      break;
    case 6: // 64
      newValue = (CCCUSB::GlobalModeRegister::bufferLen64
                        << CCCUSB::GlobalModeRegister::bufferLenShift);
      break;
    case 7: // Single event
      newValue = (CCCUSB::GlobalModeRegister::bufferLenSingle
                        << CCCUSB::GlobalModeRegister::bufferLenShift);
      break;
  }

  // Mask this new value so that we don't disturb other bits
  newValue &= CCCUSB::GlobalModeRegister::bufferLenMask;

  // Clear the buffer len bits
  registerValue &= (~CCCUSB::GlobalModeRegister::bufferLenMask);
  registerValue |= newValue;

  // Write the new bits
  controller.writeGlobalMode(registerValue);

}


/**
 * configureGlobalMode
 *
 * @param controller - reference to the CC-USB  object that represents the CAMAC controller.
 *
 */
void CCCUSBModule::configureGlobalMode(CCCUSB& controller)
{

  uint16_t registerValue = 0;
  uint16_t newValue = 0;

  if (controller.readGlobalMode(registerValue)<0) {
    throw std::string("CCCUSBModule::configureGlobalMode(CCCUSB&) Failure reading global mode");
  } 

  newValue = registerValue;

  // update the bits for each option
  newValue = setMixedBufferBits(newValue);
  newValue = setForceScalerDumpBits(newValue);
//  newValue = setArbitrateBusBits(newValue);
//  newValue = setOptionalHeaderBits(newValue);
  newValue = setTriggerLatchBits(newValue);
  
  // Write the new bits
  controller.writeGlobalMode(newValue);

}


uint16_t CCCUSBModule::setMixedBufferBits(uint16_t reg_content)
{
  if (m_pConfiguration->getBoolParameter("-mixedbuffer")) {
    // set it unconditionally
    reg_content |= (0x20);
  } else {
    // unset it 
    reg_content &= (~(0x20));
  }

  return reg_content;
}

uint16_t CCCUSBModule::setForceScalerDumpBits(uint16_t reg_content)
{
  if (m_pConfiguration->getBoolParameter("-forcescalerdump")) {
    // set it unconditionally
    reg_content |= (0x40);
  } else {
    // unset it 
    reg_content &= (~(0x40));
  }

  return reg_content;

}

uint16_t CCCUSBModule::setArbitrateBusBits(uint16_t reg_content)
{
  if (m_pConfiguration->getBoolParameter("-arbitratebus")) {
    // set it unconditionally
    reg_content |= (0x1000);
  } else {
    // unset it 
    reg_content &= (~(0x1000));
  }

  return reg_content;

}

uint16_t CCCUSBModule::setOptionalHeaderBits(uint16_t reg_content)
{
  if (m_pConfiguration->getBoolParameter("-optionalheader")) {
    // set it unconditionally
    //    reg_content |= (0x100);
    std::cerr << "Optional header bit is not supported!" << std::endl;
  } else {
    // unset it 
    //  reg_content &= (~(0x100));
  }

  // make sure it is always unset 
  reg_content &= (~(0x100));

  return reg_content;

}

uint16_t CCCUSBModule::setTriggerLatchBits(uint16_t reg_content)
{
  if (m_pConfiguration->getBoolParameter("-triggerlatch")) {
    // set it unconditionally
    reg_content |= (0x10);
  } else {
    // unset it 
    reg_content &= (~(0x10));
  }

  return reg_content;

}
