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

#include <COutputThread.h>
#include <CCCUSBusb.h>

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <TCLException.h>
#include <TCLLiveEventLoop.h>
#include <CBeginRun.h>
#include <CEndRun.h>
#include <CInit.h>
#include <CPauseRun.h>
#include <CResumeRun.h>
#include <CExit.h>
#include <Exception.h>
#include <tcl.h>
#include <DataBuffer.h>
#include <TclServer.h>
#include <CRingBuffer.h>
#include <CAcquisitionThread.h>
#include <CRunState.h>
#include <CControlQueues.h>
#include <NSCLDAQLog.h>
#include <CMutex.h>
#include <io.h>

#include <CPortManager.h>
#include <Events.h>

#include <vector>

#include <usb.h>
#include <sysexits.h>
#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>

#include "cmdline.h"
#include <XXUSBUtil.h>
#include <USBDevice.h>


#ifndef NULL
#define NULL ((void*)0)
#endif

using namespace std;

//   Configuration constants:
static const char* versionString = VERSION ;


static int    tclServerPort(27000); // Default Tcl server port.
static  string daqConfigBasename("daqconfig.tcl");
static  string ctlConfigBasename("controlconfig.tcl");
static const uint32_t bufferCount(32); // Number of buffers that can be inflight.

CMutex logLock;

// Static member variables and initialization.

bool CTheApplication::m_Exists(false);
std::string CTheApplication::m_InitScript("");

/*!
   Construct ourselves.. Note that if m_Exists is true,
   we BUGCHECK.
*/
CTheApplication::CTheApplication()
  : m_sysControl()
{
  if (m_Exists) {
    cerr << "Attempted to create more than one instance of the application\n";
    exit(EX_SOFTWARE);
  }
  m_Exists = true;
  m_pInterpreter = static_cast<CTCLInterpreter*>(NULL);
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
  struct gengetopt_args_info arg_struct; // parsed command line args.

  m_Argc   = argc;		// In case someone else wants them.
  m_Argv   = argv; 


  // Parse the command line parameters.  This exits on failure:

  cmdline_parser(argc, argv, &arg_struct);
  
  // Save the state of  the --quickstart flag and warn the user if they've
  // enabled it:
  
  if (arg_struct.quickstart_arg == quickstart_arg_off) {
    m_quickstartEnabled = false;
  } else if (arg_struct.quickstart_arg == quickstart_arg_on) {
    m_quickstartEnabled = true;
    
    std::cerr << "** WARNING - you have enabled the --quickstart option.\n";
    std::cerr << "   This will only turn out well for you if you have a self-contained\n";
    std::cerr << "   daqconfig file.  That is a daqconfig file that does not \n";
    std::cerr << "   depend in *any* way on external files.  If you are not sure\n";
    std::cerr << "   if this is the case, don't enable this flag.  If you *are*\n";
    std::cerr << "   sure this is the case you may have improved run start times\n";
    std::cerr << "YOU. HAVE. BEEN. WARNED!!!\n";
  } else {
    std::cerr << "Invalid value for --quickstart\n";
    exit(EXIT_FAILURE);
  }
  
  // Set up logging data and logger:
  
  if (arg_struct.log_given) m_logFile = arg_struct.log_arg;
  m_logLevel = arg_struct.debug_arg;
  setupLogging();
  Globals::pApplication = this;

  // Save the data source id:
  
  Globals::sourceId = arg_struct.sourceid_arg;
  
  // If a timstamp lib was given save that as well:
  
  
  
  Globals::pTimestampExtractor = 0;
  if (arg_struct.timestamplib_given) {
    size_t libLen = strlen(arg_struct.timestamplib_arg);
    Globals::pTimestampExtractor = reinterpret_cast<char*>(malloc(libLen + 1));
    strcpy(
        Globals::pTimestampExtractor, arg_struct.timestamplib_arg
    );
  }


  //cerr << "CC-USB scriptable readout version " << versionString << endl;

  // If we were just asked to enumerate the interfaces do so and exit:

  if(arg_struct.enumerate_given) {
    enumerate();
    exit(EXIT_SUCCESS);
  }

    // Set up the --init-script if it's been supplied:
    
    if (arg_struct.init_script_given) {
        m_InitScript = arg_struct.init_script_arg;
        m_sysControl.setInitScript(arg_struct.init_script_arg);
    }

  try {				// Last chance exception catching...
    
    createUsbController(arg_struct.serialno_given ? arg_struct.serialno_arg : NULL);
    setConfigFiles(arg_struct.daqconfig_given ? arg_struct.daqconfig_arg : NULL,
                   arg_struct.ctlconfig_given ? arg_struct.ctlconfig_arg : NULL);
    initializeBufferPool();
    startOutputThread(destinationRing(arg_struct.ring_given ? arg_struct.ring_arg : NULL));
    
    // Figure out which port to ask the tcl server to start on (see Issue #435).
    
    if (arg_struct.port_given) {
      std::string portString = arg_struct.port_arg;
      if (portString == "managed") {      // Use port manager.
         // We'll use CCUSBReadout:connectionstring as our app.
         
         std::string appName="CCUSBReadout:";
         if (arg_struct.serialno_given) {
            appName += arg_struct.serialno_arg;
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
            exit(EXIT_FAILURE);
        } else {
            tclServerPort = port;
        }
      }
    }
    // Start the tcl server.
    
    startTclServer( tclServerPort);

    startInterpreter();
  }
  catch (string msg) {
    cerr << "CTheApplication caught a string exception: " << msg << endl;
    
  }
  catch (const char* msg) {
    cerr << "CTheApplication caught a char* excpetion " << msg << endl;
    

  }
  catch (CException& error) {
    cerr << "CTheApplication caught an NCLDAQ exception: " 
	 << error.ReasonText() << " while " << error.WasDoing() << endl;
    

  }
  catch (...) {
    cerr << "CTheApplication thread caught an excpetion of unknown type\n";
    

  }
  //  If acquisition is active shut it down.
  
  CRunState* pState = CRunState::getInstance();
  if (pState->getState() == CRunState::Active) {
    CControlQueues* pControl = CControlQueues::getInstance();
    pControl->EndRun();
  }
  
  return EX_SOFTWARE; // keep compiler happy, startInterpreter should not return.
}
/**
 * getInstance
 *    Returns the the application instance.
 * @return CTheApplication*
 */
