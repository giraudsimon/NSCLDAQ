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
#ifndef __C1205COMMAND_H
#define __C1205COMMAND_H

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
  This class provides a command that can create manipulate and query
  the configuration of a CC1205 module via the configuration Tcl interpreter.
  The CC1205, in turn supports reading the CAEN C1205 QDC module.
  For more information about how this module is supported, see
  ../devices/CC1205.h

  This class implements a command ensemble:
\verbatim
  c1205   create name -slot n
  c1205   config name option-value-pairs
  c1205   cget   name

    
\endverbatim
Where:
   - name is the name used to refer to the module.  The create command
     defines a module name and associates it with a module. config and cget
     use that name to refer to the underlying module.
   - option-value-pairs  are pairs of command line parameters. The first element
     of each pair is a configuration parameter name, the second its value.
     the -slot n  in the create command is an example of an option-value-pair.

  Note that while we ensure you don't create two devices with the same name,
  we don't prevent you from putting two devices in the same slot...which could
  have amusing consequences.

*/
class C1205Command : public CTCLObjectProcessor
{
private:
  CConfiguration&   m_Config;	// reference to the global device configuration.

  // Canonicals:
public:
  C1205Command(CTCLInterpreter& interp,
		CConfiguration& config,
		std::string     commandName = std::string("c1205"));
  ~C1205Command();

  // Forbidden canonicals:
private:
  C1205Command(const C1205Command& rhs);
  C1205Command& operator=(const C1205Command& rhs);
  int operator==(const C1205Command& rhs) const;
  int operator!=(const C1205Command& rhs) const;

  // Remaining public members:

public:
  CConfiguration*   getConfiguration();
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

  // The following private members actually do the detailed subcommand
  // processing:
private:
  int create(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int config(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int cget(CTCLInterpreter& interp,
	   std::vector<CTCLObject>& objv);


  // Simple Utilities:

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
