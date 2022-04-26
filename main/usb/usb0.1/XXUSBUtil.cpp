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
#include <NSCLDAQLog.h>
#include <sstream>
#include <errno.h>
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

namespace XXUSBUtil {
    bool logTransactions = false;
    std::string logFile;
}
////////////////////////////////////////////////////////////////
// Private utilities:
/**
 * formatBlock
 *    Format a block of bytes as consecutive uint16_t's
 * @param stream - an ostream into which the formatting is done.
 * @param block - pointer to the block.
 * @param nBytes - number of _bytes_.
 * @note nBytes will be assumed to be a multiple of uint16_t's.
 * @note the caller must set up the desired radix.
 * 
 */
static void
formatBlock(std::ostream& stream, const void* block, size_t nBytes)
{
    const uint16_t* pBlock = reinterpret_cast<const uint16_t*>(block);
    size_t n = nBytes / sizeof(uint16_t);
    for (int i =0; i < n; i += 8) {
        for (int j = 0; (j < 8) && ((i+j) < n); j++ ) {
            stream << *pBlock++ << " ";
        }
        stream << std::endl;
    }
}

/**
 * logBulkWrite
 *    Does a trace log of the initiation of a bulk write
 *  @param endpoint - the USB endpoint to which the write is directed.
 *  @param block    - the block to write.
 *  @param nBytes   - Bytes in the block.
 */
static void
logBulkWrite(int endpoint, void* block, size_t nBytes)
{
    std::stringstream strMsg;
    strMsg << "About to initiate a bulk write of " << nBytes << " bytes: \n";
    strMsg << std::hex;
    formatBlock(strMsg, block, nBytes);
    strMsg << std::dec;
    strMsg << std::endl;
    
    std::string msg = strMsg.str();
    daqlog::trace(msg);
}
/**
 * logBulkWriteReswult
 *    @param status - status of the operation.
 *    @param xferred - bytes transferred.
 */
static void
logBulkWriteResult(int status, size_t xferred)
{
    std::stringstream strMsg;
    
    strMsg << "Bulk read completed status: " << status
        << " bytes transferred: " << xferred << std::endl;
    
    std::string msg = strMsg.str();
    daqlog::trace(msg);
}

/**
 * logBulkRead
 *    Log the completion of a bulk read.
 *  @param int endpoint - USB endpoint being read from.
 *  @param reqSize - requested size.
 *  @param pBlock  - Block read.
 *  @param nread   - Bytes actually read.
 *  @param msTimeout - Read timeout used in ms.
 */
static void
logBulkRead(
    int endpoint, size_t reqSize, const void* pBlock, size_t nread,
    unsigned msTimeout
)
{
    std::stringstream strMsg;
    strMsg << "Completed bukl read of USB endpoint "
        << endpoint << " Requested: " << reqSize << " bytes "
        << " received " << nread << " bytes (timeout ="
        << msTimeout
        << "ms)" << std::endl;
    strMsg << "Data: \n";
    strMsg << std::hex;
    formatBlock(strMsg, pBlock, nread);
    strMsg << std::dec << std::endl;
    
    std::string msg = strMsg.str();
    daqlog::trace(msg);
}
/**
 * copy32To16
 *    Copy a vector of 32 bit words to the back of a vector
 *    of 16 bit words.
 *
 *  @param[out] v16  - Output vector.
 *  @param[in]  v32  - Input vector.
 */
template<class T>
static void
copy32To16(std::vector<uint16_t>& v16, const std::vector<T>& v32)
{
    const uint16_t* src = reinterpret_cast<const uint16_t*>(v32.data());
    size_t nWds         = v32.size()*(sizeof(T)/sizeof(uint16_t));
    for (int i =0; i < nWds; i++) {
        v16.push_back(*src++);
    }
}

