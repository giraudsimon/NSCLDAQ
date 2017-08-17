/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
       NSCL DAQ Development Group 
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

#ifndef CCCUSBCONTROL_H
#define CCCUSBCONTROL_H


#include "CControlHardware.h"

#include <string>
#include <vector>
#include <stdint.h>


#include <CControlModule.h>
class CCCUSB;

/**
 * CCCUSBControl allows arbitrary CAMAC access by a client.  The idea is that a set command
 * will provide basically a remote procedure call for the CCCUSB::executeList method.
 * The syntax is
 *   Set vmusb [list hexadecimalized-vmusbreadoutlist read-buffer-requirements]
 * Where 
 *  - hexadedimalized-vmusbreadoutlist is the contents of a CCCUSBReadoutList converted to a
 *     Tcl list of hexadecimal long words.
 *  - read-buffer-requirements is the size of the required read buffer for the readout list.
 *
 * Success will return:
 *  OK hexadecimalized-output-buffer
 * Where
 *  - hexadecimalized-output-buffer is the reply buffer converted to a hexadecimal representation
 *    of each byte as a Tcl list.  Note that if no output data are available, and empty list will
 *    be returned.
 *
 * Note that this function can therefore also provide all single shot operations as those are just
 * lists with one element... however if the run is active each list execution will pause/resume
 * the run so be aware and use with caution.
 */
class CCCUSBControl : public CControlHardware
{

  // Canonical operations:
public:
  CCCUSBControl();
  CCCUSBControl(const CCCUSBControl& rhs);
  virtual ~CCCUSBControl();
  CCCUSBControl& operator=(const CCCUSBControl& rhs);
  int operator==(const CCCUSBControl& rhs)const;
  int operator!=(const CCCUSBControl& rhs)const;

  // Virtual overrides:
  
public:
  virtual void onAttach(CControlModule& configuration);  //!< Create config.
  virtual void Initialize(CCCUSB& ctlr);
  virtual std::string Update(CCCUSB& ctlr);               //!< Update module.
  virtual std::string Set(CCCUSB& ctlr, 
                          std::string parameter, 
                          std::string value);            //!< Set parameter value
  virtual std::string Get(CCCUSB& ctlr, 
                          std::string parameter);        //!< Get parameter value.
  virtual CControlHardware* clone() const;       //!< Virtual copy constr.

  // Utilities if any required.

private:
  size_t                decodeInputSize(std::string& list);
  std::vector<uint16_t> decodeList(std::string& list);
  std::string           marshallOutput(uint8_t* buffer, size_t numBytes);
};

#endif
