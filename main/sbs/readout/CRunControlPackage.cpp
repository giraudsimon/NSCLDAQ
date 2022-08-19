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

#include <config.h>
#include "CRunControlPackage.h"
#include "RunState.h"
#include "CExperiment.h"
#include <TCLInterpreter.h>
#include <TCLObjectProcessor.h>
#include <TCLApplication.h>
#include <CReadoutMain.h>
#include <StateException.h>
#include "CBeginCommand.h"
#include "CPauseCommand.h"
#include "CResumeCommand.h"
#include "CEndCommand.h"
#include "CInitCommand.h"
#include <TCLTimer.h>

using namespace std;

/*------------------------------------------------
 * Helper class: Extends TCLTimer to maintain the
 * elapsed run time in the run state.
 */
class RunTimer : public CTCLTimer
{
public:
  RunTimer(CTCLInterpreter* pInterp) :
    CTCLTimer(pInterp, 1000)
  {
    Set();
  }
  virtual void operator()() {
    RunState* pState = RunState::getInstance();
    pState->m_timeOffset++;
    Set();
  }
};

// Static member data:

CRunControlPackage*  CRunControlPackage::m_pInstance(0);

// Application global data:

extern CTCLApplication* gpTCLApplication;



/*!
  Constructs the singleton.  This is private so that nobody
  but getInstance() can invoke it.
  \param Interp Interpreter to which the commands will be bound:

*/

CRunControlPackage::CRunControlPackage(CTCLInterpreter& Interp) :
  m_pTheState(RunState::getInstance()),
  m_pTheExperiment(0),
  m_pInterp(&Interp),
  m_pTimer(0)
{

  CReadoutMain* pReadout = reinterpret_cast<CReadoutMain*>(gpTCLApplication);
  m_pTheExperiment = pReadout->getExperiment();
  createCommands(Interp);
}
/*!
   I think I May need a destructor to avoid undefined globals:

*/
CRunControlPackage::~CRunControlPackage()
{
}

/*!
    Return the singleton instance of the package,createing it if necessary:
    \param interp - interpreter on which commands are registered.

    \note while this is a true singleton, the inclusion of the interp parameter
          opens the door architecturally to a per interpreter singleton instead
          (where perhaps only the run state and experiment objects are singleton).

*/
CRunControlPackage*
CRunControlPackage::getInstance(CTCLInterpreter& interp)
{
  if (!m_pInstance) {
    m_pInstance = new CRunControlPackage(interp);
  }
  return m_pInstance;
}

/*!
   Start a new run; Run state must be halted.
*/
void
CRunControlPackage::begin()
{
  CReadoutMain* pInstance = CReadoutMain::getInstance();
  if (m_pTheState->m_state == RunState::inactive) {
    pInstance->logProgress("Asking experiment to start the run");
    m_pTheExperiment->Start(false);	// not a resume.
    pInstance->logProgress("Experiment started the run");
    m_pTimer = new RunTimer(m_pInterp);
    pInstance->logProgress("New run timer created");
  }
  else {
    pInstance->logStateChangeStatus("Attempted start of run when not inactive");
    throw CStateException(m_pTheState->stateName().c_str(),
			 RunState::stateName(RunState::inactive).c_str(),
			 "Attempting to start a run");
			 
  }
}
/*!
   Halt a run.  The state must be activfe or paused for this to be legal.
*/
void
CRunControlPackage::end()
{
  CReadoutMain* pMain = CReadoutMain::getInstance();
  RunState::State state = m_pTheState->m_state;
  if((state == RunState::active) || (state == RunState::paused)) {
    pMain->logProgress("Asking the experiment to stop the run");
    m_pTheExperiment->Stop(false);	// Not a pause.
    delete m_pTimer;
    pMain->logProgress("Experiment timer deleted");
    m_pTimer = reinterpret_cast<RunTimer*>(0);
  }
  else {
    pMain->logStateChangeStatus(
      "Failed to end a run because state was neither active nor paused"
    );
    string validstates = RunState::stateName(RunState::active);
    validstates += " or ";
    validstates += RunState::stateName(RunState::paused);
    throw CStateException(m_pTheState->stateName().c_str(),
			 validstates.c_str(),
			 "Attempting to end a run.");
  }
}
/*!
    Pause a run.  The run state must be active.
*/
void
CRunControlPackage::pause()
{
  CReadoutMain* pMain = CReadoutMain::getInstance();
  if(m_pTheState->m_state == RunState::active) {
    pMain->logProgress("Asking the experiment to stop");
    m_pTheExperiment->Stop(true);
    delete m_pTimer;
    pMain->logProgress("Deleted the experiment timer");
    m_pTimer = reinterpret_cast<RunTimer*>(0);
  }
  else {
    pMain->logStateChangeStatus(
      "Attempting to pause the run when the state is not active"
    );
    throw CStateException(m_pTheState->stateName().c_str(),
			 m_pTheState->stateName(RunState::active).c_str(),
			 "Attempting to pause a run");
  }
}

/*!
   Resume a run --state willing.  The state must be paused.
*/
void
CRunControlPackage::resume()
{
  CReadoutMain* pMain = CReadoutMain::getInstance();

  if (m_pTheState->m_state == RunState::paused) {
    pMain->logProgress("Asking the experiment to Start");
    m_pTheExperiment->Start(true);
    m_pTimer = new RunTimer(m_pInterp);
    pMain->logProgress("Created a new experiment run timer");
  }
  else {
    pMain->logStateChangeStatus("Resum attempted when run was not paused");
    throw CStateException(m_pTheState->stateName().c_str(),
			  m_pTheState->stateName(RunState::paused).c_str(),
			  "Attempting to resume a run");
  }
}


/*!
  Return the state object. This can be done via the state singleton, or
  via the run control package in case in the future the run state somehow 
  stops being a singleton.
  \return RunState*
  \retval The saved pointer to the run state object.
*/
const RunState*
CRunControlPackage::getState() const
{
  return m_pTheState;
}

/*!
  Create all the commands in the package and add them:
*/

void CRunControlPackage::createCommands(CTCLInterpreter& interp)
{
  addCommand(new CBeginCommand(interp));
  addCommand(new CPauseCommand(interp));
  addCommand(new CResumeCommand(interp));
  addCommand(new CEndCommand(interp));
  addCommand(new CInitCommand(interp));
}
