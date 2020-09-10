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

/** @file:  USBDevice.h
 *  @brief: Encapsulate a libusb_device_handle
 */
#ifndef USBDEVICE_H
#define USBDEVICE_H
#include <libusb.h>
#include <string>
/**
 * @class USBDevice
 *    Encapsulates the sorts of things you can do with an open
 *    usb device (represented by a libusb_device_handle)
 *    Note that opening the device is an implicit reference of
 *    the underlying libusb_device so it's allowed to destroy the
 *    USBDeviceInfo object that opened us once we're opened.
 *
 *  @note This implementation does not support asynchronous transfers.
 *  
 */
class USBDevice
{
private:
    libusb_device_handle* m_pHandle;
public:
    USBDevice(libusb_device_handle* pHandle);
    virtual ~USBDevice();            // Closes.
    
    // Since libusb_device_handle's are not refcounted:
private:
    USBDevice(const USBDevice& rhs);
    USBDevice& operator=(const USBDevice& rhs);
    
public:
    // If you use getHandle you really should be adding methods
    // to this class.
    
    libusb_device_handle* getHandle() {return m_pHandle; }
    void claim(int interface);
    void release(int interface);
    std::string getSerial();
    
    int getConfig();
    void setConfig(int confignum);
    
    void clearHalt();
    void reset();
    
    int controlTransfer(
        uint8_t reqType, uint8_t request, uint16_t wValue, uint16_t windex,
        unsigned char* pData, uint16_t wLength, unsigned int msTimeout
    );
    void bulkTransfer(
        unsigned char endpoint, unsigned char* pData, int dLength,
        int& transferred, unsigned msTimeout
    );
    void interrupt(
        unsigned char endpoint, unsigned char* pData, int dLength,
        int& transferred, unsigned int msTimeout
    );

};

#endif