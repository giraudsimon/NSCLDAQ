#ifndef CREADOUTMAIN_H
#define CREADOUTMAIN_H

/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <TCLApplication.h>
#include <string>
#include "options.h"

// Forward class definitions.

class CTCLServer;
class CTCLInterpreter;
class CExperiment;

/*!

  This class is the entry point class for the
  production readout software.   It is  subclassed
  (see Skeleton.cpp) by the user who must provide the
  SetupReadout virtual member.

*/

class CReadoutMain : public CTCLApplication
{
private:
  CTCLServer*   m_pTclServer;
  CExperiment*  m_pExperiment;
  std::string   m_logFile;          // Empty means no logging.
  unsigned      m_nDebugLevel;      // 0, 1, 2
public:
  CReadoutMain();
  virtual ~CReadoutMain();


  // Selectors:

  CTCLServer*  getTclServer();
  static CExperiment* getExperiment();
  static CReadoutMain* getInstance();
  
  unsigned getDebugLevel() const {
    return m_nDebugLevel;
  }
  
   // These methods allow logging; they do the filtering as needed.
  
  void logStateChangeRequest(const char* message);
  void logStateChangeStatus(const char* message);
  void logProgress(const char* message);
  
  // Entry point

  virtual int operator()();
protected:
  virtual CExperiment*  CreateExperiment(void* parsed);
  virtual void          SetupRunVariables(CTCLInterpreter* pInterp);
  virtual void          SetupStateVariables(CTCLInterpreter* pInerp);
  virtual void          SetupReadout(CExperiment* pExperiment);
  virtual void          SetupScalers(CExperiment* pExperiment);
  virtual void          addCommands(CTCLInterpreter* pInterp);
  virtual void          addCommands();
  

  
  

protected:
  void setupLogging();
  void startTclServer(std::string port);
  std::string getRingName(struct gengetopt_args_info& arguments);
};

#endif
