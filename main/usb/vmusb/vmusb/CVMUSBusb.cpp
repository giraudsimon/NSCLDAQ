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

// Implementation of the CVMUSB class.

#include "CVMUSBusb.h"
#include "CVMUSBReadoutList.h"
#include <os.h>

#include <usb.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <stdexcept>

#include <pthread.h>
#include <CMutex.h>
#include <USB.h>
#include <USBDeviceInfo.h>
#include <USBDevice.h>
#include <XXUSBUtil.h>


#include <thread>
#include <chrono>
using namespace std;

// Constants:

// Identifying marks for the VM-usb:

static const short USB_WIENER_VENDOR_ID(0x16dc);
static const short USB_VMUSB_PRODUCT_ID(0xb);

// Bulk transfer endpoints

static const int ENDPOINT_OUT(2);
static const int ENDPOINT_IN(0x86);

// Timeouts:

static const int DEFAULT_TIMEOUT(2000);	// ms.

// Retries for flushing the fifo/stopping data taking:

static const int DRAIN_RETRIES(5);    // Retries.






//   The following flag determines if enumerate needs to init the libusb:

static bool usbInitialized(false);



static void print_stack(const char* beg, const char* end, size_t unitWidth)
{
  using namespace std;

  cout << "Stack dump:" << endl;
  auto it = beg;
  size_t datum=0;
  while (it < end) {
    std::copy(it, it+unitWidth, reinterpret_cast<char*>(&datum));
    cout << dec << setfill(' ') << setw(3) << distance(beg, it)/unitWidth;
    cout << " : 0x";
    cout << hex << setfill('0') << setw(2*unitWidth) << datum << endl;

    //reset value
    datum = 0;
    advance(it, unitWidth);
  }

  // dump any remainder

  cout << dec << setfill(' ');
  cout << "--------" << std::endl;
}
///////////////////////////////////////////////////////////////
// Implementation of static methods.

static CMutex sectionMutex;      // Mutext for critical sections
USB*   CVMUSBusb::m_pContext(0); // USB Context singleton.
/**
 * getUSBContext
 *    Get the USB context singleton.
 * @return USB*  - the one and only USB context used in the program.
 */
USB*
CVMUSBusb::getUSBContext()
{
  CriticalSection s(sectionMutex);   // for thread safety.
  if (!m_pContext) {
    m_pContext = new USB;
  }
  return m_pContext;
}
/**
 * findBySerial
 *    Finds the VMUSB that matches a serial number string.
 *
 * @param pSerial  - The serial number string to match.
 * @return USBDevice* - The matching device.
 * @retval nullptr    - If there is no match.
 */
USBDevice*
CVMUSBusb::findBySerial(const char* pSerial)
{
  USBDevice* pResult(nullptr);
  auto allControllers = XXUSBUtil::enumerateVMUSB(*(getUSBContext()));
  
  /*
   Note this loop  can't break out early on a match
   because we need to delete _all_ unused USBDevice objects.
   The loop assumes serial numbers are uniqe, that we won't have
   more than one match.
  */
  for (int i = 0; i < allControllers.size(); i++) {
    if (allControllers[i].first == pSerial) {
      pResult = allControllers[i].second;
    } else {
      delete allControllers[i].second;
    }
  }
  
  return pResult;
}
/**
 * findFirst
 *   Returns a pointer to the first VM:USB device enumerated.
 *
 * @return USBDevice*
 * @retval nullptr - there are no devices.
 */
USBDevice*
CVMUSBusb::findFirst()
{
  USBDevice* pResult(nullptr);
  auto allControllers = XXUSBUtil::enumerateVMUSB(*(getUSBContext()));
  
  // Might be no VMUSBs:
  
  if (allControllers.size()) {
    pResult = allControllers[0].second;
    
    // Might be more than one -- though then using find-first is foolish.
    
    for (int i = 1; i < allControllers.size(); i++) {
      delete allControllers[i].second;
    }
  }
  
  return pResult;
}
////////////////////////////////////////////////////////////////////
/*!
  Construct the CVMUSB object.  This involves storing the
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
CVMUSBusb::CVMUSBusb(USBDevice* device) :
    m_pHandle(device),
    m_timeout(DEFAULT_TIMEOUT)
{
  m_serial = m_pHandle->getSerial();
  CMutexAttr  attr;
  attr.setType(PTHREAD_MUTEX_RECURSIVE_NP);
  m_pMutex  = new CMutex(attr);
  
  openVMUsb();
}
////////////////////////////////////////////////////////////////
/*!
    Destruction of the interface involves releasing the claimed
    interface, followed by closing the device.
*/
CVMUSBusb::~CVMUSBusb()
{
    m_pHandle->release(0);
    delete m_pHandle;
    
    delete m_pMutex;
    
    Os::usleep(5000);
}

