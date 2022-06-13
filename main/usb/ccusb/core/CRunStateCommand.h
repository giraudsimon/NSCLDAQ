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

/** @file:  CRunStateCommand.h
 *  @brief: Provides the 'runstate' command.
 */
#ifndef CRUNSTATECOMMAND_H
#define CRUNSTATECOMMAND_H
#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CRunStateCommand
 *    Provides a 'runstate' command that allows Tcl scripts,  such as the REST
 *    plugin to determine the state of the run.  The valid return values from
 *    'runstate' are standard across all Readout frameworks and are the strings:
 *    -  idle   - No run is active.
 *    -  active - The run is active and data taking is in progress.
 *    -  paused - The run is acxtive but data taking is paused.
 */
class CRunStateCommand : public CTCLObjectProcessor
{
public:
    CRunStateCommand(CTCLInterpreter& interp, const char* command);
    virtual ~CRunStateCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};


#endif