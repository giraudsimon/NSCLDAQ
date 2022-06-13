


#ifndef CVMECAMACBRANCH_H
#define CVMECAMACBRANCH_H 

#include <vector>
#include <CReadoutHardware.h>
#include "CCamacCompat.hpp" // This must be included before CCamacCrate.h
#include "CCamacCrate.h"
#include <CReadoutModule.h>

class CCamacBranchDriver;
class CCBD8210CamacBranchDriver;
class CCBD8210CrateController;
class CCBD8210ReadoutList;

typedef CCamacCrate<CCBD8210CrateController,CCBD8210ReadoutList> CCBD8210Crate;

// This builds a CBD8210CamacBranch
///! \brief A camac branch rooted in VME
/**!
*   
*   This class is the backbone to the the branch driver 
*   infrastructure. It maintains a pointer to a CamacBranchDriver
*   and also the set of camac crates that live on the branch.
*   It calls the initialization routine for the branch and 
*   then for all of the crates that are registered to it. Furthermore,
*   it iterates through the registered crates and calls each
*   one's addReadoutList in the order it was called.
*
*   It is important to note that this does not know anything
*   about actual hardware besides the CVMUSB. It only knows
*   about abstract camac crates. The hardware must registered 
*   to the camac crates registered to this in order to be used.
*
*   At the moment this is specialized to the CBD8210. However,
*   it could be replaced by anything in the future that is 
*   based on the CCamacBranchDriver interface in the future.
*/
class CCBDCamacBranch : public CReadoutHardware
{
    private:
    // Define some useful types to improve readability
    // and ensure easy reconfiguration
    typedef CCBD8210Crate BranchElement;
    typedef std::vector<BranchElement*> BranchElements;

    public:
    // This is a composite object so we will iterate 
    // over its elements. Here we define the iterators
    typedef typename std::vector<BranchElement* >::iterator iterator;
    typedef typename std::vector<BranchElement* >::const_iterator const_iterator;

    private:
    CCBD8210CamacBranchDriver*  m_brDriver; ///!< The branch driver
    CReadoutModule* m_pConfig;  ///!< The configuration and also the owner of this
   
    public:
    ///! \brief Default constructor
    /**!
    *
    *  Instantiates the camac branch driver. Sets m_pConfig to 0.
    * 
    */
    CCBDCamacBranch(); 

    ///! \brief Copy constructor
    /**!
    *
    *   Creates a new camac branch driver and sets m_pConfig to 0.
    *   It doesn;t actually do any copying because it doesn't
    *   own anything besides the branch driver and that has no state.
    *
    *   \param rhs the target to copy
    */
    CCBDCamacBranch(const CCBDCamacBranch& rhs); 

    /**! \brief Assignment operator
    *
    *   It doesn;t actually do any copying because it doesn't
    *   own anything besides the branch driver and that has no state.
    *
    *   \param rhs the target to copy
    */
    CCBDCamacBranch& operator=(const CCBDCamacBranch& rhs); 

    /**! \brief Destructor
    *
    *   Deletes the camac branch driver.
    */
    ~CCBDCamacBranch();

    /**! \brief Set up the configuration parameters
    *
    *   Captures the address of the parent, which is the 
    *   argument.
    *
    *   Adds the following configuration flags to parent:
    *   -branch     the branch index (must be in range [0-7]
    *   -crates     the names of the crates on the branch
    *
    *   \param config the parent configuration
    */
    void onAttach(CReadoutModule& config); 

    /**! \brief Initialization routine
    *
    *   This function is called at the start of a run. It 
    *   acquires the list of crates registered to the branch
    *   each time it is called so that the user can reconfigure
    *   between runs. Once the crates are retrieved, the
    *   branch is initialized and then each registered crate
    *   is initialized.
    *
    *   \param controller the CVMUSB controller
    */
    void Initialize(CVMUSB& controller); 

    /**! \brief Readout routine
    *
    *   Adds a set of camac commands to the readout list provided
    *   as an argument to be executed for each event.
    *
    *   The list of registered crates is acquired and then
    *   they are iterated over in the order they were registered.
    *   Each of the crate's addReadoutList methods is invoked.
    *
    *   \param list the readout list to add a routine to.
    */
    void addReadoutList(CVMUSBReadoutList& list);

    /**! \brief onEndRun routine
    *
    *   This function is called at the end of a run. It 
    *   acquires the list of crates registered to the branch
    *   each time it is called so that the user can reconfigure
    *   between runs. Once the crates are retrieved, each registered crate's
    *   onEndRun routine is called.
    *
    *   \param controller the CVMUSB controller
    */
    void onEndRun(CVMUSB& controller);

    /**! \brief Polymorphic copy constructor 
    *
    *   \return a copy of this object
    */
    virtual CCBDCamacBranch* clone() const { return new CCBDCamacBranch(*this);}

    /**! \brief Acquire the registered crates
    *
    *   Queries the parent for the list of registered crates.
    *   The actual objects that correspond to the names in this list
    *   are acquired from the global configuration. (Note that every 
    *   object created in the tcl script has a corresponding object registered
    *   to the global configuration). These crate objects are then returned.
    *
    *   \return the set of registered crates (i.e. BranchElements)
    *
    */
    BranchElements getBranchElements();

    /**! \brief Verify that the -registered modules correspond to actual objects
    *   
    *   Given a list of objects, verify that the string list can be split
    *   into names that are all associated with a ReadoutHardware object.
    *   
    *   \param name is unused
    *   \param proposedValue the string list of registered crate names
    *   \param arg is unused
    */
    static bool crateChecker(std::string name, std::string proposedValue, void* arg);

}; // class CCBDCamacBranch

#endif
