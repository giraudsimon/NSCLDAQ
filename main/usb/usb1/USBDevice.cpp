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
#include <string.h>

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