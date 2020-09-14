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

/** @file:  XXUSBUtil.cpp
 *  @brief: Implements the  functions in the XXUSBUtil namespace.
 */
#include "XXUSBUtil.h"
#include "USB.h"
#include "USBDeviceInfo.h"
#include "USBDevice.h"
#include <CMutex.h>
/**
 * The following mutex is used for the critical section we use
 * to make the code threadsafe.
 */
static CMutex XXUSBMutex;

// Definitions:

// Bits in the target address word of the outpacket:

static const uint16_t TAVcsID0 = 1; // Bit mask of Stack id bit 0.
static const uint16_t TAVcsSel = 2; // Bit mask to select list dnload
static const uint16_t TAVcsWrite = 4; // Write bitmask.
static const uint16_t TAVcsIMMED = 8; // Target the VCS immediately.
static const uint16_t TAVcsID1 = 0x10;
static const uint16_t TAVcsID2 = 0x20;
static const uint16_t TAVcsID12MASK = 0x30; // Mask for top 2 id bits
static const uint16_t TAVcsID12SHIFT = 4;

/**
 * The macro CRITICAL_BLOCK can be used to mark a block
 * as a critical section.  Usage is normally:
 *   \verbatim
 *      {
 *         CRITICAL_BLOCK;
 *         // protected code here.
 *      }
 *   This is just a shorthand for
 *     CriticalSection __critical_section__(XXUSBMutex)
 */
#define CRITICAL_BLOCK CriticalSection __critical_section__(XXUSBMutex)

////////////////////////////////////////////////////////////////
// Private utilities:

/**
 * imlistToOutPacket
 *    Converts an operation list into an out packet.
 *
 * @param ta   - Transfer address word.
 * @param list - std::vector<uint32_t> containing the list of operations.
 * @return std::vector<uint16_t> containing the resulting list.
 * 
 */
static std::vector<uint16_t>
listToOutPacket(uint16_t ta, const std::vector<uint32_t>& list)
{
    size_t listLongs = list.size();
    size_t listwords = listLongs*sizeof(uint32_t)/sizeof(uint16_t);
    const uint16_t* pList  =
        reinterpret_cast<const uint16_t*>(list.data());
    
    // Add the TA word;
    
    std::vector<uint16_t> result;
    result.push_back(ta);
    result.push_back(listwords+1);
    for(int i = 0 ; i < listwords; i++) {
        result.push_back(*pList++);
    }
    
    // If an immdediat operation, 
    
    return result;
}
////////////////////////////////////////////////////////////////
// Public functions:

/**
 * enumerateVMUSB
 *    Returns a container of VMUSB devices and their serial numbers.
 *    The VMUSB devices are represented by a USBDevice*
 *    If you choose not to use one or more of the elements of the
 *    container, you must delete them both to prevent memory leaks
 *    and to close the device recovering libusb resources
 * @param context - References the libUSB context object.
 * @return XXUSBUtil::XXUSBDevices
 */
XXUSBUtil::XXUSBDevices
XXUSBUtil::enumerateVMUSB(USB& context)
{
    return enumerateVendorAndProduct(
        context, vendorId(), VMUSBProductId()
    );

}
/**
 * enumerateCCUSB
 *    same as enumerateVMUSB but the CCUSB's are returned.
 * @param context - references the libUSB context object.
 * @return XXUSBUtil::XXUSBDevices
 */
XXUSBUtil::XXUSBDevices
XXUSBUtil::enumerateCCUSB(USB& context)
{
    return enumerateVendorAndProduct(
        context, vendorId(), CCUSBProductId()
    );
}
/**
 * enumerateVendorAndProduct
 *    Enumerate the devices that match the specified
 *    vendor and product ids.
 * @param context - References the libusb Context object.
 * @param vendor  - Vendor ID to match.
 * @param product - Product id to match
 * @return XXUSBUtil::XXUSBDevices - devics and serial number strings
 *                  of matches (might be empty)
 */
XXUSBUtil::XXUSBDevices
XXUSBUtil::enumerateVendorAndProduct(
    USB& context, uint16_t vendor, uint16_t product
)
{
    CRITICAL_BLOCK;            // Thread safe enumeration etc.
    auto allDevices = context.enumerate();   //ALl devices
    XXUSBUtil::XXUSBDevices result;
    for (int i =0; i < allDevices.size(); i++) {
        if ((vendor == allDevices[i]->getVendor()) &&
            (product == allDevices[i]->getProduct())) {
            USBDevice* pDevice = allDevices[i]->open();
            std::string serial = pDevice->getSerial();
            result.push_back({serial, pDevice});
        }
        delete allDevices[i];
    }
    
    return result;
}
/**
 * transaction
 *   Performs an XXUSB transaction.. This is, ordinarily,
 *   a write followed by a read.
 *
 * @param pDevice - pointer to the USBDevice object to which
 *     the read gets directed.
 * @param writePackage - pointer to the write packet.
 * @param writeSize    - Size of the write packet.
 * @param readPacket   - pointer to where to put the read data.
 * @param readMax      - Maximum bytes to read.
 * @param msTimeout    - Milliseconds to wait for reads to complete
 * @return int - > 0 - the total number of read bytes.
 * @retval LIBUSB_ERROR_TIMEOUT - the read timed out.
 * @throw USBException - if there are errors.
 * @throw std::logic_error - write data failed to write completely.
 * @note If there's a timeout but data were  transferred,
 *       we assume there could be more and assemble into the
 *       buffer until we hit the readMax value or timeout with
 *       no transfers.
 */
