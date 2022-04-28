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

#ifndef _CV1495SCCOMMAND_H
#define _CV1495SCCOMMAND_H

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

/*!
 * This class provides the command hooks to the driver module for the CV1495sc 128 channel
 * scaler module.  See the driver itself in ../devices for a description of the configuration
 * parameters supported.
 *
 *  This class defines/implements a Tcl commane ensemble named v1495sc with the following subcommands:
 *
 * \verbatim
 * create module-name base-address
 * config module-name config_params
 * cget   module-name
 *
 * \endverbatim
 */
class CV1495scCommand : public CDeviceCommand
{
private:
  CConfiguration& m_Config;

public:
  CV1495scCommand(CTCLInterpreter& interp,
	      CConfiguration&  config,
	      std::string      commandName = std::string("v1495sc"));
  virtual ~CV1495scCommand();
private:
  CV1495scCommand(const CV1495scCommand& rhs);
  CV1495scCommand& operator=(const CV1495scCommand& rhs);
  int operator==(const CV1495scCommand& rhs) const;
  int operator!=(const CV1495scCommand& rhs) const;
public:

  // command entry point:
protected:
  CConfiguration* getConfiguration();

private:
  virtual int create(CTCLInterpreter& interp, 
		     std::vector<CTCLObject>& objv);
};

#endif