CTheApplication*
CTheApplication::getInstance()
{
  return Globals::pApplication;
}

/**
 * logStateChangeRequest
 *    Log (info level) a state change request message.
 *
 *  @param msg -log message.
 */
void
CTheApplication::logStateChangeRequest(const char* msg)
{
  if(!m_logFile.empty()) {
    CriticalSection l(logLock);
    daqlog::info(msg);
  }
}
/**
 * logStateChangeStatus
 *    Logs the final status of a state change (debug).
 *
 *  @param msg - the message to log.
 */
void
CTheApplication::logStateChangeStatus(const char* msg)
{
  if (!m_logFile.empty()) {
    CriticalSection l(logLock);
    daqlog::debug(msg);
  }
}
/**
 * logProgress
 *   Logs progress (trace) through a state transition.
 *
 * @param msg - message to log.
 */
void
CTheApplication::logProgress(const char* msg)
{
  if(!m_logFile.empty()) {
    CriticalSection l(logLock);
    daqlog::trace(msg);
  }
}
/*
   Start the output thread.  This thread is responsible for 
   reformatting and transferring buffers of data from the CC-USB to 
   a ring.  This thread is continuously running for the life of the program.
   .. therefore we are sloppy with storage management.

   @param ring - Name of the ring COutputThread will use as its output

*/
void
CTheApplication::startOutputThread(std::string ring)
{
  COutputThread* router = new COutputThread(ring.c_str(), m_sysControl);
  m_pOutputThread = router;
  router->start();
  

}

/*
    Start the Tcl interpreter, we use the static AppInit as a trampoline into the
    interpreter configuration and back to the interpreter event loop so the
    default Tcl event loop can be used.
*/
void
CTheApplication::startInterpreter()
{
//  Tcl_CreateExitHandler(CTheApplication::ExitHandler, reinterpret_cast<ClientData>(this));
//  Tcl_Main(m_Argc, m_Argv, CTheApplication::AppInit);
  m_sysControl.run(m_Argc, m_Argv);
}

