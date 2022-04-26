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

/** @file:  USBDeviceInfo.cpp 
 *  @brief: Implement the device information class.
 */
#include "USBDeviceInfo.h"
#include "USB.h"
#include "USBDevice.h"


#include <string.h>


/**
 * construct from a libusb_device
 *    It's necessary for the device to have a non-zero
 *    reference count that assumes it's going to be
 *    encapsulated. We don't touch the reference count here
 *    (the destructor dereferences it).
 *
 *   @param pDevice - libusb_device to wrap.
 */
USBDeviceInfo::USBDeviceInfo(libusb_device* pDevice) :
    
{
     memcpy(&m_Device, pDevice, sizeof(usb_device_descriptor));
}
/**
 * Copy construction
 *   We'll add a reference count to this device as we'll now
 *   have another wrapping object.
 * @param rhs - the object we're constructing from:
 */
USBDeviceInfo::USBDeviceInfo(const USBDeviceInfo& rhs) :
   USBDeviceInfo(&(rhs.m_Device) 
{

}
/**
 * assignment
 *    This will, if not a self assignment,
 *    increase the rhs device's reference count. We also decrease
 *    the refcount of any prior device.
 *
 *   @param rhs - right hand side of the assignment.
 *   @return reference to the left hand side after the assigment.
 */
USBDeviceInfo&
USBDeviceInfo::operator=(const USBDeviceInfo& rhs)
{
    if (this != &rhs) {
        memcpy(&m_Device, &(rhs.m_device), sizeof(usb_device_descriptor));
    }
    return *this;
}

 /**
  * destructor
  *   If we have a device, we decrement its reference coun.t
  *   libusb will do the final destruction when time comes.
  */
 USBDeviceInfo::~USBDeviceInfo()
 {
 
 }
 /**
  * Get the bus number associated with a usb device.
  *
  * @return uint8_t - Bus number.
  * @note  If the m_pDevice is not initialized, likely we'll
  *        throw a segfault.
  */
 uint8_t
 USBDeviceInfo::getBus()
 {
   return libusb_get_bus_number(m_pDevice);
 }
 /**
  * getPort
  *   Get the device port number
  *   
  * @return uint8_t
  * @note  If the m_pDevice is not initialized, likely we'll
  *        throw a segfault.
  */
 uint8_t
 USBDeviceInfo::getPort()
 {
  return m_Device.devnum;
 }
 
 /**
  * getVendor
  *   Return the vendor id of the device:
  *
  *  @return uint16_t - the vendor code.
  *  @throw USBException if there are errors getting this information.
  */
 uint16_t
 USBDeviceInfo::getVendor()
 {
    return m_Device.descriptor.idVendor;
 }
 /**
  * getProduct
  *   @return uint16_t - the product id of the device.
  *   @throw USBException if there are errors getting this information.
  */
 uint16_t
 USBDeviceInfo::getProduct()
 {
   return m_Device.descriptor.id_Product;
 }
 
 
 /**
  * open
  *    Opens a device making it available for transfers.
  *
  *  @return USBDevice* object that can be used t operform data transfers.
  *         destroying the resulting device closes it.
  *
  *  @note the return value was new'd into existence.
  */
 USBDevice*
 USBDeviceInfo::open()
 {
    auto handle = usb_open(&m_Device);
    if (handle) {
       return new USBDevice(handle);
    } else {
       throw USBException(status, "Unable to open the device"); 
    }
    
 }
 