/**
 * listToOutPacket
 *    Converts an operation list into an out packet.
 *
 * @param ta   - Transfer address word.
 * @param list - std::vector<uint32_t> containing the list of operations.
 * @return std::vector<uint16_t> containing the resulting list.
 * 
 */
template<class T>
static std::vector<uint16_t>
listToOutPacket(uint16_t ta, const std::vector<T>& list)
{
    size_t listLongs = list.size();
    size_t listwords = listLongs*(sizeof(T)/sizeof(uint16_t));
    
    // Add the TA word;
    
    std::vector<uint16_t> result;
    result.push_back(ta);
    
    if (sizeof(T) == sizeof(uint32_t)) {     // VMUSB
        result.push_back(listwords+1);
	result.push_back(0);
    } else {                                 // CCUSB
        result.push_back(listwords);  
    }
    copy32To16(result, list);
    
    return result;
}
/**
 * vmusbListToOutPacket
 *   Takes a VMUSB list and creates the output packet needed to
 *   load it.
 *
 * @param listNum  - List number
 * @param list     - List data.
 * @param offset   - Where in list memory to load the list.
 * @return std::vector<uint16_t> - the full out packet.
 * 
*/
static std::vector<uint16_t>
vmusblistToOutPacket(
    uint8_t listNum, const std::vector<uint32_t>& list, size_t offset
)
{
    std::vector<uint16_t> result;
    
    // The header:
    
    uint16_t ta = TAVcsSel | TAVcsWrite;
    if (listNum & 1) ta    |= TAVcsID0;
    if (listNum & 2) ta    |= TAVcsID1;
    if (listNum & 4) ta    |= TAVcsID2;
    result.push_back(ta);
    result.push_back(list.size()*(sizeof(uint32_t)/sizeof(uint16_t)));
    result.push_back(offset);
    
    copy32To16(result, list);
    
    
    return result;
}
/**
 * ccusblistToOutPacket
 *    Convert a CCUSB readout list (to be downloaded) to an output packet.
 *
 * @param list - The list
 * @param scaler - True if this is a scaler list.
 */
