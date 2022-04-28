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
using namespace std;

#include "TclServer.h"
#include "server.h"
#include "serverinstance.h"
#include "CModuleCommand.h"
#include "CSetCommand.h"
#include "CGetCommand.h"
#include "CUpdateCommand.h"
#include "CWatchCommand.h"
#include <CSystemControl.h>


#include <tcl.h>
#include <TCLInterpreter.h>
#include <TCLVariable.h>
#include <CCCUSB.h>
#include "CControlModule.h"
#include "CControlHardware.h"
#include <Exception.h>
#include <string>
#include <iostream>
#include <CRunState.h>
#include <DataBuffer.h>
#include <Globals.h>
#include <DataFormat.h>

static const int VarUpdateInterval(1);

/*!  Constructor is not very interesting 'cause all the action is in 
    start and operator()<
*/
TclServer::TclServer(CSystemControl& systemControl) :
  m_port(-1),
  m_configFilename(string("")),
  m_pVme(0),
  m_pInterpreter(0),
  m_dumpAllVariables(true),
  m_exitNow(false),
  m_config(),
  m_systemControl(systemControl)
{}
/*!
  These threads are built to live 'forever' so the destructor is also 
uninteresting.
*/
TclServer::~TclServer()
{}

/*!
  Start sets up the variables the entry point needs to initialize
  and schedules the thread.   The thread id is returned to the caller.
  Note that while the entry point includes parameters, it's just so much
  easier to stuff them into the member data rather than trying to force fit
  them to argc,argv formalism in the caller and receiver.
  \param port : int
     Number of the Tcp/IP port on which we will listen for connections.
  \param configFile : const char* 
     Name of the configuration file that is used to instantiate our controllers.
     This is intended to be a Tcl script and is therefore processed by the
     interpreter after it has been initialized with all the added commands,
     but prior to entering the event loop.
   \param vme : CCCUSB& vme
     Vme controller that is used to interact with the hardware.

*/

void
TclServer::start(int port, const char* configFile, CCCUSB& vme)
{
  // Set up the member data needed to run the thread...

  m_port           = port;
  m_configFilename = configFile;
  m_pVme           = &vme;

  // Schedule the thread for execution:

  this->CSynchronizedThread::start();


}

/**
 * Bridges nscldaq-10 and spectrodaq threadingmodel.
 */
void
TclServer::init()
{
  try {
    initInterpreter();		// Create interp and add commands.
    readConfigFile();		// Initialize the modules.
    initModules();              // Initialize the fully configured modules.
    startTcpServer();		// Set up the Tcp/Ip listener event.
  } catch (string msg) {
    cerr << "TclServer thread caught a string exception: " << msg << endl;
    m_systemControl.scheduleExit(EXIT_FAILURE);
  }
  catch (char* msg) {
    cerr << "TclServer thread caught a char* exception: " << msg << endl;
    m_systemControl.scheduleExit(EXIT_FAILURE);
  }
  catch (CException& err) {
    cerr << "CTclServer thread caught a daq exception: "
	       << err.ReasonText() << " while " << err.WasDoing() << endl;
    m_systemControl.scheduleExit(EXIT_FAILURE);
  }
  catch (...) {
    cerr << "TclServer thread caught some other exception type.\n";
    m_systemControl.scheduleExit(EXIT_FAILURE);
  }
}
/**
 * scheduleExit
 *   This is called to set an exit event into my event queue.
 *   Normally this is called from a different thread.
 *
 */
void
TclServer::scheduleExit()
{
    Tcl_Event* pEvent = reinterpret_cast<Tcl_Event*>(Tcl_Alloc(sizeof(Tcl_Event)));
    pEvent->proc = TclServer::Exit;
    
    Tcl_ThreadQueueEvent(m_tclThreadId, pEvent, TCL_QUEUE_HEAD);   // exit is urgent.
}


/*!
   Entry point for the thread.  This will be called when the thread is first
   scheduled after start was called.  We just need to call our
   private functions in order :-).
   Parameters are ignored (start stocked the member data with everything
   we need) and we never return.
*/
void
TclServer::operator()()
{
  m_tclThreadId = Tcl_GetCurrentThread();
  try {
    EventLoop();			// Run the Tcl event loop forever.
  }
  catch (string msg) {
    cerr << "TclServer thread caught a string exception: " << msg << endl;
    m_systemControl.scheduleExit(EXIT_FAILURE);
  }
  catch (char* msg) {
    cerr << "TclServer thread caught a char* exception: " << msg << endl;
    m_systemControl.scheduleExit(EXIT_FAILURE);
  }
  catch (CException& err) {
    cerr << "TclServer thread caught a daq exception: "
	       << err.ReasonText() << " while " << err.WasDoing() << endl;
    m_systemControl.scheduleExit(EXIT_FAILURE);
  }
  catch (...) {
    cerr << "TclServer thread caught some other exception type.\n";
    m_systemControl.scheduleExit(EXIT_FAILURE);
  }
}

