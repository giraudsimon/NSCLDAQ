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

// Implementation of the CCCUSBusb class.

#include "CCCUSBusb.h"
#include "CCCUSBReadoutList.h"
#include <usb.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <os.h>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <USB.h>
#include <USBDeviceInfo.h>
#include <USBDevice.h>
#include <XXUSBUtil.h>


using namespace std;

// Constants:

// Identifying marks for the VM-usb:



//   The following flag determines if enumerate needs to init the libusb:

static bool usbInitialized(false);
static CMutex StaticMethodProtector;    // for critical setions in static methods
USB* CCCUSBusb::m_usbContext(0);


///////////////////////////////////////////////////////////////
// static methods:

/**
 * getUsbContext
 *    Return the context associated with the USB.
 *    (singlethon method).
 * @return USB*   Pointer to the singleton USB context.
 */
USB*
CCCUSBusb::getUsbContext()
{
  CriticalSection s(StaticMethodProtector);
  if(m_usbContext == nullptr) {
    m_usbContext = new USB;
  }
  return m_usbContext;
}
/**
 * findBySerial
 *    Locate a CCUSB device by serial number.
 *
 * @serial - serial number string
 * @return USBDevice* device for the serial number open and
 *                   claimed.
 * @retval nullptr - no matching serial number.
 */
USBDevice*
CCCUSBusb::findBySerial(const char* serial)
{
  USB* pUsb = getUsbContext();   // has a critical setion
  auto ccusbs = XXUSBUtil::enumerateCCUSB(*pUsb); // has its own critsec
  
  // No critical section required (I think).
  // Note we need to go through all of these devices
  // to prevent any leakage.  The assumption is only one match at most.
  
  USBDevice* pResult(0);
  for (int i =0; i < ccusbs.size(); i++) {
    if(ccusbs[i].first == serial) {
      pResult = ccusbs[i].second;      // Can't break, may not be last.
    } else {
      delete ccusbs[i].second;        // closes unmatched devs.
    }
  }
  if(pResult) pResult->claim(0);     // IF there's a match, claim it.
  return pResult;                   // Matching device.
}

////////////////////////////////////////////////////////////////////
/*!
  Construct the CCCUSBusb object.  This involves storing the
  device descriptor we are given, opening the device and
  claiming it.  Any errors are signalled via const char* exceptions.
  \param vmUsbDevice   : usb_device*
      Pointer to a USB device descriptor that we want to open.

  \bug
      At this point we take the caller's word that this is a CC-USB.
      Future implementations should verify the vendor and product
      id in the device structure, throwing an appropriate exception
      if there is aproblem.

*/
CCCUSBusb::CCCUSBusb(USBDevice* device) :
  m_device(device),
  m_timeout(DEFAULT_TIMEOUT),
  m_pMutex(0)
{
  CMutexAttr attributes;
  attributes.setType(PTHREAD_MUTEX_RECURSIVE_NP);
  m_pMutex = new CMutex(attributes);

  m_serial = m_device->getSerial();
  openUsb();

}
////////////////////////////////////////////////////////////////
/*!
  Destruction of the interface involves releasing the claimed
  interface, followed by closing the device.
 */
CCCUSBusb::~CCCUSBusb()
{
  m_device->release(0);  // unclaim.
  delete m_device;     //  Close and relese resource
  delete m_pMutex;

  Os::usleep(5000);   // Seems needed.
}


