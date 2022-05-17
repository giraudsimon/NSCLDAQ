/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Genie Jhang
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include "CMDPP32QDC.h"
#include "CReadoutModule.h"
#include <unistd.h>
#include <CVMUSB.h>

using std::vector;
using std::string;

/////////////////////////////////////////////////////////////////////////////////
// Data that drives parameter validity checks.

static XXUSB::CConfigurableObject::limit Zero(0);    // Useful for many of the integer limits.

static const char* ADCResolutionStrings[] = {"64k", "32k", "16k", "8k", "4k"};
static const int   ADCResolutionValues[] = {0, 1, 2, 3, 4};

static const char* GainCorrectionStrings[] = {"div4", "mult4", "none"};

static const char* IrqSourceStrings[] = {"event", "data"};
static const int   IrqSourceValues[]  = {0, 1};

CMDPP32QDC::EnumMap m_adcResolutionMap(CMDPP32QDC::adcResolutionMap());
CMDPP32QDC::EnumMap m_gainCorrectionMap(CMDPP32QDC::gainCorrectionMap());

//////////////////////////////////////////////////////////////////////////////////////////////
// Constructors and other 'canonical' methods

/**
 * Construct an instance of the device.  Note that in this framework this will
 * typically only be used to make a 'template' instance which will be cloned to
 * create instances that are bound to configurations and actual hardware.
 */
CMDPP32QDC::CMDPP32QDC() 
{
  m_pConfiguration = 0;
}

/**
 * Copy construction.  This cannot be virtual by the rules of C++ the clone()
 * method normally creates a new object from an existing template object.
 * 
 * @param rhs  - Template device that is being copied to create the  new device.
 */
CMDPP32QDC::CMDPP32QDC(const CMDPP32QDC& rhs)
{
  m_pConfiguration = 0;
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
}
/**
 * Destruction.  If your object creatd any dynamic data it must be freed here:
 */
CMDPP32QDC::~CMDPP32QDC() 
{
}
///////////////////////////////////////////////////////////////////////////////////////
// Interfaces the driver provides to the framework.

/**
 * This function is called when an instance of the driver has been associated with
 * its configuration database.  The template code stores that in m_pConfiguration
 * The configuration is a CReadoutModule which in turn is derived from
 * XXUSB::CConfigurableObject which encapsulates the configuration database.
 *
 *  You need to invoke methods from XXUSB::CConfigurableObject to create configuration parameters.
 *  by convention a configuration parameter starts with a -.  To illustrate this,
 *  template code will create a -base parameter that captures the base address of the module.
 *  In addition we'll create an -id parameter which will be the value of a marker that will
 *  be put in the event.  The marker value will be constrainted to be 16 bits wide.
 *
 * @parm configuration - Reference to the configuration object for this instance of the driver.
 */
void
CMDPP32QDC::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration; 

  m_pConfiguration -> addParameter("-base", XXUSB::CConfigurableObject::isInteger, NULL, "0");
  m_pConfiguration -> addIntegerParameter("-id", 0, 255, 0);
  m_pConfiguration -> addIntegerParameter("-ipl", 0, 7, 0);
  m_pConfiguration -> addIntegerParameter("-vector", 0, 255, 0);

  m_pConfiguration -> addIntegerParameter("-irqdatathreshold", 0, 32256, 1);
  m_pConfiguration -> addIntegerParameter("-maxtransfer", 0, 32256, 1);
  m_pConfiguration -> addEnumParameter("-irqsource", IrqSourceStrings, IrqSourceStrings[1]);
  m_pConfiguration -> addIntegerParameter("-irqeventthreshold", 0, 32256, 1);

  m_pConfiguration -> addIntegerParameter("-datalenformat", 0, 4, 2);
  m_pConfiguration -> addIntegerParameter("-multievent", 0, 15, 0);
  m_pConfiguration -> addIntegerParameter("-markingtype", 0, 3, 0);

  m_pConfiguration -> addIntegerParameter("-outputformat", 0, 3, 3);
  m_pConfiguration -> addEnumParameter("-adcresolution", ADCResolutionStrings, ADCResolutionStrings[4]);

  m_pConfiguration -> addIntListParameter("-signalwidth", 8, 16);
  m_pConfiguration -> addIntListParameter("-inputamplitude", 8, 1024);
  m_pConfiguration -> addIntListParameter("-jumperrange", 8, 0);
  m_pConfiguration -> addBoolListParameter("-qdcjumper", 8, false);
  m_pConfiguration -> addIntListParameter("-intlong", 2, 506, 8, 8, 8, 16);
  m_pConfiguration -> addIntListParameter("-intshort", 1, 127, 8, 8, 8, 2);
  m_pConfiguration -> addIntListParameter("-threshold", 1, 0xffff, 32, 32, 32, 0xff);
  m_pConfiguration -> addStringListParameter("-gaincorrectionlong", 8, GainCorrectionStrings[2]);
  m_pConfiguration -> addStringListParameter("-gaincorrectionshort", 8, GainCorrectionStrings[2]);
}
/**
 * This method is called when a driver instance is being asked to initialize the hardware
 * associated with it. Usually this involves querying the configuration of the device
 * and using VMUSB controller functions and possibily building and executing
 * CVMUSBReadoutList objects to initialize the device to the configuration requested.
 * 
 * @param controller - Refers to a CCUSB controller object connected to the CAMAC crate
 *                     being managed by this framework.
 *
 */
