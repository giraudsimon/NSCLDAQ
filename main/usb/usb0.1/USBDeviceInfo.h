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

/** @file:  USBDeviceInfo.h
 *  @brief: Provides a class to encapsulate a libusb_device 
 */

#ifndef USBDEVICEINFO_H
#define USBDEVICEINFO_H
#include <usb.h>


class USBDevice;

/**
 * @class USBDeviceInfo
 *    This class is instantiated by USBDevice when
 *    asked to enumerate.  Each libusb_device is wrapped
 *    in a USBDeviceInfo object; which has a reference count
 *    of 1 at the time. Destruction, dereferences the device,
 *    Copy construction and assignment increment the reference.
 *
 *    The default constructor is provided but using any of its
 *    methods before it's assigned to will result in a segfault.
 */
class USBDeviceInfo
{
private:
    usb_device  m_Device;
public:
    USBDeviceInfo(usb_device* pDevice);
    USBDeviceInfo(const USBDeviceInfo& rhs);
    virtual ~USBDeviceInfo();
    USBDeviceInfo& operator=(const USBDeviceInfo& rhs);
    // In case you need to do something not supported (yet) by
    // the class -note that really you shouild implement that
    // operation here rather than use this:
    
    libusb_device* getHandle() { return &m_Device; }
    
    // Informationals
    
    uint8_t getBus() const;
    uint8_t getPort() const;
    
    uint16_t getVendor() const;
    uint16_t getProduct() const;
    
    
    
    USBDevice* open();

    
};

#endif