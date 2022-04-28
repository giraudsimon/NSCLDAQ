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
#ifndef __CPH7132COMMAND_H
#define __CPH7132COMMAND_H

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
   This class provides a command that creates the CPH7132 class.  See
   ../devices/CPH7132.h for information about the configuration options
   that device supports. 
\verbatim
   This command is an ensemble of the form:
   ph7132 create name -slot n
   ph7132 config name option-value-pairs
   ph7132 cget   name
\endverbatim

  Note that while we ensure you don't create two devices with the same name,
  we don't prevent you from putting two devices in the same slot...which could
  have amusing consequences.


*/
class CPH7132Command : public CTCLObjectProcessor
{
private:
  CConfiguration& m_Config;	// This is the global configuration of devices.

  // Allowed canonicals
public:
  CPH7132Command(CTCLInterpreter& interp,
		CConfiguration& config,
		std::string     commandName = std::string("ph7132"));
  virtual ~CPH7132Command();

  // Forbidden canonicals:
private:
  CPH7132Command(const CPH7132Command& rhs);
  CPH7132Command& operator=(const CPH7132Command& rhs);
  int operator==(const CPH7132Command& rhs) const;
  int operator!=(const CPH7132Command& rhs) const;
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
