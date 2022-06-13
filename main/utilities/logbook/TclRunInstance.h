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

/** @file:  TclRunInstance.h    
 *  @brief: Define a class to do a Tcl object wrapping of logbook runs.
 */
#ifndef TCLRUNINSTANCE_H
#define TCLRUNINSTANCE_H
#include <TCLObjectProcessor.h>

#include <memory>
#include <map>
#include <string>

#include "LogBookRun.h"
#include <TCLObject.h>

class CTCLInterpreter;
class LogBook;
class LogBookShift;


/**
 * @class TclRunInstance
 *    This class provides an object encapsulation of a logbook run object.
 *    it supports the following methods:
 *
 *    - destroy
 *    - id - get the id of the run.
 *    - number - get the run number.
 *    - title  - get the run title.
 *    - transitions - return a list of dicts that contain all transitions. Each
 *                    dict has the keys:
 *                    -  id - transition id in the database table.
 *                    - transition - Transition integer code.
 *                    - transitionName - name of the transition (e.g. BEGIN)
 *                    - transitionTime - [clock seconds] at which the transition was logged.
 *                    - transitionComment - Comment logged with the transition.
 *                    - shift - command wrapping the shift for the transition.
 *       The elements of the list will be time ordered and, therefore, reflect the
 *       evolution of the run.
 *     - isActive -  Boolean true if the run is still active (not ended or
 *                  emergency stopped).
 */
class TclRunInstance : public CTCLObjectProcessor
{
private:
    LogBookRun*   m_pRun;
    std::shared_ptr<LogBook> m_logbook;
    static std::map<std::string, TclRunInstance*> m_instanceRegistry;
public:
    TclRunInstance(
        CTCLInterpreter& interp, const char* name, LogBookRun* pRun,
        std::shared_ptr<LogBook>& book
    );
    virtual ~TclRunInstance();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    LogBookRun* getRun() {return m_pRun;}
    void setRun(LogBookRun* pRun) {m_pRun = pRun;} 
public:
    static TclRunInstance* getCommandObject(const std::string& name);
private:
    void id(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void title(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void number(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void transitions(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void isActive(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
private:
    CTCLObject makeTransitionDict(
        CTCLInterpreter& interp, const LogBookRun::Transition& transition
    );
    std::string copyShift(CTCLInterpreter& interp, LogBookShift* shift);
};

#endif