/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
       NSCL DAQ Development Group 
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/


#ifndef __CCCUSBREMOTE_H
#define __CCCUSBREMOTE_H

#include <CCCUSB.h> 
#include <vector>
#include <stdint.h>
#include <sys/types.h>
#include <string>
#include <cstdlib>

// Forward Class definitions:

class CCCUSBReadoutList;
class CSocket;
class CTCLInterpreter;

/**
 *  This class provides remote access to the VM-USB through the
 *  VMUSBModule support in the tcl server.
 *  An external program can link to libVMUSBRemote.so, instantiate
 *  this class and pretty much have its way with the VM-USB
 *  as if it owned it.  
 *  Restrictions:
 *   - lists cannot be downloaded (only immediate lists can be run).
 *   - the action register is not accessible.
 *   - The IRQ Mask cannot be written as that abomination requires 
 *     action register operations
 *   - while internal registers can be manipulated be aware that
 *     # This may cuase an active readout to have undefined results.
 *     # Registers you write may be overwritten by Readout's startup 
 *     # code.
 */
class CCCUSBRemote : public CCCUSB
{

    // Class member data.
private:
  std::string      m_deviceName;
  std::string      m_host;    /* host name */
  std::string      m_portStr; /* port number as a string */
  CSocket*         m_pSocket; /* Connection with remote system. */
  CTCLInterpreter* m_pInterp; /* Having this makes TclList processing easier. */
  std::string      m_lastError;
public:

    // Constructors and other canonical functions.
    // Note that since destruction closes the handle and there's no
    // good way to share usb objects, copy construction and
    // assignment are forbidden.
    // Furthermore, since constructing implies a usb_claim_interface()
    // and destruction implies a usb_release_interface(),
    // equality comparison has no useful meaning either:

  CCCUSBRemote(std::string deviceName, 
               std::string host,
               unsigned int port);
    virtual ~CCCUSBRemote();    // Although this is probably a final class.

    // Disallowed functions as described above.
private:
    CCCUSBRemote(const CCCUSBRemote& rhs);
    CCCUSBRemote& operator=(const CCCUSBRemote& rhs);
    int operator==(const CCCUSBRemote& rhs) const;
    int operator!=(const CCCUSBRemote& rhs) const;
public:
    std::string getLastError() {
      return m_lastError;
    }
    // Register I/O operations.
public:

    /*! Force a reconnection
    *
    * Immediately forces a disconnection and then subsequently
    * tries to connect.
    *
    */
    virtual bool reconnect();
    
    /*! Not a permitted action.
    *
    * Only stack executions are allowed and writing to the action 
    * register is not possible in a stack. So this is not supported
    * by this class.
    * 
    */
    virtual void writeActionRegister(uint16_t);

    /*!
      Execute a list immediately.  It is the caller's responsibility
      to ensure that no data taking is in progress and that data taking
      has run down (the last buffer was received).  
      The list is transformed into an out packet to the VMUSB and
      transaction is called to execute it and to get the response back.
      \param list  : CCCUSBReadoutList&
          A reference to the list of operations to execute.
      \param pReadBuffer : void*
          A pointer to the buffer that will receive the reply data.
      \param readBufferSize : size_t
          number of bytes of data available to the pReadBuffer.
      \param bytesRead : size_t*
          Return value to hold the number of bytes actually read.

      \return int
      \retval  0    - All went well.
      \retval -1    - The Set to the server failed.
      \retval -2    - The Receive of data from the server indicated an error.
      \retval -3    - The server returned an error, use getLastError to retrieve it.
      \retval -4    - The socket or the interpreter owned by the CCCUSBRemote is NULL. 
      This happens when disconnect is called before this method.

      In case of failure, the reason for failure is stored in the
      errno global variable.

     */
    virtual int executeList( CCCUSBReadoutList& list,
                             void*              pReadBuffer,
                             size_t             readBufferSize,
                             size_t*            bytesRead);
    
    /*! Load a list for later execution
    * 
    * THIS IS NOT SUPPORTED FOR THIS CLASS. Only the immediate execution
    * of stacks is supported by this class so this will never succeed.
    */
    virtual int loadList( uint8_t listNumber, CCCUSBReadoutList& list);
      

    // Once the interface is in DAQ auntonomous mode, the application
    // should call the following function to read acquired data.

    virtual int usbRead( void* data, size_t bufferSize, size_t* transferCount,
                         int timeout = 2000);
    
    
    // Some simple getters
    std::string  getHost() { return m_host; }
    unsigned int getPort() { return atoi(m_portStr.c_str()); }



    // Local functions:
private:
    /**
     *  Marshall a CVMEReadoutList into a string that is a Tcl list of integers representing the
     *  contents of the list.
     *  @param list  - The CVMEReadoutList object.
     *  @return string
     *  @retval the Tcl list constructed from the vme readout list.
     */
    std::string marshallList(CCCUSBReadoutList& list);

    /*!
     *  Marshall a reply buffer from the server into the user's buffer.
     *  @param pOutputBuffer - Pointer to the  user's output buffer.
     *  @param reply         - Pointer to the server's reply, a Tcl list.
     *  @param maxOutputSize - Size of the output buffer... no more than this number of bytes will be
     *                         written to the output buffer.
     * @return size_t
     * @retval Actual number of bytes written to the output buffer.
     */
    size_t marshallOutputData(void* pOutputBuffer, const char* reply, size_t maxOutputSize);

    /*! Connect to the slow controls server
    *
    * A new TCL interpreter is created as well as a new connection to the
    * the socket at m_host:m_portStr.
    */
    void connect();

    /*! Force an immediate disconnection from server
    *
    * The socket is shutdown and then deleted as well as the tcl interpreter
    * owned by this class. Before calling any more operations, the server must
    * be connected to again using the connect method. 
    */
    void disconnect();

};



#endif
