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
    
    // NOw get the serial string as a C string
    
    unsigned char czserial[MAX_SERIAL];
    memset(czserial, 0, MAX_SERIAL);
    status = libusb_get_string_descriptor_ascii(
        m_pHandle, index,  czserial, MAX_SERIAL-1
    );
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
