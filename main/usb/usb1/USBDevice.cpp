/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  USBDevice.cpp
 *  @brief: Implement the device object.
 */

#include "USBDevice.h"
#include "USB.h"
#include <string.h>
#include <iostream>

static const unsigned MAX_SERIAL(256); // longest supported serial string.

/**
 * constructor:
 *   @param pHandle - device handle we'll be wrapping.
 */
USBDevice::USBDevice(libusb_device_handle* pHandle) :
    m_pHandle(pHandle)
{}

/**
 * destructor
 *    close the handle. Note that destructors are not allowed
 *    to throw exceptions so alas we have to ignore any
 *    errors from libusb_close here.
 */
USBDevice::~USBDevice()
{
    libusb_close(m_pHandle);
}

/**
 * claim
 *   Claim the device.  This should be done before doing
 *   any transfers.  It specifies this handle should be considered
 *   the owner of the device interface specified
 * @param interface - device interface to claim.
 */
void
USBDevice::claim(int interface)
{
    int status = libusb_claim_interface(m_pHandle, interface);
    if (status) {
        throw USBException(status, "Unable to claim_interface");
    }
}
/**
 * release
 *   Un claims a claimed interface.
 *
 *  @param interface -interface selector.
 */
void
USBDevice::release(int interface)
{
    int status = libusb_release_interface(m_pHandle, interface);
    if (status) {
        throw USBException(status, "Unable to release_interface");
    }
}
/**
 * getSerial
 *    Returns the serial number string.  This requires
 *    we:
 *    *  Get the device descriptor.
 *    *  Get the index of the string descriptor holding the serial.
 *    *  Get that string and transform it into an std::string.
 *
 *  @return std::string - the serial number string.
 */
std::string
USBDevice::getSerial()
{
    // Figure out the index of the string descriptor:
    
    libusb_device* pDevice = libusb_get_device(m_pHandle);
    libusb_device_descriptor descriptor;
    int status = libusb_get_device_descriptor(pDevice, &descriptor);
    if (status) {
        throw USBException(status, "Unable to get_device_descriptor");
    }
    uint8_t index = descriptor.iSerialNumber;
 

    unsigned char czserial[MAX_SERIAL];
    memset(czserial, 0, MAX_SERIAL);
    if (index == 0) {
      strcpy(reinterpret_cast<char*>(czserial), "CCUSB-OLD");
      std::cerr << "This CC-USB does not have a good Serial # string\n";
      std::cerr << "Update its firmware to fix this\n";
      std::cerr << "We can work in a single CC-USB system\n";
    } else {
    
      // NOw get the serial string as a C string
    

      status = libusb_get_string_descriptor_ascii(
	  m_pHandle, index,  czserial, MAX_SERIAL
     );
      std::cerr << "Got serial " << czserial << std::endl;
    }
    
    
    if (status < 0) {
        throw USBException(status, "Unable to get_string_descriptor_ascii");
    }
    // return as a std::string.
    
    return std::string(reinterpret_cast<char*>(czserial));
}
/**
 * getConfig
 *   Get the current device configuration
 *
 *  @return configuration number.
 *  @throw USBException
 */
int
USBDevice::getConfig()
{
    int result;
    int status = libusb_get_configuration(m_pHandle, &result);
    if (status) {
        throw USBException(status, "Unable to get_configuration");
    }
    return result;
}
/**
 * setConfig
 *    Selects the device configures
 *  @param config - configuration number.
 */
void
USBDevice::setConfig(int config)
{
    int status = libusb_set_configuration(m_pHandle, config);
    if (status) {
        throw USBException(status, "Unable to set_configuration");
    }
}
/**
 * clearHalt
 *    Clears a stalled end point.
 *
 *  @param endpoint - the endpoint to operate on.
 */
void
USBDevice::clearHalt(unsigned char endpoint)
{
    int status = libusb_clear_halt(m_pHandle, endpoint);
    if (status) {
        throw USBException(status, "Unable to clear_halt");
    }
}
/**
 * reset
 *    Reset the device.  After doing this it's safest to
 *    delete/close this object and any USBDeviceInfo that
 *    spawned it.  This is because the reset can cause
 *    the device to renumerate and appear in a different spot
 *    on the USB device map.
 */
void
USBDevice::reset()
{
    int status = libusb_reset_device(m_pHandle);
    
    // LIBUSB_ERROR_NOT_FOUND menas a renumeration is required
    // or the device disconnected and is therefore ok.
    
    if (status && (status != LIBUSB_ERROR_NOT_FOUND)) {
        throw USBException(status, "reset_device failed");
    }
}
//////////////////////////// I/O //////////////////////////
/** Note that we only support synchronous transfers in this
*   Implementation.  That's sufficient for our XXUSBReadout
*   software with some judicious use of threading.
*/

