
#include "CCamacCrate.h"
#include <CCamacCompat.hpp>
#include "CConfiguration.h"
#ifndef __GLOBALS_H
#include <Globals.h>
#endif
#include <tcl.h>
#include <iostream>
#include <string>
#include <typeinfo>
// this the template source code CCamacCrate.hpp

template<class Controller, class RdoList> 
CCamacCrate<Controller,RdoList>::CCamacCrate() 
  : CReadoutHardwareT<Controller,RdoList>()
{}


template<class Controller, class RdoList> 
CCamacCrate<Controller,RdoList>::CCamacCrate(const CCamacCrate<Controller,RdoList>& rhs)
{}

template<class Controller, class RdoList> 
CCamacCrate<Controller,RdoList>& 
CCamacCrate<Controller,RdoList>::operator=(const CCamacCrate<Controller,RdoList>& rhs)
{
    return *this;
}


template<class Controller, class RdoList> 
CCamacCrate<Controller,RdoList>::~CCamacCrate() 
{
}

template<class Controller, class RdoList> 
void CCamacCrate<Controller,RdoList>::onAttach(CReadoutModule& config)
{
    m_pConfig = &config;
    m_pConfig->addIntegerParameter("-crate",0);
    m_pConfig->addParameter("-modules",CCamacCrate::moduleChecker, NULL, "");
}

template<class Controller, class RdoList> 
void CCamacCrate<Controller,RdoList>::Initialize(Controller& controller)
{
   std::cout << "*** Initializing crate : "<< getCrateIndex() << std::endl;  

   // get the registered hardware
   CrateElements elements = getCrateElements();
   std::cout << "    Crate has " << elements.size() << " modules" << std::endl;

   // iterate through them,
   typename CrateElements::iterator it = elements.begin();
   const typename CrateElements::iterator end = elements.end();

   while (it!=end) {
        (*it)->Initialize(controller);
        ++it;
   }

}

template<class Controller, class RdoList> 
void CCamacCrate<Controller,RdoList>::addReadoutList(RdoList& list)
{
   // get the registered hardware
   CrateElements elements = getCrateElements();
   typename CrateElements::iterator it = elements.begin();
   const typename CrateElements::iterator end = elements.end();
    
   // iterate through them,
   while (it!=end) {
        (*it)->addReadoutList(list);
        ++it;
   }

}

template<class Controller, class RdoList> 
void CCamacCrate<Controller,RdoList>::onEndRun(Controller& controller)
{
   // get the registered hardware
   CrateElements elements = getCrateElements();

   // iterate through them,
   typename CrateElements::iterator it = elements.begin();
   const typename CrateElements::iterator end = elements.end();

   while (it!=end) {
        (*it)->onEndRun(controller);
        ++it;
   }

}

template<class Controller, class RdoList>
int CCamacCrate<Controller,RdoList>::getCrateIndex() const
{
    return m_pConfig->getIntegerParameter("-crate"); 
}

template<class Controller, class RdoList>
typename CCamacCrate<Controller,RdoList>::CrateElements 
CCamacCrate<Controller,RdoList>::getCrateElements()
{

  CConfiguration* pConfiguration   = Globals::pConfig;
  int             argc;
  const char**    argv;
  CrateElements   result;
  std::string          sValue;

  // Split the list.. this must work because our validator ensured it:

  sValue = m_pConfig->cget("-modules");
  Tcl_SplitList(NULL, sValue.c_str(), &argc, &argv);
  if (argc <= 0) {
      std::cout << std::string("CCamacCrate::getCrateElements() found empty argument list");
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
//          std::cout << "Verifing that the module (" << name << ") is a CCamacCompat" << std::endl;

            // force these to a CCamacCompate type object
            // the dynamic cast does not work in this scenario...
          CCamacCompat<CrateElement>* wrapped_module 
              = static_cast<CCamacCompat<CrateElement >*>(pModule->getHardwarePointer());

          if (wrapped_module) {
//              std::cout << "Indeed it is!" << std::endl;

            // unwrap the hybrid object and add it to the result list
              CrateElement* module = wrapped_module->getWrappedObj();
              result.push_back(module);
          } else {
              std::cout << "Incompatible module added to crate!" << std::endl;
          }
      }
  }
  // Free the storage allocated by Split list and provide the list to the caller:

  Tcl_Free(reinterpret_cast<char*>(argv));
  return result;


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
template<class Controller, class RdoList>
bool 
CCamacCrate<Controller,RdoList>::moduleChecker(std::string name, std::string proposedValue, void* arg)
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

