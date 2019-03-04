
#ifndef CMXDCRESET_H
#define CMXDCRESET_H

#include "CControlHardware.h"
#include <string> 

// Forward class definitions:

#include <CControlModule.h>

class CVMUSB;

///////////////////////////////////////////////////////////////////////////////
//////////////////////// SLOW CONTROLS FOR SOFT RESETS ////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**! Slow control support for resetting any of the MxDC family of digitizers
*
* Implementation of a CControlHardware derived class to be used through
* the Module command. There is a corresponding CMxDCResetCreator class
* that accompanies this.
*
* This only provides the ability to perform a soft reset of any Mesytec MxDC device
* via the Initialize method. All other methods are no-ops and simply return an
* error indicating that they do not have have implementation.
*
*/
class CMxDCReset : public ::CControlHardware
{
  public:
    CMxDCReset();

    virtual CControlHardware* clone() const; 

    virtual void onAttach (CControlModule& config);
    virtual void Initialize(CVMUSB& controller);
    virtual std::string Update(CVMUSB& vme);
    virtual std::string Set(CVMUSB& vme, std::string what, std::string value);
    virtual std::string Get(CVMUSB& vme, std::string what);

};

#endif
