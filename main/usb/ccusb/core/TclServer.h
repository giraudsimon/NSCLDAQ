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

#include <CSynchronizedThread.h>
#include <CControlModule.h>
#include <CCtlConfiguration.h>

#include <vector>
#include <string>
#include <tcl.h>


class CCCUSB;
class CTCLInterpreter;
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
  CCCUSB*                      m_pVme;		// VME controller.
  std::vector<CControlModule*> m_Modules;       // Hardware we can access.
  CTCLInterpreter*             m_pInterpreter;
  bool                         m_dumpAllVariables;
  bool                         m_exitNow;
  Tcl_ThreadId                 m_tclThreadId;
  CCtlConfiguration            m_config;
  CSystemControl&              m_systemControl;


public:
  TclServer(CSystemControl& systemControl);
  ~TclServer();			// This is a final class.
private:
  TclServer(const TclServer& rhs);
  TclServer& operator=(const TclServer& rhs);
  int operator==(const TclServer& rhs) const;
  int operator!=(const TclServer& rhs) const;


public:
  void start(int port, const char* configFile, CCCUSB& vme);

  virtual void    init();
  CTCLInterpreter* getInterp() { return m_pInterpreter; }

  void readConfigFile();
  void initModules();

  void  scheduleExit();
protected:
  void operator()();

private:
  void stackTrace();
  void initInterpreter();
  void startTcpServer();
  void EventLoop();
  static void updateVariables(void* pThis);
  void sendWatchedVariables();
  static int Exit(Tcl_Event* pEvent, int flags);
};
 



#endif
