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

/** @file:  USB.cpp
 *  @brief: Implements the USB, libusb-1.0 encapsulation class.
 */

#include "USB.h"
#include "USBDeviceInfo.h"
#include <stdexcept>

//////////////////////////////////////////////////////////////
// Implement  USBException class.
//

/**
 * constructor:
 *    @param code - USB Error code.
 *    @param msg  - User's error message.
 */
USBException::USBException(int code, const std::string& msg) noexcept :
    m_code(code),
    m_usermessage(msg)
{
    
}
/**
 * copy construction
 */
USBException::USBException(const USBException& rhs) noexcept :
    m_code(rhs.m_code), m_usermessage(rhs.m_usermessage),
    m_what(rhs.m_what)
{
    // Since what is const we need to make m_what now:
    
    m_what = m_usermessage;
    m_what += ": ";
    m_what += libusb_error_name(m_code);
}
/**
 * assignment
 */
USBException&
USBException::operator=(const USBException& rhs) noexcept
{
    if (&rhs != this) {
        m_code        = rhs.m_code;
        m_usermessage = rhs.m_usermessage;
        m_what        = rhs.m_what;
    }
    
    return *this;
}
/**
 * what
 *    Returns a string describing the message.
 *    Since a const char* must be returned,
 *    this is constructed into m_what and a
 *    pointer to its c_string is returned.
 * @return const char*
 */
const char*
USBException::what() const noexcept
{
    
    
    return m_what.c_str();
}

//////////////////////////////////////////////////////////
// USB class implementation.
//

/**
 * constructor
 *   Initialize access to the library and save the context.
 *   If that fails we toss an appropriate USBException.
 */
USB::USB() :m_pContext(nullptr)
{
    int status = libusb_init(&m_pContext);
    if (status) throw USBException(status, "Failed in call to libusb_init");

}
/**
 * destructor
 *   Finalize USB library access. Note that any USBDevice objects
 *   that were created should first be destroyed.
 */
USB::~USB()
{
    libusb_exit(m_pContext);
}
/**
 * setDebug
 *    Set the debug logging level.
 *   @param level is the new level to use.  See
 *          lib_usb_log_level  for information about
 *          the valid levels which at this time are:
 *          
 *    *  LIBUSB_LOG_LEVEL_NONE (0) : no messages ever printed by the library (default)
 *    *  LIBUSB_LOG_LEVEL_ERROR (1) : error messages are printed to stderr
 *    *  LIBUSB_LOG_LEVEL_WARNING (2) : warning and error messages are printed to stderr
 *    *  LIBUSB_LOG_LEVEL_INFO (3) : informational messages are printed to stderr
 *    *  LIBUSB_LOG_LEVEL_DEBUG (4) : debug and informational messages are printed to stderr
 *
 * @throw USBException if the call to libusb_set_option fails.
 */
void
USB::setDebug(int level)
{
    int status = libusb_set_option(
        m_pContext, LIBUSB_OPTION_LOG_LEVEL, level
    );
    if (status) {
        throw USBException(status, "Setting libusb-1.0 debug level failed");
    }
}
/**
 * enumerate
 *    Return a vector of device description object pointers
 *    for each device attached to the system. The individual
 *    objects are new' into existence so all must be deleted
 *    when the application  no longer needs them (as will be the
 *    case for most of them).
 * @return std::vector<USBDeviceInfo*> - The devices.
 * @throw USBException if the lib_get_device_list fails.
 */
std::vector<USBDeviceInfo*>
USB::enumerate()
{
    libusb_device** pDevices;
    ssize_t status = libusb_get_device_list(m_pContext, &pDevices);
    if (status < 0) {
        throw USBException(status, "Enumeration failed in get_device_list");
    }
    
    std::vector<USBDeviceInfo*> result;
    for (int i=0; i < status; i++) {
        result.push_back(new USBDeviceInfo(pDevices[i]));
    }
    libusb_free_device_list(pDevices, status);
    
    return result;
}
