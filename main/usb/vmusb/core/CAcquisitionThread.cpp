
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
#include "CAcquisitionThread.h"
#include "CTheApplication.h"
#include <CSystemControl.h>
#include <CReadoutModule.h>
#include <CStack.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <DataBuffer.h>
#include <CControlQueues.h>
#include <event.h>
#include "CRunState.h" 
#include <CConfiguration.h>
#include <Globals.h>     // Need to maintain the running global.
#include <CMutex.h>
#include <assert.h>
#include <time.h>
#include <string>
#include <Exception.h>
#include <TclServer.h>
#include <os.h>
#include <tcl.h>

#include <iostream>

#include <string.h>
#include <errno.h>
#include <unistd.h>

using namespace std;


static const unsigned DRAINTIMEOUTS(5); // # consecutive drain read timeouts before giving up.
static const unsigned USBTIMEOUT(2);

static const unsigned ReadoutStackNum(0);
static const unsigned ScalerStackNum(1);

static const unsigned HungCount(3); // Hung if HungCount * USBTimeout seconds without data.


// buffer types:
//


bool                CAcquisitionThread::m_Running(false);
CVMUSB*             CAcquisitionThread::m_pVme(0);
CAcquisitionThread* CAcquisitionThread::m_pTheInstance(0);
vector<CReadoutModule*> CAcquisitionThread::m_Stacks;

unsigned long CAcquisitionThread::m_tid; // Thread id of the running thread.


/*!
  Construct the the acquisition thread object.
  This is just data initialization its the start member that is
  important.
 */

CAcquisitionThread::CAcquisitionThread() {

}

/*!
  Return the singleton instance of the acquisition thread object:

 */
  CAcquisitionThread*
CAcquisitionThread::getInstance() 
{
  if (!m_pTheInstance) {
    m_pTheInstance =  new CAcquisitionThread;
  }
  return m_pTheInstance;
}

/*!
  Start the thread.
  - get an instance which, if necessary forces creation.
  - Set the adc/scaler lists.
  - Initiate the thread which gets the ball rolling.
  \param usb    :CVMUSB*
  Pointer to the vme interface object we use to deal with all this stuff.

 */
  void
CAcquisitionThread::start(CVMUSB* usb)
{

  CAcquisitionThread* pThread = getInstance();
  m_pVme = usb;



  // starting the thread will eventually get operator() called and that
  // will do all the rest of the work in thread context.

  pThread->CSynchronizedThread::start();


}
/**
 * Adaptor to between spectrodaq and nextgen threading models:
 */
//  void
//CAcquisitionThread::run()
//{
//  m_tid = getId();
//  operator()();
//}

/**
 * 
 */
void CAcquisitionThread::init()
{
  m_tid = getId();

  try {

    cout << "CAcquisitionThread::init started" << endl;

    Globals::running = true;
    CRunState::getInstance()->setState(CRunState::Starting);

    m_Running = true;   // Thread is off and running now.

    startDaq();
    beginRun();     // Emit begin run buffer.
  
    cout << "CAcquisitionThread::init done" << endl;
  }
    catch (string msg) {
    Globals::running = false;
    cerr << "CAcquisition thread caught a string exception: " << msg << endl;
  }
  catch (char* msg) {
    Globals::running = false;
    cerr << "CAcquisition thread caught a char* exception: " << msg << endl;
  }
  catch (CException& err) {
    Globals::running = false;
    cerr << "CAcquisition thread caught a daq exception: "
      << err.ReasonText() << " while " << err.WasDoing() << endl;
  }
  catch (...) {
    Globals::running = false;
    cerr << "CAcquisition thread caught some other exception type.\n";
  }

}

/*!
  Returns true if the thread is running.
 */
  bool
CAcquisitionThread::isRunning()
{
  return m_Running;
}

/*!
  Wait for the thread to exit.
 */
  void
CAcquisitionThread::waitExit()
{
  getInstance()->join();
}

/*!
  Entry point for the thread.
 */
