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

#ifndef __CCONTROLHARDWARET_H
#define __CCONTROLHARDWARET_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#include <memory>

// Forward declaration of CControlModuleT
template<class Ctlr> class CControlModuleT;

/*!
   CControlHardwareT is an abstract base class for programmable electronics that is
   not read out as part of the event stream.  It provides a set of interfaces
   that the TclServer commands can use to communicate with a module.
   It also defines the configuration parameters the module can accept.
   The tacit assumption here is that modules only need to be initialized by
   'setting' controllable parameters.  If other initialization is required,
   it will be necessary for the module itself to maintain sufficient state to ensure
   that this happens at the first access, as presumably by then the configuration is
   initialized.

   The template parameter Ctlr is expected to be either CVMUSB or CCCUSB. 
   However, it is possible for any other type that provides the requisite 
   functionality to be used.
   
   Make the distinction between:
   - Configuration: Mostly static information used to access the module.
     for most devices this will consist solely of the base address.
   - Parameters: Control points within the device that are dynamically modified.
     For example for the GDG a parameter might be a channel delay.
*/

template<class Ctlr>
class CControlHardwareT
{
  public:
    using RdoList = typename Ctlr::RdoList; //!< ReadoutList compatible 
                                            //   with Ctlr

  protected:
    CControlModuleT<Ctlr>* m_pConfig; //!< Non-owning reference to parent

  public:
    // Canonicals:

    /**! \brief Default constructor
     *  
     *  Initializes the reference to nullptr.
     */
    CControlHardwareT();

    /**! \brief Copy constructor
     *
     *  Shallow copies the value of the reference.
     *
     *  \param rhs  the object whose state will be copied
     */
    CControlHardwareT(const CControlHardwareT& rhs);

    /**! \brief Destructor constructor
     *
     *  No-op.
     */
    virtual ~CControlHardwareT();

    /**! \brief Assignment operator
     *
     * Shallow-copies reference value. 
     *
     * \param rhs   object whose state will be copied
     */
    CControlHardwareT& operator=(const CControlHardwareT& rhs);

    /**! \brief Equality operator
     *
     *  \returns boolean
     *  \retval true - references are the same
     *  \retval false - otherwise 
     */
    int operator==(const CControlHardwareT& rhs) const;

    /**! \brief Inequality operator
     *
     *  The opposite of equality... see equality docs
     */
    int operator!=(const CControlHardwareT& rhs) const;

  public:

    /**! \brief Retrieve the configuration that is known by base class
     *
     */
    CControlModuleT<Ctlr>* getConfiguration() const { return m_pConfig;}

  public:
    ///////////////////////////////////////////////////////////////////////////
    //
    // Public interface.
    //

    /**! \brief Virtual copy constructor
     *
     *  This is actually a factory method produces a copy of the current
     *  instance. It is purely virtual such that any derived type must
     *  implement it.
     * 
     *  The ownership is transferred to the caller as is indicated by the
     *  returning of a std::unique_ptr.
     *
     *  \return unique_ptr holding new object 
     *
     */
    virtual CControlHardwareT* clone() const = 0;


    /**! \brief Adds parameters to CControlModule
     *
     * Typically in this method the user will cache the reference to the parent
     *
     *  \param configuration  the CControlModuleT that owns this
     */
    virtual void onAttach(CControlModuleT<Ctlr>& configuration);  

    /**! \brief Initialization routine
     *
     * Routine to properly initialize the hardware for proper function.
     *
     * \param ctlr   the controller
     *
     */
    virtual void Initialize(Ctlr& ctlr);

    /**! \brief Update routine
     *
     * Routine to either read current state of a device or write a 
     * state to the device. There is no demand made on what is actually
     * being updated (i.e. caller's state or hardware's state). 
     *
     * \param ctlr   the controller
     */
    virtual std::string Update(Ctlr& ctlr) = 0;    

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
     * \param ctlr        controller
     * \param parameter   parameter name
     * \param value       value to write to parameter
     *
     * \return string representing return value 
     */
    virtual std::string Set(Ctlr& ctlr, 
                            std::string parameter, 
                            std::string value) = 0; 

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
     * \param ctlr        controller
     * \param parameter   parameter name
     *
     * \returns string representing return value
     */
    virtual std::string Get(Ctlr& ctlr, 
                            std::string parameter) = 0;  


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
    virtual void addMonitorList(RdoList& list);  

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
    virtual void* processMonitorList(void* pData, size_t remaining);

    /**! \brief Retrieve most recently received data from monitor list
     *
     *  Devices that do not use monitor lists should return "ERROR"
     *
     *  \return string representation of most recently received data
     */
    virtual std::string getMonitoredData();

}; // end of template declaration


// Because this is a template, the entire implementation must live in the
// header. The following include provides the implementation.
#include <CControlHardwareT.hpp>

#endif
