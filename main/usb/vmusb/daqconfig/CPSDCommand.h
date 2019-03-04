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

#ifndef __CPSDCOMMAND_H
#define __CPSDCOMMAND_H

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
    This class creates and configures Washington U. PSD chips.
    The command supports the following syntaxes:
\verbatim
    psd create module-name base-address
    psd config module-name config_params...
    psd cget   module-name
\verbatim

   The psd create form is used to create a new psd module.  Unlike the 'standard
   scripted readout' all psds are used for data acquisition \em module-name
   is a name to be used to refer to the module in other psd subcommands.
   \em base-address is the VME base address configured from jumpers or the CERN V.430 geographical addressing.

   psd config is used to configure a module that has been created.
   \em module-name is the name of the module given to the create subcommand.
   \em config_params... are a set of keyword value pairs.  The keyword defines
   what is being configured, and the value is the value of that configuration
   item.
  Configuration keywords supported are:
   - -ipl         The interrupt priority level the module should use.
                  for the llnl neutron daq this should always be configured to be
                  6.
   - -vector      The interrupt vector the module should use. This is an integer
                  and should always be configured to be 0x80.

   The hinp cget command returns the entire module configuration for \em module-name

   example:
\verbatim
psd create psd1 0x800000
psd config psd1 -ipl 6 -vector 0x80
puts [psd cget psd1]
\endverbatim

*/
class CPSDCommand : public CDeviceCommand
{
private:
  CConfiguration& m_Config;

public:
  CPSDCommand(CTCLInterpreter& interp,
	      CConfiguration&  config,
	      std::string      commandName = std::string("psd"));
  virtual ~CPSDCommand();
private:
  CPSDCommand(const CPSDCommand& rhs);
  CPSDCommand& operator=(const CPSDCommand& rhs);
  int operator==(const CPSDCommand& rhs) const;
  int operator!=(const CPSDCommand& rhs) const;
public:


private:
  virtual int create(CTCLInterpreter& interp, 
		     std::vector<CTCLObject>& objv);
protected:
  CConfiguration* getConfiguration();
};
#endif
