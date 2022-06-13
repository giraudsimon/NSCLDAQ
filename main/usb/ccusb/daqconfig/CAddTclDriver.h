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
#ifndef __CADDTCLDRIVER_H
#define __CADDTCLDRIVER_H

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
class CReadoutModule;
class CConfiguration;

/**
 * This class supports adding a Tcl command ensemble as a device support module instance.
 * The base command is wrapped in a CTclDriver object and registered with the configuration
 * as a digitizer device with a name that matches the name of the command ensemble.
 * The intent is that one writes device support classes/types using one of the Tcl 
 * object oriented extensions such as itcl or snit and then create instances for each
 * physical device you support, configuring the device and then invoking this command to add
 * it to the set known by the readout software.  The readout software will then use the wrapper
 * to invoke Initialize and addReadoutList subcommands of the registered command to perform
 * those deeds at the appropriate time for instances that were added to stacks.
 *
 * \verbatim
 *   addtcldriver command-base-name
 */

class CAddTclDriver : public CTCLObjectProcessor
{
private:
  CConfiguration& m_Config;	// The global configuration object.

  // Allowed canonicals:
public:
  CAddTclDriver(CTCLInterpreter& interp,
		CConfiguration&  config,
		std::string      commandName = std::string("addtcldriver"));
  virtual ~CAddTclDriver();

  // Forbidden canonicals:
private:
  CAddTclDriver(const CAddTclDriver&); // copy construtor.
  CAddTclDriver& operator+(const CAddTclDriver&); // assignment.
  int operator==(const CAddTclDriver&);		  // equality compare.
  int operator!=(const CAddTclDriver&);		  // inequality compare.

  // public members:

  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);
};

#endif
