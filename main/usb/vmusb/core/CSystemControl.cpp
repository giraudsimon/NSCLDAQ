

#include "CSystemControl.h"
#include "tclUtil.h"
#include <CBeginRun.h>
#include <CEndRun.h>
#include <CPauseRun.h>
#include <CResumeRun.h>
#include <CInit.h>
#include <CExit.h>
#include <Globals.h>
#include <event.h>

#include <TCLLiveEventLoop.h>
#include <TCLInterpreter.h>
#include <Exception.h>
#include <stdexcept>
#include <string>
#include <CErrnoException.h>
#include <CMonVar.h>
#include <CRunState.h>
#include "CStatisticsCommand.h"

#include <tcl.h>
#include <unistd.h>
#include <stdlib.h>

#include <iostream>

using namespace std;

// static data member initialization
//
string CSystemControl::m_initScript;
unique_ptr<CBeginRun>  CSystemControl::m_pBeginRun;
unique_ptr<CEndRun>    CSystemControl::m_pEndRun;
unique_ptr<CPauseRun>  CSystemControl::m_pPauseRun;
unique_ptr<CResumeRun> CSystemControl::m_pResumeRun;
unique_ptr<CInit>      CSystemControl::m_pInit;
unique_ptr<CExit>      CSystemControl::m_pExit;
unique_ptr<CMonvarCommand>    CSystemControl::m_pMonVar;
unique_ptr<CStatisticsCommand> CSystemControl::m_pStats;


// The entry point
void CSystemControl::run(int argc, char** argv) 
{
  Tcl_Main(argc, argv, CSystemControl::AppInit);
}


void CSystemControl::setInitScript(const string& path)
{
  m_initScript = path;
}

/*
   Initialize the interpreter.  This invoves:
   - Wrapping the interpreter into a CTCLInterpreter Object.
   - Creating the commands that extend the interpreter.
   - Returning TCL_OK so that the interpreter will start running the main loop.

*/
int CSystemControl::AppInit( Tcl_Interp* interp) 
{
  Globals::mainThreadId     = Tcl_GetCurrentThread();

  Tcl_Init(interp);		// Get all the paths etc. setup.

  Globals::pMainInterpreter = new CTCLInterpreter(interp);

  m_pBeginRun.reset(new CBeginRun(*Globals::pMainInterpreter));
  m_pEndRun.reset(new CEndRun(*Globals::pMainInterpreter));
  m_pPauseRun.reset(new CPauseRun(*Globals::pMainInterpreter));
  m_pResumeRun.reset(new CResumeRun(*Globals::pMainInterpreter));
  m_pInit.reset(new CInit(*Globals::pMainInterpreter));
  m_pExit.reset(new CExit(*Globals::pMainInterpreter));
  m_pMonVar.reset(new CMonvarCommand(*Globals::pMainInterpreter));
  m_pStats.reset(new CStatisticsCommand(*Globals::pMainInterpreter, "statistics"));
  
  CMonitorVariables* pMon = new CMonitorVariables(*Globals::pMainInterpreter, 2000);
  CRunState::getInstance()->setVarMonitor(pMon);
  
  // If there's an initialization script then run it now:
  
  if (m_initScript != "") {
    bool ok(true);
    std::string msg;
    if (access(m_initScript.c_str(), R_OK) == 0) {
      try {
            Globals::pMainInterpreter->EvalFile(m_initScript.c_str());
      } catch (CException& e) {
        msg = e.ReasonText();
        ok = false;
      } catch (std::exception& e) {
        msg = e.what();
        ok = false;
      }
      catch (std::string m) {
        msg = m;
        ok = false;
      }
      catch (...) {
        msg = "Unanticipated exception type processing init script";
        ok = false;
      }
      if (!ok) {
        msg += " : ";
        msg += tclUtil::getTclTraceback(*Globals::pMainInterpreter);
        std::cerr << "Failure processing initialization script: "
          << msg << std::endl;
        exit(EXIT_FAILURE);
      }
    } else {
            throw CErrnoException("Checking accessibility of --init-script");
    }
  }
  // Save the main thread id and interpreter:
  
  
    // Instantiate the live event loop and run it.
    
  CTCLLiveEventLoop* pEventLoop = CTCLLiveEventLoop::getInstance();
  pEventLoop->start(Globals::pMainInterpreter);

  return TCL_OK;
}
/**
 * AcquisitionErrorHandler
 *    The event handler for errors from the readout thread
 *    * construct and invoke the onTriggerFail command
 *    * If that fails, construct and invoke the bgerror command.
 *
 * @param pEvent - pointer to the event.
 * @param flags  - event flags.
 *
 * @return int - 1 -indicating the event storage can be Tcl_Free'd.
 */
