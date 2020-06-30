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

Control driver for Phililps Ph7106 Leading edge discriminator.

*/

#ifndef CPH7106_H
#define CPH7106_H
#include "CControlHardware.h"
#include <string>
#include <stdint.h>
#include <CControlModule.h>
class CCCUSB;

/*!
  This device has the following parameters:
  - threshold - sets/gets the threshold value.. This is gotten from the hardware.
  - mask      - sets/gets the mask register.  The mask register  has 1's to enable channels.
  - mode      - "local" or 'camac' indicating if the module can be controlled.
*/
class CPH7106 : public CControlHardware
{
private:

  // canoncials:
public:
  CPH7106();
  CPH7106(const CPH7106& rhs);
  virtual ~CPH7106();

  CPH7106& operator=(const CPH7106& rhs);
  int operator==(const CPH7106& rhs) const;
  int operator!=(const CPH7106& rhs) const;

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

  // private utility functions:

private:
  uint32_t getSlot();
  std::string iToS(int value);

  std::string setThreshold(CCCUSB& camac,  uint16_t value);
  std::string getThreshold(CCCUSB& camac);
  std::string setMask(CCCUSB& camac, uint16_t value);
  std::string getMask(CCCUSB& camac);
  std::string getMode(CCCUSB& camac);

  void forceCamacMode(CCCUSB& camac);
};

#endif
