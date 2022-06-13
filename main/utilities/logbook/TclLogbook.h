/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  TclLogbook.h
 *  @brief: Tcl logbook::logbook command processor header.
 */
#ifndef TCLLOGBOOK_H
#define  TCLLOGBOOK_H
#include <TCLObjectProcessor.h>
#include <string>

/**
 * @class TclLogbook
 *    This class defines and implements the logbook::logbook command
 *    This command implements the Tcl encapsulation of the API defined in the
 *    LogBook.h header encapsulating it in a Tcl command ensemble.  Ensemble
 *    sub-commands are:
 *
 *  -  create filename experiment spokesperson purpose
 *  -  open filename
 *  -  tempdir
 *
 *  The open command returns a new command in the logbook namespace that is itself
 *  a command ensemble that encapsulates the public object methods of the
 *  LogBook class. Note that these methods may, in turn create commands that
 *  represent objects that the logbook can produce.   One subcommand each
 *  object command includes is 'destroy' which eliminates the wrapper command
 *  and destroys any resources consumed by the object.
 *
*/
class TclLogbook : public CTCLObjectProcessor
{
private:
    static int m_instanceCounter;              // Used to make unique object names.
public:
    TclLogbook(CTCLInterpreter* pInterp, const char* pCommand);
    virtual ~TclLogbook();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    static std::string createObjectName(const char* prefix);
    static std::string reconstructCommand(std::vector<CTCLObject>& objv);
private:
    void create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void open(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void tempdir(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
private:
    
    std::string usage(std::vector<CTCLObject>& objv, const char* msg);
};

#endif