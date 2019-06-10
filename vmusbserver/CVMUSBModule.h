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

#ifndef __CVMUSBMODULE_H
#define __CVMUSBMODULE_H



#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif


#ifndef __CRT_STDINT
#include <stdint.h>
#ifndef __CRT_STDINT
#define __CRT_STDINT
#endif
#endif


class CControlModule;
class CVMUSB;

/**
 * CVMUSBModule allows arbitrary  VME access by a client.  The idea is that a set command
 * will provide basically a remote procedure call for the CVMUSB::executeList method.
 * The syntax is
 *   Set vmusb [list hexadecimalized-vmusbreadoutlist read-buffer-requirements]
 * Where 
 *  - hexadedimalized-vmusbreadoutlist is the contents of a CVMUSBReadoutList converted to a
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
class CVMUSBModule
{
  // Canonical operations:
public:
  CVMUSBModule(std::string name);
  virtual ~CVMUSBModule();
  int operator==(const CVMUSBModule& rhs)const;
  int operator!=(const CVMUSBModule& rhs)const;

  // Virtual overrides:
  
public:
 virtual std::string ImmediateList(CVMUSB& vme, 
			  std::string value);            //!< Set parameter value
 virtual std::string writeActionRegister(CVMUSB& vme,
					 uint16_t value);
 std::string loadList(CVMUSB& vme, int number, int offset, std::string list);
 

  // Utilities if any required.

private:
  size_t                decodeInputSize(std::string& list);
  std::vector<uint32_t> decodeList(std::string& list);
  std::string           marshallOutput(uint8_t* buffer, size_t numBytes);
};

#endif
