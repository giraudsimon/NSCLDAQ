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

/** @file:  XXUSButil.h
 *  @brief: Provide functions/methods that are useful for VMUSB/CCUSB devices.
 */
#ifndef XXUSBUTIL_H
#define XXUSBUTIL_H

#include <stdint.h>
#include <string>
#include <vector>
#include <utility>

class USBDevice;
class USB;
/**
 * @namespace XXUSBUtil
 *   Contains utility functions for the XXUSB. Note that
 *   all operations in this namespace are performed in a critical
 *   section so that they are threadsafe.  THis is important for
 *   the XXUSBReadout programs and not that much overhead for
 *   un-threaded programs.
 */
namespace XXUSBUtil {
    
    /**
     * @type XXUSBDevices
     *   Contains serial number/USBDevice pairs for
     *   appropriate XXUSB devices.  The USBDevices are all open
     *   but not claimed and dynamically allocated so the ones
     *   you don't want to use must be deleted.
     */
    typedef std::pair<std::string, USBDevice*> XXUSBDevice;
    typedef std::vector<XXUSBDevice> XXUSBDevices;
    
    extern bool logTransactions;
    
    
    XXUSBDevices enumerateVMUSB(USB& context);
    XXUSBDevices enumerateCCUSB(USB& context);
    XXUSBDevices enumerateVendorAndProduct(
        USB& context, uint16_t vendor, uint16_t product
    );
    
    int transaction(
        USBDevice* pDevice,
        void* writePacket, size_t writeSize,
        void* readPacket,  size_t readMax, int msTimeout = 0
    );
    void writeActionRegister(USBDevice* pDevice, uint16_t data);

    // Exporting it this way prevents the need to export template
    // functions:
    void executeList(               // VMUSB
        USBDevice* pDevice, const std::vector<uint32_t>& list,
        void* pReturnedData, size_t maxRead, size_t& bytesRead,
        int msTimeout = 0
    );
    
    void executeList(               // CCUSB
        USBDevice* pDevice, const std::vector<uint16_t>& list,
        void* pReturnedData, size_t maxRead, size_t& bytesRead,
        int msTimeout = 0
    );

    uint8_t readEndpoint();
    uint8_t writeEndpoint();
    uint16_t vendorId();
    uint16_t VMUSBProductId();
    uint16_t CCUSBProductId();
    
    // A few operations are somewhat device dependent:
    
    namespace VMUSB {
        void loadList(
            USBDevice* pDevice, uint8_t listNum,
            const std::vector<uint32_t>& list, size_t offset
        );
                
    }
    namespace CCUSB {
        void loadList(
            USBDevice* pDevice, const std::vector<uint16_t>& list,
            bool scaler=false
        );
    }
}


#endif
