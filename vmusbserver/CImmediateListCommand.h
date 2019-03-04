/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __CIMMEDIATELISTCOMMAND_H
#define __CIMMEDIATELISTCOMMAND_H

#include <TCLObjectProcessor.h>


// Forward declarations:

class CTCLInterpeter;
class CTCLObject;
class CVMUSBModule;
class CVMUSB;

/**
 * @file  CImmediateListCommand.h
 * @class CImmdediateListCommand
 *
 *  This class provides a command that accesss the VM-USB Immediate
 *  list functionality.  The form of this command is
 * \verbatim
 *  vmusb immediateList maxRead list
 *  vmusb actionWrite   value
 * \endverbatim
 * Where:
 *   -  maxRead - are the maximum number of bytes of data that can be returned 
 *                from the VM-USB as a result of this list execution.
 *   -  list    - is a Tcl formatted list that is the VM-USB list to perform.
 *   -  value   - is a value to write.
 */
class CImmediateListCommand : public CTCLObjectProcessor
{
  // Internal data:

private:
  CVMUSB*       m_pController;
  CVMUSBModule* m_pExecutor;
 
  // Available canonicals:

public:
  CImmediateListCommand(CTCLInterpreter& interp, std::string command="vmusb",
			const char* serial = NULL);
  virtual ~CImmediateListCommand();

  // forbidden canonicals:

private:
  CImmediateListCommand(const CImmediateListCommand&);
  CImmediateListCommand& operator=(const CImmediateListCommand&);
  int operator==(const CImmediateListCommand&) const;
  int operator!=(const CImmediateListCommand&) const;

  // Virtual overrides - specifically, the command entry point:

public:
  virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

  // commane executors:

 private:
  int immediateList(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
  int writeActionRegister(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
  int getSerialNum(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
  int loadList(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
  int bulkRead(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

  // utility methods:

private:
  CTCLObject* makeRequestList(CTCLInterpreter& interp, int readBytes, CTCLObject& list);
  CVMUSB*     createController(const char* serial);
  void        destroyMembers();

};


#endif
