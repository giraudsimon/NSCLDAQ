

#include "CSystemControl.h"

#include <CBeginRun.h>
#include <CEndRun.h>
#include <CPauseRun.h>
#include <CResumeRun.h>
#include <CInit.h>
#include <CExit.h>
#include <Globals.h>
#include <Events.h>
#include "CStatisticsCommand.h"
#include "CRunStateCommand.h"

#include <TCLLiveEventLoop.h>
#include <TCLInterpreter.h>
#include <TCLException.h>
#include <CErrnoException.h>
#include <XXUSBConfigurableObject.h>
#include <tclUtil.h>

#include <tcl.h>

#include <unistd.h>

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
unique_ptr<CStatisticsCommand>      CSystemControl::m_pStats;
unique_ptr<CRunStateCommand> CSystemControl::m_pRunState;

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
  Tcl_Init(interp);		// Get all the paths etc. setup.
  Globals::pMainInterpreter = new CTCLInterpreter(interp);
  Globals::mainThread     = Tcl_GetCurrentThread();

  m_pBeginRun.reset(new CBeginRun(*Globals::pMainInterpreter));
  m_pEndRun.reset(new CEndRun(*Globals::pMainInterpreter));
  m_pPauseRun.reset(new CPauseRun(*Globals::pMainInterpreter));
  m_pResumeRun.reset(new CResumeRun(*Globals::pMainInterpreter));
  m_pInit.reset(new CInit(*Globals::pMainInterpreter));
  m_pExit.reset(new CExit(*Globals::pMainInterpreter));
  m_pStats.reset(new CStatisticsCommand(*Globals::pMainInterpreter, "statistics"));
  m_pRunState.reset(new CRunStateCommand(*Globals::pMainInterpreter, "runstate"));
  
  // Look for readoutRC.tcl in the config directory.  If it exists, run it.

  // If there's an initialization script then run it now:
  

  try {
    if (m_initScript != "") {
      if (access(m_initScript.c_str(), R_OK) == 0) {
        try {
          Globals::pMainInterpreter->EvalFile(m_initScript.c_str());
        }
        catch (...) {
          Tcl_Interp* pRaw = Globals::pMainInterpreter->getInterpreter();
          std::cerr << "Error executing init script " << m_initScript << std::endl;
          std::cerr << ": " << Tcl_GetStringResult(pRaw) << ": "
		    << tclUtil::getTclTraceback(*Globals::pMainInterpreter) << std::endl;
          std:: cerr << "Processing continues\n";
        }
      } else {
        throw CErrnoException("Checking accessibility of --init-script");
      }
    }
  } catch (CTCLException except) {
    cerr << "Failed to run initialization file.\n";
    cerr << except.ReasonText() << endl;
  }
  
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
    // Pull out the message and release its storage so that the event
    // can be deleted without hanging storage around:
    
    typedef struct _AcqFailEvent {
        Tcl_Event              event;
        AcquisitionFailedEvent moreData;
        
    } *pAcqFailEvent;
    pAcqFailEvent pE = reinterpret_cast<pAcqFailEvent>(pEvent);
    std::string message = pE->moreData.pMessage;
    Tcl_Free(pE->moreData.pMessage);
    
    // First try to execute the onTriggerFail command:
    
    CTCLObject cmd = makeCommand(Globals::pMainInterpreter, "onTriggerFail", message);
    
    try {
        Globals::pMainInterpreter->GlobalEval(cmd);
    } catch(...) {
        // Failed so run the bgerror with a try /catch ignore...
        
        cmd = makeCommand(Globals::pMainInterpreter, "bgerror", message);
        try {
            Globals::pMainInterpreter->GlobalEval(cmd);
        }
        catch (...) {
            
        }
    }
    
    return 1;                        // Can deallocate the event.
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

    event* pEvent = reinterpret_cast<event*>(Tcl_Alloc(sizeof(event)));
    string msg = to_string(status);
    pEvent->event.proc = CSystemControl::tclExit;
    pEvent->message.pMessage = Tcl_Alloc( msg.size()+1 );
    strcpy(pEvent->message.pMessage, msg.c_str() );

    Tcl_ThreadQueueEvent(Globals::mainThread, 
                         reinterpret_cast<Tcl_Event*>(pEvent),
                         TCL_QUEUE_TAIL);

  }

  return TCL_OK;
}

int CSystemControl::tclExit(Tcl_Event* pEvent, int flags)
{
    // Get the message text:
    
    struct event {
        Tcl_Event     event; 
        StringPayload message;
    };
    event* pFullEvent = reinterpret_cast<event*>(pEvent);
    std::string msg = pFullEvent->message.pMessage;
    Tcl_Free(pFullEvent->message.pMessage);
    
    // Try the onTriggerFail command:
    
    CTCLInterpreter* pInterp = Globals::pMainInterpreter;
    pInterp->GlobalEval(string(makeCommand( pInterp, "exit", msg)));
    return 1;    
}
