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
 *  @brief: SBSReadout framework's runstate command.
 */
#ifndef CRUNSTATECOMMAND_H
#define CRUNSTATECOMMAND_H
#include <TCLObjectProcessor.h>

class RunState;
class CTCLInterpreter;
class CTCLObject;

/**
 * @class CRunStateCommand
 *     This class provides the 'runstate' command.  This is a Readout
 *     standard command required by the REST interface plugin.  It
 *     returns the current run state which will be (for SBSReadout)
 *     one of the following:
 *     -   idle - the run is not active.
 *     -   active - data taking is in progress.
 *     -   paused - the run is active but data taking is paused.
 *
 *     These values have been set by the VMUSB runstate command which is the
 *     first of its name and therefore we retain compatibility.
 *     Unfortunately, these are not the states returned by
 *     RunState::stateName so we'll have to duplicate that effort here.
 */
class CRunStateCommand : public CTCLObjectProcessor {
private:
    RunState&  m_State;
public:
    CRunStateCommand(
        CTCLInterpreter& interp, const char* command, RunState* pState
    );
    virtual ~CRunStateCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif