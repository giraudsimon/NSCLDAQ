
#include <typeinfo>
#include <iostream>
#include "CCBDCamacBranch.h"
#include "CCBD8210CamacBranchDriver.h"
#include "CCBD8210CrateController.h"
#include "CCamacCompat.hpp"
#include "Globals.h"
#include <tcl.h>
#include "CConfiguration.h"

CCBDCamacBranch::CCBDCamacBranch() 
: CReadoutHardware(), 
  m_brDriver(new CCBD8210CamacBranchDriver()),
  m_pConfig(0)
{ 
}

// Make a deep copy
CCBDCamacBranch::CCBDCamacBranch(const CCBDCamacBranch& rhs)
 : CReadoutHardware(rhs),
   m_brDriver(new CCBD8210CamacBranchDriver()),
   m_pConfig(0)
{
//    if (rhs.m_pConfig) {
//        m_pConfig = new CReadoutModule(*(rhs.m_pConfig));
//    }
}

// Make a deep copy
// no need to copy CCBD8210CamacBranchDriver b/c it doesn't hold
// any state. 
CCBDCamacBranch& CCBDCamacBranch::operator=(const CCBDCamacBranch& rhs)
{
    if (this!=&rhs) {

//        if (rhs.m_pConfig) {
//            CReadoutModule* temp = new CReadoutModule(*(rhs.m_pConfig));
//            delete m_pConfig;
//            m_pConfig = temp;   
//        } 
    }
    return *this;
}

// objects are owned by the CConfiguration and not this. No need to delete
// the crates
CCBDCamacBranch::~CCBDCamacBranch() 
{
   
    // Only this needs to be copied
    delete m_brDriver;
}


void CCBDCamacBranch::onAttach(CReadoutModule& config)
{

    // store the pointe to the parent
    m_pConfig = &config;

    // add the parameters.
    m_pConfig->addIntegerParameter("-branch",0);

    m_pConfig->addParameter("-crates",
            CCBDCamacBranch::crateChecker, NULL, "");
}


/**!
*   Handles the creatio of the virtual crate modules that get 
*   passed to the crates registered to it.
*/
void CCBDCamacBranch::Initialize(CVMUSB& controller)
{
    int branchID = m_pConfig->getIntegerParameter("-branch");

    std::cout << "*** Initializing branch : "<< branchID << std::endl;  

    // This passes the hardware back from it...
    // The wrapped crates are unwrapped before passing back the list
    BranchElements crates = getBranchElements();
    std::cout << "    Branch has "<< crates.size() << " crates:" << std::endl;  

    CCBD8210CrateController* adapted_controller; 

    // Initialize the branch
    // 1. create a controller for the branch and the cbd8210 crate (i.e. c=0)
    // 2. Give it the vmusb 
    // 3. Pass the controller to the branch driver to do its initialization
    adapted_controller = m_brDriver->createCrateController(branchID,0);
    adapted_controller->setController(controller);
    m_brDriver->initializeBranch(*adapted_controller, branchID);
    delete adapted_controller; // make sure we don't leak.
  
    iterator crate_it = crates.begin();
    const iterator crate_end = crates.end();
    
    for(; crate_it != crate_end; ++crate_it) {

        // Set p the crate controller
        int crate_index = (*crate_it)->getCrateIndex();

        adapted_controller = m_brDriver->createCrateController(branchID, 
                                                               crate_index);
        adapted_controller->setController(controller);

        // Initialize the crate on the branch w/ the fake controller
        m_brDriver->initializeCrate(*adapted_controller, branchID, crate_index);
       
        // Turn control over to the crate to initialize its components 
        (*crate_it)->Initialize(*adapted_controller);

        delete adapted_controller; // don't leak
    }
}