/*!
   Create the USB controller.  
   
   @param  pSerialNo - If not null, the serial number of the CC-USB to
                       use. Otherwise, the first one in the enumeration is used.

*/
void
CTheApplication::createUsbController(const char* pSerialNo)
{
  
  USBDevice* pMyController(nullptr);
  if (pSerialNo) {
    pMyController = CCCUSBusb::findBySerial(pSerialNo);
  } else {
    auto devices = XXUSBUtil::enumerateCCUSB(*(CCCUSBusb::getUsbContext()));
    if (devices.size()) {
      pMyController = devices[0].second;
      for (int i =1; i < devices.size(); i++) { // Kill off other controllers
        delete devices[i].second;
      }
    } else {
      throw std::string("There are no CCUSB devices attached to the system");
    }
  }
  
  // or there are no devices:
  
  if (!pMyController) {
    std::string msg = "Unable to find a CC-USB with the serial number: ";
    msg += pSerialNo;
    throw msg;
  }

  Globals::pUSBController = new CCCUSBusb(pMyController);

}

/**
 * Enumerate the controller serial numbers to stdout:
 */
void 
CTheApplication::enumerate()
{
  auto ccusbs = XXUSBUtil::enumerateCCUSB(*(CCCUSBusb::getUsbContext()));
  

  for (int i = 0; i < ccusbs.size(); i++) {
    std::cout << "[" << i << "] : " << ccusbs[i].second << std::endl;
    delete ccusbs[i].second;               // Kill the controller.
  }
}
  
/* 
  Set the configuration files to the global storage

  @param pDaqConfig - if not null a pointer to the to the path to the daq configuration
                      file.  If null a default is used.
  @param pCtlConfig - if not null a pointer to the path to the contrl configuration.
                      if null a default is used.


*/
void
CTheApplication::setConfigFiles(const char* pDaqConfig, const char* pCtlConfig)
{
  Globals::configurationFilename = pDaqConfig ? pDaqConfig : makeConfigFile(daqConfigBasename);
  Globals::controlConfigFilename = pCtlConfig ? pCtlConfig : makeConfigFile(ctlConfigBasename);

}


/*
   Make a configuration filename:  This is done by taking a basename
   and prepending the home directory and config subdir to its path:

*/
string
CTheApplication::makeConfigFile(string baseName)
{
  std::string result = io::getReadableFileFromEnvdir("CONFIGDIR", baseName.c_str());
  if (result != "") return result;
  
  std::string path = "/config/";
  path            += baseName;
  return io::getReadableFileFromHome(path.c_str());
  
}


void* gpTCLApplication(0);	// Need this for tclplus.


/*
   Create the buffer pool.  The following are configurable parameters at the
   top of this file;
   - bufferCount  - Number of buffers to create.
   - Globals::bufferSize   - Size (in bytes) of the buffer (payload).

*/
void
CTheApplication::initializeBufferPool()
{
  for(uint i =0; i < bufferCount; i++) {
    DataBuffer* p = createDataBuffer(Globals::bufferSize);
    gFreeBuffers.queue(p);
  }
}
/* 
   Start the Tcl server.  It will listen on port tclServerPort, seee above..
   Again, the tcl server runs the lifetime of the program so we are 
   sloppy about storage management.

   @param port - the port on which the tcl server will listen.
*/
void
CTheApplication::startTclServer(int port)
{
  TclServer* pServer = new TclServer(m_sysControl);
  pServer->start(port, Globals::controlConfigFilename.c_str(),
		   *Globals::pUSBController);
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
 * setupLogging
 *    Do whatever logging initialization is required.
 */
void
CTheApplication::setupLogging()
{
   if (!m_logFile.empty()) {
    daqlog::setLogFile(m_logFile);
    switch (m_logLevel) {
      case 0:
        daqlog::setLogLevel(daqlog::Info);
        break;
      case 1:
        daqlog::setLogLevel(daqlog::Debug);
        break;
      case 2:
        daqlog::setLogLevel(daqlog::Trace);
        break;
      default:
        std::cerr << "Invalid log level : " << m_logLevel
          << " must be 0,1,2\n";
        exit(EXIT_FAILURE);
    }
   }
}
/**
 * Create the application object and transfer control to it.
 *
 * @param argc - number of command line words.
 * @param argv - array of pointers to comandline words argv[0] points to the
 *              name of the program as typed on the command line.
 *
 * @return int
 *
 */
int 
main(int argc, char** argv)
{
  CTheApplication app;
  return app(argc, argv);
}