static std::vector<uint16_t>
ccusblistToOutPacket(const std::vector<uint16_t>& list, bool scaler)
{
    uint16_t ta = TAVcsWrite | TAVcsSel;
    if (scaler) ta |= TAVcsID0;                 // Scaler bit.
    
    std::vector<uint16_t> result;
    result.push_back(ta);
    result.push_back(list.size());
    copy32To16(result, list);
    
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
    if (logTransactions) {
        logBulkWrite(writeEndpoint(), writePacket, writeSize);
    }
    int status = pDevice->bulkTransfer(
        writeEndpoint(),
        static_cast<unsigned char*>(writePacket), int(writeSize),
        xferred, 0                            // NO timeout.
    );
    if (logTransactions) {
        logBulkWriteResult(status, xferred);
    }
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
    if (logTransactions) {
        daqlog::trace("Starting USB block read: ");
    }
    while(nRemaining) {
        int nTransferred(0);        // In case it's not filled in.
        
        status = pDevice->bulkTransfer(
            readEndpoint(),
            pCursor, nRemaining, nTransferred, msTimeout
        );
        if(logTransactions) {
            logBulkRead(
                readEndpoint(), nRemaining, pCursor, nTransferred, msTimeout
            );
        }
        // Fold in what we got.
        
        pCursor    += nTransferred;
        nRemaining -= nTransferred;
        nRead      += nTransferred;
        
        // If there was a timeout but nothing was transferred,
        // then figure we're done:
        
        if(!nTransferred && (status == -ETIMEDOUT)) {
            break;
        }
    }
    if (logTransactions) {
        daqlog::trace("USB Block read completed");
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
template<class T>
static void
executeList(
    USBDevice* pDevice, const std::vector<T>& list,
    void* pReturnedData, size_t maxRead, size_t& bytesRead,
    int msTimeout
)
{
    std::vector<uint16_t> outPacket = listToOutPacket(
        TAVcsWrite | TAVcsIMMED,
        list
    );
    int result = XXUSBUtil::transaction(
        pDevice, outPacket.data(), outPacket.size()*sizeof(uint16_t),
        pReturnedData, maxRead, msTimeout
    );
    
    if (result < 0) {
        throw USBException(result, "Execute immediate list failed on read");
    }
    bytesRead = result;
}
// Specializations for the two controllers:

void
XXUSBUtil::executeList(
    USBDevice* pDevice, const std::vector<uint32_t>& list,
    void* pReturnedData, size_t maxRead, size_t& bytesRead,
    int msTimeout

)
{
    // VMUSB:
    
    ::executeList(
        pDevice, list, pReturnedData, maxRead, bytesRead, msTimeout
    );
}
void
XXUSBUtil::executeList(
    USBDevice* pDevice, const std::vector<uint16_t>& list,
    void* pReturnedData, size_t maxRead, size_t& bytesRead,
    int msTimeout

)
{
    // CCUSB
    
    ::executeList(
        pDevice, list, pReturnedData, maxRead, bytesRead, msTimeout
    );
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
 * vendorId
 *   @return uint16_t - WIENER USB Vendor id.
 */
uint16_t
XXUSBUtil::vendorId()
{
    return 0x16dc;
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
/**
 * VMUSB::loadList
 *    Loads a list into the VMUSB for later triggered execution
 *    in datataking mode. Note that there are  differences
 *    between this for the VMUSB and CCUSB. Therefore,
 *    there are two loadList function, one in the VMUSB child
 *    namespace and one in the CCUSB child namespace.
 *  @param pDevice - Pointer to the USBDevice object through which
 *                   USB operations are done.
 *  @param listNum - number of the list to load [0-7]
 *  @param list   - Vector of uint32_t's containing the list.
 *  @param offset - Offset in VMUSB list memory at which the list is to be loaded.
 *  @throw std::range_error - if the list number is illegal.
 *  @throw USBException - if the USBDevice detected errors.
 *                        or we had a timeout.
 */
void
XXUSBUtil::VMUSB::loadList(
    USBDevice* pDevice, uint8_t listNum,
    const std::vector<uint32_t>& list, size_t offset
)
{
    if (listNum > 7) {
        throw std::range_error("VMUSB lists must be in the range [0-7]");
    }
    std::vector<uint16_t> outList = vmusblistToOutPacket(listNum, list, offset);
    
    int transferred;
    int status = pDevice->bulkTransfer(
        writeEndpoint(),
        reinterpret_cast<unsigned char*>(outList.data()), list.size()*sizeof(uint16_t),
        transferred, 0
    );
    // Timeout is not possible but...
    
    if (status < 0) {
        throw USBException(status, "Load list timed out with infinite timeout");
    }
}
/**
 * CCUSB::loadList
 *    Loads a list into the CCUSB for later triggered execution.  The
 *    CCUSB has only an event and a scaler list, in contrast to the
 *    8 lists a VMUSB has.  Furthermore, the memory for the two  lists
 *    is dedicated rather than a soup across which all lists span.
 *    Therefore there's no list number (just a scaler flag), and no
 *    memory offset parameter:
 *
 *  @param pDevice - USBDevice object pointer through which output flows.
 *  @param list    - The list of operations to perform
 *  @param scaler  - True if this is a scaler list.
 */
void
XXUSBUtil::CCUSB::loadList(
    USBDevice* pDevice, const std::vector<uint16_t>& list, bool scaler
)
{
    std::vector<uint16_t> outList = ccusblistToOutPacket(list, scaler);
    int transferred;
    int status = pDevice->bulkTransfer(
        writeEndpoint(),
        reinterpret_cast<unsigned char*>(outList.data()), outList.size()*sizeof(uint16_t),
        transferred, 0
    );
    if (status < 0) {
        throw USBException(status, "Load list (infinite timeout) failed.");
    }
}