/**
 * controlTransfer
 *    Control transfers are normally used at USB device setup
 *    time.   Endpoint 0 is always used for control transfers.
 *    As such in libusb the term "setup packet" is often used to
 *    describe the fields of a control transfer.
 *    For what it's worth a setup packet is described in:
 *    https://www.beyondlogic.org/usbnutshell/usb6.shtml
 *
 *  @param reqType - the request type field for the packet.
 *  @param request - The request field for the packet.
 *  @param wValue  - the value field for the packet.
 *  @param windex  - the packet's index field.
 *  @param data    - Buffer into which data will be read for
 *                   transfers from the device and from which data
 *                   will be written in transfers to the device.
 * @param transferred - receives the actual number of bytes transferred.
 * @param msTimeout- Maximum number of milliseconds the
 *                   method blocks for the transfer.
 * @return int -   Number of bytes transferred but:
 * @retval <0  -   The transfer timed out.
 * @throw USBException - for all abnormal completions other than a timeout
 * @note Since there's no way to get the actual transfer count
 *       in the case of a timeout, we have to assume the transfers
 *       are an all or nothing thing.
 */
int
USBDevice::controlTransfer(
    uint8_t reqType, uint8_t request, uint16_t wValue, uint16_t windex,
    unsigned char* pData, uint16_t wLength, unsigned int msTimeout
)
{
    int status = libusb_control_transfer(
        m_pHandle,
        reqType, request, wValue, windex, pData, wLength, msTimeout
    );
    if ((status < 0) && (status != LIBUSB_ERROR_TIMEOUT)) {
        throw USBException(status, "control_transfer failed");
    }
    return status;
}
/**
 * bulkTransfer
 *   Performs a bulk transfer operation.  This is the core of
 *   USB operations - data transfers to or from the device.
 *
 * @param endpoint - The device endpoint for the transfer. This
 *                   also determines the direction of transfer.
 *                   If the top bit is set, this is a write otherwise
 *                   this is a read.  There are only 16 endpoints,
 *                   endpoint 0 is used for control operations.
 *                   The endpoints numbered 1-15 transfer data to
 *                   the host (reads) while those numbered
 *                   0x81-0x8f transfer data to the device (writes).
 * @param pData  - Pointer to the data to transfer.
 * @param dLength - Length of data to transfer;  For reads, this
 *                  is the amount of data pData can accomodate
 *                  while for writes, the number of bytes we want
 *                  to transfer.
 * @param[out] transferred - Returns the actual number of bytes
 *                  transferred.
 * @param msTimeout - Maximum number of milliseconds to allow the
 *                   transfer to take place.
 * @return 0 on succes, LIBUSB_ERROR_TIMEOUT on timeout.  See the note below.
 * @throw USBException on error.
 * @note The value of transferred should be checked for any read operation
 *       to determine how many bytes of data were actually received.
 *       If the operation completed with a timeout for either a
 *       read or write it's possible there was a partial transnfer
 *       prior to the timeout and the value of transferred should
 *       be examined to see where in pDta, dLength to continue the
 *       transfer.
 */
int
USBDevice::bulkTransfer(
   unsigned char endpoint, unsigned char* pData, int dLength,
   int& transferred, unsigned msTimeout
)
{
    int status = libusb_bulk_transfer(
        m_pHandle,
        endpoint, pData, dLength, &transferred, msTimeout
    );
    if (status && (status != LIBUSB_ERROR_TIMEOUT)) {
        throw USBException(status, "bulk_transfer failed");
    }
    return status;
}
/**
 * interrupt
 *   Sends/receives data as an interrupt (out of band) packet.
 *
 *  @param endpoint - endpoint to/from the data are sent,
 *                   as with bulkTransfer, the top bit sets the
 *                   transfer direction.
 *  @param pData   - Pointer to the data buffer.
 *  @param dLength - Requested transfer size.
 *  @param[out] transferred - receives the number of bytes
 *                  actually transferred.
 *  @param msTimeout  Timeout in milliseconds.
 *  @return 0 for success, LIBUSB_ERROR_TIMEOUT if timed out.
 *  @throw USBException if there was an error.
 *  
 */
int
USBDevice::interrupt(
   unsigned char endpoint, unsigned char* pData, int dLength,
   int& transferred, unsigned int msTimeout 
)
{
    int status = libusb_interrupt_transfer(
        m_pHandle,
        endpoint, pData, dLength, &transferred, msTimeout
    );
    if (status && (status != LIBUSB_ERROR_TIMEOUT)) {
        throw USBException(status, "interrupt_transfer failed");
    }
    return status;
}
