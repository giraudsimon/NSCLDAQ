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
#include <CConfiguration.h>
#include <CStack.h>
#include <CReadoutModule.h>
#include <CCCUSB.h>
#include <CCCUSBReadoutList.h>
#include <assert.h>
#include <tcl.h>
#include <Globals.h>
#include <set>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <Globals.h>


#include <iostream>
using namespace std;



// -delay is in the range 0 - 0xff  number of microseconds of delay 
//        between trigger and list start (to allow for ADC conversinos).
//
static XXUSB::CConfigurableObject::limit delayLow(0);
static XXUSB::CConfigurableObject::limit delayHigh(0x7f);
static XXUSB::CConfigurableObject::Limits delayRange(delayLow, delayHigh);

static XXUSB::CConfigurableObject::limit lamtoLow(0);
static XXUSB::CConfigurableObject::limit lamtoHigh(0xff);
static XXUSB::CConfigurableObject::Limits lamtoRange(lamtoLow, lamtoHigh);

bool CStack::m_incrementalScalers(true);

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Canonicals /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*!
   Construct a stack. The work of preparing the configuration and attaching it
   is done in onAttach.
*/
CStack::CStack() :
  m_pConfiguration(0)
{}

/*!
  Copy constrution has the same deal except that we will clone the configuration
  object if it exists in the rhs.
 \param rhs const CStack&  refrence to the source that will be copyied.
*/
CStack::CStack(const CStack& rhs) :
  m_pConfiguration(0)
{
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
 
  }
}



/*!
   Destruction is a no-op .. which, in the presence of copy construction
   can lead to memory leaks.
*/
CStack::~CStack()
{
  
}
/*!
   Assignment just returns itself...as copying the configuration can result in 
   an infinite recursion loop.
*/
CStack&
CStack::operator=(const CStack& rhs)
{
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
///////////////////// CReadoutHardware interface implementation ////////////////
////////////////////////////////////////////////////////////////////////////////

/*!
   Called when a configuration is attached to the module.
   we need to define the set of configuration parameters we can support,
   and their validators

   See the header file comments for the set of legal config parameters
   and their meanings..

  \param configuration : CReadoutModule&
      The configuration module and delegator that surrounds us.

*/
void
CStack::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration;

  // Options valid for all types of stacks:

  // set up the stack type valid options:

  static XXUSB::CConfigurableObject::isEnumParameter validTypes;
  validTypes.insert("event");
  validTypes.insert("scaler");


  m_pConfiguration->addParameter("-type", 
				 XXUSB::CConfigurableObject::isEnum, &validTypes,
				 "event");


  m_pConfiguration->addParameter("-modules",
				CStack::moduleChecker, NULL, "");


  // options valid for event stacks:

  m_pConfiguration->addParameter("-lams", 
				 XXUSB::CConfigurableObject::isInteger, NULL, "0");
 m_pConfiguration->addParameter("-delay",
				 XXUSB::CConfigurableObject::isInteger, &delayRange, "0");
 m_pConfiguration->addParameter("-lamtimeout",
				XXUSB::CConfigurableObject::isInteger, &lamtoRange, "0");

 // options only valid for scaler sacks:

  m_pConfiguration->addParameter("-period",
				 XXUSB::CConfigurableObject::isInteger, NULL, "2");

  m_pConfiguration->addBooleanParameter("-incremental", "true");


  
}
/*!
  Initializes the stack prior to data taking, not to be confused with loading the
  stack. This does one-time initialization of the stack modules. We will iterate 
  through all modules read out by the stack, initializing them.
*/
void
CStack::Initialize(CCCUSB& controller)
{
  StackElements modules = getStackElements();
  StackElements::iterator p = modules.begin();
  while(p != modules.end()) {
    CReadoutHardware* pModule = *p;                                // Wraps the hardware.

    pModule->Initialize(controller); 

    p++;
  }
  if (m_pConfiguration->cget("-type") == std::string("scaler")) {
    m_incrementalScalers = m_pConfiguration->getBoolParameter("-incremental");
  }
}

