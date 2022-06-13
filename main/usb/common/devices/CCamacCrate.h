
#ifndef CCAMACCRATE_H
#define CCAMACCRATE_H

#include <string>
#include <stdint.h>
#include "CReadoutHardwareT.h"

///////////////////
// Fwd declarations
template<class Controller, class RdoList> class CCamacCrate;

/**! \class CCBD8210Crate
*   A specialization of this template class for use with the CBD8210 
*   branch driver.
*/
//typedef CCamacCrate<CCBD8210CrateController, CCBD8210ReadoutList> CCBD8210Crate;

//typedef template<> 
//class CCamacCrate<CCCUSB,CCCUSBReadoutList> CCUSBCrate;

/**!  \brief A camac crate for hybrid hardware
*
*   This is a composite object that organizes modules into their
*   respective crates. This is the an essential component of the CBD8210 driver 
*   because the branch is expected to be composed of these crates.
*
*   This is a template with two template parameters: Controller and RdoList. 
*   The controller must be an object that implements the same functionality 
*   as defined in the CCrateController interface. The RdoList must be a type 
*   with the same functinality as specified in the CCamacReadoutList.
*
*   Modules are to registered to this crate by passing their
*   names to the -modules parameter. 
*
*   Parameters:
*   -crate      the index of the crate
*   -modules    the list of hybrid hardware modules
*
*/
template<class Controller, class RdoList>
class CCamacCrate : public CReadoutHardwareT<Controller,RdoList>
{
    private:
        // some useful typedefs
        typedef CReadoutHardwareT<Controller,RdoList> CrateElement; ///!< hybrid hardware type
        typedef std::vector<CrateElement* > CrateElements; ///!< a set of hybrid hardware

    private:
        CReadoutModule* m_pConfig;  ///!< the parent configuration and owner of this object

    public:
        /**! \brief Default constructor
        *   
        * This effectively does nothing.  
        */
        CCamacCrate(); 

        /**! \brief Copy constructor
        *
        * This does nothing. Copying doesn't make any sense.
        *
        *   \param rhs is unused
        */
        CCamacCrate(const CCamacCrate& rhs); 

        /**! \brief Assignment operator
        *
        * Does nothing more than return a reference to this.
        *   
        *   \param rhs is unused
        *   \return a reference to this
        */
        CCamacCrate& operator=(const CCamacCrate& rhs); 

        
        /**! \brief Destructor 
        *
        * This does nothing. The object doesn't manage any resources.
        *
        */
        virtual ~CCamacCrate();

        /**! \brief Set up configuration parameters
        *
        *   Records the address of the parent configuration for later
        *   use and sets up the two parameters.
        *
        *   Parameters:
        *   -crate      the index of the crate
        *   -modules    the list of hybrid hardware modules
        *
        *   \param config the parent configuration
        */
        void onAttach(CReadoutModule& config); 

        /**! \brief Initialization routine
        *
        *   Acquires the set of registered hardware that was spcified
        *   in the -modules parameter. This doesn't break if there
        *   are no hardware modulesregistered. it simply doesn't 
        *   initialize anything. Anything registered to the crate should not
        *   call any controller methodsa than are available in the 
        *   CCrateController interface.
        *
        *   \param controller a controller type that has the same interface 
        *           as the CCrateController.
        */
        void Initialize(Controller& controller); 

    
        /**! \brief Readout routine
        *
        *   Acquires the set of hardware registered to the crate via the 
        *   -modules parameter and calls their respective addReadoutList 
        *   routines in the order they were registered.
        *
        *   \param list the readout list to fill
        */
        void addReadoutList(RdoList& list);

        /**! \brief End of run routine  
        *
        *   Acquires the set of hardware registered to the crate via the 
        *   -modules parameter and calls their respective onEndRun 
        *   routines in the order they were registered.
        *
        *   \param list the readout list to fill
        */
        void onEndRun(Controller& controller);

        /**! \brief Polymorphic copy constructor
        *
        *   \return a pointer to a dynamically allocated copy of this object.
        */
        virtual CCamacCrate* clone() const 
        { return new CCamacCrate(*this);}

        /**! \brief Get the index of the crate
        *
        *   \return the value specified for the -crate parameter
        */
        int getCrateIndex() const;

        /**! \brief Get the list of registered hybrid hardware 
        *
        *   This acquires the list of hardware that was registered to the crate. 
        *  Because the hardware used by this object must be hybrid hardware,  
        *  this method gets the CreadoutHardware and then unwraps it to 
        *   get the hybrid hardware (i.e. CReadoutHardwareT<Controller,RdoList>).
        *  
        *   \return a list of the hybrid hardware objects
        */
        CrateElements getCrateElements();

        /**! \brief Verifies that the list of modules is legitimate
        *   
        *   Checks that the list of modules specified with the -modules
        *   can be parsed into a set of hardware names that are associated with 
        *   hardware objects known by the global configuration.
        *
        *   \param name is unused
        *   \param proposedValue is the list of module names
        *   \param arg is unused
        *
        *   \return true if all registered modules are valid
        */
        static bool moduleChecker(std::string, std::string proposedValue, void* arg); 

}; // class CCamacCrate

// include the implementation 
#include "CCamacCrate.hpp"

#endif
