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

#ifndef __CV812_H
#define __CV812_h


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


#include <CControlModule.h>
class CVMUSB;


/*!
   CV812 controls  a CAEN V812 constant fraction discriminator.
   This module is a bit odd because its settable parameters are
   all in write only registers.  We therefore maintain a shadow state
   in this file and, on initialization time, pre-load this state from a file
   and set the device to that state.  This allows us to work correctly
   an accurately with remote control panels.  An intelligent
   control panel/configuration system will have the control panel save to the
   file we use for initialization...we hope to be intelligent.

   Parameters of the module include:

   threshold$n$  (n in [0..15]) the threshold for a channel.
   width0        Width for channels 0-7.
   width1        Width for channels 8-15
   deadtime0     Deadtime for channels 0-7
   deadtime1     Deadtime for channels 8-15
   inhibits      Bit mask of channel inhibits.
   majority      Majority threshold.

  Configuration parameters include:

  -base         - Base address of the module.
  -file     - Initialization file that describes how to load the module initial state.

*/

class CV812 : public CControlHardware
{
private:
  CControlModule*     m_pConfiguration;

  // shadow state:

  int16_t             m_thresholds[16];
  uint16_t            m_widths[2];
  uint16_t            m_deadtimes[2];
  uint16_t            m_inhibits;
  uint16_t            m_majority;

  bool                m_is812;	// V895 supported by this but it has n deadtime registers

public:
  // canonicals:

  CV812();
  CV812(const CV812& rhs);
  virtual ~CV812();

  CV812& operator=(const CV812& rhs);
  int operator==(const CV812& rhs) const;
  int operator!=(const CV812& rhs) const;


  // virtual overrides:

public:
  virtual void onAttach(CControlModule& configuration);  //!< Create config.
  virtual void Initialize(CVMUSB& vme);	                 //!< init module after configuration is done.
  virtual std::string Update(CVMUSB& vme);               //!< Update module.
  virtual std::string Set(CVMUSB& vme, 
			  std::string parameter, 
			  std::string value);            //!< Set parameter value
  virtual std::string Get(CVMUSB& vme, 
			  std::string parameter);        //!< Get parameter value.
  virtual CControlHardware* clone() const;	     //!< Virtual

  // utilities:
  
private:

  // Confiuration parameter getters:

  uint32_t base();
  std::string initializationFile();
  void configFileToShadow();


  // Parameter setters.
  
  std::string setThreshold(CVMUSB& vme, unsigned int channel, uint16_t value);
  std::string setWidth(CVMUSB& vme, unsigned int selector, uint16_t value);
  std::string setDeadtime(CVMUSB& vme, unsigned int selector, uint16_t value);
  std::string setMajority(CVMUSB& vme, uint16_t value);
  std::string setInhibits(CVMUSB& vme, uint16_t value);

  // Parameter getters (operate on the shadow values):

  std::string getThreshold(unsigned int channel);
  std::string getWidth(unsigned int selector);
  std::string getDeadtime(unsigned int selector);
  std::string getMajority();
  std::string getInhibits();

 
  static std::string iToS(int value);
  static uint16_t majorityToRegister(int value);

};




#endif