void
CMDPP32QDC::Initialize(CVMUSB& controller)
{
  uint32_t base = m_pConfiguration -> getUnsignedParameter("-base");
  controller.vmeWrite16(base + Reset, initamod, static_cast<uint16_t>(0));
  sleep(1);
  controller.vmeWrite16(base + StartAcq, initamod, static_cast<uint16_t>(0));
  controller.vmeWrite16(base + ReadoutReset, initamod, static_cast<uint16_t>(0));

  CVMUSBReadoutList list;	// Initialization instructions will be added to this.

  // First disable the interrupts so that we can't get any spurious ones during init.
  list.addWrite16(base + Ipl, initamod, 0);
  list.addDelay(MDPPDELAY);

  // Now retrieve the configuration parameters:
  uint16_t       id                  = m_pConfiguration -> getIntegerParameter("-id");
  uint16_t       ipl                 = m_pConfiguration -> getIntegerParameter("-ipl");
  uint16_t       ivector             = m_pConfiguration -> getIntegerParameter("-vector");

  uint16_t       irqdatathreshold    = m_pConfiguration -> getIntegerParameter("-irqdatathreshold");
  uint16_t       maxtransfer         = m_pConfiguration -> getIntegerParameter("-maxtransfer");
  uint16_t       irqsource           = IrqSourceValues[m_pConfiguration -> getEnumParameter("-irqsource", IrqSourceStrings)];
  uint16_t       irqeventthreshold   = m_pConfiguration -> getIntegerParameter("-irqeventthreshold");

  uint16_t       datalenformat       = m_pConfiguration -> getIntegerParameter("-datalenformat");
  uint16_t       multievent          = m_pConfiguration -> getIntegerParameter("-multievent");
  uint16_t       markingtype         = m_pConfiguration -> getIntegerParameter("-markingtype");
  uint16_t       outputformat        = m_pConfiguration -> getIntegerParameter("-outputformat");
	int            adcresolution       = ADCResolutionValues[m_pConfiguration -> getEnumParameter("-adcresolution", ADCResolutionStrings)];
  vector<int>    signalwidths        = m_pConfiguration -> getIntegerList("-signalwidth");
  vector<int>    inputamplitude      = m_pConfiguration -> getIntegerList("-inputamplitude");
  vector<int>    jumperrange         = m_pConfiguration -> getIntegerList("-jumperrange");
  vector<int>    qdcjumper           = m_pConfiguration -> getIntegerList("-qdcjumper");
  vector<int>    intlong             = m_pConfiguration -> getIntegerList("-intlong");
  vector<int>    intshort            = m_pConfiguration -> getIntegerList("-intshort");
  vector<int>    thresholds          = m_pConfiguration -> getIntegerList("-threshold");
  vector<string> gaincorrectionlong  = m_pConfiguration -> getList("-gaincorrectionlong");
  vector<string> gaincorrectionshort = m_pConfiguration -> getList("-gaincorrectionshort");

  list.addWrite16(base + ModuleId,          initamod, id); // Module id.

  list.addWrite16(base + DataFormat,        initamod, datalenformat);
  list.addWrite16(base + MultiEvent,        initamod, multievent);
  list.addWrite16(base + MarkType,          initamod, markingtype); 

  list.addWrite16(base + OutputFormat,      initamod, outputformat);
  list.addWrite16(base + ADCResolution,     initamod, adcresolution);

  for (uint16_t channelPair = 0; channelPair < 8; channelPair++) {
    list.addWrite16(base + ChannelSelection,    initamod, channelPair);
    list.addWrite16(base + SignalWidth,         initamod, (uint16_t)signalwidths.at(channelPair));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + InputAmplitude,      initamod, (uint16_t)inputamplitude.at(channelPair));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + JumperRange,         initamod, (uint16_t)jumperrange.at(channelPair));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + QDCJumper,           initamod, (uint16_t)qdcjumper.at(channelPair));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + IntegrationLong,     initamod, (uint16_t)intlong.at(channelPair));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + IntegrationShort,    initamod, (uint16_t)intshort.at(channelPair));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + Threshold0,          initamod, (uint16_t)thresholds.at(channelPair*4));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + Threshold1,          initamod, (uint16_t)thresholds.at(channelPair*4 + 1));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + Threshold2,          initamod, (uint16_t)thresholds.at(channelPair*4 + 2));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + Threshold3,          initamod, (uint16_t)thresholds.at(channelPair*4 + 3));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + LongGainCorrection,  initamod, (uint16_t)gainCorrectionMap()[gaincorrectionlong.at(channelPair)]);
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + ShortGainCorrection, initamod, (uint16_t)gainCorrectionMap()[gaincorrectionshort.at(channelPair)]);
    list.addDelay(MDPPCHCONFIGDELAY);
  }

  // Finally clear the converter and set the IPL which enables interrupts if
  // the IPL is non-zero, and does no harm if it is zero.
  list.addWrite16(base + Vector,            initamod, ivector);
  list.addWrite16(base + Ipl,               initamod, ipl);
  list.addWrite16(base + IrqDataThreshold,  initamod, irqdatathreshold);
  list.addWrite16(base + MaxTransfer,       initamod, maxtransfer);
  list.addWrite16(base + IrqSource,         initamod, irqsource);
  list.addWrite16(base + IrqEventThreshold, initamod, irqeventthreshold);

  // Now reset again and start daq:
  list.addWrite16(base + ReadoutReset,      initamod, 1);
  list.addWrite16(base + InitFifo,          initamod, 0);

  // The VMUSB does not like lists that end in a delay so:

  controller.vmeWrite16(base + StartAcq, initamod, static_cast<uint16_t>(1));
}

