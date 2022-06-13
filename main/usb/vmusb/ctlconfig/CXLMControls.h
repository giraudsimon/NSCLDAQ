
#ifndef CXLMCONTROLS_H
#define CXLMCONTROLS_H

#include "CControlHardware.h"
#include <string> 

// Forward class definitions:

#include <CControlModule.h>
class CVMUSB;
class CVMUSBReadoutList;

///////////////////////////////////////////////////////////////////////////////
//////////////////////// SLOW CONTROLS FOR FIRMWARE LOADS /////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace XLM
{

/**! Slow control support for the XLM 
*
* Implementation of a CControlHardware derived class to be used through
* the Module command. There is a corresponding CXLMControlsCreator class
* that accompanies this.
*
* This only provides the ability to configure the module through the 
* Initialize method. All other methods are no-ops and simply return an
* error indicating that they do not have have implementation.
*
*/
class CXLMControls : public ::CControlHardware
{
  public:
  CXLMControls();

  virtual CControlHardware* clone() const; 

  virtual void onAttach (CControlModule& config);
  virtual void Initialize(CVMUSB& controller);
  virtual std::string Update(CVMUSB& vme);
  virtual std::string Set(CVMUSB& vme, std::string what, std::string value);
  virtual std::string Get(CVMUSB& vme, std::string what);

};



} // end XLM namespace

#endif
