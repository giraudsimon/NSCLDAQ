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
#ifndef __CUSERCOMMAND_H
#define __CUSERCOMMAND_H

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
class CReadoutHardware;

/*!
   This class provides support for generic user written device drivers in C++
   A new device driver is implemented in a Tcl Loadable package (.so).  The
   xxx_Init for that package will then invoke 

   \verbatim
     CxxDriver* pTemplate = new CxxDriver;
     CUserCommand::addDriver("xxx", pTemplate);
   \endverbatim

   Where CxxDriver is derived from CReadoutHardware, xxx is the new Tcl command 
   ensemble that's created to run the driver from the configuration script and
   pTemplate is duplicated each time a driver instance is required.

\verbatim
   This command is an ensemble of the form:
   xxx create name -slot n
   xxx config name option-value-pairs
   xxx cget   name
\endverbatim


*/
class CUserCommand : public CTCLObjectProcessor
{
private:
  CConfiguration&   m_Config;	// This is the global configuration of devices.
  CReadoutHardware* m_pTemplate;
  

  // Allowed canonicals
public:
  CUserCommand(CTCLInterpreter& interp,
	       CConfiguration& config,
	       std::string     commandName, CReadoutHardware* pTemplate);
  virtual ~CUserCommand();

  // Forbidden canonicals:
private:
  CUserCommand(const CUserCommand& rhs);
  CUserCommand& operator=(const CUserCommand& rhs);
  int operator==(const CUserCommand& rhs) const;
  int operator!=(const CUserCommand& rhs) const;
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

public:
  // static entry that allows packages to register drivers with commands:

  static void addDriver(std::string command, CReadoutHardware* pTemplateObject);

};

#endif
