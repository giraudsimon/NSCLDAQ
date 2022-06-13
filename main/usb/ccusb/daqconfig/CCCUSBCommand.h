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
#ifndef __CCCUSBCOMMAND_H
#define __CCCUSBCOMMAND_H

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
  Provides the Tcl command extension for creating/manipulating and querying a
  CCUSB controller object
  Syntax:
\verbatim
   This command is an ensemble of the form:
   ccusb create name option-value-pairs
   ccusb config name option-value-pairs
   ccusb cget   name
\endverbatim

Note that the CC-USB does not have a -slot parameter, unlike all the other modules since the object
only controls the internal resources of the module accessed directly via USB.


*/
class CCCUSBCommand : public CTCLObjectProcessor
{
private:
  CConfiguration& m_Config;
  
  // Canonicals
  //    Implemented:

public:
  CCCUSBCommand(CTCLInterpreter& interp,
		CConfiguration& config,
		std::string commandName = std::string("ccusb"));
  virtual ~CCCUSBCommand();

  //    Unimplemented
private:
  CCCUSBCommand(const CCCUSBCommand& rhs);
  CCCUSBCommand& operator=(const CCCUSBCommand& rhs);
  int operator==(const CCCUSBCommand& rhs) const;
  int operator!=(const CCCUSBCommand& rhs) const;

  // public member functions:

public:
  CConfiguration* getConfiguration();
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

  // Processors for the subcommands are private.
public:

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
