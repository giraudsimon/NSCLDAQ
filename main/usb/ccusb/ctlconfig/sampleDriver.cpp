/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2013.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


#include <tcl.h>
#include <CModuleFactory.h>
#include <CModuleCreator.h>
#include <CControlHardware.h>
#include <CControlModule.h>
#include <string>



/**
 * @file sampleDrivers.cpp
 * @brief Sample driver for CCUSB Slow controls sytsem.
 */


/**
 * Slow control drivers are divided into three sections:
 * 1. The driver code itself.  This is responsible for
 *    initializing the device and processing client requests.
 * 2. The object creator which is resopnsible for knowing how to create
 *    instances of a driver class.
 * 3. Initialization which is responsible for registering the driver 
 *    with the module factor which is what makes its type known to the
 *    Module command.
 */


/**
 * 1. The driver code:
 *
 *  Drivers must implement the following entry points:
 *  -    Construction and base class initialzation.
 *  -    OnAttach - Configuration parameter declaration.
 *  -    Update - Update driver state from shadow reegister sets.
 *  -    Set    - Set a device paramter to  a new value.
 *  -    Get    - Retrive the value of a device paramter to the client.
 *  -    clone  - Copy some other object into our state (virtual copy construtor).
 */


/**
 * Here's the class 
 *  It must be derived from a CControlHardware class:
 */

class SampleDriver : public CControlHardware 
{
private:
  CControlModule* m_pConfig;
public:
  SampleDriver(std::string name);
  virtual ~SampleDriver ();

  // Forbidden methods

private:
  SampleDriver(const SampleDriver& rhs); // Can't copy construct
  SampleDriver& operator=(const SampleDriver& rhs); //  Cant' assign
  int operator==(const SampleDriver& rhs);	    // Cant compare for equality
  int operator!=(const SampleDriver& rhs);	    // Can't compare for inequalty.


public:
  virtual void onAttach(CControlModule& configuration);  //!< Create config.
  virtual void Initialize(CCCUSB& camac);	           //!< init module after configuration is done.
  virtual std::string Update(CCCUSB& camac);               //!< Update module.
  virtual std::string Set(CCCUSB& camac, 
			  std::string parameter, 
			  std::string value);              //!< Set parameter value
  virtual std::string Get(CCCUSB& camac, 
			  std::string parameter);          //!< Get parameter value.
  virtual void clone(const CControlHardware& rhs);	     //!< Virtual

};

/**
 * Constructor
 *   We need to construct the base classand also initialize
 *   any data.  At this point, we don't have a configuration.
 *   That gets assigned and initialized in onAttach below
 *
 * We at least have our m_pConfig member to initialize.
 *
 * @param name =- name to assoicate with this module. 
*/
SampleDriver::SampleDriver(std::string name) :
  CControlHardware(name),
  m_pConfig(0)
{
  
}

/**
 * Destructor
 *   If your driver has dynamic storage it should be deleted here to prevent
 *   storage leaks.
 */
SampleDriver::~SampleDriver()
{}

/**
 * onAttach
 *   This is called when the configuration has been built and
 *    attached to the device object.  We are going to:
 *    -   Save  a pointer to the configurationin m_pConfig
 *    -   Define anint - to be an integer.
 *    -   Define anarray - to be an array of integer.
 *    -   Define abool to be a boolean
 *
 *  Normal devices would probably at least define a 'slot' parameter
 *  to reflect which CAMAC crate slot the module lives in.
 *
 * @param config -reference to out configuation databas.
 */
void
SampleDriver::onAttach(CControlModule& configuration)
{
  m_pConfig = & configuration;
  m_pConfig->addIntegerParameter("anint");
  m_pConfig->addIntListParameter("test", 16);
  m_pConfig->addBooleanParameter("abool");
}

/**
 * Initialize
 *   Some devices require one-time initialization.  This method
 *   should contain code for that.  By the time Initialize
 *   is called, the configuration database has been set so it
 *   can be interrogated (e.g. to find out which slot the module
 *   lives in.
 *
 * @param camac - Reference to a CCCUSB controller object.  This can be
 *                used to perform any single shot CAMAC operations or to
 *                execute operations that have been batched into a 
 *                CCCUSBReadoutList.
 *
 */
