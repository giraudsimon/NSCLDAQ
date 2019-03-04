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

#ifndef __CCAENV830COMMAND_H
#define __CCAENV830COMMAND_H

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
    This class creates and configures CAEN V830 scaler modules.
    The command supports the following syntaxes:
\verbatim
    v830 create module-name base-address
    v830 config module-name config_params...
    v830 cget   module-name
\verbatim

   The v830 create form is used to creat a new adc module.
   \em base-address is the VME base address configured into the module rotary
   switches.

   v830 config is used to configure a module that has been created.
   \em module-name is the name of the module given to the create subcommand.
   \em config_params... are a set of keyword value pairs.  The keyword defines
   what is being configured, and the value is the value of that configuration
   item.  Configuration keywords supported are:

\verbatim
v830 config name option value ?...?

where the options/values are:

Name        Type/range      Default       Purpose
-channels   integer         0xffffffff    Selectively enable channels.
-dwelltime  integer         0             If in periodic trigger mode, the time between triggers
-header     bool            false         If true data will include a header.
-trigger    random |        vme           Specifies how the module triggers an event. random
            periodic |                    means the external latch signal latches the scalers
            vme                           to an event, periodic means the scaler is triggered every
                                          'dwelltime' * 400ns.  vme means the readout list triggers
                                          the scaler.
-wide      bool             true          true  - Scalers are 32 bit value, false, 26 bit values.
                                          note that 26 bit wide scaler readout is tagged in the
                                          top bits with the channel number.
-header    bool              false        If true scaler data includes a header as described in
                                          fig 3.1 of the manual.
-autoreset bool              true         If true, the latch operation also clears the counters.
-geo       integer (0-0x1f)  0            The geograhpical address of the module (from PAUX or
                                          as programmed, see below).
-setgeo    bool              false        If true, the geographical address is programmed,
                                          this can only be done for modules without the PAUX
                                          connector...otherwise, the geo address comes from
                                          the PAUX connector.
-ipl       integer 0-7       0            If non zero enables the module to interrupt when
                                          the highwater mark is reached.
-vector    integer 0-255     0xdd         Status/id value to interrupt on.
-highwatermark integer (0-0xffff) 1       How many events need to be in the module to interrupt.

\endverbatim

   The v830 cget command returns the entire module configuration for \em module-name

   example:
\verbatim
v830 create scaler1 0x800000
v830 config scaler1 -ipl 6 -vector 0x80 -supressrange 10
puts [v830 cget scaler1]
\endverbatim

*/
class CCAENV830Command : public CDeviceCommand
{
private:
  CConfiguration& m_Config;

public:
  CCAENV830Command(CTCLInterpreter& interp,
	      CConfiguration&  config,
	      std::string      commandName = std::string("v830"));
  virtual ~CCAENV830Command();
private:
  CCAENV830Command(const CCAENV830Command& rhs);
  CCAENV830Command& operator=(const CCAENV830Command& rhs);
  int operator==(const CCAENV830Command& rhs) const;
  int operator!=(const CCAENV830Command& rhs) const;
public:

  // command entry point:
protected:


  CConfiguration* getConfiguration();

private:
  virtual int create(CTCLInterpreter& interp, 
		     std::vector<CTCLObject>& objv);
};
#endif
