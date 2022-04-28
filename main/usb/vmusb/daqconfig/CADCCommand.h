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

#ifndef __CADCCOMMAND_H
#define __CADCCOMMAND_H

#include <CDeviceCommand.h>

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
    This class creates and configures ADCs for the LLNL neutrons daq system.
    The command supports the following syntaxes:
\verbatim
    adc create module-name base-address
    adc config module-name config_params...
    adc cget   module-name
\endverbatim

   The adc create form is used to creat a new adc module.  Unlike the 'standard
   scripted readout' all adcs are used for data acquisition \em module-name
   is a name to be used to refer to the module in other adc subcommands.
   \em base-address is the VME base address configured into the module rotary
   switches.

   adc config is used to configure a module that has been created.
   \em module-name is the name of the module given to the create subcommand.
   \em config_params... are a set of keyword value pairs.  The keyword defines
   what is being configured, and the value is the value of that configuration
   item.  Configuration keywords supproted are:
   - -thresholds  The value is a list of 32 values that are the module 
                  thresholds.
   - -smallthresholds  The value is a boolean (e.g. \em on or \em off).
                  A true boolean means that the threshold is applied as is
                  a false boolean means the threshold value is multiplied by 16
                  and then applied.
   - -ipl         The interrupt priority level the module should use.
                  for the llnl neutron daq this should always be configured to be
                  6.
   - -vector      The interrupt vector the module should use. This is an integer
                  and should always be configured to be 0x80.
   - -highwater   The number of events the ADC should accumulate before it fires
                  an interrupt.  This parameter will tune the readout efficiency.
                  The important thing is to allow sufficient space that the
                  ADC will not drop (many?) events while it is being emptied by the
                  VM-USB.
   - -fastclear  The fast clear window value for the adc (see the hardware docs
                 for the module.  The integer value of this parametr is directly
                 programmed into the fast clear window register of the module.
   - -supressrange Enables zero, overflow and invalid suppression.  This is on 
                 by default and should probably only be disabled for test purposes.

   The adc cget command returns the entire module configuration for \em module-name

   example:
\verbatim
adc create adc1 0x800000
adc config adc1 -ipl 6 -vector 0x80 -supressrange 10
puts [adc cget adc1]
\endverbatim

*/
class CADCCommand : public CDeviceCommand
{
private:
  CConfiguration& m_Config;

public:
  CADCCommand(CTCLInterpreter& interp,
	      CConfiguration&  config,
	      std::string      commandName = std::string("adc"));
  virtual ~CADCCommand();
private:
  CADCCommand(const CADCCommand& rhs);
  CADCCommand& operator=(const CADCCommand& rhs);
  int operator==(const CADCCommand& rhs) const;
  int operator!=(const CADCCommand& rhs) const;
public:



  CConfiguration* getConfiguration();

private:
  virtual int create(CTCLInterpreter& interp, 
		     std::vector<CTCLObject>& objv);

};
#endif
