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
#include "CTheApplication.h"
#include "Globals.h"
#include "event.h"

#include <COutputThread.h>
#include <TclServer.h>
#include <CVMUSBusb.h>
#include <CVMUSBFactory.h>

#include <CMutex.h>
#include <CCondition.h>
#include <TCLInterpreter.h>
#include <TCLLiveEventLoop.h>
#include <CBeginRun.h>
#include <CEndRun.h>
#include <CPauseRun.h>
#include <CResumeRun.h>
#include <CInit.h>
#include <CExit.h>
#include <Exception.h>
#include <ErrnoException.h>
#include <tcl.h>
#include <DataBuffer.h>
#include <CRingBuffer.h>
#include <CAcquisitionThread.h>
#include <os.h>

#include <CPortManager.h>

#include <vector>

#include <usb.h>
#include <sysexits.h>
#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>

#include "cmdline.h"


#include <string>


#ifndef NULL
#define NULL ((void*)0)
#endif

using namespace std;

//   Configuration constants:

static const char* versionString = VERSION ; // NSCLDAQ-version.

static const string   daqConfigBasename("daqconfig.tcl");
static const string   ctlConfigBasename("controlconfig.tcl");
static const uint32_t bufferCount(32);                      // Number of buffers that can be inflight.
static const uint32_t bufferSize(13*1024*sizeof(uint16_t)); // 13kword buffers...+pad
static       int      tclServerPort(27000);		    // Default value.


// Static member variables and initialization.

bool CTheApplication::m_Exists(false);
std::string CTheApplication::m_initScript;

/*!
   Construct ourselves.. Note that if m_Exists is true,
   we BUGCHECK.
*/
CTheApplication::CTheApplication()
{
  if (m_Exists) {
    cerr << "Attempted to create more than one instance of the application\n";
    Tcl_Exit(EX_SOFTWARE);
  }
  m_Exists = true;
  m_pInterpreter = static_cast<CTCLInterpreter*>(NULL);
  Globals::running = false;
  
}
/*!
   Destruction is a no-op since it happens at program exit.
*/
CTheApplication::~CTheApplication()
{
}

/*!
   Thread entry point.  We don't care that much about our command line parameters.
   Note that the configuration files are as follows:
   $HOME/config/daqconfig.tcl     - Data acquisition configuration.
   $HOME/config/controlconfig.tcl - Controllable electronics configuration.
   We will not be returned to after calling startInterpreter().
   startInterpreter will eventually return control to the Tcl event loop which
   will exit directly rather than returning up the call chain.

   \param argc : int
      Number of command line parameters (ignored).
   \param argv : char**
      The command line parameters (ignored).

*/
int CTheApplication::operator()(int argc, char** argv)
{
  m_Argc   = argc;		// In case someone else wants them.
  m_Argv   = argv; 

  // Process them via gengetopt:

  struct gengetopt_args_info parsedArgs;
  cmdline_parser(argc, argv, &parsedArgs);	// Actually fails if the commandline is incorrect.
  
  // Save the data source id:
  
  Globals::sourceId = parsedArgs.sourceid_arg;
  
  // If a timstamp lib was given save that as well:
  
  m_pMutex = shared_ptr<CMutex>(new CMutex); 
  m_pCondition = shared_ptr<CConditionVariable>(new CConditionVariable);
  
  Globals::pTimestampExtractor = 0;
  if (parsedArgs.timestamplib_given) {
    size_t libLen = strlen(parsedArgs.timestamplib_arg);
    Globals::pTimestampExtractor = reinterpret_cast<char*>(malloc(libLen + 1));
    strcpy(
        Globals::pTimestampExtractor, parsedArgs.timestamplib_arg
    );
  }

  //  cerr << "VM-USB scriptable readout version " << versionString << endl;

  // If --enumerate was given, just enumerate the VM-USB modules connected to the
  // system on stdout and exit.

  if (parsedArgs.enumerate_given) {
    enumerateVMUSB();
    Tcl_Exit(EXIT_SUCCESS);
  }
  if (parsedArgs.init_script_given) {
    m_initScript = parsedArgs.init_script_arg;
  }
  try {				// Last chance exception catching...
    
    // How the USB controller is created depends on the parameters.
    // if host_given we need a remote server otherwise local with the serialno.
    //

    const char* connectionString;
    CVMUSBFactory::ControllerType type;
#ifdef HOST_ARG_DEFINED
    if (parsedArgs.host_given) {
      type             = CVMUSBFactory::remote;
      connectionString = parsedArgs.host_arg;
    } else {
#endif
      type             = CVMUSBFactory::local;
      connectionString = parsedArgs.serialno_given ? parsedArgs.serialno_arg :  0;
#ifdef HOST_ARG_DEFINED
    }
#endif

    Globals::pUSBController  = CVMUSBFactory::createUSBController(type, connectionString);
    
    std::cerr << "Attached VMUSB controller with firmware: " << std::hex << 
      Globals::pUSBController->readFirmwareID() << std::dec << std::endl;

    

    // Set default configuration file names and then override with the ones supplied on
    // the command line (if any).
    
    setConfigFiles();
    if (parsedArgs.daqconfig_given) {
      Globals::configurationFilename = std::string(parsedArgs.daqconfig_arg);
    }
    if (parsedArgs.ctlconfig_given) {
      Globals::controlConfigFilename = std::string(parsedArgs.ctlconfig_arg);
    }
    
    initializeBufferPool();
    startOutputThread(destinationRing(parsedArgs.ring_given ? parsedArgs.ring_arg :
				      reinterpret_cast<const char*>(NULL)));

    // Replace the default server port if the user supplied one and start the Tcl server.

    if (parsedArgs.port_given) {
      std::string portString = parsedArgs.port_arg;
      if (portString == "managed") {      // Use port manager.
         // We'll use VMUSBReadout:connectionstring as our app.
         
         std::string appName="VMUSBReadout:";
         if (connectionString) {
            appName += connectionString;
         } else {
            appName += "FirstController";
         }
         CPortManager* pManager = new  CPortManager();      // Hold connection for app lifetime.
         tclServerPort = pManager->allocatePort(appName);
      } else {
        char* end;
        long port = strtol(portString.c_str(), &end, 0);
        if(end == portString.c_str()) {       // failed.
            std::cerr << "--port string must be either a number or 'managed'\n";
            cmdline_parser_print_help();
            Tcl_Exit(EXIT_FAILURE);
        } else {
            tclServerPort = port;
        }
      }
    }

    startTclServer();
    startInterpreter();
  }
  catch (string msg) {
    cerr << "CTheApplication caught a string exception: " << msg << endl;
    Tcl_Exit(EXIT_FAILURE);
  }
  catch (const char* msg) {
    cerr << "CTheApplication caught a char* excpetion " << msg << endl;
    Tcl_Exit(EXIT_FAILURE);

  }
  catch (CException& error) {
    cerr << "CTheApplication caught an NCLDAQ exception: " 
	 << error.ReasonText() << " while " << error.WasDoing() << endl;

    Tcl_Exit(EXIT_FAILURE);
  }
  catch (...) {
    cerr << "CTheApplication thread caught an excpetion of unknown type\n";
    Tcl_Exit(EXIT_FAILURE);

  }
    return EX_SOFTWARE; // keep compiler happy, startInterpreter should not return.
}


