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

Driver for C894 - CAEN LED 
*/

#ifndef C894_H
#define C894_h


#include "CControlHardware.h"
#include <string>
#include <stdint.h>


#include <CControlModule.h>
class CCCUSB;


class C894 : public CControlHardware
{
private:

  uint16_t            m_thresholds[16];
  uint16_t            m_widths[2];
  uint16_t            m_inhibits;
  uint16_t            m_majority;

public:
  // canonicals:

  C894();
  C894(const C894& rhs);
  virtual ~C894();

  C894& operator=(const C894& rhs);
  int operator==(const C894& rhs) const;
  int operator!=(const C894& rhs) const;


  // virtual overrides:

public:
  virtual void onAttach(CControlModule& configuration);  //!< Create config.
  virtual void Initialize(CCCUSB& camac);	                 //!< init module after configuration is done.
  virtual std::string Update(CCCUSB& camac);               //!< Update module.
  virtual std::string Set(CCCUSB& camac, 
			  std::string parameter, 
			  std::string value);            //!< Set parameter value
  virtual std::string Get(CCCUSB& camac, 
			  std::string parameter);        //!< Get parameter value.
  virtual CControlHardware* clone() const;	     //!< Virtual

  // utilities:
private:
  uint32_t getSlot();
  uint16_t majorityToRegister(int value);
  std::string iToS(int value);

  std::string setThreshold(CCCUSB& camac, unsigned int channel, uint16_t value);
  std::string setWidth(CCCUSB& camac, unsigned int selctor, uint16_t value);
  std::string setMajority(CCCUSB& camac, uint16_t value);
  std::string setInhibits(CCCUSB& camac, uint16_t value);

  std::string  getThreshold(unsigned int channel);
  std::string  getWidth(unsigned int selector);
  std::string  getMajority();
  std::string  getInhibits();

  // Configuration file handling:

  void configFileToShadow();
  std::string initializationFile();

};


#endif
