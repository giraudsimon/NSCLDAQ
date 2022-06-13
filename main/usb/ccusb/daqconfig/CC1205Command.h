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
#ifndef __CC1205COMMAND_H
#define __CC1205COMMAND_H

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
  This class provides a command that generates configures and inquires about a 
  CAEN C1205 16 channel QDC module.   See ../devices/CC1205.h for information
  about the configuration parameters supported by this command.  
 
  As is usual, this command implements an ensemble command:

\verbatim
  c1205 create name -slot n
  c1205 config name option-value-pairs
  c1205 cget   option-name
\endverbatim

While we ensure that each device name is unique it is the responsibility of the 
person writing the config file to ensure that each module is in a separate slot.
*/

class CC1205Command : public CTCLObjectProcessor
{
private:
  CConfiguration& m_Config;	// This is the global configuration of devices.

  // Allowed canonicals
public:
  CC1205Command(CTCLInterpreter& interp,
		CConfiguration& config,
		std::string     commandName = std::string("c1205"));
  virtual ~CC1205Command();

  // Forbidden canonicals:
private:
  CC1205Command(const CC1205Command& rhs);
  CC1205Command& operator=(const CC1205Command& rhs);
  int operator==(const CC1205Command& rhs) const;
  int operator!=(const CC1205Command& rhs) const;
public:


  // Public members like selectors and the command entry point:

  CConfiguration* getConfiguration();
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

  // Processors for the individual ensemble subcommands.
private:
  int create(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int config(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int cget(CTCLInterpreter& interp,
	   std::vector<CTCLObject>& objv);


  // Utitilities:

private:
  virtual void Usage(std::string msg, std::vector<CTCLObject> objv);
  int    configure(CTCLInterpreter&         interp,
		   CReadoutModule*          pModule,
		   std::vector<CTCLObject>& config,
		   int                      firstPair = 3);
  std::string configMessage(std::string base,
			    std::string key,
			    std::string value,
			    std::string errorMessage);
};

#endif
