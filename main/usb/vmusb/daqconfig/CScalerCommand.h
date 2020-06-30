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

#ifndef CSCALERCOMMAND_H
#define CSCALERCOMMAND_H

#include "CDeviceCommand.h"
#include <vector>
#include <string>



class CTCLInterpreter;
class CTCLObject;
class CConfiguration;

/*!
   This class creates and configures SIS3820 scalers for the LLNL neutrons daq
   system.  The command supports the following syntaxes:
\verbatim
   sis3820 create module-name base-address
\endverbatim
    Unlike the 'normal' scripted readout. this software will read all defined
    scalers. Currently scaler stacks are supported as first class stacks.
    This module does not require configuration.

*/
class CScalerCommand : public CDeviceCommand
{
  CConfiguration&     m_Config;
public:
  CScalerCommand(CTCLInterpreter& interp, CConfiguration& config);
  virtual ~CScalerCommand();
private:
  CScalerCommand(const CScalerCommand& rhs);
  CScalerCommand& operator=(const CScalerCommand& rhs);
  int operator==(const CScalerCommand& rhs) const;
  int operator!=(const CScalerCommand& rhs) const;
public:

  // Command entry point:

  virtual int create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
  

};
   

#endif