int
XXUSBUtil::transaction(
    USBDevice* pDevice,
    void* writePacket, size_t writeSize,
    void* readPacket,  size_t readMax, int msTimeout
)
{
    CRITICAL_BLOCK;
    int xferred;
    int status = pDevice->bulkTransfer(
        writeEndpoint(),
        static_cast<unsigned char*>(writePacket), int(writeSize),
        xferred, 0                            // NO timeout.
    );
    if (status) {
        //  Only error possible is a timeout:
        
        throw USBException(status, "bulk_write failed with infinit timeout");
    }
    if (xferred != writeSize) {
        // Partial write are bad too!
        
        throw std::logic_error("bulk_write - only got a partial transfer");
    }
    // Now we can loop over the reads.
    
    unsigned char* pCursor = static_cast<unsigned char*>(readPacket);
    int            nRead(0);
    int            nRemaining = readMax;
    while(nRemaining) {
        int nTransferred(0);        // In case it's not filled in.
        status = pDevice->bulkTransfer(
            readEndpoint(),
            pCursor, nRemaining, nTransferred, msTimeout
        );
        // Fold in what we got.
        
        pCursor    += nTransferred;
        nRemaining -= nTransferred;
        nRead      += nTransferred;
        
        // If there was a timeout but nothing was transferred,
        // then figure we're done:
        
        if(!nTransferred && (status == LIBUSB_ERROR_TIMEOUT)) {
            break;
        }
    }
    return nRead;
}
/**
 * writeActionRegister
 *   Writes a uint16_t word to the XXUSB action register.
 *   The meaning of the bits written depends on the device...
 *   a bit.
 *
 *  @param pDevice - the device open for write.
 *  @param data -- what to write.
 *  @throw USBException on error.
 *  @throw std::Logic_error - could not completely write action reg.
 */
void
XXUSBUtil::writeActionRegister(USBDevice* pDevice, uint16_t data)
{
    CRITICAL_BLOCK;
    
    uint16_t packet[3];
    
    packet[0] = 5;             // Select register block.
    packet[1] = 10;            // action register.
    packet[2] = data;
    
    // Write with infinite timeout:
    int nTransferred;
    int status = pDevice->bulkTransfer(
        writeEndpoint(),
        reinterpret_cast<unsigned char*>(packet), sizeof(packet),
        nTransferred, 0
    );
    if (nTransferred != sizeof(packet)) {
        throw std::logic_error("Incomplete write of action register");
    }
    
}
/**
 * executeList
 *   Executes an immediate list in an XXUSB controller.  The
 *   list is supplied as a vector of uint32_t:
 *
 * @param pDevice - device controller object.
 * @param list    - list of operations to perform.
 * @param[out] pReturnedData - buffer to contain any data read by the list.
 * @param maxRead - Number of bytes in the returned data.
 * @param[out] bytesRead - Reference to the value that receives the
 *                   number of bytes that will be put in pReturnedData
 * @param msTimeout - Milliseconds we allow for the read to timeout.
 * @throw std::logic_error for some errors detected at this level.
 * @throw USBException - for error detected by the direct calls to libusb
 *                    function by the pDevice.
 * @note the transaction method has the CRITICAL_BLOCK
 */
void
XXUSBUtil::executeList(
    USBDevice* pDevice, const std::vector<uint32_t>& list,
    void* pReturnedData, size_t maxRead, size_t& bytesRead,
    int msTimeout
)
{
    std::vector<uint16_t> outPacket = listToOutPacket(
        TAVcsWrite | TAVcsIMMED,
        list
    );
    int result = transaction(
        pDevice, outPacket.data(), outPacket.size()*sizeof(uint16_t),
        pReturnedData, maxRead, msTimeout
    );
    
    if (result < 0) {
        throw USBException(result, "Execute immediate list failed on read");
    }
    bytesRead = result;
}
/**
 * readEndpoint
 *   Returns the USB endpoint to use for XXUSB read operations.
 *
 * @return uint8_t
 */
uint8_t
XXUSBUtil::readEndpoint()
{
    return 0x86;
}
/**
 * writeEndpoint
 *    Returns the USB endpoint to use for XXUSB write operations.
 * @return uint8_t
 */
uint8_t
XXUSBUtil::writeEndpoint()
{
    return 0x2;
}
/**
 * VMUSBProductId
 *   Returns the product id of a VMUSB.
 * @return uint16_t
 */
uint16_t
XXUSBUtil::VMUSBProductId()
{
    return 0xb;
}
/**
 * CCSUSBProductId
 *    Returns the product ID of a CCUSB
 * @return uint16_t
 */
uint16_t
XXUSBUtil::CCUSBProductId()
{
    return 1;
}