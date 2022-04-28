/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
       NSCL DAQ Development group 
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

// Implementation of the CCCUSB class.

#include <config.h>
#include "CCCUSBRemote.h"
#include "CCCUSBReadoutList.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <TCLList.h>
#include "CSocket.h"

#include <errno.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

using namespace std;

// Constants:

static const int DEFAULT_TIMEOUT(2000); // ms.

/////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
/*!
  Construct the CCCUSBRemote object.  This involves storing the
  device descriptor we are given, opening the device and
  claiming it.  Any errors are signalled via const char* exceptions.
  @param deviceName - Name of the VMUSB Module in the controlconfig.tcl file.
  defaults to vmusb
  @param host   - Name of the host to which we are connecting.  This
  defaults to localhost.
  @param port   - port port on which the tcl server is running.  This
  defaults to 27000

  In a single machine environment you should not need to modify these
  settings... in a multisystem environment where you are remote from the
  Readout, you should typically only need to modify host.

  @note - A Tcl server must be running else an exception is thrown.

 */
CCCUSBRemote::CCCUSBRemote(string deviceName, string host, unsigned int port) 
: m_deviceName(deviceName),
  m_host(host),
  m_portStr(),
  m_pSocket(0),
  m_pInterp(0)
{
  try {
    ostringstream portStr; 
    portStr << port;
    m_portStr = portStr.str();
    connect();
  }
  catch (...) {     // Exception catch prevents memory leaks and...
    delete m_pSocket;
    m_pSocket = 0;
    delete m_pInterp;
    m_pInterp = 0;
    throw;      // lets the caller deal with the error.
  }
}


/*! 
* connect 
*
* Attempts to connect to the host (m_host) at port
* (m_portStr). In addition it starts up a new TCL interpreter.
*/
void CCCUSBRemote::connect() {
  m_pSocket = new CSocket;
  m_pSocket->Connect(m_host, m_portStr);
  m_pInterp = new CTCLInterpreter();
}

/*!
* Forces a disconnection of the socket. All operations ultimately
* clean up the state as well. The socket is deleted and the 
* interpreter is deleted as well.
*/
void CCCUSBRemote::disconnect() 
{
  if (m_pSocket) {
    m_pSocket->Shutdown();
    delete m_pSocket;
    m_pSocket = 0;
  }
  if (m_pInterp) { 
    delete m_pInterp;
    m_pInterp = 0;
  }
}


////////////////////////////////////////////////////////////////
/*!
  Destruction of the interface involves shutting down the socket 
  to the server

 */
CCCUSBRemote::~CCCUSBRemote()
{
  disconnect(); 
}

bool CCCUSBRemote::reconnect()
{
  uint32_t fware;
  if (readFirmware(fware) != 0) {
    if (m_pSocket) {
      disconnect();
    }
    connect();
    return true;                         // Actually did reconnect,.
  }
  return false;                          // Didn't need to reconnect.
}


void CCCUSBRemote::writeActionRegister(uint16_t action) 
{
  std::cerr << "CCCUSBRemote::writeActionRegister() : remote writes to action register "
            << "are not permitted." 
            << std::endl;

}

int CCCUSBRemote::loadList(uint8_t listNumber, CCCUSBReadoutList& list)
{

  std::cerr << "CCCUSBRemote::loadList() : loading lists is not permitted in remote "
            << "controlled VM-USB"
            << std::endl;

  return -1;

}

int CCCUSBRemote::usbRead(void* data, size_t bufferSize, size_t* transferCount,
    int timeout)
{

  std::cerr << "CCCUSBRemote::usbRead() : usbRead is not permitted in remote "
            << "controlled VM-USB"
            << std::endl;

  return -1;

}

