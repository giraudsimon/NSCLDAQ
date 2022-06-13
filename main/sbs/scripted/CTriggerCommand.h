/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef CTRIGGERCOMMAND_H
#define CTRIGGERCOMMAND_H

#include <TCLProcessor.h>
#include <string>

// Forward class definitions.

class CCAENModule;
class CCAENTrigger;
class CTCLInterpreter;
class CTCLResult;
class CDigitizerDictionary;

/*!
   This module provides a tcl command that allows us to 
   select a trigger.  The trigger module operates at define time
   as well as at run initiation time:
   - At define time the user issues a command like

      trigger modulename 

     where modulename is the name of a module in the module dictionary.
     At this point, the only thing he trigger module does is locate this
     module or complain if it does not exist.

  - At run initiation time, the trigger module locates the underlying
    CCAEN card module driver and instantiates a CCAENTrigger object,
    and uses it to substitute for the Readout's normal trigger system.
    If the CAEN card module driver has not been instantiated yetm, this
    is also an error.

*/
class CTriggerCommand : public CTCLProcessor
{
private:
  CDigitizerDictionary* m_pDictionary;
  CCAENModule*       m_pModule;
  CCAENTrigger*      m_pTrigger;
public:
  CTriggerCommand(const std::string&      rCommand, 
		  CTCLInterpreter&   rInterp,
		  CDigitizerDictionary* pDictionary);
  virtual ~CTriggerCommand();
private:
  CTriggerCommand(const CTriggerCommand& rhs);
  CTriggerCommand& operator= (const CTriggerCommand& rhs);
  int              operator==(const CTriggerCommand& rhs);
  int              operator!=(const CTriggerCommand& rhs);
public:

  // Selectors:

  CCAENModule* getModule() {
    return m_pModule;
  }
  CCAENTrigger* getTrigger() {
    return m_pTrigger;
  }
  CDigitizerDictionary* getDictionary() {
    return m_pDictionary;
  }

  // Mutators:
protected:
  void setModule(CCAENModule* pModule) {
    m_pModule = pModule;
  }
  void setTrigger(CCAENTrigger* pTrigger) {
    m_pTrigger = pTrigger;
  }
  void setDictionary(CDigitizerDictionary* pDict) {
    m_pDictionary = pDict;
  }

  // class operations:

public:
  int operator()(CTCLInterpreter& rInterp, 
		 CTCLResult&      rResult,
		 int argc, char** argv);
  void Initialize();

private:
  void Usage(CTCLResult& rResult);

};


#endif