void CAcquisitionThread::operator()()
{
  std::string errorMessage;
  try {

    mainLoop();     // Enter the main processing loop.
  }
  catch (std::string msg) {
    stopDaq();                       // Ensure we're not in ACQ moe.
    errorMessage = msg;
  }
  catch (const char* msg) {
    stopDaq();                       // Ensure we're not in ACQ moe.
    errorMessage = msg;
  }
  catch (CException err) {
    stopDaq();                       // Ensure we're not in ACQ moe.
    errorMessage = err.ReasonText();
  }
  catch (...) {
    //  This is a normal exit...
  }
  Globals::running = false;

  CRunState* pState = CRunState::getInstance();
  pState->setState(CRunState::Idle);
  endRun();			// Emit end run buffer.


  m_Running = false;		// Exiting.

  // If there's an error message report the error to the main thread:

  if (errorMessage != "") {
    reportErrorToMainThread(errorMessage);
    return;
  }

  Globals::running = false;
  pState->setState(CRunState::Idle);
  m_Running = false;

}

/*!
  The main loop is simply one that loops:
  - Reading a buffer from the vm-usb and processing it if one comes in.
  - Checking for control commands and processing them if they come in.
 */
  void
CAcquisitionThread::mainLoop()
{
  DataBuffer*     pBuffer   = gFreeBuffers.get();
  CControlQueues* pCommands = CControlQueues::getInstance(); 
  try {
    unsigned int consecutiveTimeouts = 0;
    while (true) {

      // Event data from the VM-usb.

      size_t bytesRead;
      int status = m_pVme->usbRead(pBuffer->s_rawData, pBuffer->s_storageSize,
          &bytesRead,
          USBTIMEOUT*1000 );
      if (status == 0) {
        consecutiveTimeouts = 0;
        pBuffer->s_bufferSize = bytesRead;
        pBuffer->s_bufferType   = TYPE_EVENTS;
        processBuffer(pBuffer); // Submitted to output thread so...
        pBuffer = gFreeBuffers.get(); // need a new one.
      } 
      else {
        if (errno != ETIMEDOUT) {
          cerr << "Bad status from usbread: " << strerror(errno) << endl;
          cerr << "Ending the run... check the VME Crate.. If it power cycled restart this program\n";
          throw 1;
        }

        consecutiveTimeouts++;
        // 
        // The VM-USB can drop out of data taking mode.
        // in that case all our reads will time out.
        // so if we have a bunch of of timeouts in a row, restart the VM-USB

        if((consecutiveTimeouts > HungCount) && 0) { // disable hung reset.
          cerr << "Looks like VM-USB may be hung.. attempting to restart\n";
          stopDaq();
          startDaq();
          consecutiveTimeouts = 0;

        }
      }
      // Commands from our command queue.

      CControlQueues::opCode request;
      bool   gotOne = pCommands->testRequest(request);
      if (gotOne) {
        processCommand(request);
      }
    }
  }
  catch (...) {
    gFreeBuffers.queue(pBuffer);  // Don't lose buffers!!
    throw;
  }
}
/*!
  Process a command from our command queue.  The command can be one of the
following:
ACQUIRE   - The commander wants to acquire the VM-USB we must temporarily
shutdown data taking and drain the VMusb...then ack the message
and wait paitently for a RELEASE before restarting.
PAUSE     - Commander wants the run to pause.
END       - The commander wants the run to end.
 */
  void
CAcquisitionThread::processCommand(CControlQueues::opCode command)
{
  CControlQueues* queues = CControlQueues::getInstance();
  CRunState* pState = CRunState::getInstance();
  pState->setState(CRunState::Stopping);
  if (command == CControlQueues::ACQUIRE) {
    stopDaqImpl();
    queues->Acknowledge();
    CControlQueues::opCode release  = queues->getRequest();
    assert(release == CControlQueues::RELEASE);
    queues->Acknowledge();
    VMusbToAutonomous();
    pState->setState(CRunState::Active);
  }
  else if (command == CControlQueues::END) {
    stopDaq();
    queues->Acknowledge();
    throw 0;
  }
  else if (command == CControlQueues::PAUSE) {
    pauseRun();
    pauseDaq();

  }
  else {
    // bad error:

    assert(0);
  }
}
/*!
  Process a buffer of data from the VM-USB (not artificially generated
  buffers.  There are only two types of buffers that can come from the
  VM-USB:
  - Event data
  - Scalers
  These are distinguishable from the Scaler bit in the header word of the
  data from the VM-USB.
  \param buffer : DataBuffer*
  the buffer of data from the VM-USB.  The following fiels have been set:
  - s_storageSize  : number of bytes of physical storage in a buffer. 
  - s_bufferType.
  - s_buffersSize  : number of bytes read from the USB.
  - s_rawData      : the data read from the USB.

  The disposition of full buffers is done by another thread that is connected
  to us by a buffer queue.  Once we fill in the buffer type we just have to
  submit the buffer to the queue.. which can wake up the output thread and
  cause a scheduler pass... or not.

 */
  void
