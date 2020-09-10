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




/**
 * default constructor
 *   Intended to be used and then assigned to immediately.
 *   Anything else will likely result in a segfault.
 */

USBDeviceInfo::USBDeviceInfo() : m_pDevice(nullptr)
{
    
}
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
    m_pDevice(pDevice)
{
        
}
/**
 * Copy construction
 *   We'll add a reference count to this device as we'll now
 *   have another wrapping object.
 * @param rhs - the object we're constructing from:
 */
USBDeviceInfo::USBDeviceInfo(const USBDeviceInfo& rhs) :
    m_pDevice(libusb_ref_device(rhs.m_pDevice))
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
        if (m_pDevice) {
            libusb_unref_device(m_pDevice);  // unref current dev.
        }
        m_pDevice = libusb_ref_device(rhs.m_pDevice);
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
    if (m_pDevice) libusb_unref_device(m_pDevice);
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
  return libusb_get_port_number(m_pDevice);
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
  libusb_device_descriptor d;
  getDescriptor(d);
  return d.idVendor;
 }
 /**
  * getProduct
  *   @return uint16_t - the product id of the device.
  *   @throw USBException if there are errors getting this information.
  */
 uint16_t
 USBDeviceInfo::getProduct()
 {
  libusb_device_descriptor d;
  getDescriptor(d);
  return d.idProduct;
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
    libusb_device_handle* pHandle;
    
    int status = libusb_open(m_pDevice, &pHandle);
    if (status) {
     throw USBException(status, "Unable to open the device");
    }
    return new USBDevice(pHandle);
 }
 //////////////////////////////////////////////////////////
 // Privet methods.
 
 /**
  * getDescriptor
  *    Gets the device descriptor.  Throws as needed for errors.
  * @param[out] desc - reference to a buffer to receive the device
  *      descrptor.
  * @throw USBException - if there's an error.
  */
 void
 USBDeviceInfo::getDescriptor(libusb_device_descriptor& desc)
 {
   int status = libusb_get_device_descriptor(m_pDevice, &desc);
   
   if (status) {
    throw USBException(status, "Unable to get_device_descriptor");
   }
 }