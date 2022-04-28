/**
 *  This is a sample control device driver.
 *  It has the following three key components:
 *
 *  -  The driver itself and its configuration (CUserDriver)
 *  -  A creational objectd (CUserDriverCreator)
 *  -  A Tcl package initialization function userDriver_Init which 
 *     registers the user driver creator with the control module factory.
 *  
 */

#include <CControlHardware.h>
#include <CControlModule.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>

#include <CModuleCreator.h>

#include <CModuleFactory.h>

#include <string>

#include <tcl.h>


 /*------------------------------------------------------------------------------------------------------
 *  The driver:
 *
 *   1.   It must be derived from a CControlHardware class.
 *   2.   It must define the following pure virtual methods:
 *        *   onAttach - Get access to the configuration and define configuration parameters.
 *        *   Update   - Update the hardware from internal settings.
 *        *   Set      - Make a setting.
 *        *   Get      - Return the value of a setting.
 *        *   clone    - Make a copy of this (virtual constructor)
 *   3. Optionally the driver can define Initialize
 *   4. See below for periodic monitoring.
 *
 *  Some devices need periodic monitoring.  For example, if you have a VME HV unit you need
 *  to be able to periodically poll for trips even during data taking.  This is done by defining
 *  the following methods:
 *  
 *   *  addMonitorList - Adds a set of VME operations to the list that polls for needed information.
 *   *  processMonitorList - Process the chunk of the data from the list that pertains to you.
 *   *  getMonitoredData   - Produce the most recent chunk of monitored data to clients.
 *
 * Monitor lists and their use will be described in the formal docs and won't be shown in this sample driver.
 */

/**
 * Our sample driver is not going to actually touch any hardware though we will indicate where/how
 * to do that.  What we're going to do instead is define a driver that has a configuration
 * that can be set/gotten remotely.
 * We'll support the configuration parameters:
 *
 *   * -anint
 *   * -astring
 *   * -alist
 */


class CUserDriver : public CControlHardware
{

public:
  CUserDriver();
  virtual void onAttach(CControlModule& configuration);
  virtual std::string Update(CVMUSB& vme);
  virtual std::string Set(CVMUSB& vme, 
			  std::string parameter, 
			  std::string value);
  virtual std::string Get(CVMUSB& vme, 
			  std::string parameter);
  virtual CControlHardware* clone() const;

};

/**
 *  constructor
 *
 *  This only instantiates a base CControlHardware object that maintains a 
 *  protected pointer m_pConfig. This pointer does not refer to anything
 *  unit onAttach is called and the user explicitly stores the reference.
 */
CUserDriver::CUserDriver() :
  CControlHardware()
{}

/**
 * onAttach
 *   Called when a driver instance is attached to the software.  This
 *   is where we maintain a pointer to our configuration (CControlModule) and
 *   provide configurable options.  The CControlModule is a XXUSB::CConfigurableObject
 *   so that part works just like a readout driver.
 *
 * @param configuration - Reference to the configuration object.
 */
void
CUserDriver::onAttach(CControlModule& configuration)
{
  m_pConfig = &configuration;	// m_pConfig is protected in the base class.
  m_pConfig->addIntegerParameter("-anint", 0);
  m_pConfig->addParameter("-astring", NULL, NULL, "");
  m_pConfig->addIntListParameter("-alist", 16);

}
/**
 * Update
 *   This is generally intended for devices with write only settings.  It is requested
 *   by the client to update the hardware from shadow registers or the current settings.
 *
 * @param vme - This is a reference to a CVMUSB object.  You can use it to directly  perform
 *              VME operations or you can create and populate a CVMUSBReadoutList and 
 *              use this to execute the list.
 *
 * @return std::string 
 * @retval "OK"  - Successful completion.
 * @retval "ERROR - some error message"  - Failure because of "some error message".
 */
std::string
CUserDriver::Update(CVMUSB& vme)
{
  return "OK";
}
/**
 * Set 
 *
 *   This is used to modify a parameter in the hardware.  Since this driver has no hardware
 *   we will modify a configuration parameter.
 *
 * @param vme - Reference to a VM-USB controller object.  This can be used to do single
 *              shot VME operations or to execute a CVMUSBReadoutList you build here.
 *
 * @param parameter - Identifies the name of the parameter to modify.
 * @param value     - The value to which the parameter should be set.
 *
 * @return std::string
 * @retval "OK"  the set was successful.
 * @retval "ERROR - some error message"  The set failed because of "some error message"
 */
std::string
CUserDriver::Set(CVMUSB& vme, std::string parameter, std::string value)
{
  try {
    m_pConfig->configure(parameter, value);
    return "OK";
  }
  catch (std::string msg) {	// configure reports errors via std::string exceptions.
    std::string status = "ERROR - ";
    status += msg;
    return status;
  }
}
/**
 * Get
 *    This is used to retrieve a parameter from the hardware.  Since this driver has
 *    no hardware, we're just going to fetch a configuration parameter.
 *
 * @param vme - Reference to a VMUSB controller object that can be used to either
 *              directly perform VME operations or to perform lists of operations
 *              built by this method.
 * @param parameter - name of the parameter we want to retrieve.
 *
 * @return std::string
 * @retval "ERROR - some error message" The get failed due to "some error message"
 * @retval anything else - the value from the 'hardware'.
 */
std::string
CUserDriver::Get(CVMUSB& vme, std::string parameter)
{
  try {
    return m_pConfig->cget(parameter);
  }
  catch (std::string msg) {
    std::string retval = "ERROR - ";
    retval += msg;
    return msg;
  }
}
/**
 * clone
 *  Return a copy of this object.
 * 
 * @param rhs - the object we are being cloned from.
 */
CControlHardware*
CUserDriver::clone() const
{
  return (new CUserDriver(*this));
}

/*--------------------------------------------------------------------------------------
 * The creator.
 *   This is a very simple class it just needs
 *
 *   1. To be derived from CModuleCreator.
 *   2. Supply an operator() method that creates an instance of our driver.
 **/

class CUserDriverCreator : public CModuleCreator
{
public:
  virtual CControlHardware* operator()();
};
/**
 * operator()
 *   Create a CUserDriver instance of the specified name:
 *
 * @return CControlHardware*  - Pointer to the newly, dynamically created object.
 */
CControlHardware*
CUserDriverCreator::operator()()
{
  return (new CUserDriver);
}

/*-----------------------------------------------------------------------------------
 * Tcl package initialization.
 *
 *  1. Must be a "C" binding function 
 *  2. Name must be derived from the name of the package/library. The library will be named
 *     something like libuserdriver.so  in which case the package name is 
 *     Userdriver_Init.
 *  3. The init function needs to register a creator with the module factory providing the module
 *     type name that's needed to create an intstance of our driver.
 *  4. The init function returns TCL_OK on success or TCL_ERROR on failure with the 
 *     interpreter result set to an error message.
 */
extern "C" {
  int Userdriver_Init(Tcl_Interp* pInterp)
  {
    CModuleFactory* pFact = CModuleFactory::instance(); // the module factory is a singleton.
    pFact->addCreator("mydriver", 
                      unique_ptr<CModuleCreator>(new CUserDriverCreator));

    return TCL_OK;
  }
  
}