CAcquisitionThread::processBuffer(DataBuffer* pBuffer)
{
  timespec acquiredTime;
  clock_gettime(CLOCK_REALTIME, &acquiredTime); // CLOCK_REALTIME is the only gaurantee
  pBuffer->s_timeStamp  = acquiredTime;

  // In this version, all stack ids are good. 
  // stack 7 is queued as an event to the tcl server as it contains monitor events.
  //
  // The output thread get all other stack data and will ensure that
  // stack 1 completions are scalers and all others are events.

  if ((pBuffer->s_bufferType == TYPE_EVENTS) &&((pBuffer->s_rawData[1] >> 13) & 0x7) == 7) {
    ::Globals::pTclServer->QueueBuffer(pBuffer);
    gFreeBuffers.queue(pBuffer);
  } 
  else {
    gFilledBuffers.queue(pBuffer);  // Send it on to be routed to spectrodaq in another thread.
  }
}
/*!
  startDaq start data acquisition from a standing stop. To do this we need to:
  - Emit a begin run buffer.
  - Create VMUSBReadoutLists from the m_scalers and m_adcs
  - Download those lists into the VM-USB, list 2 will be used for event readout.
  list 1 for scalers.
  - Set the trigger for list 2 to be interrupt number 1 with vmeVector and
  vmeIPL as described in the static consts in this module
  \bug May want to make this parameterizable, but probably not necessary.
  - Set the buffersize to 13k (max). to get optimum throughput.
  - Setup scaler readout to happen every 10 seconds.
  - Initialize the hardware
  - Start USB data acuisition.

 */
  void
CAcquisitionThread::startDaq()
{
  CriticalSection lock(CVMUSB::getGlobalMutex());

  //  First do a bulk read just to flush any crap that's in the VM-USB
  // output fifo..as it appears to sometimes leave crap there.
  // ignore any error status, and use a short timeout:

  cerr << "Flushing any garbage in the VM-USB fifo...\n";

  char junk[1000];
  size_t moreJunk;
  m_pVme->usbRead(junk, sizeof(junk), &moreJunk, 1*1000); // One second timeout.

  cerr << "Starting VM-USB initialization\n";


  // Now we can start preparing to read...

  //  m_pVme->writeActionRegister(CVMUSB::ActionRegister::sysReset);
  m_pVme->writeActionRegister(0);

  // Set up the buffer size and mode:

  m_pVme->writeBulkXferSetup(0 << CVMUSB::TransferSetupRegister::timeoutShift); // don't want multibuffering...1sec timeout is fine.

  // The global mode:
  //   13k buffer
  //   Single event seperator.
  //   Aligned on 16 bits.
  //   Single header word.
  //   Bus request level 4.
  //   Flush scalers on a single event.
  //
  m_pVme->writeGlobalMode((4 << CVMUSB::GlobalModeRegister::busReqLevelShift) | 
                            //        CVMUSB::GlobalModeRegister::flushScalers |
                            // CVMUSB::GlobalModeRegister::mixedBuffers        |
                            // CVMUSB::GlobalModeRegister::spanBuffers         |
                            (CVMUSB::GlobalModeRegister::bufferLen13K << 
                                  CVMUSB::GlobalModeRegister::bufferLenShift));



  // Process the configuration. This must be done in a way that preserves the
  // Interpreter since loadStack and Initialize for each stack will need the
  // interpreter for our support of tcl drivers.

  Globals::pConfig = new CConfiguration;
  Globals::pConfig->processConfiguration(Globals::configurationFilename);
  m_Stacks = Globals::pConfig->getStacks();

  //  Get all of the stacks.  load and enable them.  First we'll reset the
  // stack load offset.

  CStack::resetStackOffset();

  cerr << "Loading " << m_Stacks.size() << " stacks to vm-usb\n";
  m_haveScalerStack = false;
  for(int i =0; i < m_Stacks.size(); i++) {
    CStack* pStack = dynamic_cast<CStack*>(m_Stacks[i]->getHardwarePointer());
   
    assert(pStack);
    if (pStack->getTriggerType()  == CStack::Scaler) {
      m_haveScalerStack = true;
    }
    pStack->Initialize(*m_pVme);    // INitialize daq hardware associated with the stack.
    pStack->loadStack(*m_pVme);     // Load into VM-USB
    pStack->enableStack(*m_pVme);   // Enable the trigger logic for the stack.


  }

  // there could be a TclServer stack as well:

  TclServer*          pServer = ::Globals::pTclServer;
  CVMUSBReadoutList   list    = pServer->getMonitorList();
  if (list.size() != 0) {
    std::cerr << "Loading monitor stack of size: " << list.size() << " at offset: " << CStack::getOffset() << std::endl;
    size_t currentOffset = CStack::getOffset();
    m_pVme->loadList(7, list, currentOffset); // The tcl server will periodically trigger the list.
  }


  // Start the VMUSB in data taking mode:

  VMusbToAutonomous();

}
/*!
  Stop data taking this involves:
  - Forcing a scaler trigger (action register write)
  - Setting clearing the DAQ start bit (action register write)
  - draining data from the VMUSB:
  - Call shutdown the hardware in the stacks
 */
  void CAcquisitionThread::stopDaqImpl()
  {
      if (m_haveScalerStack) {
          m_pVme->writeActionRegister(CVMUSB::ActionRegister::scalerDump);
      }
      m_pVme->writeActionRegister(0);
      drainUsb();

      cerr << "Running on end routines" << endl;
      for(int i =0; i < m_Stacks.size(); i++) {
          CStack* pStack = dynamic_cast<CStack*>(m_Stacks[i]->getHardwarePointer());

          assert(pStack);
          pStack->onEndRun(*m_pVme);   // Enable the trigger logic for the stack.
      }

      // turn off interrupts so that interrupts don't continue to trigger if the
      // user chose to turn them off between runs
      disableInterrupts();
  }

  /*!
   * \brief CAcquisitionThread::stopDaq
   *
   * This delegates all logic to stopDaqImpl but adds thread_local
   * synchronization to it.
   */
  void
