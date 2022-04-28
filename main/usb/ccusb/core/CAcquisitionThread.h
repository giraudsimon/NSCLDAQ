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

#ifndef CACQUISITIONTHREAD_H
#define CACQUISITIONTHREAD_H

using namespace std;		// required for spectrodaq includes.


#include <vector>
#include <string>
#include "CControlQueues.h"

#include <CSynchronizedThread.h>


// forward class definitions.

class CCCUSB;
class CReadoutModule;
class CCCUSBReadoutList;
struct DataBuffer;


/*!
   This is the thread that does the data acquisition.
   As coded this is a singleton class as well, however as a thread of execution,
   it gets started at the beginning of a run and politely requested to stop at
   the end of a run.
*/
class CAcquisitionThread : public CSynchronizedThread
{
private:
  static bool                   m_Running;	//!< thread is running.
  static CCCUSB*                m_pCamac;		//!< VME interface.

  bool                          m_haveScalerStack;
  bool                          m_reconnected;
  std:: string                  m_lastChecksum;
  //Singleton pattern stuff:


private:
  static CAcquisitionThread*    m_pTheInstance;
  CAcquisitionThread();

public:
  static CAcquisitionThread*   getInstance();

  // Thread functions:

public:
  static void start(CCCUSB* usb);
  static bool isRunning();
  static void waitExit();	/* Wait for this thread to exit (join). */

  virtual void init();

  void setReconnected(bool value) {
    m_reconnected = value;
  }
protected:
  virtual void operator()();
private:
  void mainLoop();
  void processCommand(CControlQueues::opCode command);
  void processBuffer(DataBuffer* pBuffer);
  void startDaq();
public:				// To allow exit handler to stop acquisition
  void stopDaq();
  void stopDaqImpl();
private:
  void pauseDaq();
  void CCusbToAutonomous();
  void drainUsb();
  void beginRun();
  void endRun();
  void pauseRun();     // Bug #5882
  void resumeRun();    // Bug #5882
  void reportErrorToMainThread(std::string msg);
  bool mustReload();
  bool isChecksumChanged();
  std::string computeMd5(const char* filename);
  
};

#endif
