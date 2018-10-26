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

#include "CControlQueues.h"
#include <CSynchronizedThread.h>

#include <vector>
#include <string>


// forward class definitions.

class CVMUSB;
class CReadoutModule;
class CVMUSBReadoutList;
struct DataBuffer;


/*!
   This is the thread that does the data acquisition.
   As coded this is a singleton class as well, however as a thread of execution,
   it gets started at the beginning of a run and politely requested to stop at
   the end of a run.
*/
class CAcquisitionThread : public CSynchronizedThread
{
    // Static member data.
    
private:
  static bool                   m_Running;	//!< thread is running.
  static unsigned long          m_tid;          //!< ID of thread when running.
  static CVMUSB*                m_pVme;		//!< VME interface.
  
  static std::vector<CReadoutModule*>  m_Stacks;       //!< the stacks to run.

  // Per object data (really don't need this 'cause it's a singleton).
  
private:
  bool                         m_haveScalerStack;
  bool                         m_controllerReset;
  std::string                  m_lastChecksum;

  //Singleton pattern stuff:
private:
  static CAcquisitionThread*    m_pTheInstance;
  CAcquisitionThread();

public:
  static CAcquisitionThread*   getInstance();

  // Thread functions:

public:
  static void start(CVMUSB* usb);
  static bool isRunning();
  static void waitExit();	/* Wait for this thread to exit (join). */
  virtual void init(); /* thread-unsafe operations */

  void setControllerResetState(bool value) {
    m_controllerReset = value;
  }
  
protected:
  virtual void operator()();
private:
  void mainLoop();
  void processCommand(CControlQueues::opCode command);
  void processBuffer(DataBuffer* pBuffer);
  void startDaq();
public:
  void stopDaq();		// public for the exit handler.
private:
  void stopDaqImpl();
  void pauseDaq();
  void VMusbToAutonomous();
  void drainUsb();
  void beginRun();
  void endRun();
  void pauseRun();
  void resumeRun();
  void bootToTheHead();
  void reportErrorToMainThread(std::string msg);
  void disableInterrupts();
  bool mustReload();
  bool isChecksumChanged();
  std::string computeMd5(const char* filename);
};

#endif
