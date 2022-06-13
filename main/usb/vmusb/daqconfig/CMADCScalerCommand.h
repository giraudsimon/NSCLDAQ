
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
#ifndef __CMADCSCALERCOMMAND_H
#define __CMADCSCALERCOMMAND_H


#ifndef __TCLOBJECTPROCESSOR_H
#include <TCLObjectProcessor.h>
#endif

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
class CReadoutModule;

/*!
  The madcscaler command creates and configures Mesytec adc modules
  so that tht daq time and time couners are read as scalers.

  The format of this command is:

\verbatim
   madcscaler  create module ?config_params?
   madcscaler  config module ?config_param?
   madcscaler  cget   module
\endverbatim
  

   - create is used to create a new module, and optionally configure it.
   - config is used to configure an existing module
   - cget returns a module's configuration as a property list.

   See the CMADCScaler.h header for information about the configuration options supported.

*/

class CMADCScalerCommand : public CTCLObjectProcessor
{
private:
  CConfiguration& m_Config;

  // Constructors and canonicals.
public:
  CMADCScalerCommand(CTCLInterpreter& interp,
	       CConfiguration& config,
	       std::string commandName = std::string("madcscaler"));
  virtual ~CMADCScalerCommand();

private:
  CMADCScalerCommand(const CMADCScalerCommand& rhs);
  CMADCScalerCommand& operator=(const CMADCScalerCommand& rhs);
  int operator==(const CMADCScalerCommand& rhs) const;
  int operator!=(const CMADCScalerCommand& rhs) const;

  // Implementng the normal command interface:

public:

  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

  CConfiguration* getConfiguration();


public:
  int create(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int config(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int cget(CTCLInterpreter& interp,
	   std::vector<CTCLObject>& objv);
  void Usage(std::string msg, std::vector<CTCLObject>& objv);

private:
  int configure(CTCLInterpreter& interp,
		std::vector<CTCLObject>& objv,
		CReadoutModule* pModule,
		int startAt = 3);


};




#endif
