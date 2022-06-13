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
#include <usb.h>
#include <sstream>

// System initialization flag.

bool USB::m_initialized(false);

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
    std::stringstream smsg;
    smsg << msg << " : " << code;
    m_what = smsg.str();
}
/**
 * copy construction
 */
USBException::USBException(const USBException& rhs) noexcept :
    m_code(rhs.m_code), m_usermessage(rhs.m_usermessage),
    m_what(rhs.m_what)
{
    // Since what is const we need to make m_what now:
    
    std::stringstream msg;
    msg << rhs.m_usermessage << " : "<< rhs.m_code;
    m_what = msg.str();
    
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
USB::USB()
{
    init();

}
/**
 * destructor
 *   Finalize USB library access. Note that any USBDevice objects
 *   that were created should first be destroyed.
 */
USB::~USB()
{
    
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
  usb_set_debug(level);
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
    std::vector<USBDeviceInfo*> result;
    
    // We assume that since we're enumerating we may need to
    // refresh the system's idea of the USB bus population:
    
    usb_find_busses();
    usb_find_devices();
    
    
    // The devices and busses are linked listss:
    
    usb_bus* pBus = usb_get_busses();   // Gets the first.
    while (pBus) {
       struct usb_device* pDevice = pBus->devices;   // first device on the bus.
       while (pDevice) {
          result.push_back(new USBDeviceInfo(pDevice));
          pDevice = pDevice->next;
       }
       pBus = pBus->next;
    }
    
    return result;
    
}
/**
 * init if necessary initialize the USB library.
 */
void
USB::init()
{
   if(!m_initialized) {
    usb_init();
    m_initialized = true;
   }
}