CAcquisitionThread::stopDaq()
{
  CriticalSection lock(CVMUSB::getGlobalMutex());
  stopDaqImpl();
}

void CAcquisitionThread::disableInterrupts()
{
  // disable the interrupt service vectors
  for (int regIdx=1; regIdx<=4; ++regIdx) {
    m_pVme->writeVector(regIdx, 0);
  }
  // further... mask all of the interrupt request levels
  m_pVme->writeIrqMask(0x7f);
}

/*!
  Pause the daq. This means doing a stopDaq() and fielding 
  requests until resume or stop was sent.

 */
  void
CAcquisitionThread::pauseDaq()
{
  CControlQueues* queues = CControlQueues::getInstance();
  CRunState* pState = CRunState::getInstance();
  pState->setState(CRunState::Stopping); // No monitoring for now.
  stopDaq();
  pState->setState(CRunState::Paused);    // Fully paused.
  queues->Acknowledge();
  

  while (1) {
    CControlQueues::opCode req = queues->getRequest();

    // Acceptable actions are to acquire the USB, 
    // End the run or resume the run.
    //

    if (req == CControlQueues::ACQUIRE) {
      queues->Acknowledge();
      CControlQueues::opCode release = queues->getRequest();
      assert (release == CControlQueues::RELEASE);
      queues->Acknowledge();
    }
    else if (req == CControlQueues::END) {
      queues->Acknowledge();
      throw 0;
    }
    // Bug #5882 - pause/resume elements need to be emitted.
    else if (req == CControlQueues::PAUSE) {    // Really should not happen.
      pauseRun();
    }
    else if (req == CControlQueues::RESUME) {
      resumeRun();             // Bug #5882
      startDaq();
      queues->Acknowledge();
      return;
    }
    else {
      assert(0);
    }
  }
  pState->setState(CRunState::Active);

}
/*!
  Turn on Data taking this is just a write of CVMUSB::ActionRegister::startDAQ
  to the action register
 */
  void
CAcquisitionThread::VMusbToAutonomous()
{
    CRunState* pState = CRunState::getInstance();

    m_pVme->writeActionRegister(CVMUSB::ActionRegister::startDAQ);
    pState->setState(CRunState::Active);
}
/*!
  Drain usb - We read buffers from the DAQ (with an extended timeout)
  until the buffer we get indicates it was the last one (data taking turned off).
  Each buffer is processed normally.
 */
  void
