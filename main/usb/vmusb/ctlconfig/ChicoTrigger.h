/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __CHICOTRIGGER_H
#define __CHICOTRIGGER_H
#ifndef __CCONTROLHARDWARE_H
#include "CControlHardware.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#include <memory>

#include <CControlModule.h>
class CVMUSB;


/*!
   ChicoTrigger controls a JTEC/Wiener Gate and delay generator. This is an 8 channel
   unit.  At this time we only know of the following parameters:
   - delay$n$  (n = 0 through 7).  The delay for channel n.
   - width$n$  (n = 0 through 7).  The output width for channel n

   The configuration parameters are just:
   - base  - The base address of the module. We assume that address modifiers
             will be extended user data.
*/
class ChicoTrigger : public CControlHardware
{
private:
  CControlModule*      m_pConfiguration;


public:
  // Cannonical operations:

  ChicoTrigger();
  ChicoTrigger(const ChicoTrigger& rhs);
  virtual ~ChicoTrigger();
  ChicoTrigger& operator=(const ChicoTrigger& rhs);
  int operator==(const ChicoTrigger& rhs) const;
  int operator!=(const ChicoTrigger& rhs) const;

  // virtual overrides:

public:
  virtual void onAttach(CControlModule& configuration);  //!< Create config.
  virtual void Initialize(CVMUSB& vme);
  virtual std::string Update(CVMUSB& vme);               //!< Update module.
  virtual std::string Set(CVMUSB& vme, 
			  std::string parameter, 
			  std::string value);            //!< Set parameter value
  virtual std::string Get(CVMUSB& vme, 
			  std::string parameter);        //!< Get parameter value.
  virtual CControlHardware* clone() const;	     //!< Virtual copy constr.

private:
  uint32_t base();

};
#endif
