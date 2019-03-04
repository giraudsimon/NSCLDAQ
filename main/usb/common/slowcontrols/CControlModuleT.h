/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
       Ron Fox
       Jeromy Tompkins
	     NSCL
       Michigan State University
       East Lansing, MI 48824-1321
       */

#ifndef CCONTROLMODULET_H
#define CCONTROLMODULET_H

#include <XXUSBConfigurableObject.h>

#include <string>

#include <memory>

// Forward declaration of CControlHardwareT
template<class Ctlr> class CControlHardwareT;

/**!
  A control module is a configurable object that owns an object
  derived from CControlHardwareT. 
  - This object maintains the configuration for the module.
  - This object defines and delegates to the CControlHardwareT object
  the following functions:
  - Update  - Updates the internal state of the object from the hardware.
  - Set     - Sets some controllable point in the hardware to a new value
  - Get     - Retreives the value of some controllable point in the hardware
              from the most recently updated values.
  - addMonitorList - Adds operations to execute periodically
  - processMonitorList - Processes data produced by periodic executions
  - getMonitorList - Provides access to most recently received data
      

  Hardware objects must also implement onAttach, which is called when
  the hardware is attached to the configuration.  The hardware is required
  to register configuration parameters (e.g. -base or -slot) with the 
  configuration at that time.

  In delegating these operations, CControlModuleT takes care to synchronize
  with the readout thread if necessary, so that the
  individual hardware modules can be written without any knowledge of
  the existence of the readout thread.

  This is a template that was defined with the CVMUSB and CCCUSB classes
  in mind for the Ctlr template parameter. It need n t be used with those, 
  however, if some other controller implements the proper functionality.
  */
template<class Ctlr>
class CControlModuleT : public XXUSB::CConfigurableObject
{
 private:
    CControlHardwareT<Ctlr>*         m_pHardware; //!< Hardware


 public:
    using RdoList = typename Ctlr::RdoList; //!< Readoutlist compatible with
    //   Ctlr


  public:

    // Canonicals

    /**! \brief Constructor
     *  
     *  Steals ownership from the CControlHardwareT<Ctlr> passed in and
     *  identifies itself with a name.
     */
    CControlModuleT(std::string name, 
                    CControlHardwareT<Ctlr>* hardware);

    /**! \brief Destructor
     *
     * The CControlHardwareT will be deleted here.
     */
    virtual ~CControlModuleT();

    /**! \brief Copy constructor
     *
     * A deep copy of the CControlHardwareT owned by other is made
     * and the onAttach method is called.
     *
     * \param rhs   the object to copy
     */
    CControlModuleT(const CControlModuleT& rhs);

    /**! \brief Assignment operator
     *
     * The configuration is cleared first. Next the state 
     * of the base class is copied over. A deep copy of the 
     * hardware owned by the other object is made and it is 
     * attached to this.
     *
     * \param rhs   the object to copy
     *
     * \returns reference to this
     */
    CControlModuleT& operator=(const CControlModuleT& rhs);

  private:
    /**! Equality operator is not allowed */
    int operator==(const CControlModuleT& rhs) const;
    
    /**! Inequality operator is not allowed */
    int operator!=(const CControlModuleT& rhs) const;
  public:
    // Functions:


    /**! \brief Initialization routine
     *
     * Routine to properly initialize the hardware for proper function.
     * This also provides the logic of gaining control of the controller 
     * first.
     *
     * \param ctlr   the controller
     */
    void        Initialize(Ctlr& crate);

    /**! \brief Update routine
     *
     * Routine to either read current state of a device or write a 
     * state to the device. There is no demand made on what is actually
     * being updated (i.e. caller's state or hardware's state). 
     * This also provides the logic of gaining control of the controller 
     * first.
     *
     * \param ctlr   the controller
     */
    std::string Update(Ctlr& crate);


    /**! \brief Set a parameter
     *
     * This is the entry point called by the Set command. Often this is just a
     * dispatch method that parses a parameter name and calls appropriate
     * functionality for that parameter. 
     *
     * The convention on the return value is that if an error occurs, the string
     * passed back to the caller should begin with "ERROR". If the operation
     * succeeds, then the return value should just be some value represented as a
     * string.
     *
     * This also provides the logic of gaining control of the controller 
     * first.
     * 
     * \param ctlr        controller
     * \param parameter   parameter name
     * \param value       value to write to parameter
     *
     * \return string representing return value 
     */
    std::string Set(Ctlr& crate, const char* what, const char* value);
    
    /**! \brief Retrieve a parameter
     *
     * This is the entry point of the Get command. It is typically a dispatch
     * method that will parse a parameter name and call the appropriate
     * functionaity. 
     *
     * The convention on the return value is that if an error occurs, the string
     * passed back to the caller should begin with "ERROR". If the operation
     * succeeds, then the return value should just be some value represented as a
     * string.
     * 
     * This also provides the logic of gaining control of the controller 
     * first.
     *
     * \param ctlr        controller
     * \param parameter   parameter name
     *
     * \returns string representing return value
     */
    std::string Get(Ctlr& crate, const char* what);


    //
    // Interface to support monitoring. 
    //
    // \important Depending on the context that an
    // instance of this class is being use in, these may or may not be called.
    // For example, the CCUSBReadout program never calls monitoring methods.
    // However, the VMUSBReadout program does do so.
    //
    // 

    /**! \brief Add operations to be executed periodically by the slow-controls
     *          server
     *
     *  \param list   the readout list to add operations to
     */
    void addMonitorList(RdoList& vmeList);
    
    /**! \brief Process data produced by the monitor list
     *
     *  This should process only the amount of data expected to be produced by 
     *  the operations added in addMonitorList. 
     * 
     *  \param pData      a pointer to the data to begin processing
     *  \param remaining  maximum number of bytes remaining to be processed
     *
     *  \returns pointer to byte following the last byte of processed data
     */
    void* processMonitorList(void* pData, size_t remaining);

    
    /**! \brief Retrieve most recently received data from monitor list
     *
     *  Devices that do not use monitor lists should return "ERROR"
     *
     *  \return string representation of most recently received data
     */
    std::string getMonitoredData();


    /**! \brief Retrieve non-owning reference to underlying hardware
     *
     * \param pointer to hardware
     */
    CControlHardwareT<Ctlr>* getHardware() { return m_pHardware; }
};

// the implementation
#include <CControlModuleT.hpp>

#endif
