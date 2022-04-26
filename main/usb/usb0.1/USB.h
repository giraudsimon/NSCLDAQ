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

/** @file:  USB.h
 *  @brief: Encapsultes the initialization/shutdown and enumeration of libusb-1.0
 *  
 */
#ifndef USB_H
#define USB_H

#include <stdexcept>
#include <string>
#include <vector>

class USBDeviceInfo;

/**
 * @class USBException
 *     This exception is thrown rather than returning error
 *     codes.  It encapsulates a libusb error code
 *     what() will produce a message string that merges
 *     the user's string with the string appropriate to the
 *     error.
 */
class USBException : public std::exception
{
private:
    int m_code;
    std::string m_usermessage;
    std::string m_what;       // Filled in by what.
public:
    USBException(int code, const std::string& msg)  noexcept;
    USBException(const USBException& rhs) noexcept;
    USBException& operator=(const USBException & rhs) noexcept;
    
    virtual const char* what() const noexcept;
    
};

/**
 * @class USB
 *    Thisclass provides for operations libusb-1.0 that are not device specific
 *    See USBDevice for per device operations. Note that instantiation
 *    implies initialization and destruction implies de-initialization.
 *    It's therefore important to keep this object alive as long as you
 *    use objects it generates.
 *
 *    Note that libusb-1.0 provides support for multiple simultaneous contexts,
 *    so this is not a singleton as it wraps a single context for its lifetime.
 */
class USB
{
private:
    static bool m_initialized;
public:
    USB();
    virtual ~USB();
private:
    USB(const USB& rhs);
    USB& operator=(const USB& rhs);

public:
    // Things that affect operation.
    
    // void setLogCallback(libusb_log_cb cb, int mode) // not supported in our libusb!!
    void setDebug(int level);
    
public:
    // enumeration and device management
    
    std::vector<USBDeviceInfo*> enumerate();
protected:
    void init();
};


#endif