/**
 * reconnect
 *
 * re open the VM-USB.
 * this is done by closing the device and then invoking
 * openVMUSBUsb which has code common to us and
 * the construtor.
 *   If we can read the firmware register in the VMUSB we assume we don't need
 *   to reconnect.
 *   
 *   @return bool - true if necessary.false if not
 */
bool
CVMUSBusb::reconnect()
{
  try {
    int fwid = readFirmwareID();
    return false;                      // Success so don't need to reconnect.
  }
  catch (...) {
    m_pHandle->release(0);
    delete m_pHandle;
    m_pHandle = findBySerial(m_serial.c_str());
    
    Os::usleep(1000);			// Let this all happen
    openVMUsb();
    return true;
  }
}

/*!
*  writeActionRegister
*
    Writing a value to the action register.  This is really the only
    special case for this code.  The action register is the only
    item that cannot be handled by creating a local list and
    then executing it immediately.
    Action register I/O requires a special list, see section 4.2, 4.3
    of the Wiener VM-USB manual for more information
    \param value : uint16_t
       The register value to write.
 */
void CVMUSBusb::writeActionRegister(uint16_t data) 
{
    CriticalSection s(*m_pMutex);
    
    XXUSBUtil::writeActionRegister(m_pHandle, data);
  

}

//////////////////////////////////////////////////////////////////////////
/////////////////////////// List operations  ////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
/*!
    Execute a list immediately.  It is the caller's responsibility
    to ensure that no data taking is in progress and that data taking
    has run down (the last buffer was received).  
    The list is transformed into an out packet to the VMUSB and
    transaction is called to execute it and to get the response back.
    \param list  : CVMUSBReadoutList&
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
CVMUSBusb::executeList(CVMUSBReadoutList&     list,
		   void*                  pReadoutBuffer,
		   size_t                 readBufferSize,
		   size_t*                bytesRead)
{
  CriticalSection s(*m_pMutex);
  try {
    XXUSBUtil::executeList(
        m_pHandle, list.get(),
        pReadoutBuffer, readBufferSize, *bytesRead
    );
    
  }
  catch (USBException) {
    return -2;
  }
  return 0;  
}



/*!
   Load a list into the VM-USB for later execution.
   It is the callers responsibility to:
   -  keep track of the lists and their  storage requirements, so that 
      they are not loaded on top of or overlapping
      each other, or so that the available list memory is not exceeded.
   - Ensure that the list number is a valid value (0-7).
   - The listOffset is valid and that there is room in the list memory
     following it for the entire list being loaded.
   This code just load the list, it does not attach it to any specific trigger.
   that is done via register operations performed after all the required lists
   are in place.
    
   \param listNumber : uint8_t  
      Number of the list to load. 
   \param list       : CVMUSBReadoutList
      The constructed list.
   \param listOffset : off_t
      The offset in list memory at which the list is loaded.
      Question for the Wiener/Jtec guys... is this offset a byte or long
      offset... I'm betting it's a longword offset.
*/
int
CVMUSBusb::loadList(uint8_t  listNumber, CVMUSBReadoutList& list, off_t listOffset)
{
  CriticalSection s(*m_pMutex);
  try {
    XXUSBUtil::VMUSB::loadList(
          m_pHandle, listNumber, list.get(), listOffset
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
CVMUSBusb::usbRead(void* data, size_t bufferSize, size_t* transferCount, int timeout)
{
  CriticalSection s(*m_pMutex);
  int status;
  int xfercount;
  try {
    status = m_pHandle->bulkTransfer(
        XXUSBUtil::readEndpoint(),
        static_cast<unsigned char*>(data), bufferSize, xfercount,
        timeout
    );
  }
  catch(...) {
    errno = EIO;
    return -1;
  }
  *transferCount = xfercount;
  
  // Timeout is error though partial transfers are possible:
  
  if (status < 0) {
    errno = ETIMEDOUT;
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
CVMUSBusb::setDefaultTimeout(int ms)
{
  m_timeout = ms;
}


////////////////////////////////////////////////////////////////////////
/////////////////////////////// Utility methods ////////////////////////
////////////////////////////////////////////////////////////////////////
/*
   Utility function to perform a 'symmetric' transaction.
   Most operations on the VM-USB are 'symmetric' USB operations.
   This means that a usb_bulk_write will be done followed by a
   usb_bulk_read to return the results/status of the operation requested
   by the write. Depending on the transaction and the amount of data
   expected to be received, there will be one or more calls to usb_bulk_read.
   If a failure occurs after the first read, the data from all subsequent reads
   that succeeded is returned to the user. It is not an error to timeout on
   any read operation after the first.

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

   NOTE:  The m_timeout is used for both write and read timeouts. To change
   the value of m_timeout, use setDefaultTimeout().

*/
int
CVMUSBusb::transaction(void* writePacket, size_t writeSize,
		    void* readPacket,  size_t readSize)
{
  CriticalSection(*m_pMutex);
  int status;
  try {
    status = XXUSBUtil::transaction(
      m_pHandle, writePacket, writeSize, readPacket, readSize, m_timeout
    );
  }
  catch (...) {
    errno = EIO;
    return -2;
  }
  
  if (status < 0) {
    errno = ETIMEDOUT;
    return -2;
  }
  return status;
  
}

/*
   Reading a register is just creating the appropriate CVMUSBReadoutList
   and executing it immediatly.
*/
uint32_t
CVMUSBusb::readRegister(unsigned int address)
{
    CVMUSBReadoutList list;
    uint32_t          data;
    size_t            count;
    list.addRegisterRead(address);

    int status = executeList(list,
			     &data,
			     sizeof(data),
			     &count);
    if (status == -1) {
      char message[100];
      sprintf(message, "CVMUSBusb::readRegister USB write failed: %s",
      strerror(errno));
      throw string(message);
    }
    if (status == -2) {
      char message[100];
      sprintf(message, "CVMUSBusb::readRegister USB read failed %s",
    	strerror(errno));
      throw string(message);

    }

    return data;

}
/*
  
   Writing a register is just creating the appropriate list and
   executing it immediately:
*/
void
CVMUSBusb::writeRegister(unsigned int address, uint32_t data)
{
  CVMUSBReadoutList list;
  uint16_t          rdstatus;
  size_t            rdcount;
  list.addRegisterWrite(address, data);

  int status = executeList(list,
      &rdstatus, 
      sizeof(rdstatus),
      &rdcount);

  if (status == -1) {
    char message[100];
    sprintf(message, "CVMUSBusb::writeRegister USB write failed: %s",
        strerror(errno));
    throw string(message);
  }
  if (status == -2) {
    char message[100];
    sprintf(message, "CVMUSBusb::writeRegister USB read failed %s",
        strerror(errno));
    throw string(message);

  }
}

/**
 * openVMUsb
 *
 *   Open the VM-USB.  This contains code that is 
 *   common to both the constructor and reconnect.
 *
 *   We assume that m_serial is set to the
 *   desired VM-USB serial number.
 *
 *   @throw std::string on errors.
 */
void
CVMUSBusb::openVMUsb()
{
    
    // Resetting the device causes the usb device to lose its enumeration.
    // It must be found again and then reopened.
#if 0
    m_pHandle->reset();
    m_pHandle = findBySerial(m_serial.c_str());
    if (!m_pHandle) {
        throw "CVMUSBusb::CVMUSBusb  - unable to find/open the device";
    }
#endif
    // Set the configuration and claim the interface:
    
    m_pHandle->setConfig(1);
    m_pHandle->claim(0);
   
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // reset the module
    
    writeActionRegister(ActionRegister::clear);

    // Turn off DAQ mode and flush any data that might be trapped in the VMUSB
    // FIFOS.  To write the action register may require at least one read of the FIFO.
    //

    int retriesLeft = DRAIN_RETRIES;
    uint8_t buffer[1024*13*2];  // Biggest possible VM-USB buffer.
    size_t  bytesRead;

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
        std::cerr << "** Warning - not able to stop data taking VM-USB may need to be power cycled\n";
    }
    
    while(usbRead(buffer, sizeof(buffer), &bytesRead) == 0) {
        fprintf(stderr, "Flushing VMUSB Buffer\n");
    }
    
    // Now set the irq mask so that all bits are set..that:
    // - is the only way to ensure the m_irqMask value matches the register.
    // - ensures m_irqMask actually gets set:

    writeIrqMask(0x7f);

    // Read the state of the module
    initializeShadowRegisters();
}


void CVMUSBusb::initializeShadowRegisters()
{
    readGlobalMode();
    readDAQSettings();
    readLEDSource();
    readDeviceSource();
    readDGG_A();
    readDGG_B();
    readDGG_Extended();
    for (int i=1; i<5; ++i) readVector(i);
    readIrqMask();
    readBulkXferSetup();
    readEventsPerBuffer();
}


/*!
 * \brief Reset the already opened VMUSB
 *
 * The reset operation destroys the enumeration so that the caller must
 * reenumerate the devices.
 *
 * \throws std::runtime_error if reset operation failed.
 */
void CVMUSBusb::resetVMUSB()
{
  m_pHandle->reset();
  
}

