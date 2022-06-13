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

/** @file:  TclShiftInstance.h
 *  @brief: Define a command wrapped shift.
 */
#ifndef TCLSHIFTINSTANCE_H
#define TCLSHIFTINSTANCE_H
#include <TCLObjectProcessor.h>
#include <memory>
#include <map>

class CTCLInterpreter;
class CTCLObject;
class LogBookShift;
class LogBook;

/**
 * @class TclShiftInstance
 *    Wraps a logbook shift in a command ensemble. Subcommands are:
 *
 *    - name -- Returns the shift name.
 *    - id   -- Returns the  id of the shift
 *    - members - Return shift members wrapped in command instances.
 */
class TclShiftInstance : public CTCLObjectProcessor
{
private:
    std::shared_ptr<LogBookShift> m_shift;
    std::shared_ptr<LogBook>      m_logBook;
    static std::map<std::string, TclShiftInstance*> m_shifts;
public:
    TclShiftInstance(
        CTCLInterpreter& interp, const char* cmd, LogBookShift* pShift,
        std::shared_ptr<LogBook>& pLogBook
    );
    virtual ~TclShiftInstance();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    //getters:
public:
    LogBookShift* getShift() {return m_shift.get();}
public:
    static TclShiftInstance* getCommandObject(const std::string& name);
private:
    void name(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void members(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void id(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif