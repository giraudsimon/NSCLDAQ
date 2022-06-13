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
class LogBookPerson;
class LogBookShift;
class LogBookRun;
class LogBookNote;

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
 *     - createShift shiftname ?list-of-person-commands-for-people-in-shift
 *     - getShift id
 *     - addShiftMember shiftCommand personCommand
 *     - removeShiftMember shifCommand personCommand
 *     - listShifts
 *     - findShift shiftname
 *     - setCurrentShift shiftname
 *     - getCurrenShift
 *
 *  API to access Runs
 *
 *     - begin number title ?remark?
 *     - end  runCommand ?remark?
 *     - pause runCommand ?remark?
 *     - resume runCommand ?remark?
 *     - emergencyStop runCommand ?remark?
 *     - listRuns
 *     - findRun run_number
 *     - runId  run-command
 *     - currentRun
 *
 * API for notes:
 *
 *     - createNote author text ?image-list offset-list? ?runCommand?
 *     - getNote id
 *     - listAllNotes
 *     - listNotesForRunId id
 *     - listNotesForRunNumber run-number
 *     - listNotesForRun run-command
 *     - listNonrunNotes
 *     - getNoteRun note-command
 *     - getNoteAuthor note-command
 *     - kvExists key
 *     - kvGet    key
 *     - kvSet    key value
 *     - kvCreate key value
 */
class TclLogBookInstance : public CTCLObjectProcessor
{
private:
    std::shared_ptr<LogBook> m_logBook;
    int                      m_commandIndex;
public:
    TclLogBookInstance(CTCLInterpreter* pInterp, const char* cmd, LogBook* pBook);
    TclLogBookInstance(
        CTCLInterpreter* pInterp, const char* cmd, std::shared_ptr<LogBook> book
    );
    virtual ~TclLogBookInstance();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
private:
    void addPerson(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void findPeople(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void listPeople(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void getPerson(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    void createShift(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void getShift(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void addShiftMember(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void removeShiftMember(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void listShifts(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void findShift(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void setCurrentShift(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void getCurrentShift(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    void beginRun(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void endRun(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void pauseRun(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void resumeRun(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void emergencyEnd(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void listRuns(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void findRun(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void runId(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void currentRun(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    void createNote(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void getNote(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void listAllNotes(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void listNotesForRunId(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void listNotesForRunNumber(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv
    );
    void listNotesForRun(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void listNonRunNotes(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void getNoteRun(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void getNoteAuthor(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    void kvExists(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void kvGet(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void kvSet(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void kvCreate(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
public:
    static std::string wrapPerson(CTCLInterpreter& interp, LogBookPerson* pPerson);
    std::string wrapShift(CTCLInterpreter& interp, LogBookShift* pShift);
    std::string wrapRun(CTCLInterpreter& interp, LogBookRun* pRun);
private:
    
    std::string wrapNote(CTCLInterpreter& interp, LogBookNote* pNote);
    std::pair<std::vector<std::string>, std::vector<size_t>>
        getImageInformation(
            CTCLInterpreter& interp, CTCLObject& images, CTCLObject& offsets
        );
    void notesVectorToResultList(
        CTCLInterpreter& interp, const std::vector<LogBookNote*>& pNotes
    );
};

#endif