void
SampleDriver::Initialize(CCCUSB& camac)
{
}
/**
 * Update
 *   For devices with write only registers it can be useful to have
 *   a method to set the devices to some known state (usually some shadow
 *   last known state).  That is the purpose of this
 *   method.  In our case the 'registers' are not write only so we don't
 *   need to do antyhing.
 *
 * @param crate - Reference to a CCCUSB controller object.  This can be
 *                used to perform any single shot CAMAC operations or to
 *                execute operations that have been batched into a 
 *                CCCUSBReadoutList.
 *
 * @return std::string
 * @retval 'OK' means everything worked just fine.
 * @retval 'ERROR...' means something failed and the stuff after 'ERROR ' is a human
 *          readable error message.
 */
std::string
SampleDriver::Update(CCCUSB& camac)
{
    return "OK";
}
/**
 * Set
 *   Used to set a device parameter.  For the purposes of our sample driver,
 *   the device parameters will just be the values of the configuration parameters.
 *   
 * @param camac - Reference to a CCCUSB controller object.  This can be
 *                used to perform any single shot CAMAC operations or to
 *                execute operations that have been batched into a 
 *                CCCUSBReadoutList.
 * @param parameter - Name of the parameter for us this is the name of one of the
 *                configuration parameters.
 * @param value   - New value of the parameter.
 *
 * @return std::string
 * @retval 'OK' means everything worked just fine.
 * @retval 'ERROR...' means something failed and the stuff after 'ERROR ' is a human
 *          readable error message.
 */
std::string
SampleDriver::Set(CCCUSB& camac, std::string parameter, std::string value)
{
  try {
    m_pConfig->configure(parameter, value);
  }
  catch(std::string msg) {
    std::string error = "ERROR ";
    error += msg;

    return error;
  }
  return "OK";
}
/**
 * Get
 *  Used to retrieve a device parameter.  For the purposes of our sample driver,
 *   the device parameters will just be the values of the configuration parameters.
 *   
 * @param camac - Reference to a CCCUSB controller object.  This can be
 *                used to perform any single shot CAMAC operations or to
 *                execute operations that have been batched into a 
 *                CCCUSBReadoutList.
 * @param parameter - Name of the parameter for us this is the name of one of the
 *                configuration parameters.
 *
 * @return std::string
 * @retval  value -  means everything worked just fine and this is the parameter value.
 * @retval 'ERROR. - ' means something failed and the stuff after 'ERROR - ' is a human
 *          readable error message.
 */
std::string
SampleDriver::Get(CCCUSB& camac, std::string parameter)
{
  try {
    return m_pConfig->cget(parameter);
  } catch(std::string msg) {
    std::string error = "ERROR - ";
    error += msg;
    return error;
  }
  
}
/**
 * clone
 *   Copies a rhs into this - not supported at present
 */
void
SampleDriver::clone(const CControlHardware& rhs)
{
}

/**
 * Module creator.  
 */

/**
 * The module creator class for the sample driver.
 */

class SampleCreator : public CModuleCreator {
public:
  CControlHardware* operator()(std::string name);
};

/**
 * operator()
 *    Create a SampleDriver objet
 *
 * @param name - The name of the module type. 
 * @return CControlHardware* new module.
 */
CControlHardware*
SampleCreator::operator()(std::string name)
{
  return new SampleDriver(name);
}

/**
 * Initialization. This function must have C binding. Its name is computed from the
 *  library file name and control is transfered as soon as the load module maps the
 *  shared library into virtual memory.  Note that the name is the name of the library
 *  with the first character capitalized followed by _Init.  Thuse we need to define
 *  Sampledriver_Init:
 * 
 *  pInterp - Pointer to the interpreter object. 
 */
extern "C" {
int
Sampledriver_Init(Tcl_Interp* pInterp)
{
  int status;
  
  status = Tcl_PkgProvide(pInterp, "Sampledriver", "1.0");    
  if (status != TCL_OK) {
    return status;
  }
  
  CModuleFactory* pFact = CModuleFactory::instance();
  pFact->addCreator("sample", new SampleCreator);
  
  return TCL_OK;
}
}
