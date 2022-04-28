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
#include "CMonCommand.h"
#include "CWatchCommand.h"
#include "CRunStateCommand.h"
#include <CCtlConfiguration.h>
#include <DataBuffer.h>
#include <CBufferQueue.h>
#include <DataFormat.h>
#include <make_unique.h>
#include <CControlQueues.h>
#include <CSystemControl.h>
#include <TCLVariable.h>

#include <tcl.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include "CControlModule.h"
#include <Exception.h>
#include <string>
#include <iostream>
#include <CRunState.h>
#include <stdlib.h>
#include <Globals.h>
#include <errno.h>
#include <string.h>

#include <iostream>

#include <vector>

static const int MonitorInterval(1);   // Number of seconds between monitor interval.
static const int VarUpdateInterval(1); // Number of seconds between variable update items
/* Pointer to the single instance in order to support transition from
** static functions to object context.
*/
TclServer* TclServer::m_pInstance(0); // static->object context. ptr.

/*!  Constructor is not very interesting 'cause all the action is in 
    start and operator()
*/
TclServer::TclServer(CSystemControl& systemControl) :
  m_port(-1),
  m_configFilename(string("")),
  m_pVme(0),
  m_config(),
  m_pInterpreter(0),
  m_pMonitorList(0),
  m_waitingMonitor(false),
  m_pMonitorData(0),
  m_nMonitorDataSize(0),
  m_dumpAllVariables(true),
  m_exitNow(false),
  m_isRunning(false),
  m_systemControl(systemControl)
{
  m_pInstance = this;		// static->object context.
}
/*!
  These threads are built to live 'forever' so the destructor is also 
uninteresting.
*/
TclServer::~TclServer()
{
  if(m_pMonitorData) {
    free(m_pMonitorData);
  }
}

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
   \param vme : CVMUSB& vme
     Vme controller that is used to interact with the hardware.

*/

void
TclServer::start(int port, const char* configFile, CVMUSB& vme)
{
  // Set up the member data needed to run the thread...

  m_port           = port;
  m_configFilename = configFile;
  m_pVme           = &vme;

  // Schedule the thread for execution:

  this->CSynchronizedThread::start();

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
    
    Tcl_ThreadQueueEvent(m_threadId, pEvent, TCL_QUEUE_HEAD);   // exit is urgent.
}

/*!
   Set the interpreter result to a string value.
*/
void
TclServer::setResult(string msg)
{
  Tcl_Obj* result = Tcl_NewStringObj(msg.c_str(), -1);
  Tcl_SetObjResult(m_pInterpreter->getInterpreter(), result);
}