/*!
   Adds the modules in the stack to the readout list.  This can be done by
   an external 'force' setting up the stack manually, or it can/will be called
   by loadStack as it creates the list to load.
   \param list : CCCUSBReadoutList
       The USB Readout list that is being built by this object.

*/
void
CStack::addReadoutList(CCCUSBReadoutList& list)
{
  StackElements modules = getStackElements();
  StackElements::iterator p = modules.begin();
  while (p != modules.end()) {
    CReadoutHardware* pModule = *p;
    pModule->addReadoutList(list);
    
    p++;
  }
}

/*!
  Calls the end-of-run routins for the stack after data taking.  We will iterate 
  through all modules read out by the stack. 
*/
void
CStack::onEndRun(CCCUSB& controller)
{
  StackElements modules = getStackElements();
  StackElements::iterator p = modules.begin();
  while(p != modules.end()) {
    CReadoutHardware* pModule = *p;                       // Wraps the hardware.

    pModule->onEndRun(controller); 

    p++;
  }

}
/*!
  Clone virtualizes copy construction.

*/
CReadoutHardware*
CStack::clone() const
{
  return new CStack(*this);
}
////////////////////////////////////////////////////////////////////////////////
///////////////// Implementation of interfaces specific to CStack //////////////
////////////////////////////////////////////////////////////////////////////////


/*!
   Generates and loads the stack that reads out this trigger.
   This is done by creating a readout list, filling it in via addReadoutList,
   and loading it at the offset designated by m_listOffset using our list number.
   m_listOffset will be updated to reflect the number of stack lines that we have
   loaded.

   \param controller : CCCUSB&
     The CCUSB controller handle connected to the CC-USB that we will be managing.
*/
void
CStack::loadStack(CCCUSB& controller)
{
  uint8_t           listNumber;
  CCCUSBReadoutList readoutList;

  addReadoutList(readoutList);
  listNumber = getTriggerType();	// 0 for event, 1 for scaler.


  // Load the list:... unless it has no elements!

  if (readoutList.size() > 0) {
    int status = controller.loadList(listNumber,
				     readoutList);
    if (status < 0) {
      int e = errno;
      std::string msg(strerror(errno));
      std::string which;
      if (status == -1) {
	which = "usb_bulk_write";
      } else if (status == -2) {
	which = "usb_bulk_read";
      } else {
	which = "list  number decode";
      }
      msg += " during ";
      msg += which;
      throw msg;
    }
  }
}
/*!
  To do this we need to get the stack number...
  - For event stacks we need to program the lam mask register, and delay registers.
  - For scaler stacks, the period.

\param controller : CCCUSB&
    Reference to the CCUSB controller object connected to the CC-USB module.
*/
void
CStack::enableStack(CCCUSB& controller)
{
  uint8_t   listNumber  = getListNumber();

  if (listNumber == 0) {
    int lams    = getIntegerParameter("-lams");
    int timeout = getIntegerParameter("-lamtimeout");
    int delay   = getIntegerParameter("-delay");

    // forma and write the delay register:

    uint16_t delayRegister = static_cast<uint16_t>(delay | (timeout << 8));
    controller.writeDelays(delayRegister);

    // Write the lam trigger register:

    controller.writeLamTriggers(static_cast<uint32_t>(lams));
  }
  else {
    int period = getIntegerParameter("-period");
    Globals::scalerPeriod = period;
    period     = (period*2) << 16;	// position to register.
    controller.writeScalerControl(static_cast<uint32_t>(period));
  }
 
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// Helper functions //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


/*
  Get an integer parameter.  The parameter is assumed to already have been 
validated by the configurator.
Parameters:
   std::string name     - Name of the parameter ot get.
Returns
   parameter value as an integer.

*/
unsigned int
CStack::getIntegerParameter(string name)
{
  string sValue =  m_pConfiguration->cget(name);
  unsigned int    value  = strtoul(sValue.c_str(), NULL, 0);

  return value;
}
/*
   Get the trigger type.  The trigger type is assumed to alread have been validated
   by the configurator.
Returns:
   Enumerated value that reflects the trigger value configured into the system.
*/
CStack::TriggerType
CStack::getTriggerType()
{
  string sValue = m_pConfiguration->cget("-type");

  if (sValue == "event")      return Event;
  if (sValue == "scaler")    return Scaler;

  assert(0);			// Should not get here.
}

/*
   Gets the elements that make up our stack.  This is an stl list of pointers to
   modules that match the set of module names that were put in the stack via the
  -modules configuration parameter.  These modules were validated at configuration time.
   Since there's no way to destroy modules it's a really fatal error for them to not exist.
   To permit scaler stacks and the readout of scalers in response to other triggers,
   we will search for the modules in both the Adcs and Scalers list of the configuration.

Returns:
   A  list of pointers to the modules that should be included in the stack.

*/
CStack::StackElements
CStack::getStackElements()
{
  CConfiguration* pConfiguration   = Globals::pConfig;
  int             argc;
  const char**    argv;
  StackElements   result;
  string          sValue;

  // Split the list.. this must work because our validator ensured it:

  sValue = m_pConfiguration->cget("-modules");
  Tcl_SplitList(NULL, sValue.c_str(), &argc, &argv);
  assert(argc > 0);		// Can't have an empty module list.

  // Iterate searching for the modules first in the ADcs list and then in the Scalers list.
  // it's a fatal error to fail to find them.  Each module pointer is added to the result list.

  for (int i=0; i < argc; i++) {
    string name(argv[i]);
    CReadoutModule* pModule = pConfiguration->findAdc(name);
    if (!pModule) {
      pModule = pConfiguration->findScaler(name);
    }
    assert(pModule);		// Must exist in one or the other lists of modules.

    result.push_back(pModule->getHardwarePointer());
  }
  // Free the storage allocated by Split list and provide the list to the caller:

  Tcl_Free(reinterpret_cast<char*>(argv));
  return result;

}
/* Return the number of the list assigned to the stack.  This is done as follows:
   o If the list is triggered by nim1, the stack number is 0.
   o If the list is triggered by scaler the stack number 1.
   o If the list is triggered by interrupts, the stack number is defined by the -stack
     parameter.
 RETURNS:
    The stack number after all the figuring out described above.
*/
int
CStack::getListNumber()
{
  TriggerType trigger = getTriggerType();
  if (trigger == Event) {
    return 0;
  }
  if (trigger == Scaler) {
    return 1;
  }
  assert(0);			// Error to be here.
}
/*
   Custom validator for the -modules switch.  This validator checks that
   - The proposed value is a valid Tcl list.
   - There is at least one element in the list.
   - The proposed value contains list elements that are known modules in either the
     ADC or Scaler lists of the configurator.
Parameters:
   string name          - The name of the configuration parameter (most likely -modules).
   string proposedValue - The new value proposed for the configuration parameter.
   void*  arg           - Unused argument to the validator from the application.
Returns:
   true   - The conditions described above are true.
   false  - Any of the above conditions failed.

*/
bool
CStack::moduleChecker(string name, string proposedValue, void* arg)
{
  int             argc;
  const char**    argv;
  int             status;
  CConfiguration* pConfiguration = Globals::pConfig;
  string          Name;
  CReadoutModule* pModule;


  // Break the proposed value up in to a list..and return false if the parameter is not a well
  // formed list.


  status = Tcl_SplitList(NULL, proposedValue.c_str(), &argc, &argv);
  if (status != TCL_OK) {
    return false;
  }
  // Iterate through the module names attempting to find them in the ADCs and SCALER lists.
  // if one is not found, return the argv storage and return false.

  for ( int i=0; i < argc; i++) {
    Name    = argv[i];
    pModule = pConfiguration->findAdc(Name);
    if (!pModule) {
      pModule = pConfiguration->findScaler(Name);
    }
    if (!pModule) {
      Tcl_Free(reinterpret_cast<char*>(argv));
      return false;
    }
  }
  // If we got this far all the modules validated, return the argv storage and 
  // a true value.

  Tcl_Free(reinterpret_cast<char*>(argv));
  return true;
}