/**
 * reconnect
 *   
 * Drop connection with the CC-USB and re-open.
 * this can be called when you suspect the CC-USB might
 * have been power cycled.
I*
*/
bool
CCCUSBusb::reconnect()
{
  uint32_t fw;
  if (readFirmware(fw) != 0) {         /// need to reconnect.
    m_device->release(0);
    delete m_device;
    Os::usleep(1000);
  
    m_device = findBySerial(m_serial.c_str());
    
    openUsb();
    return true;
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//////////////////////// Register operations ///////////////////////
////////////////////////////////////////////////////////////////////
/*!
    Writing a value to the action register.  This is really the only
    special case for this code.  The action register is the only
    item that cannot be handled by creating a local list and
    then executing it immediately.
    Action register I/O requires a special list, see section 4.2, 4.3
    of the Wiener VM-USB manual for more information
    \param value : uint16_t
       The register value to write.
*/
void
CCCUSBusb::writeActionRegister(uint16_t value)
{
  XXUSBUtil::writeActionRegister(m_device, value);
  
}


//////////////////////////////////////////////////////////////////////////
/////////////////////////// List operations  ////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
/*!
    Execute a list immediately.  It is the caller's responsibility
    to ensure that no data taking is in progress and that data taking
    has run down (the last buffer was received).  
    The list is transformed into an out packet to the CCUSB and
    transaction is called to execute it and to get the response back.
    \param list  : CCCUSBReadoutList&
       A reference to the list of operations to execute.
    \param pReadBuffer : void*
       A pointer to the buffer that will receive the reply data.
    \param readBufferSize : size_t
       number of bytes of data available to the pReadBuffer.
    \param bytesRead : size_t*
       Return value to hold the number of bytes actually read.
 
    \return int
    \retval  0    - All went well.
    \retval -1    - The usb_bulk_write failed.
    \retval -2    - The usb_bulk_read failed.

    In case of failure, the reason for failure is stored in the
    errno global variable.
*/
int
CCCUSBusb::executeList( CCCUSBReadoutList&     list,
                        void*                  pReadoutBuffer,
                        size_t                 readBufferSize,
                        size_t*                bytesRead)
{
  try {
    XXUSBUtil::executeList(
      m_device, list.get(), &pReadoutBuffer, readBufferSize,
      *bytesRead
    );
  } catch (...) {
    return -2;
  }
  return 0;
  
}

/*!
   Load a list into the CC-USB for later execution.
   It is the callers responsibility to:
   -  keep track of the lists and their  storage requirements, so that 
      they are not loaded on top of or overlapping
      each other, or so that the available list memory is not exceeded.
   - Ensure the list number is valid and map it to a TAV.
   - The listOffset is valid and that there is room in the list memory
     following it for the entire list being loaded.
   This code just load the list, it does not attach it to any specific trigger.
   that is done via register operations performed after all the required lists
   are in place.
    
   \param listNumber : uint_t  
      Number of the list to load. 
      - 0 - Data list
      - 1 - Scaler list.
   \param list       : CCCUSBReadoutList
      The constructed list.


\return int
\retval 0  - AOK.
\retval -4 - List number is illegal
\retval other - error from transaction.

*/
int
CCCUSBusb::loadList(uint8_t  listNumber, CCCUSBReadoutList& list)
{
  if ((listNumber != 0) && (listNumber != 1)) {
    return -4;
  }
  try {
    XXUSBUtil::CCUSB::loadList(
      m_device, list.get(), listNumber == 1
    );
  }
  catch (...) {
    return -1;
  }
  return 0;
}
/*!
  Execute a bulk read for the user.  The user will need to do this
  when the VMUSB is in autonomous data taking mode to read buffers of data
  it has available.
  \param data : void*
     Pointer to user data buffer for the read.
  \param buffersSize : size_t
     size of 'data' in bytes.
  \param transferCount : size_t*
     Number of bytes actually transferred on success.
  \param timeout : int [2000]
     Timeout for the read in ms.
 
  \return int
  \retval 0   Success, transferCount has number of bytes transferred.
  \retval -1  Read failed, errno has the reason. transferCount will be 0.

*/
int 
CCCUSBusb::usbRead(void* data, size_t bufferSize, size_t* transferCount, int timeout)
{
  *transferCount = 0;
  CriticalSection s(*m_pMutex);
  try {
    int actual;
    int status = m_device->bulkTransfer(
        XXUSBUtil::readEndpoint(),
        static_cast<unsigned char*>(data),
        bufferSize, actual, timeout
    );
    *transferCount = actual;
    if (status < 0) {
      errno = -ETIMEDOUT;
      return -1;
    }
  }
  catch (...) {
    errno = -EIO;
    return -1;
  }
  return 0;
}
/*! 
   Set a new transaction timeout.  The transaction timeout is used for
   all usb transactions but usbRead where the user has full control.
   \param ms : int
      New timeout in milliseconds.
*/
void
CCCUSBusb::setDefaultTimeout(int ms)
{
  m_timeout = ms;
}

//////////////////////////////////////////////////////////////////////
// UTILITY METHODS
//
// Debug methods:

// #define TRACE      // Comment out if not tracing
void dumpWords(void* pWords, size_t readSize)
{
  readSize = readSize / sizeof(uint16_t);
  uint16_t* s = reinterpret_cast<uint16_t*>(pWords);

 
  for (int i =0; i < readSize; i++) {
    fprintf(stderr, "%04x ", *s++);
    if (((i % 8) == 0) && (i != 0)) {
      fprintf(stderr, "\n");
    }
  }
  fprintf(stderr, "\n");
}

static void dumpRequest(void* pWrite, size_t writeSize, size_t readSize)
{
#ifdef TRACE
  fprintf(stderr, "%d write, %d read\n", writeSize, readSize);
  dumpWords(pWrite, writeSize);
#endif
}

static void dumpResponse(void* pData, size_t readSize)
{
#ifdef TRACE
  fprintf(stderr, "%d bytes in response\n", readSize);
  dumpWords(pData, readSize);
#endif
}

/*
   Utility function to perform a 'symmetric' transaction.
   Most operations on the VM-USB are 'symmetric' USB operations.
   This means that a usb_bulk_write will be done followed by a
   usb_bulk_read to return the results/status of the operation requested
   by the write.
   Parametrers:
   void*   writePacket   - Pointer to the packet to write.
   size_t  writeSize     - Number of bytes to write from writePacket.
   
   void*   readPacket    - Pointer to storage for the read.
   size_t  readSize      - Number of bytes to attempt to read.


   Returns:
     > 0 the actual number of bytes read into the readPacket...
         and all should be considered to have gone well.
     -1  The write failed with the reason in errno.
     -2  The read failed with the reason in errno.

   NOTE:  The m_timeout is used for both write and read timeouts.

*/
int
CCCUSBusb::transaction( void* writePacket, size_t writeSize,
                        void* readPacket,  size_t readSize)
{
  try {
    int status = XXUSBUtil::transaction(
      m_device, writePacket, writeSize, readPacket, readSize, m_timeout
    );
    if (status < 0) {
      errno = ETIMEDOUT;
      return -1;
    }
  } catch (...) {
    errno = EIO;
    return -1;
  }
  return 0;
}



/**
 * openUsb
 *
 *  Does the common stuff required to open a connection
 *  to a CCUSB given that the device has been filled in.
 *
 *  Since the point of this is that it can happen after a power cycle
 *  on the CAMAC crate, we are only going to rely on m_serial being
 *  right and re-enumerate.
 *
 *  @throw std::string - on errors.
 */
void
CCCUSBusb::openUsb()
{
    
  m_device->setConfig(1);
  

  Os::usleep(10000);

  // Turn off data taking and flush any data in the buffer:

  uint8_t buffer[8192*2];  // Biggest possible CC-USB buffer.
  size_t  bytesRead;
  int retriesLeft = 50;

  // flush the data first with a read. For some reason, the
  // CCUSB can get into a funk that disallows a write before 
  // a read occurs.
  while (retriesLeft) {
      try {
          usbRead(buffer, sizeof(buffer), &bytesRead, 1);
          writeActionRegister(0);     // Turn off data taking.
          break;                      // done if success.
      } catch (...) {
          retriesLeft--;
      }
  }
  if (!retriesLeft) {
      std::cerr << "** Warning - not able to stop data taking CC-USB may need to be power cycled\n";
  }

  while(usbRead(buffer, sizeof(buffer), &bytesRead) == 0) {
       fprintf(stderr, "Flushing CCUSB Buffer\n");
  }

  m_device->clearHalt(XXUSBUtil::readEndpoint());
  m_device->clearHalt(XXUSBUtil::writeEndpoint());
  Os::usleep(10000); // sleep 10 ms
}


/*!
 * \brief Reset the already opened VMUSB
 *
 * The reset operation destroys the enumeration so that the caller must
 * reenumerate the devices.
 *
 * \throws std::runtime_error if reset operation failed.
 */
void CCCUSBusb::resetUSB()
{
  m_device->reset();
}

