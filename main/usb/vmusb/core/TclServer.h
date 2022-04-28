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

#ifndef TCLSERVER_H
#define TCLSERVER_H

#include <CVMUSBReadoutList.h>
#include <CSynchronizedThread.h>
#include <CControlModule.h>
#include <CCtlConfiguration.h>

#include <tcl.h>
#include <vector>
#include <string>
#include <memory>

class CVMUSB;
class CTCLInterpreter;
struct DataBuffer;
class CSystemControl;

/*!
  The TclServer class implements a little Tcl server for our readout software.
  The server is intended to allow external software to perform control operations
  to the VMUSB.  It does so by:
  - Reading a configuration file that is used to create the control objects
    that are available to the remote system (Subclasses of CControlHardware).
  - Adding the commands SET and GET to the interpreter, that manipulate
    or query the hardware (synchronizing with the readout thread if needed
    for access to the VMUSB.
  - runing the Tcp/IP event loop so that Tcp/IP connections and file events can be honored.

   The Tcp/IP server section is completely stolen from the TclServer that comes with the
   nscldaq software.

   Note that this TclServer is a thread in spectrodaq and therefore this class must be
   instantiated and scheduled.  Scheduling is done by invoking the Start method
   which is a simple way to get the thread its parameters.
   Note that the connection authorization has been dumbed down to only accept
   connections from localhost.

*/
class TclServer : public CSynchronizedThread
{
// Member data:
private:
  int                          m_port;		// Port we are listening to for connections.
  std::string                  m_configFilename;
  CVMUSB*                      m_pVme;		// VME controller.
  CCtlConfiguration            m_config;
  CTCLInterpreter*             m_pInterpreter;
  CVMUSBReadoutList*           m_pMonitorList; /* List to perform periodically. */
  Tcl_ThreadId                 m_threadId;
  bool                         m_waitingMonitor;
  static TclServer*            m_pInstance;
  uint16_t*                    m_pMonitorData;
  size_t                       m_nMonitorDataSize;
  bool                         m_dumpAllVariables;
  bool                         m_exitNow;
  bool                         m_isRunning;
  CSystemControl&              m_systemControl;


  // Public data structures:

public:
  // Structure of the event posted to us when we have data from the 
  // monitor list.
  //
  typedef struct _TclServerEvent {
    struct Tcl_Event event;
    void*            pData;
  } TclServerEvent;
  
public:
  TclServer(CSystemControl& sysControl);
  ~TclServer();			// This is a final class.
private:
  TclServer(const TclServer& rhs);
  TclServer& operator=(const TclServer& rhs);
  int operator==(const TclServer& rhs) const;
  int operator!=(const TclServer& rhs) const;


public:
  void            start(int port, const char* configFile, CVMUSB& vme);
  virtual void    setResult(std::string resultText); // virtual for testing purposes only
  void            processMonitorList(void* pData, size_t nBytes);
  void QueueBuffer(void* pBuffer);

  // selectors:

  CVMUSBReadoutList getMonitorList(); /* Allow rdothread to get a copy. */
  CTCLInterpreter*  getInterp() {return m_pInterpreter;} /* For Tcl drivers. */
  Tcl_ThreadId      getTclThreadId()  {return m_threadId; }

  bool              isRunning() const { return m_isRunning; }

  // Adaptor to spectrodaq threading.

  void scheduleExit();
  
public:

  // Thread unsafe initialization
  void init();

  // Initialize the the modules
  void readConfigFile();
  void initModules();

protected:
  void operator()();

private:
  void sendWatchedVariables();
  void initInterpreter();
  void startTcpServer();
  void createMonitorList();
  void EventLoop();
  static void MonitorDevices(ClientData pData); 
  static int  receiveMonitorData(Tcl_Event* pEvent, int flags); /* Gets status buffers when run active */

  static void updateVariables(ClientData pData);

  static int Exit(Tcl_Event* pEvent, int flags);
  void stackTrace();

};
 



#endif