/*
   Start the output thread.  This thread is responsible for 
   reformatting and transferring buffers of data from the VM-USB to 
   spectrodaq.  This thread is continuously running for the life of the program.
   .. therefore we are sloppy with storage management.

   @param ring - std::ring that contains the name of the ring buffer into which data should be put.

*/
void
CTheApplication::startOutputThread(std::string ring)
{
  COutputThread* router = new COutputThread(ring);
  router->start();
  Os::usleep(500);

}
/* 
   Start the Tcl server.  It will listen on port tclServerPort, seee above..
   Again, the tcl server runs the lifetime of the program so we are 
   sloppy about storage management.
*/
void
CTheApplication::startTclServer()
{
  TclServer* pServer = new TclServer(m_pMutex, m_pCondition);
  pServer->start(tclServerPort, Globals::controlConfigFilename.c_str(),
		   *Globals::pUSBController);
  Globals::pTclServer = pServer; // Save for readout.
  Os::usleep(500);
}
/*
    Start the Tcl interpreter, we use the static AppInit as a trampoline into the
    interpreter configuration and back to the interpreter event loop so the
    default Tcl event loop can be used.
*/
void
CTheApplication::startInterpreter()
{
  cout << "main waiting" << endl;
  m_pCondition->wait(*m_pMutex);
  cout << "initializing main interp" << endl;

//  Tcl_CreateExitHandler(CTheApplication::ExitHandler, reinterpret_cast<ClientData>(this));
  Tcl_Main(m_Argc, m_Argv, CTheApplication::AppInit);

  // we own the mutex so we need to unlock it... not that the main thread will
  // own it for the duration of the program because Tcl_Main will not 
  // actually return until all is done
  m_pMutex->unlock();
}


/**
 * Enumerate the set of VM-USB serial numgers that are currently active in the system.
 * The enumeration goes to cout.;
 *
 */
void
CTheApplication::enumerateVMUSB()
{
  try {
    vector<struct usb_device*> controllers = CVMUSBusb::enumerate();
    for (int i = 0; i < controllers.size(); i++) {
      std::string serial = CVMUSBusb::serialNo(controllers[i]);
      std::cout << "[" << i << "] : " << serial << std::endl;
    }
    cout.flush();
  }
  catch(std::string msg) {
    std::cerr << "Unable to enumerate VM-USB modules: " << msg << std::endl;
  }
}
/* 
  Set the configuration files to the global storage
*/
void
CTheApplication::setConfigFiles()
{
  Globals::configurationFilename = makeConfigFile(daqConfigBasename);
  Globals::controlConfigFilename = makeConfigFile(ctlConfigBasename);

}