int
CSystemControl::AcquisitionErrorHandler(Tcl_Event* pEvent, int flags)
{
    // Get the message text:
    
    std::string msg = getEventString(pEvent);
    
    // Try the onTriggerFail command:
    
    CTCLInterpreter* pInterp = Globals::pMainInterpreter;
    try {
        pInterp->GlobalEval(
            std::string(makeCommand(pInterp, "onTriggerFail", msg))
        );
    }
    catch (...) {
        // If that failed try bgerror:
        
        try {
            pInterp->GlobalEval(
                std::string(makeCommand(pInterp, "bgerror", msg))
            );
        }
        catch(...) {}
    }

    return 1;    
}

/**
 * makeCommand
 *    Create a command as a CTCLObject
*/
CTCLObject
CSystemControl::makeCommand(
    CTCLInterpreter* pInterp, const char* verb, std::string param
)
{
    CTCLObject result;
    result.Bind(pInterp);
    result += verb;
    result += param;
    
    return result;
}

int CSystemControl::scheduleExit(int status) 
{

  CTCLInterpreter* pInterpreter = Globals::pMainInterpreter;

  if (pInterpreter == nullptr) {
    exit(status);
  } else {
    struct event {
      Tcl_Event     event;
      StringPayload message;
    };
    Tcl_Event* pEvent = makeStringEvent(to_string(status));
    pEvent->proc = CSystemControl::tclExit;

    Tcl_ThreadQueueEvent(Globals::mainThreadId, 
                         pEvent,
                         TCL_QUEUE_TAIL);

  }

  return TCL_OK;
}

int CSystemControl::tclExit(Tcl_Event* pEvent, int flags)
{
    // Get the message text:
    std::string msg = getEventString(pEvent);
    
    // Try the onTriggerFail command:
    
    CTCLInterpreter* pInterp = Globals::pMainInterpreter;
    pInterp->GlobalEval(string(makeCommand( pInterp, "exit", msg)));
    return 1;    
}
/**
 * getEventString
 *   Given a Tcl event with a string hanging on the back,
 *   retursn the string freeing the extra storage in the event.
 * @param pEvent pointer to Raw Tcl Event.
 * @return std::string
 */
string
CSystemControl::getEventString(Tcl_Event* pEvent)
{
   struct event {
        Tcl_Event     event; 
        StringPayload message;
    } ;
    event* pFullEvent = reinterpret_cast<event*>(pEvent);
    std::string msg = pFullEvent->message.pMessage;
    Tcl_Free(pFullEvent->message.pMessage);
   
    return  msg; 
}
/**
 * makeStringEvent
 *    Create a string event (event with string payload).
 * @param msg - string
 * @return Tcl_Event* event.
 */
Tcl_Event*
CSystemControl::makeStringEvent(std::string msg)
{
   struct event {
        Tcl_Event     event; 
        StringPayload message;
    };

    event* pEvent = reinterpret_cast<event*>(Tcl_Alloc(sizeof(event)));
    pEvent->message.pMessage = Tcl_Alloc( msg.size()+1 );
    strcpy(pEvent->message.pMessage, msg.c_str() );
    
    return reinterpret_cast<Tcl_Event*>(pEvent);
  
}