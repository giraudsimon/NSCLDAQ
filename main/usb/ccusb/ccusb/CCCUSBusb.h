/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

#ifndef __CCCUSBusb_H
#define __CCCUSBusb_H


#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __CRT_SYS_TYPES_H
#include <sys/types.h>
#ifndef __CRT_SYS_TYPES_H
#define __CRT_SYS_TYPES_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __CCCUSBREADOUTLIST_H
#include <CCCUSBReadoutList.h>
#ifndef __CCCUSBREADOUTLIST_H
#define __CCCUSBREADOUTLIST_H
#endif
#endif


#ifndef __CCCUSB_H
#include <CCCUSB.h>
#ifndef __CCCUSB_H
#define __CCCUSB_H
#endif
#endif

#include <CMutex.h>

//  The structures below are defined in <usb.h> which is included
//  by the implementation and can be treated as opaque by any of our
//  clients (they are in fact opaque in usb.h if memory servers.

struct usb_device;
struct usb_dev_handle;


// Forward Class definitions:

/*! Controls a Wiener CC-USB connected to localhost

  This class is the low level support for the Wiener/JTEC CCUSB module.
  the CCUSB is a USB CAMAC controller.  To use this class you must first locate
  a module by invoking enumerate.  enumerate  returns a vector of
  usb_device*s.   One of those can be used to instantiate the CCCUSBusb 
  object which can the ben operated on.

  Most all of the functionality is implemented in te base class CCCUSB. 
  The methods defined in this class are the minimum needed to be implemented
  in order to attach to a locally connected usb device (i.e. the usb device
  is directly connected to the computer).

*/
class CCCUSBusb : public CCCUSB
{


  // Class member data.
  private:
    struct usb_dev_handle*  m_handle;  //!< Handle open on the device.
    struct usb_device*      m_device;  //!< Device we are open on.
    int                     m_timeout; //!< Timeout used when user doesn't give one.
    std::string             m_serial;  //!< Connected device serial number.
    CMutex*                 m_pMutex;  // Basis for critical sections.

  public:
    // Constructors and other canonical functions.
    // Note that since destruction closes the handle and there's no
    // good way to share usb objects, copy construction and
    // assignment are forbidden.
    // Furthermore, since constructing implies a usb_claim_interface()
    // and destruction implies a usb_release_interface(),
    // equality comparison has no useful meaning either:

    CCCUSBusb(struct usb_device* vmUsbDevice);
    virtual ~CCCUSBusb();   // Although this is probably a final class.

    // Disallowed functions as described above.
  private:
    CCCUSBusb(const CCCUSBusb& rhs);
    CCCUSBusb& operator=(const CCCUSBusb& rhs);
    int operator==(const CCCUSBusb& rhs) const;
    int operator!=(const CCCUSBusb& rhs) const;

  public:

    /*! Force a reconnection 
    *
    * Releases the usb device and then tries
    * to open it again. 
    */
    virtual bool reconnect();

    // Register I/O operations.
  public:
    // The action register is purely abstract in the base class and 
    // therefore needs to be define here. It is fundamentally differnt
    // than all other registers in the CC-USB.
    void     writeActionRegister(uint16_t value);


    // ALL CAMAC operations are inherited from the base class CCCUSB
    // because there is nothing special to do differently than the 
    // base class implementation, we will just inherit the default
    // behavior.

  public:

    /*! Execute a list immediately.  
      
      It is the caller's responsibility
      to ensure that no data taking is in progress and that data taking
      has run down (the last buffer was received).  
      The list is transformed into an out packet to the CCUSB and
      transaction is called to execute it and to get the response back.
    
      \param list           : CCCUSBReadoutList&
                              A reference to the list of operations to execute.
      \param pReadBuffer    : void*
                              A pointer to the buffer that will receive the reply data.
      \param readBufferSize : size_t 
                              number of bytes of data available to the pReadBuffer.
      \param bytesRead      : size_\t* 
                              Return value to hold the number of bytes actually read.

      \return int
      \retval  0    - All went well.
      \retval -1    - The usb_bulk_write failed.
      \retval -2    - The usb_bulk_read failed.

      In case of failure, the reason for failure is stored in the
      errno global variable.
     */
    int executeList(CCCUSBReadoutList&  list,
                    void*               pReadBuffer,
                    size_t              readBufferSize,
                    size_t*             bytesRead);


    /*! Load a list into the CC-USB for later execution.
      
        It is the callers responsibility to:
        -   keep track of the lists and their  storage requirements, so that 
            they are not loaded on top of or overlapping
            each other, or so that the available list memory is not exceeded.
        -   Ensure the list number is valid and map it to a TAV.
        -   The listOffset is valid and that there is room in the list memory
            following it for the entire list being loaded.
            This code just load the list, it does not attach it to any specific trigger.
            that is done via register operations performed after all the required lists
            are in place.

      \param listNumber : uint8_t  
          Number of the list to load. 
          - 0 - Data list
          - 1 - Scaler list.
      \param list       : CCCUSBReadoutList
          The constructed list.


      \return int
      \retval 0  - AOK.
      \retval -4 - List number is illegal
      \retval other - error from transaction.

     */
    int loadList( uint8_t               listNumber,
                  CCCUSBReadoutList&    list);



    /*! Execute a bulk read for the user.  
      
      The user will need to do this
      when the VMUSB is in autonomous data taking mode to read buffers of data
      it has available.
  
      \param data : void*
          Pointer to user data buffer for the read.
      \param buffersSize : size_t
          size of 'data' in bytes.
      \param transferCount : size_t*
          Number of bytes actually transferred on success.
      \param timeout : int [2000]
          Timeout for the read in ms.

      \return int
      \retval 0   Success, transferCount has number of bytes transferred.
      \retval -1  Read failed, errno has the reason. transferCount will be 0.

     */
    int usbRead(void* data, size_t bufferSize, size_t* transferCount,
                int timeout = 2000);

    /*!
    *
    */  
    void setDefaultTimeout(int ms); // Can alter internally used timeouts.

    // Register bit definintions.

    // Local functions:
  private:

    /*
       Utility function to perform a 'symmetric' transaction.
       Most operations on the VM-USB are 'symmetric' USB operations.
       This means that a usb_bulk_write will be done followed by a
       usb_bulk_read to return the results/status of the operation requested
       by the write.
       Parametrers:
        void*   writePacket   - Pointer to the packet to write.
        size_t  writeSize     - Number of bytes to write from writePacket.

        void*   readPacket    - Pointer to storage for the read.
        size_t  readSize      - Number of bytes to attempt to read.


       Returns:
        > 0 the actual number of bytes read into the readPacket...
            and all should be considered to have gone well.
        -1  The write failed with the reason in errno.
        -2  The read failed with the reason in errno.

       NOTE:  The m_timeout is used for both write and read timeouts.

     */
    int transaction(void* writePacket, size_t writeSize,
                    void* readPacket,  size_t readSize);

    /**
     * openUsb
     *
     *  Does the common stuff required to open a connection
     *  to a CCUSB given that the device has been filled in.
     *
     *  Since the point of this is that it can happen after a power cycle
     *  on the CAMAC crate, we are only going to rely on m_serial being
     *  right and re-enumerate.
     *
     *  @throw std::string - on errors.
     */
    void openUsb();

    void resetUSB();
    void enumerateAndIdentify();

};

#endif