/*
   Initialize the interpreter.  This invoves:
   - Wrapping the interpreter into a CTCLInterpreter Object.
   - Creating the commands that extend the interpreter.
   - Returning TCL_OK so that the interpreter will start running the main loop.

*/
int
CTheApplication::AppInit(Tcl_Interp* interp)
{
  Tcl_Init(interp);		// Get all the paths etc. setup.
  CTCLInterpreter* pInterp = new CTCLInterpreter(interp);
  new CBeginRun(*pInterp);
  new CEndRun(*pInterp);
  new CPauseRun(*pInterp);
  new CResumeRun(*pInterp);
  new CInit(*pInterp);
  new CExit(*pInterp);
  
  // If there's an initialization script then run it now:
  
  if (m_initScript != "") {
    if (access(m_initScript.c_str(), R_OK) == 0) {
            pInterp->EvalFile(m_initScript.c_str());
    } else {
            throw CErrnoException("Checking accessibility of --init-script");
    }
  }
  // Save the main thread id and interpreter:
  
  Globals::mainThreadId     = Tcl_GetCurrentThread();
  Globals::pMainInterpreter = pInterp;
  
    // Instantiate the live event loop and run it.
    
  CTCLLiveEventLoop* pEventLoop = CTCLLiveEventLoop::getInstance();
  pEventLoop->start(pInterp);

  return TCL_OK;
}
/*
   Create the buffer pool.  The following are configurable parameters at the
   top of this file;
   - bufferCount  - Number of buffers to create.
   - bufferSize   - Size (in bytes) of the buffer (payload).

*/
void
CTheApplication::initializeBufferPool()
{
  Globals::usbBufferSize = bufferSize;
  for(uint i =0; i < bufferCount; i++) {
    DataBuffer* p = createDataBuffer(bufferSize);
    gFreeBuffers.queue(p);
  }
}

/*
   Make a configuration filename:  This is done by taking a basename
   and prepending the home directory and config subdir to its path:

*/
string
CTheApplication::makeConfigFile(string baseName)
{
  string home(getenv("HOME"));
  string pathsep("/");
  string config("config");
  string dir;
  
  // The user can define a CONFIGDIR env variable to move the configuration dir.

  if (getenv("CONFIGDIR")) {
    dir =getenv("CONFIGDIR");
  } else {
    dir = home + pathsep + config;
  }


  string result = dir +  pathsep + baseName;
  return result;

}

/**
 * Determine the output ring.  If one is specified that one is used.
 * if not, a ring named after the current logged in user is used instead.
 * 
 * @param pRingName - If not null, this is the name of the ring and overrides the default ring.
 *                    If null a default ring name is constructed and returned.
 * 
 * @return std::string
 * @retval Name of ring to which we should connnect.
 *
 * @throws std::string - If the ring name is defaulted but  we can't figure out what it should be
 *                       due to system call failures.
 */
std::string
CTheApplication::destinationRing(const char* pRingName)
{
  if (pRingName) {
    return std::string(pRingName);
  } else {
    return CRingBuffer::defaultRing();
  }
}

/** 
 * ExitHandler
 *   This is invoked when the application is about to exit.  If the acquisition thread
 *   is actively taking data, we stop data taking and flush the buffers.  This is an attempt
 *   to reduce the number of times the VM-USB is left in the hung state.
 * 
 * @param pData - Actually a pointer to the application (in case we need it for later).
 *
 */
void
CTheApplication::ExitHandler(ClientData pData)
{
  CAcquisitionThread* pReadout = CAcquisitionThread::getInstance();
  if (pReadout->isRunning()) {
    pReadout->stopDaq();	// Flushes buffers etc. too.
  }
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
CTheApplication::AcquisitionErrorHandler(Tcl_Event* pEvent, int flags)
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
CTheApplication::makeCommand(
    CTCLInterpreter* pInterp, const char* verb, std::string param
)
{
    CTCLObject result;
    result.Bind(pInterp);
    result += verb;
    result += param;
    
    return result;
}

/*-------------------------------------------------------------------------------------------*/


/*
  Initialize the application.  All we need to do is instantiate a CTheApplication object
  and pass it the argc/argv unmolested.

  @param argc - The count of command line words.
  @param argv - pointer to an array of command word pointers.

  

*/

int
main(int argc, char** argv)
{
  CTheApplication app;
  
  return app(argc, argv);
}

void* gpTCLApplication(0);	// Needed by the TclPlus library
