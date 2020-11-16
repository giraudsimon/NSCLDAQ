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

/** @file:  TclLogbookInstance.h
 *  @brief: Provides a logbook instance command.
 */
#ifndef TCLLOGBOOKINSTANCE_H
#define TCLLOGBOOKINSTANCE_H
#include "TCLObjectProcessor.h"
#include <memory>
#include <string>
class LogBook;

/**
 * @class TclLogBookInstance
 *   This command processor encapsulates a single instance of a logbook and
 *   provides a command ensemble that allows access to the object methods of
 *   the LogBook class.  Note that:
 *
 *   - Some of these methods will produce additional command objects.
 *   - The order in which command objects are destroyed is immaterial as
 *     they are held in std::shared_ptr smart pointers so that the underlying
 *     objecdts won't get destroyed until the last reference is gone.
 *
 *  Subcommands are:
 *
 *    - destroy - destroys this command and all other instance data.
 *
 *    API to access LogBookPerson objects:
 *  
 *     - addPerson lastname firstname salutation
 *     - findPeople ?whereclause?
 *     - listPeople
 *     - getPerson id
 *
 *   API to access LogBookShift objects:
 *
 *     - getShift id
 *     - addShiftMember shiftCommand personCommand
 *     - removeShiftMember shifCommand personCommand
 *     - createShift shiftname ?list-of-person-commands-for-people-in-shift
 *
 *  API to access Runs
 *
 *     - runId  run-command
 *     - currentRun
 *     - begin number title ?remark?
 *     - end  runCommand ?remark?
 *     - pause runCommand ?remark?
 *     - resume runCommand ?remark?
 *     - emergencyStop runCommand ?remark?
 *     - listRuns
 *     - findRun run_number
 *
 * API for notes:
 *
 *     - createNote author text image-list offset-list ?runCommand?
 *     - getNote id
 *     - listAllNotes
 *     - listNotesForRunId id
 *     - listNotesForRunNumber run-number
 *     - listNotesForRun run-command
 *     - listNonrunNotes
 *     - getNoteRun note-command
 *     - getNoteAuthor note-command
 */
class TclLogBookInstance : public CTCLObjectProcessor
{
private:
    std::shared_ptr<LogBook> m_logBook;
    int                      m_commandIndex;
public:
    TclLogBookInstance(CTCLInterpreter* pInterp, const char* cmd, LogBook* pBook);
    virtual ~TclLogBookInstance();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
private:
    void addPerson(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
};

#endif