/*
   - Create and initialize an interpreter
   - Objectify it and store it in m_pInterpreter
   - Add the following three commands:
   - module   - Create/configure a module
   - Set      - Set an item in a module to a value.
   - Get      - Get an item's value from a module.
   - Update   - Update all module's state for get


   On exit we should be able to read a configuration file.


*/
void
TclServer::initInterpreter()
{

  // Create the interpreter:
  Tcl_Interp*  pInterp = Tcl_CreateInterp();
  Tcl_Init(pInterp);
  m_pInterpreter       = new CTCLInterpreter(pInterp);

  ::Globals::pTclServer = this;

  
  // Add the commands... these don't get saved.. as they will live forever
  m_config.addCommand( 
      unique_ptr<CTCLObjectProcessor>(new CModuleCommand(*m_pInterpreter,
		                                                     m_config)));
  m_config.addCommand( 
      unique_ptr<CTCLObjectProcessor>(new CSetCommand(*m_pInterpreter,
		                                                  m_config, *m_pVme)));
  m_config.addCommand( 
      unique_ptr<CTCLObjectProcessor>(new CGetCommand(*m_pInterpreter,
                                                      m_config, *m_pVme)));
  m_config.addCommand( 
      unique_ptr<CTCLObjectProcessor>(new CUpdateCommand(*m_pInterpreter,
                                                         m_config, *m_pVme)));
  m_config.addCommand( 
      unique_ptr<CTCLObjectProcessor>(new CWatchCommand(*m_pInterpreter)));
                                                        
  
}
/*
   Read the configuration file.  This is just sourcing the the file
   into our brand new interpreter.  This should cause the 
   modules vector to get stocked with the appropriate set of preconfigured
   modules.  It will be up to external control programs to 
   set values for the parameters of these modules.
*/
void
TclServer::readConfigFile()
{
  try {
    m_pInterpreter->EvalFile(m_configFilename);
  }
  catch (string msg) {
    cerr << "TclServer::readConfigFile - string exception: " << msg << endl;
    stackTrace();
    throw;
  }
  catch (char* msg) {
    cerr << "TclServer::readConfigFile - char* exception: " << msg << endl;
    stackTrace();
    throw;
  }
  catch (CException& error) {
    cerr << "TclServer::readConfigFile - CException: " 
	 << error.ReasonText() << " while " << error.WasDoing() << endl;
    stackTrace();
    throw;
  }
  catch (...) {
    cerr << "TclServer::readConfigFile - unanticipated exception type\n";
    stackTrace();
    throw;
  }
}
/*
   Start the Tcl server.
   This just means calling ::Server_Init giving it the interpreter and
   the port
*/
void
TclServer::startTcpServer()
{
  ::Server_Init(m_pInterpreter->getInterpreter(),
		m_port);
}


/*
  run the event loop.  This should never exit (although in theory the
  user could poke an exit into the interpreter which would finish us off
  right quick).
  Running the server as an event loop is the only way to ensure that Tcp/IP
  events get served (honoring connections and accepting commands).
  We are actually a pretty wierd interpreter. We don't have stdin...
  just 0 or more Tcp/Ip sockets on which commands can be accepted.

*/
void
TclServer::EventLoop()
{
    // Initialize the periodic veriable watch stuff:
    
    updateVariables(this);
    
    // The actual event loop:
    
  while(!m_exitNow) {
    Tcl_DoOneEvent(TCL_ALL_EVENTS);
  }
 std::cerr << "The Tcl Server event loop has exited.\n"; 
}


/*   
   Initialize the modules by calling all their Update functions:
*/
void
TclServer::initModules()
{ 
//  for (int i =0; i < m_Modules.size(); i++) {
//    m_Modules[i]->Initialize(*m_pVme);
//  }
  auto& modules = m_config.getModules();
  for (auto& pModule : modules) {
    pModule->Initialize(*m_pVme);
  }
}
/**
 * updateVariables(ClientData pData)
 *
 * Handle and re-schedule watched variable updates.
 *
 * @param pData - Actually a pointer to a TclServer object.
 *
 */
