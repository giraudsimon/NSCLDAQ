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
public:
    TclPersonInstance(
        CTCLInterpreter& interp, const char* name, LogBookPerson* pPerson
    );
    virtual ~TclPersonInstance();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};


#endif