//////////////////////////////////////////////////////////////////////////
/////////////////////////// List operations  ////////////////////////////
/////////////////////////////////////////////////////////////////////////

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
int
CCCUSBRemote::executeList( CCCUSBReadoutList& list,
                           void*              pReadoutBuffer,
                           size_t             readBufferSize,
                           size_t*            bytesRead)
{

  m_lastError = "";

  // Check that the interpreter and the socket exist
  if (m_pSocket==0 || m_pInterp==0) {
    return -4;
  }


  string vmeList      = marshallList(list);
  CTCLObject datalist;
  datalist.Bind(m_pInterp);
  datalist += static_cast<int>(readBufferSize);
  datalist += vmeList;


  // Send the list to the remote with a "\n" on the back end of it:

  string request = "Set ";
  request       += m_deviceName;
  request       += " list {";
  request       += (string)datalist;
  request       += "}\n";
  try {
    m_pSocket->Write(request.c_str(), request.size());
  }
  catch(...) {
    return -1;
  }

  // Get the reply  data back.. each  of the read buffer can be at most
  // {127 } So allow 8 chars just for fun for each byte in allocating the receive buffer:
  // In getting the reply we keep reading until we get a \n in the input buffer.
  //
  char* response = new char[readBufferSize*8+256];
  size_t remaining = readBufferSize*8 + 200; // Safety buffer.
  int   offset   = 0;
  bool  done     = false;
  int   nread;
  while(!done && (remaining > 0)  ) {
    try {
      nread = m_pSocket->Read(&(response[offset]), readBufferSize*8);
    }
    catch (...) {
      delete []response;
      return -2;
    }
    response[offset+nread] = 0; // Null terminate the data...
    if (strchr( response, '\n')) {
      done = true;
    } 
    else {
      offset    += nread;
      remaining -= nread;
    }
  }
  // The entire response is here... marshall the respones into the output buffer

  try {
    *bytesRead = marshallOutputData(pReadoutBuffer, response, readBufferSize);
  }
  catch (...) {
    delete []response;
    return -3;
  }
  delete[]response;
  return 0;
}


/**
 *  Marshall a CVMEReadoutList into a string that is a Tcl list of integers representing the
 *  contents of the list.
 *  @param list  - The CVMEReadoutList object.
 *  @return string
 *  @retval the Tcl list constructed from the vme readout list.
 */
string
CCCUSBRemote::marshallList(CCCUSBReadoutList& list)
{
  vector<uint16_t> listVect = list.get();
  CTCLObject       TclList;
  TclList.Bind(m_pInterp);
  for (int i =0; i < listVect.size(); i++) {
    char item[100];
    sprintf(item, "0x%x", listVect[i]);
    TclList += item;
  }
  return (string)TclList;
}
/**
 *  Marshall a reply buffer from the server into the user's buffer.
 *  @param pOutputBuffer - Pointer to the  user's output buffer.
 *  @param reply         - Pointer to the server's reply, a Tcl list.
 *  @param maxOutputSize - Size of the output buffer... no more than this number of bytes will be
 *                         written to the output buffer.
 * @return size_t
 * @retval Actual number of bytes written to the output buffer.
 */
size_t
CCCUSBRemote::marshallOutputData(void* pOutputBuffer, const char* reply, size_t maxOutputSize)
{
  uint8_t* o = reinterpret_cast<uint8_t*>(pOutputBuffer);

  //  throw the reply data into a list and split it:

  CTCLList tclList(m_pInterp, reply);
  StringArray list;
  tclList.Split(list);

  // The first element is the status.
  // if it is OK no problems. if it is ERROR set the error informatino.
  //

  if (list[0] == "ERROR") {
    m_lastError = reply;
    throw m_lastError;    // Error in reply.
  }

  //  OK second element is  '-' and third element is the response data: 

  CTCLList resultList(m_pInterp, list[2]);
  list.clear();
  resultList.Split(list);

  // Figure out how many bytes the user gets...

  size_t actualSize = list.size();
  if (actualSize > maxOutputSize) actualSize = maxOutputSize;

  // Each list element is an ascii encoded byte:

  for (int i =0; i < actualSize; i++) {
    unsigned long aByte;
    aByte = strtoul(list[i].c_str(), NULL, 0);
    *o++ = static_cast<uint8_t>(aByte & 0xff);
  }

  return actualSize;
}


void* gpTCLApplication(0);