void CCBDCamacBranch::addReadoutList(CVMUSBReadoutList& list)
{
    int branchID = m_pConfig->getIntegerParameter("-branch");

    // This passes the hardware back from it...
    // The wrapped crates are unwrapped before passing back the list
    BranchElements crates = getBranchElements();
    iterator crate_it = crates.begin();
    const iterator crate_end = crates.end();

    CCBD8210ReadoutList* adapted_rdolist=0;
    for(; crate_it != crate_end; ++crate_it) {
        int crate_index = (*crate_it)->getCrateIndex();
        
        // create a rdolist for the crate and give a real rdolist
        adapted_rdolist = m_brDriver->createReadoutList(branchID, crate_index);
        adapted_rdolist->setReadoutList(list);

        (*crate_it)->addReadoutList(*adapted_rdolist);

        delete adapted_rdolist; // don't leak
    }

}

/**!
*   Handles the creatio of the virtual crate modules that get 
*   passed to the crates registered to it.
*/
void CCBDCamacBranch::onEndRun(CVMUSB& controller)
{
    int branchID = m_pConfig->getIntegerParameter("-branch");

    // This passes the hardware back from it...
    // The wrapped crates are unwrapped before passing back the list
    BranchElements crates = getBranchElements();

    CCBD8210CrateController* adapted_controller; 

    iterator crate_it = crates.begin();
    const iterator crate_end = crates.end();
    
    for(; crate_it != crate_end; ++crate_it) {

        // Set up the crate controller
        int crate_index = (*crate_it)->getCrateIndex();

        adapted_controller = m_brDriver->createCrateController(branchID, 
                                                               crate_index);
        adapted_controller->setController(controller);

        // Turn control over to the crate to do end of run operations 
        // on its components 
        (*crate_it)->onEndRun(*adapted_controller);

        delete adapted_controller; // don't leak
    }
}

///! This is where I get the hardware objects for the registered crates.
/**!
*   The hardware objects registered must be wrappers for the underlying
*   hybrid hardware (i.e. object deriving from ReadoutHardwareT<Controller,RdoList>). 
*
*   This means that anything registered to the branch must be a CCamacCompat object.
*/
CCBDCamacBranch::BranchElements CCBDCamacBranch::getBranchElements()
{

    CConfiguration* pConfiguration   = Globals::pConfig;
    int             argc;
    const char**    argv;
    BranchElements   result;
    std::string          sValue;

    // Split the list.. this must work because our validator ensured it:

    sValue = m_pConfig->cget("-crates");
    Tcl_SplitList(NULL, sValue.c_str(), &argc, &argv);
    if (argc <= 0) {
        std::cout << "CCBDCamacBranch::getBranchElements()";
        std::cout << " found empty argument list";
        std::cout << std::endl;
    }

    // Iterate searching for the modules first in the ADcs list and then in the Scalers list.
    // it's a fatal error to fail to find them.  Each module pointer is added to the result list.

    for (int i=0; i < argc; i++) {
        std::string name(argv[i]);
        CReadoutModule* pModule = pConfiguration->findAdc(name);
        if (!pModule) {
            pModule = pConfiguration->findScaler(name);
        }

        if (pModule) {
            // Have to static_cast this beast b/c it is too convoluted for type inferencing.
            // dynamic_cast will always return zero and fail to fill
            CCamacCompat<BranchElement>* wrapped_module 
                = static_cast<CCamacCompat<BranchElement>*>(pModule->getHardwarePointer());

//          std::cout << typeid(pModule->getHardwarePointer()).name() << std::endl;
            if (wrapped_module) {

                BranchElement* module = wrapped_module->getWrappedObj();
                result.push_back(module);
            }
        }
    }
    // Free the storage allocated by Split list and provide the list to the caller:

    Tcl_Free(reinterpret_cast<char*>(argv));
    return result;
}

bool CCBDCamacBranch::crateChecker(std::string name, std::string proposedValue, void* arg)
{
  int             argc;
  const char**    argv;
  int             status;
  CConfiguration* pConfiguration = Globals::pConfig;
  std::string          Name;
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


