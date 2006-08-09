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
#include <CReadoutModule.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <DataBuffer.h>
#include "CControlQueue.h"
#include <asserts.h>
#include <time.h>

using namespace std;

static const uint8_t vmeVector(0x80); // Some middle of the road vector.
static const uint8_t vmeIPL(6);	      // not quite NMI.
static const scalerPeriod(10);	      // Seconds between scaler readouts.
static const scalerPeriodMultiplier(1); // Scaler readout period assumed in secs.
static const DRAINTIMEOUT(5);	// # consecutive drain read timeouts before giving up.

// buffer types:
//


bool                CAcquisitionThread::m_Running(false);
CVMUSB*             CAcquisitionThread::m_pVme(0);
CAcquisitionThread* CAcquistionThread::m_pTheInstance(0);


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
   \param adcs   : std::vector<CReadoutModule*> 
      A vector consisting of the set of modules that will be read out in the
      event mode.  This is expected to be hooked to interrupt 
      80, IPL 6 and that will be used to fire off the list.
   \param scalers : std::vector<CReadoutModule*>
     A vector consisting of the set of scaler modules that will be read out
     every scalerPeriod seconds.

*/
void
CAcquistionThread::start(CVMUSB* usb,
			 vector<CReadoutModule*> adcs,
			 vector<CReadoutModule*> scalers)
{
  CAcquisitionThread* pThread = getInstance();
  m_pVme = usb;
  pThread->m_adcs    = adcs;
  pThread->m_scalers = scalers;

  // starting the thread will eventually get operator() called and that
  // will do all the rest of the work in thread context.

  m_tid = daq_dispatcher.Dispatch(*pThread);
  
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
   Entry point for the thread.
*/
int
CAcquisitionThread::operator()(int argc, char** argv)
{
  m_Running = true;		// Thread is off and running now.

  beginRun();			// Emit begin run buffer.

  startDaq();  		        // Setup and start data taking.
  try {

    mainLoop();			// Enter the main processing loop.
  }
  catch (...) {			// exceptions are used to exit the main loop.?
  }

  endRun();			// Emit end run buffer.

  m_Running = false;		// Exiting.
  return      0;		// Successful exit I suppose.
}

/*!
  The main loop is simply one that loops:
  - Reading a buffer from the vm-usb and processing it if one comes in.
  - Checking for control commands and processing them if they come in.
*/
void
CAcquisitionThread::mainLoop()
{
  DataBuffer*     pBuffer   = gFreeBuffers->get();
  CControlQueues* pCommands = CControlQueues::getInstance(); 
  try {
    while (true) {
      
      // Event data from the VM-usb.
      
      size_t bytesRead
	int status = m_pVme->usbRead(pBuffer->s_rawData, pBuffer->s_storageSize,
				     &bytesRead);
      if (status == 0) {
	pBuffer->s_bufferSize = bytesRead;
	buffer.s_bufferType   = TYPE_EVENTS;
	processBuffer(pBuffer);	// Submitted to output thread so...
	pBuffer = gFreeBuffers->get(); // need a new one.
      }
      // Commands from our command queue.
      
      string request;
      bool   gotOne = pCommands->testRequest(request);
      if (gotOne) {
	processCommand(request);
      }
    }
  }
  catch (...) {
    gFreeBuffers->put(pBuffer);	// Don't lose buffers!!
    throw;
  }
}
/*!
  Process a command from our command queue.  The command can be one of the
  following:
  ACQUIRE   - The commander wants to acquire the VM-USB we must temporarily
              shutdown data taking and drain the VMusb...then ack the message
              and wait paitently for a RELEASE before restarting.

  END       - The commander wants the run to end.
*/
void
CAcquisitionThread::processCommand(string command)
{
  CControlQueues* queues = CControlQueues::getInstance();

  if (command == string("ACQUIRE")) {
    stopDaq();
    queues->Acknowledge();
    string release  = queues->getRequest();
    assert(release == string("RELEASE"));
    queues->Acknowledge();
    VMusbToAutonomous();
  }
  else if (command == string("END")) {
    stopDaq();
    queues->Acknowledge();
    throw "Run ending";
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
  time_t acquiredTime;		// Need to generate  our own timestamps.
  time(&acquiredTime);

  buffer.s_timeStamp  = acquiredTime;
  gFilledBuffers.put(buffer);	// Send it on to be routed to spectrodaq in another thread.
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
  beginRun();			// Begin the run and set the run state.

  // Create the lists and size the event readout list.

  CVMUSBReadoutList*  adcList    = createList(m_adcs);
  CVMUSBReadoutList*  scalerList = createList(m_scalers);

  size_t adcListLines            = adcList.size(); // offset to scaler list.
  
  // Load the lists.

  m_pVme->loadList(2, adcList);	                 // Event readout list.
  m_pVme->loadList(1, scalerList, adcListLines); // Scaler list.

  // Set up the trigger for the event list; the A vector of 
  // vector register 1; the B Vector is unused.

  uint32_t vectorValue = (2 << CVMUSB::ISVRegister::AStackIDShift)      |
                         (vmeVector << CVMUSB::ISVRegister::AVectorShift)  |
                         (vmeIPL << CVMUSB::ISVRegister::AIPLShift);


  m_pVme->writeVector(1, vectorValue);
 
  // Set up the buffer size and mode:

  m_pVme->writeBulkXferSetup(0); // don't want multibuffering...1sec timeout is fine.

  // the DAQ settings;
  //  - No trigger delay is needed since IRQ implies data.
  //  - Scaler readout period 10 seconds.

  m_pVme->writeDAQSettings((scalerPeriod * scalerPeriodMultiplier) <<
			   CVMUSB::DAQSettingsRegister::scalerReadoutPeriodShift);

  // The global mode:
  //   13k buffer
  //   Single event seperator.
  //   Aligned on 16 bits.
  //   Single header word.
  //   Bus request level 4.
  //
  m_pVme->writeGlobalMode((4 << CVMUSB::GlobalModeRegister::busReqLevelShift) | 
			  (CVMUSB::GlobalModeRegister::bufferLen13K << CVMUSB::GLobalModeRegister::bufferLenShift));

  // initialize the hardware:

  InitializeHardware(m_scalers);
  InitializeHardware(m_adcs);

  // Star the VMUSB in data taking mode:

  VMusbToAutonomous();

  delete adcList;
  delete scalerList;
}
/*!
   Stop data taking this involves:
   - Forcing a scaler trigger (action register write)
   - Setting clearing the DAQ start bit (action register write)
   - draining data from the VMUSB:
*/
void
CAcquisitionThread::stopDaq()
{
  m_pVme->writeActionRegister(CVMUSB::ActionRegister::scalerDump);
  m_pVme->writeActionRegister(0);
  drainUsb();
}
/*!
   Turn on Data taking this is just a write of CVMUSB::ActionRegister::startDAQ
   to the action register
*/
void
CAcquistitionThread::VMusbToAutonomous()
{
  m_pVme->writeActionRegister(CVMUSB::ActionRegister::startDAQ);
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
  do {
    size_t bytesRead;
    int    status = m_pVme->usbRead(pBuffer->s_rawData, pBuffer->s_storageSize,
				    &bytesRead, 5000); // 5 second timeout!!
    if (status == 0) {
      pBuffer->s_bufferSize = bytesRead;
      buffer.s_bufferType   = TYPE_EVENTS;
      if (pBuffer->s_body[0] & VMUSBLastBuffer) {
	done = true;
      }
      processBuffer(pBuffer);
      pBuffer = gFreeBuffers->get();
    }
    else {
      timeouts++;		// By the time debugged this is only failure.
      if(timeouts >= DRAINTIMEOUTS) {
	cerr << "Warning: drainUsb() persistent timeout assuming already drained\n";
	done = true;
      }
    }
  } while (!done);
}
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
  processBuffer(buffer);	// Rest gets taken care of there.
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
  processBuffer(buffer);
} 
/*!
   Create a VMUSB readout list from 
   an array of readout modules.  This readout list can then be loaded into the
   VM_usb for autonomous data taking.
   The readout list is dynamically allocated and therefore must be deleted by the
   caller.
*/
CVMUSBReadoutList*
CAcquisitionThread::createList(vector<CReadoutModule*> modules)
{
  CVMUSBReadoutList *pList = new CVMUSBReadoutList;

  for (int i = 0; i < modules.size(); i++) {
    modules[i]->addReadoutList(*pList);
  }
  return *pList;
}
/*!
   Initialize a list of hardware modules by interacting with the
   VMUSB:
*/
void
CAcquisitionThread::InitializeHardware(vector<CReadoutModule* modules)
{
  for (int i =0; i < modules.size() i++) {
    modules[i]->Initialize(*m_pVme);
  }
}