CAcquisitionThread::drainUsb()
{
  bool done = false;
  DataBuffer* pBuffer = gFreeBuffers.get();
  int timeouts(0);
  size_t bytesRead;
  cerr << "CAcquisitionThread::drainUsb...\n";
  do {
    int    status = m_pVme->usbRead(pBuffer->s_rawData, pBuffer->s_storageSize,
        &bytesRead, 
        DRAINTIMEOUTS*1000); // 5 second timeout!!
    if (status == 0) {
      pBuffer->s_bufferSize = bytesRead;
      pBuffer->s_bufferType   = TYPE_EVENTS;
      cerr << "Got a buffer, with type header: " << hex << pBuffer->s_rawData[0] << endl;
      if (pBuffer->s_rawData[0] & VMUSBLastBuffer) {
        bootToTheHead();
        cerr << "Done\n";
        done = true;
      }
      processBuffer(pBuffer);
      pBuffer = gFreeBuffers.get();
    }
    else {
      timeouts++;   // By the time debugged this is only failure.
      cerr << "Read timed out\n";
      if(timeouts >= DRAINTIMEOUTS) {
        cerr << "Warning: drainUsb() persistent timeout assuming already drained\n";
        bootToTheHead();
        done = true;
      }
    }
  } while (!done);


  gFreeBuffers.queue(pBuffer);
  cerr << "Done finished\n";

}
/**
 * TODO:
 *    beginRun, endRun, pauseRun, resumeRun can be factored.
 */
/*!
  Emit a begin run buffer to the output thread.
  the key info in this buffer is just its type and timestamp.
  No info at all is in the body, however since the buffer will be returned
  to the free list we have to get it from there to begin with:
 */
  void
CAcquisitionThread::beginRun()
{
  DataBuffer* pBuffer   = gFreeBuffers.get();
  pBuffer->s_bufferSize = pBuffer->s_storageSize;
  pBuffer->s_bufferType = TYPE_START;
  processBuffer(pBuffer); // Rest gets taken care of there.
}
/*!
  Emit an end of run buffer.
  This is just like a begin run buffer except for the buffer type:
 */
  void
CAcquisitionThread::endRun()
{

  DataBuffer* pBuffer   = gFreeBuffers.get();
  pBuffer->s_bufferSize = pBuffer->s_storageSize;
  pBuffer->s_bufferType = TYPE_STOP;
  processBuffer(pBuffer);
}
/**
 *   pauseRun    (Bug #5882)
 *     Emit a pause run item.
 */
void
CAcquisitionThread::pauseRun()
{
  DataBuffer* pBuffer   = gFreeBuffers.get();
  pBuffer->s_bufferSize = pBuffer->s_storageSize;
  pBuffer->s_bufferType = TYPE_PAUSE;
  processBuffer(pBuffer);  
}
/**
 * resumeRun    (Bug#5882)
 *   Emit a resume run item.
 */
void
CAcquisitionThread::resumeRun()
{
DataBuffer* pBuffer   = gFreeBuffers.get();
  pBuffer->s_bufferSize = pBuffer->s_storageSize;
  pBuffer->s_bufferType = TYPE_RESUME;
  processBuffer(pBuffer);  
}

/*!
  Do a 'drastic purge' of the VM-USB.
 */
  void
CAcquisitionThread::bootToTheHead()
{
  uint32_t junk;
  cerr << "Attempting final drain\n";
  //   m_pVme->writeActionRegister(CVMUSB::ActionRegister::sysReset);
  m_pVme->writeActionRegister(0);
  Os::usleep(100);
  m_pVme->vmeRead32(0, CVMUSBReadoutList::a32UserData, &junk);
  uint8_t buffer[13*1024*2];
  size_t  bytesRead;
  int status = m_pVme->usbRead(buffer,
             sizeof(buffer),
             &bytesRead, DRAINTIMEOUTS*1000);

}
/**
 * report an error during acquisition to the main thread by scheduling
 * an event.
 *
 * @param msg - the error message to report.
 */
void
CAcquisitionThread::reportErrorToMainThread(std::string msg)
{
    struct event {
        Tcl_Event     event;
        StringPayload payload;
    } ;
    
    // Allocate and fill in the event:
    
    event* pEvent = reinterpret_cast<event*>(Tcl_Alloc(sizeof(event)));
    pEvent->event.proc = CSystemControl::AcquisitionErrorHandler;
    pEvent->payload.pMessage = Tcl_Alloc(msg.size() +1);
    strcpy(pEvent->payload.pMessage, msg.c_str());
    Tcl_ThreadQueueEvent(
        Globals::mainThreadId, reinterpret_cast<Tcl_Event*>(pEvent),
        TCL_QUEUE_TAIL
    );
    
    
}
