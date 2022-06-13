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

#ifndef __CV1729COMMAND_H
#define __CV1729COMMAND_H

#include "CDeviceCommand.h"

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif



class CTCLInterpreter;
class CTCLObject;
class CConfiguration;

/**
 * This class creates and configures the CAEN V1729 adc.
 * It implements the usual command ensemble for create, config and cget subcommands.
 * See the device support CV1729 class header for the set of options supported.
 * by this device driver.
 *
 */
class CV1729Command : public CDeviceCommand
{
private:
  CConfiguration& m_Config;

public:
  CV1729Command(CTCLInterpreter& interp,
	      CConfiguration&  config,
	      std::string      commandName = std::string("v1729a"));
  virtual ~CV1729Command();
private:
  CV1729Command(const CV1729Command& rhs);
  CV1729Command& operator=(const CV1729Command& rhs);
  int operator==(const CV1729Command& rhs) const;
  int operator!=(const CV1729Command& rhs) const;
public:

  // command entry point:
protected:


  CConfiguration* getConfiguration();

private:
  virtual int create(CTCLInterpreter& interp, 
		     std::vector<CTCLObject>& objv);

};
#endif
