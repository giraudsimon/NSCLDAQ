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



using namespace std;

// Constants:

// Identifying marks for the VM-usb:

static const short USB_WIENER_VENDOR_ID(0x16dc);
static const short USB_VMUSB_PRODUCT_ID(0xb);
static const short USB_CCUSB_PRODUCT_ID(1);

// Bulk transfer endpoints

static const int ENDPOINT_OUT(2);
static const int ENDPOINT_IN(0x86);

// Bits in the list target address words (TAV)

static const uint16_t TAVcsWrite(4);  // Operation writes.
static const uint16_t TAVcsDATA(2);   // DAQ event Data stack.
static const uint16_t TAVcsSCALER(3); // DAQ scaler data stack.
static const uint16_t TAVcsCNAF(0xc);   // Immediate execution of a CNAF list.
static const uint16_t TAVcsIMMED(TAVcsCNAF);


// Timeouts:

static const int DEFAULT_TIMEOUT(2000); // ms.


//   The following flag determines if enumerate needs to init the libusb:

static bool usbInitialized(false);


////////////////////////////////////////////////////////////////////
/*!
  Construct the CCCUSBusb object.  This involves storing the
  device descriptor we are given, opening the device and
  claiming it.  Any errors are signalled via const char* exceptions.
  \param vmUsbDevice   : usb_device*
      Pointer to a USB device descriptor that we want to open.

  \bug
      At this point we take the caller's word that this is a VM-USB.
      Future implementations should verify the vendor and product
      id in the device structure, throwing an appropriate exception
      if there is aproblem.

*/
CCCUSBusb::CCCUSBusb(struct usb_device* device) :
  m_handle(0),
  m_device(device),
  m_timeout(DEFAULT_TIMEOUT),
  m_pMutex(0)
{
  CMutexAttr attributes;
  attributes.setType(PTHREAD_MUTEX_RECURSIVE_NP);
  m_pMutex = new CMutex(attributes);

  m_serial = serialNo(m_device);
  openUsb();

}
////////////////////////////////////////////////////////////////
/*!
  Destruction of the interface involves releasing the claimed
  interface, followed by closing the device.
 */
CCCUSBusb::~CCCUSBusb()
{
  usb_release_interface(m_handle, 0);
  usb_close(m_handle);
  delete m_pMutex;

  Os::usleep(5000);
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
  if (readFirmware(fw) != 0) {
    usb_release_interface(m_handle, 0);
    usb_close(m_handle);
    Os::usleep(1000);
  
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
  char outPacket[100];
  CriticalSection s(*m_pMutex);

  // Build up the output packet:

  char* pOut = outPacket;

  pOut = static_cast<char*>(addToPacket16(pOut, 5)); // Select Register block for transfer.
  pOut = static_cast<char*>(addToPacket16(pOut, 1)); // Select action register wthin block.
  pOut = static_cast<char*>(addToPacket16(pOut, value));

  // This operation is write only.

  int outSize = pOut - outPacket;
  int status = usb_bulk_write(m_handle, ENDPOINT_OUT, 
                              outPacket, outSize, m_timeout);
  if (status < 0) {
    string message = "Error in usb_bulk_write, writing action register ";
    message == strerror(-status);
    throw message;
  }
  if (status != outSize) {
    throw "usb_bulk_write wrote different size than expected";
  }
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
  size_t outSize;
  uint16_t* outPacket = listToOutPacket(TAVcsIMMED,
                                        list, &outSize);
    
  // Now we can execute the transaction:
    
  int status = transaction(outPacket, outSize,
                           pReadoutBuffer, readBufferSize);
  
  
  
  delete []outPacket;
  if(status >= 0) {
    *bytesRead = status;
  } 
  else {
    *bytesRead = 0;
  }
  return (status >= 0) ? 0 : status;
  
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
    
   \param listNumber : uint8_t  
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
  // Need to construct the TA field, straightforward except for the list number
  // which is splattered all over creation.
  
  uint16_t ta =  TAVcsWrite;
  if (listNumber == 0) {
    ta |= TAVcsDATA;
  }
  else if (listNumber == 1) {
    ta |= TAVcsSCALER;
  }
  else {
    return -4; 
  }

  size_t   packetSize;
  uint16_t* outPacket = listToOutPacket(ta, list, &packetSize);

  int status = usb_bulk_write(m_handle, ENDPOINT_OUT,
                              reinterpret_cast<char*>(outPacket),
                              packetSize, m_timeout);
  if (status < 0) {
    errno = -status;
    status= -1;
  }

  delete []outPacket;
  return (status >= 0) ? 0 : status;


  
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
  CriticalSection s(*m_pMutex);
  int status = usb_bulk_read( m_handle, ENDPOINT_IN,
                              static_cast<char*>(data), bufferSize,
                              timeout);
  if (status >= 0) {
    *transferCount = status;
    status = 0;
  } 
  else {
    errno = -status;
    status= -1;
    *transferCount = 0;
  }
  return status;
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
    char buf[8192];

      CriticalSection s(*m_pMutex);
      int status = usb_bulk_write(m_handle, ENDPOINT_OUT,
                                          static_cast<char*>(writePacket), writeSize,
                                  m_timeout);
      if (status < 0) {
        errno = -status;
        return -1;		// Write failed!!
      }

      status    = usb_bulk_read(m_handle, ENDPOINT_IN,
                                    buf, sizeof(buf), m_timeout);
      if (status < 0) {
        errno = -status;
        return -2;
      }

      long bytesRead = status;
      auto pReadCursor = reinterpret_cast<char*>(readPacket);

      // Copy the newly read data into the output buffer
      pReadCursor = std::copy(buf, buf + status, pReadCursor);


      return bytesRead;

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
    enumerateAndIdentify();
    m_handle  = usb_open(m_device);
  if (!m_handle) {
    throw "CCCUSBusb::CCCUSBusb  - unable to open the device";
  }

  resetUSB();

  enumerateAndIdentify();
  m_handle  = usb_open(m_device);
  if (!m_handle) {
      throw "CCCUSBusb::CCCUSBusb  - unable to open the device";
  }

  // Now claim the interface.. again this could in theory fail.. but.

  usb_set_configuration(m_handle, 1);
  int status = usb_claim_interface(m_handle, 0);
  if (status == -EBUSY) {
    throw "CCCUSBusb::CCCUSBusb - some other process has already claimed";
  }

  if (status == -ENOMEM) {
    throw "CCCUSBusb::CCCUSBusb - claim failed for lack of memory";
  }

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
  int status = usb_reset(m_handle);
  if (status < 0) {
    throw std::runtime_error("CCCUSB::resetCCUSB() failed to reset the device");
  }
}


/*!
 * \brief Enumerate and locate the device by serial number
 *
 * It is expected that the serial number of the device has already been
 * specified. This will enumerate the USB busses and then locate the
 * device on it with a matching serial number. If found, the device is
 * stored by the class for later use.
 *
 * \throws std::string if no device with a matching serial number is found
 */
void CCCUSBusb::enumerateAndIdentify()
{
  // Since we might be re-opening the device we're going to
  // assume only the serial number is right and re-enumerate

  std::vector<struct usb_device*> devices = enumerate();
  m_device = 0;
  for (int i = 0; i < devices.size(); i++) {
    if (serialNo(devices[i]) == m_serial) {
      m_device = devices[i];
      break;
    }
  }
  if (!m_device) {
    std::string msg = "The CC-USB with serial number ";
    msg += m_serial;
    msg += " could not be enumerated";
    throw msg;
  }
}
