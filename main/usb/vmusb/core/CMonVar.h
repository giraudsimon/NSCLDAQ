/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#ifndef CMONVAR_H
#define CMONVAR_H

/** @file:  CMonVar.h
 *  @brief: Create/maintain monitored variables.
 */

#include <list>
#include <TCLTimer.h>
#include <TCLObjectProcessor.h>

class CTCLInterpreter;

/**
 * @class CMonvarDictionary
 * 
 *   Monitored variables are  maintained in the monitored variable dictionary.
 *   This singleton class maintains a dictionary of the names of the variables
 *   that are monitored.  The variables are all assumed to live in a single
 *   interpreter (usually the main).
 */

class CMonvarDictionary {
private:
    static CMonvarDictionary* m_pInstance;
private:
    std::list<std::string> m_monitoredVariableNames;

private:
    CMonvarDictionary();                         // Can't publicly.
    CMonvarDictionary(const CMonvarDictionary&);  // or copy construct.
    CMonvarDictionary& operator=(const CMonvarDictionary&); // Or assign.
    
    
public:
    static CMonvarDictionary* getInstance();
    
    void add(const std::string& variableName);
    void remove(const std::string& variableName);
    const std::list<std::string>& get() const;
    
    size_t size() const;
    std::list<std::string>::iterator begin();
    std::list<std::string>::iterator end();
    
};
/**
 * @class CMonitorVariables
 *   When a run is active, this class periodically gives the data to make 
 *   and emit CRingTextItem by way of the router.
 */
class CMonitorVariables : public CTCLTimer
{
private:

public:
    CMonitorVariables(CTCLInterpreter& interp, TCLPLUS::UInt_t msec);
    virtual void operator()();
private:
    void createItems();
    std::string createScript(const std::string& var);
    char* initBuffer(void* pBuffer);
};

/**
 * CMonvarCommand
 *    The TCL Command that manages the contents of the singleton
 *    CMonvarDictionary.   This is an ensemble with the following subcommands:
 *
 *   -   add varname - adds a variable name to the monitor list. Error if already
 *                     there.
 *   -   remove varname - Removes a monitored variable.  Error if not there.
 *   -   list   pattern - Lists all the variables that match the glob pattern.
 */

class CMonvarCommand : public CTCLObjectProcessor
{
public:
    CMonvarCommand(CTCLInterpreter& interp, const char* command = "monvar");
    virtual ~CMonvarCommand();
    
    virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
private:
    void add(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void remove(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void list(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};
#endif