/**
 * This method is called to ask a driver instance to contribute to the readout list (stack)
 * in which the module has been placed.  Normally you'll need to get some of the configuration
 * parameters and use them to add elements to the readout list using CCUSBReadoutList methods.
 *
 * @param list - A CCUSBReadoutList reference to the list that will be loaded into the
 *               CCUSB.
 */
void
CMDPP32QDC::addReadoutList(CVMUSBReadoutList& list)
{
  // Functions are directed at the slot the module is in so:

  uint32_t base  = m_pConfiguration -> getUnsignedParameter("-base"); // Get the value of -slot.
  list.addFifoRead32(base + eventBuffer, readamod, (size_t)1024); 
  list.addWrite16(base + ReadoutReset, initamod, (uint16_t)1);

  /*
  int      id    = m_pConfiguration->getIntegerParameter("-id");
  list.addMarker(id);
  */
}


/** \brief On end procedures
 *
 *  This method is called after the VMUSB has been taken out of acquisition mode. It is a hook
 *  for the user to disable their device during times when not acquiring data. The default 
 *  implementation of this in the base class is a no-op.
 *
 *  @param controller - a vmusb controller
 */
void
CMDPP32QDC::onEndRun(CVMUSB& controller)
{
  /* MODIFY ME HERE */

  /* END MODIFICATIONS */
}

/**
 * This method virtualizes copy construction by providing a virtual method that
 * invokes it.  Usually you don't have to modify this code.
 *
 * @return CMDPP32QDC*
 * @retval Pointer to a dynamically allocated driver instance created by copy construction
 *         from *this
 */
CReadoutHardware*
CMDPP32QDC::clone() const
{
  return new CMDPP32QDC(*this);
}

/*
  Creates a map from the value of -gaincorrectionlong and -gaincorrectionshort
  to the values that can be programmed into the system.
*/
CMDPP32QDC::EnumMap
CMDPP32QDC::gainCorrectionMap()
{
  EnumMap result;
  
  result["div4"]  = 0x0100;
  result["mult4"] = 0x1000; 
  result["none"]  = 0x0400;

  return result;
}

/*
  Creates a map from the value of -adcresolution to the values that can be programmed
  into the system.
*/
CMDPP32QDC::EnumMap
CMDPP32QDC::adcResolutionMap()
{
  EnumMap result;

  result["64k"] = 0;
  result["32k"] = 1;
  result["16k"] = 2;
  result["8k"]  = 3;
  result["4k"]  = 4;

  return result;
}

/**
 * setChainAddresses
 *    Called by the chain to insert this module into a CBLT readout with common
 *    CBLT base address and MCST address.
 *
 *   @param controller - The controller object.
 *   @param position   - indicator of the position of the module in chain (begining, middle, end)
 *   @param cbltBase   - Base address for CBLT transfers.
 *   @param mcstBase   - Base address for multicast writes.
 */
void
CMDPP32QDC::setChainAddresses(CVMUSB& controller, CMesytecBase::ChainPosition position,
                              uint32_t cbltBase, uint32_t mcastBase)
{                                                                 
  uint32_t base = m_pConfiguration -> getIntegerParameter("-base");

  std::cerr << "Position: " << position << std::endl;
  std::cerr << std::hex << base << std::dec << std::endl;
  // Compute the value of the control register..though we will first program
  // the addresses then the control register:

  uint16_t controlRegister = MCSTENB | CBLTENB; // This much is invariant.
  switch (position) {
  case first:
    controlRegister |= FIRSTENB | LASTDIS;
    std::cerr << "First\n";
    break;
  case middle:
    controlRegister |= FIRSTDIS | LASTDIS;
    std::cerr << "Middle\n";
    break;
  case last:
    controlRegister |= FIRSTDIS | LASTENB;
    std::cerr << "Last\n";
    break;
  }
  std::cerr << "Setting mdpp32-qdc chain address with " << std::hex << controlRegister << std::dec << std::endl;

  // program the registers, note that the address registers take only the top 8 bits.

  controller.vmeWrite16(base + CbltAddress, initamod, (uint16_t)(cbltBase >> 24));
  controller.vmeWrite16(base + McstAddress, initamod, (uint16_t)(mcastBase >> 24));
  controller.vmeWrite16(base + CbltMcstControl, initamod, (uint16_t)(controlRegister));    
}

/**
 *  initCBLTReadout
 *
 *  Initialize the readout for CBLT transfer (called by chain).
 *    @param controller - the controller object.
 *    @param cbltAddress - the chain block/broadcast address.
 *    @param wordsPerModule - Maximum number of words that can be read by this mod
 */
void
CMDPP32QDC::initCBLTReadout(CVMUSB& controller,
                            uint32_t cbltAddress,
                            int wordsPermodule)
{
  // We need our timing source
  // IRQThreshold
  // VECTOR
  // IPL
  // Timestamp on/off

  // Assumptions:  Internal oscillator reset if using timestamp
  //               ..else no reset.
  //               most modulep arameters are already set up.


  int irqThreshold   = m_pConfiguration -> getIntegerParameter("-irqthreshold");
  int vector         = m_pConfiguration -> getIntegerParameter("-vector");
  int ipl            = m_pConfiguration -> getIntegerParameter("-ipl");
  std::string markType = m_pConfiguration -> cget("-marktype");
  bool timestamping  = (markType == "timestamp") || (markType == "extended-timestamp");
  
  // Stop acquistiion
  // ..and clear buffer memory:
  controller.vmeWrite16(cbltAddress + StartAcq, initamod, (uint16_t)0);
  controller.vmeWrite16(cbltAddress + InitFifo, initamod, (uint16_t)0);

  // Set stamping


  // Note the generic configuration already set the correct marktype.

  if(timestamping) {
    // Oscillator sources are assumed to already be set.
    // Reset the timer:

    //    controller.vmeWrite16(cbltAddress + MarkType,       initamod, (uint16_t)1); // Show timestamp, not event count.
    controller.vmeWrite16(cbltAddress + TimestampReset, initamod, (uint16_t)3); // reset all counter.
  }
  else {
    // controller.vmeWrite16(cbltAddress + MarkType,       initamod, (uint16_t)0); // Use Eventcounter.
    controller.vmeWrite16(cbltAddress + EventCounterReset, initamod, (uint16_t)0); // Reset al event counters.
  }
  // Set multievent mode
  
  //  controller.vmeWrite16(cbltAddress + MultiEvent, initamod, (uint16_t)3);      // Multi event mode 3.
  controller.vmeWrite16(cbltAddress + IrqDataThreshold, initamod, (uint16_t)irqThreshold);
  controller.vmeWrite16(cbltAddress + MaxTransfer, initamod,  (uint16_t)wordsPermodule);

  // Set the IRQ

  controller.vmeWrite16(cbltAddress + Vector, initamod, (uint16_t)vector);
  controller.vmeWrite16(cbltAddress + Ipl,    initamod, (uint16_t)ipl);
  controller.vmeWrite16(cbltAddress + IrqDataThreshold, initamod, (uint16_t)irqThreshold);

  // Init the buffer and start data taking.

  controller.vmeWrite16(cbltAddress + InitFifo, initamod, (uint16_t)0);
  controller.vmeWrite16(cbltAddress + ReadoutReset, initamod, (uint16_t)0);
  controller.vmeWrite16(cbltAddress + StartAcq , initamod, (uint16_t)1);
}
//////////////////////////////////////////////////////////////////////////////////////////
