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
#ifndef __CCAEN257COMMAND_H
#define __CCAEN257COMMAND_H


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
  This class provides the command that allows users to create and
  manipulate CAEN C257 scaler modules in the daqconfig.tcl file.
  See ../devices/CCAEN257.h for information about the configuration
  options supported by these devices.

  This command is an ensemble of the form:

\verbatim
c257   create name -slot n
c257   config name option-value-pairs
c257   cget   name

\endverbatim

Note that while we ensure you don't create two devices with the same
name, we don't ensure that you don't configure two devices in the same
slot.
*/
class CCAEN257Command : public CTCLObjectProcessor
{
private:
  CConfiguration&    m_Config;	// Global device configuration.

  // Canonicals:
public:
  CCAEN257Command(CTCLInterpreter& interp,
		  CConfiguration&  config,
		  std::string      commandName = std::string("c257"));
  virtual ~CCAEN257Command();

  // Forbidden canonicals:

private:
  CCAEN257Command(CCAEN257Command& rhs);
  CCAEN257Command operator=(const CCAEN257Command& rhs);
  int operator==(const CCAEN257Command& rhs) const;
  int operator!=(const CCAEN257Command& rhs) const;
public:

  // public members like selectors and the command entry point:

public:
  CConfiguration*   getConfiguration();
  virtual int       operator()(CTCLInterpreter& interp,
			       std::vector<CTCLObject>& objv);

  // The specific subcommand functions are not exported to the world:

private:
private:
  int create(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int config(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int cget(CTCLInterpreter& interp,
	   std::vector<CTCLObject>& objv);


  // Utilities:
  
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
