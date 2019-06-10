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

#ifndef __CSTACKCOMMAND_H
#define __CSTACKCOMMAND_H

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


/*!

  This class provides a command for creating and manipulating
  objects that represent CVMUSB stacks.  Stacks represent
  list of readout actions that are attached to some trigger.
  In autonomous data taking mode, these lists are downloaded
  into the VM-USB and, when their triggers fire, the associated
  list of VME operations is performed adding data to the buffer.
  Stacks represent primitive programs that read out events.

  The stack command is an ensemble that recognizes the following
  subcommands:

  \verbatim

stack create  name;                      # Create a new stack called name.
stack config  name option-value-pairs;   # modify the configuration of stack name.
stack cget    name;                      # Return the configuration of stack name.

  \endverbatim

  The config subcommand defines what the stack actually does and how it is triggered.
  it accepts a set of key value pairs where the key determines what is being configured and
  the value determines that thing is configured.  In Tcl/Tk by convention, these key/value
  pairs are called options.  The options recognized by the config subcommand are:

  \verbatim

Option           value type               Value meaning
-trigger         enumeration              Defines the trigger source:
                                          nim1   - This will be stack 0 triggered by nim1.
					  scaler - This will be stack 1 triggered by timer
					  interrupt - this is some stack 2-n (see -stack)
                                                      triggered by an interrupt.
-period           integer                 Number of seconds between scaler triggers.  This
                                          is ignored if the trigger type is not a scaler stack.
-stack            integer                 Stack number.  This is ignored unless the -trigger
                                          option is interrupt.  The stack number will determine
                                          which interrupt list register will be programmed
                                          to trigger this list.
-vector           integer                 VME Interrupt status/ID that will be used to trigger this list.
                                          This is ignored if the trigger is not interrupt.
-ipl              integer 1-7             Interrupt priority level of the interrupt that will trigger
                                          this stack.  This will be ignored if the trigger is not 
					  interrupt.
-modules          stringlist              List of ADC, Scaler, Chain modules that will be read by
                                          this stack.

  \endverbatim

Note that while no two stack can have the same number or trigger conditions this is not 
programmatically enforced.  

*/
class CStackCommand : public CTCLObjectProcessor
{
private:
  CConfiguration& m_Config;

public:
  CStackCommand(CTCLInterpreter& interp,
	      CConfiguration&  config,
	      std::string      commandName = std::string("stack"));
  virtual ~CStackCommand();
private:
  CStackCommand(const CStackCommand& rhs);
  CStackCommand& operator=(const CStackCommand& rhs);
  int operator==(const CStackCommand& rhs) const;
  int operator!=(const CStackCommand& rhs) const;
public:

  // command entry point:
protected:
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

  CConfiguration* getConfiguration();

private:
  virtual int create(CTCLInterpreter& interp, 
		     std::vector<CTCLObject>& objv);
  virtual int config(CTCLInterpreter& interp,
		     std::vector<CTCLObject>& objv);
  virtual int cget(CTCLInterpreter& interp,
		   std::vector<CTCLObject>& objv);
  virtual void Usage(std::string msg, std::vector<CTCLObject>& objv);
};
#endif