void TclServer::init()
{
  try {
    initInterpreter();		// Create interp and add commands.
    startTcpServer();	  	// Set up the Tcp/Ip listener event.
    readConfigFile();	  	// Initialize the modules.
    initModules();        // Initialize the fully configured modules.
    createMonitorList();	// Figure out the set of modules that need monitoring.
    m_isRunning = true;
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
  catch (std::exception& exc) {
    cerr << "TclServer thread caught an exception: "
      << exc.what() << endl;
    m_systemControl.scheduleExit(EXIT_FAILURE);
  }
  catch (...) {
    cerr << "TclServer thread caught some other exception type.\n";
    m_systemControl.scheduleExit(EXIT_FAILURE);
  }
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
  m_threadId = Tcl_GetCurrentThread(); // Save for later use.
  try {
    EventLoop();		// Run the Tcl event loop forever.
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

 
  
  // Add the commands... these don't get saved.. as they will live forever

  m_config.addCommand( make_unique( new CModuleCommand(*m_pInterpreter,
                                                       m_config)) );
  m_config.addCommand( make_unique( new CSetCommand(*m_pInterpreter,
                                                     m_config,
                                                    *m_pVme)) );
  m_config.addCommand( make_unique( new CGetCommand(*m_pInterpreter,
                                                     m_config,
                                                    *m_pVme)) );
  m_config.addCommand( make_unique( new CUpdateCommand(*m_pInterpreter,
                                                        m_config,
                                                       *m_pVme)) );
  m_config.addCommand( make_unique( new CMonCommand(*m_pInterpreter,
                                                    m_config)) );
  m_config.addCommand( make_unique( new CWatchCommand(*m_pInterpreter)));
  m_config.addCommand( make_unique( new CRunStateCommand(*m_pInterpreter,
                                                          m_config)) );
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
  /** Now build the monitor list from the modules that have been configured */

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
  // If there's a nonempty monitor list we need to start its periodic execution

  if (m_pMonitorList && m_pMonitorList->size()) {
    MonitorDevices(this);	// Allow it to locate us.
  }

  // Start the timed send of monitored variable values.

  updateVariables(this);

  // Start the event loop:

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
  // this code was inside of the Initialize method of CControl 
  bool mustRelease(false);
  if (CRunState::getInstance()->getState() == CRunState::Active) {
    mustRelease = true;
    CControlQueues::getInstance()->AcquireUsb();
  }

  auto& modules( m_config.getModules());

  for (auto p = modules.begin(); p != modules.end(); p++) {
    auto module = (*p);
    module->Initialize(*m_pVme);
  }
#if 0  
  for ( auto& module : modules ) {
    module->Initialize(*m_pVme);
  }
#endif
  if (mustRelease) {
    CControlQueues::getInstance()->ReleaseUsb();
  }
}

/**
 ** Create the monitor list and populate it with contributions from the 
 ** modules that have been created.  
 ** @note Site effect: m_pMonitorList is constructed and, potentially
 **       filled in.
 */
void
TclServer::createMonitorList()
{
  m_pMonitorList = new CVMUSBReadoutList;
  
  auto& modules = m_config.getModules();
  for ( auto& pModule : modules ) {
    pModule->addMonitorList(*m_pMonitorList);
  }
}
/**
 ** Provide a copy of the monitor list to a client.
 ** if m_pModules is null an empty list will be returned.
 ** @return CVMUSBReadoutList
 ** @retval Copy of the local monitor list.
 **
 */
CVMUSBReadoutList
TclServer::getMonitorList()
{
  CVMUSBReadoutList result;
  if (m_pMonitorList) {
    result = *m_pMonitorList;
  }
  return result;
}
/*
** This function queues an event to the tclserver thread.  It is intended to be
** called by other threads.  The event indicates the arrival of a monitor
** event when the run is active.  The event handling function
** will have to funge stuff up to allow processMonitorData to be
** executed on the received data.
** @param pBuffer - Pointer to the raw event in the VM-USB buffer.
**                  note that this might have a continuation flag.
**                  in that case we get serveral events queued, one for each
**                  chunk of the data buffer.
*/
void
TclServer::QueueBuffer(void* pBuffer)
{
  uint16_t* pEvent = reinterpret_cast<uint16_t*>(pBuffer);

  // Transfer the event to a dynamically allocated buffer:

  uint16_t nWords = (*pEvent & VMUSBEventLengthMask) + 1; // Word length isn't self inclusive.
  void*    pEventCopy = (void*)Tcl_Alloc(nWords*sizeof(uint16_t));
  if (!pEventCopy) {
    throw std::string("Malloc for tcl server event failed in COutputThread");
  }
  memcpy(pEventCopy, pEvent, nWords*sizeof(uint16_t));

  // Create and fill in the event:

  TclServerEvent* pTclEvent = reinterpret_cast<TclServerEvent*>(Tcl_Alloc(sizeof(TclServerEvent)));
  pTclEvent->event.proc = receiveMonitorData;
  pTclEvent->pData      = pEventCopy;

  Tcl_ThreadQueueEvent(m_threadId, 
		       reinterpret_cast<Tcl_Event*>(pTclEvent), 
		       TCL_QUEUE_HEAD); // out of band processing.
}

/**
 ** Monitor devices:
 ** If the run is inactive, the list is done in immediate mode, and the data are dispatched
 ** Directly to the device handlers.
 ** If the run is active, the list is popped off and the readout thread will do that dispatch for us.
 ** Regardless, we reschedule ourself via Tcl_CreateTimerHandler.
 ** @param pData - Void pointer that is really a TclServer pointer that allows us to gain object
 **                context.
 */
void
TclServer::MonitorDevices(void* pData)
{
  TclServer* pObject = reinterpret_cast<TclServer*>(pData);


  // If the run is active  we just trigger list 7.
  // otherwise we execute the list immediate and ship the data around
  // to the various devices... however if the run isin transition
  // lay off the VM-USB until the run is completely live.

  CVMUSB* pController = pObject->m_pVme;
  CRunState::RunState state = CRunState::getInstance()->getState();
  if (state  == CRunState::Active) {
    try {
      pController->writeActionRegister( CVMUSB::ActionRegister::triggerL7 | 
					CVMUSB::ActionRegister::startDAQ); // StartDAQ keeps acquisition alive.
      pObject->m_waitingMonitor = true;
    }
    catch (...) {
      std::cerr << "Could not trigger monitor list.  No harm, this will be retried soon\n";
    }
  }
  else if ((state != CRunState::Starting) && (state != CRunState::Stopping)) {
    uint16_t readData[13*1024];	// Big data pot...ought to be big enough...one event buffer worth?
    size_t   dataRead(0);
    CVMUSBReadoutList* pList  = pObject->m_pMonitorList;
    if (pList->size() > 0) {
      int                status = pController->executeList(*pList, readData, sizeof(readData), &dataRead);
      if (status != 0) {
	cerr << "Warning: Monitor list read failed " << status << " \n" << strerror(errno) << std::endl;
	
      }
      else {
	pObject->processMonitorList(readData, dataRead);
      }
	
    }						 
  }

  Tcl_Interp* pInterp = pObject->m_pInterpreter->getInterpreter();
  Tcl_CreateTimerHandler(MonitorInterval*1000, TclServer::MonitorDevices, pData);
}
/*
** Process control module data:
**
** @param pData   - Pointer to the data read from the moduels.
** @param nBytes  - Number of bytes of data to process.
*/
void
TclServer::processMonitorList(void* pData, size_t nBytes)
{
  // It will be easier to track the progress through the list
  // by treating the data as uint8_t*

  uint8_t* p = reinterpret_cast<uint8_t*>(pData);
  
  auto& modules = m_config.getModules();
  for ( auto& pModule : modules ) {
    uint8_t* pNewPosition;
    pNewPosition = reinterpret_cast<uint8_t*>(pModule->processMonitorList(p, nBytes));
    nBytes      -= (pNewPosition - p);
    p            = pNewPosition;
    if (nBytes ==0) break;	// Short circuit once we're out of bytes.
					      
  }
}
/**
 **  Handle Tcl events that indicate the arrival of monitor data.
 **  note that monitor data could be broken across several segments
 **  as indicated by the continuation bit in the event header.
 **  We do the following
 **  - Extend the m_pMonitor data buffer to hold the new monitor data.
 **  - Copy the data gotten to the end of the monitor data buffer (minus thhe
 **    header word).
 *   - If the continuation flag was not set, process the monitor data and delete the
 **    m_pMonitorData buffer.
 **  - Free the event buffer and return indicating the event was serviced.
 */
int
TclServer::receiveMonitorData(Tcl_Event* pEvent, int flags)
{
  TclServer*      pServer = Globals::pTclServer;

  TclServerEvent* pTclEvent = reinterpret_cast<TclServerEvent*>(pEvent);
  uint16_t*       pData     = reinterpret_cast<uint16_t*>(pTclEvent->pData);
  uint16_t        nWords    = *pData & VMUSBEventLengthMask;

  // Require there actually be data:

  if(nWords) {
    
    // Append the new data to the data we have already:
    
    pServer->m_pMonitorData = 
      reinterpret_cast<uint16_t*>(realloc(pServer->m_pMonitorData,
					  (nWords + pServer->m_nMonitorDataSize) * sizeof(uint16_t)));
    if (!pServer->m_pMonitorData) {
      throw string("Unable to extend monitor data buffer");
    }
    memcpy(&(pServer->m_pMonitorData)[pServer->m_nMonitorDataSize], &pData[1], nWords*sizeof(uint16_t));
    pServer->m_nMonitorDataSize += nWords;
    
    // If the continuation bit is not set we can process the data:
    
    if ((*pData & VMUSBContinuation) != 0) {
      pServer->processMonitorList(pServer->m_pMonitorData, pServer->m_nMonitorDataSize * sizeof(uint16_t));
      
      free(pServer->m_pMonitorData);
      pServer->m_nMonitorDataSize = 0;
      pServer->m_pMonitorData     = 0;
    }
  }
  // Free the input data and complete event processing:

  Tcl_Free((char*)pData);
  return 1;

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
 *    Output a Tcl callback trace to stderr.
 */
void
TclServer::stackTrace()
{
  CTCLVariable errorInfo(m_pInterpreter, "errorInfo", TCLPLUS::kfFALSE);
  const char* msg = errorInfo.Get(TCL_GLOBAL_ONLY);
  
  std::cerr << msg << std::endl;
}