void 
TclServer::updateVariables(ClientData pData)
{
  TclServer* pServer = reinterpret_cast<TclServer*>(pData);

  pServer->sendWatchedVariables();

  Tcl_CreateTimerHandler(VarUpdateInterval, TclServer::updateVariables, pData); // Schedule the next update.

}
/**
 * sendWatchedVaraibles
 *
 * Handle watched variable updates.  
 * - The run must be halted.
 * - Get the set of variables that have been modified.
 * - Get their values.
 * - Create 'set' commands for them.
 * - Build a set of buffers for the router and send them.
 */
void
TclServer::sendWatchedVariables()
{
  CRunState::RunState state = CRunState::getInstance()->getState();
  if (state == CRunState::Active) {
    std::vector<CWatchCommand::TCLVariableName> modifications;

    if (m_dumpAllVariables) {
      modifications = CWatchCommand::getWatchedVariables(*m_pInterpreter);
      m_dumpAllVariables = false;
    } else {
      modifications = CWatchCommand::getModifications();
    }
    Tcl_Interp* pInterp = m_pInterpreter->getInterpreter();
    if (!modifications.empty()) {

      DataBuffer*    pBuffer  = 0;
      pStringsBuffer pStrings = 0;
      char*          pDest    = 0;

      // Create the commands:

      for (int i = 0; i < modifications.size(); i++) {
	
	// If necessary get a buffer.
	
	if (!pBuffer) {
	  pBuffer               = gFreeBuffers.get();
	  pBuffer->s_bufferSize = sizeof(StringsBuffer) - sizeof(char);
	  pBuffer->s_bufferType = TYPE_STRINGS;
	  pStrings              = reinterpret_cast<pStringsBuffer>(pBuffer->s_rawData);
	  pStrings->s_stringCount = 0;
	  pStrings->s_ringType    = MONITORED_VARIABLES;
	  pDest                 = pStrings->s_strings;
	}
	

	// construct the variable name:
	
	std::string fullName = modifications[i].first;
	if (modifications[i].second != "") {
	  fullName += "(" + modifications[i].second + ")";
	}
	const char* pValue = Tcl_GetVar(pInterp, fullName.c_str(), TCL_GLOBAL_ONLY);
	if (pValue) {		// Protect against an unset between modification and now:
	  
	  // Construct the command as a list so that stuff will be properly quoted:

	  CTCLObject setCommandObj;
	  setCommandObj.Bind(*m_pInterpreter);
	  setCommandObj += "set";
	  setCommandObj += fullName;
	  setCommandObj += pValue;

	  std::string setCommand = (std::string)(setCommandObj); // Now it's all properly quoted.

	  /*
	    If there's room, add to the buffer etc.  If not submit the buffer
	    and get a new one. This code assumes the set command will fit in an empty buffer.
	  */
	  if ((setCommand.size() + pBuffer->s_bufferSize + 1) > pBuffer->s_storageSize) {
	    gFilledBuffers.queue(pBuffer);
	    
	    pBuffer               = gFreeBuffers.get();
	    pBuffer->s_bufferSize = sizeof(StringsBuffer) - sizeof(char);
	    pBuffer->s_bufferType = TYPE_STRINGS;
	    pStrings              = reinterpret_cast<pStringsBuffer>(pBuffer->s_rawData);
	    pStrings->s_stringCount = 0;
	    pStrings->s_ringType    = MONITORED_VARIABLES;
	    pDest                 = pStrings->s_strings;
	    
	  }
	  strcpy(pDest, setCommand.c_str());
	  *pDest = 0;
	  pStrings->s_stringCount++;
	  pBuffer->s_bufferSize += setCommand.size() + 1;
	}
      }
      // Submit any partial buffer:

      gFilledBuffers.queue(pBuffer);
    }
  } else {
    m_dumpAllVariables = true;	// When the run starts next dump everything!
  }
}
/**
 * Exit
 *   Scheduled from the event loop when the main thread is about to exit.
 *
 * @param pEvent - pointer to the event (not used).
 * @param flags   - Event schedule flags.
 *
 * @return int  - 1, the event can be freed.
 */
int
TclServer::Exit(Tcl_Event* pEvent, int flags)
{
    ::Globals::pTclServer->m_exitNow = true;
    return 1;
}

/**
 * stackTrace
 *     Outputs an error stack traceback to stderr - basically
 *     this just dumps the value of the ::errorInfo variable.
 */
void
TclServer::stackTrace()
{
  CTCLVariable errorInfo(m_pInterpreter, "errorInfo", TCLPLUS::kfFALSE);
  const char* msg =  errorInfo.Get(TCL_GLOBAL_ONLY);
  if (msg) {
    std::cerr << msg << std::endl;
  }
}
