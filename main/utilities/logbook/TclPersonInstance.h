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

/** @file:  TclPersonInstance.h
 *  @brief: Encapsulate a logBookPerson object 
 */

#ifndef TCLPERSONINSTANCE_H
#define TCLPERSONINSTANCE_H
#include <TCLObjectProcessor.h>
#include <memory>
#include <map>

class LogBookPerson;

/**
 * @class TclPersonInstance
 *    Encapsulates the logBookPerson object this is a subcommand with getter
 *    methods and a destroy subcommand:
 *
 *    - destroy - destroys the command ensemble.
 *    - lastName  - returns the last name of the person encapsulated.
 *    - firstName - returns the first name of the person encapsulated.
 *    - salutation - returns the person's salutation (may be an empty string).
 *    - id        - returns the person's primary key (id).
 */
class TclPersonInstance : public CTCLObjectProcessor
{
private:
    std::shared_ptr<LogBookPerson> m_person;
    static std::map<std::string, TclPersonInstance*> m_personCommands;
public:
    TclPersonInstance(
        CTCLInterpreter& interp, const char* name, LogBookPerson* pPerson
    );
    virtual ~TclPersonInstance();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    // getters:
public:
    LogBookPerson* getPerson() { return m_person.get(); }
public:
    static TclPersonInstance* getCommandObject(const std::string& name);
    
private:
    void lastName(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void firstName(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void salutation(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void id(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};


#endif