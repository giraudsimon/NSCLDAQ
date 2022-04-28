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

// Implementation of the CVMUSB class.

#include <config.h>
#include "CVMUSBRemote.h"
#include "CVMUSBReadoutList.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <TCLList.h>
#include "CSocket.h"

#include <errno.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

// Constants:

// Identifying marks for the VM-usb:

static const short USB_WIENER_VENDOR_ID(0x16dc);
static const short USB_VMUSB_PRODUCT_ID(0xb);

// Bulk transfer endpoints

static const int ENDPOINT_OUT(2);
static const int ENDPOINT_IN(0x86);
// Timeouts:

static const int DEFAULT_TIMEOUT(2000);	// ms.



// Bits in the list target address word:

static const uint16_t TAVcsID0(1); // Bit mask of Stack id bit 0.
static const uint16_t TAVcsSel(2); // Bit mask to select list dnload
static const uint16_t TAVcsWrite(4); // Write bitmask.
static const uint16_t TAVcsIMMED(8); // Target the VCS immediately.
static const uint16_t TAVcsID1(0x10);
static const uint16_t TAVcsID2(0x20);
static const uint16_t TAVcsID12MASK(0x30); // Mask for top 2 id bits
static const uint16_t TAVcsID12SHIFT(4);

//   The following flag determines if enumerate needs to init the libusb:

static bool usbInitialized(false);

/////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
/*!
  Construct the CVMUSBRemote object.  This involves storing the
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
CVMUSBRemote::CVMUSBRemote(string deviceName, string host, unsigned int port) :
  m_deviceName(deviceName),
  m_pSocket(0),
  m_pInterp(0)
{
  try {
    char portNumber[100];
    sprintf(portNumber, "%d", port);
    m_pSocket = new CSocket;
    m_pSocket->Connect(host, string(portNumber));
    m_pInterp = new CTCLInterpreter();
  }
  catch (...) {			// Exception catch prevents memory leaks and...
    delete m_pSocket;
    m_pSocket = 0;
    delete m_pInterp;
    m_pInterp = 0;
    throw;			// lets the caller deal with the error.
  }
}
////////////////////////////////////////////////////////////////
/*!
    Destruction of the interface involves shutting down the socket 
    to the server

*/
CVMUSBRemote::~CVMUSBRemote()
{
  if (m_pSocket) {
    m_pSocket->Shutdown();
    delete m_pSocket;
  }
  delete m_pInterp;
}


void CVMUSBRemote::writeActionRegister(uint16_t action) 
{
  std::cerr << "CVMUSBRemote::writeActionRegister() : remote writes to action register "
            << "are not permitted." 
            << std::endl;

}

int CVMUSBRemote::loadList(uint8_t listNumber, CVMUSBReadoutList& list,
                            off_t listOffset)
{
  
  std::cerr << "CVMUSBRemote::loadList() : loading lists is not permitted in remote "
            << "controlled VM-USB"
            << std::endl;

  return -1;

}

int CVMUSBRemote::usbRead(void* data, size_t bufferSize, size_t* transferCount,
                          int timeout)
{
  
  std::cerr << "CVMUSBRemote::usbRead() : usbRead is not permitted in remote "
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
    \param list  : CVMUSBReadoutList&
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

    In case of failure, the reason for failure is stored in the
    errno global variable.


*/
int
CVMUSBRemote::executeList(CVMUSBReadoutList& list,
		   void*                  pReadoutBuffer,
		   size_t                 readBufferSize,
		   size_t*                bytesRead)
{

  m_lastError = "";

  string vmeList      = marshallList(list);
  CTCLObject datalist;
  datalist.Bind(m_pInterp);
  datalist += static_cast<int>(readBufferSize);
  datalist += vmeList;


  // Send the list to the remote with a "\n" on the back end of it:

  string request = "Set ";
  request       += m_deviceName;
  request       += " list {";
  request += (string)datalist;
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
    response[offset+nread] = 0;	// Null terminate the data...
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

////////////////////////////////////////////////////////////////////////
/////////////////////////////// Utility methods ////////////////////////
////////////////////////////////////////////////////////////////////////

/*
   Reading a register is just creating the appropriate CVMUSBReadoutList
   and executing it immediatly.
*/
uint32_t
CVMUSBRemote::readRegister(unsigned int address)
{
    CVMUSBReadoutList list;
    uint32_t          data;
    size_t            count;
    list.addRegisterRead(address);

    int status = executeList(list,
			     &data,
			     sizeof(data),
			     &count);
    if (status == -1) {
	char message[100];
	sprintf(message, "CVMUSBRemote::readRegister USB write failed: %s",
		strerror(errno));
	throw string(message);
    }
    if (status == -2) {
	char message[100];
	sprintf(message, "CVMUSBRemote::readRegister USB read failed %s",
		strerror(errno));
	throw string(message);

    }


    return data;
			     

    
}
/*
  
   Writing a register is just creating the appropriate list and
   executing it immediately:
*/
void
CVMUSBRemote::writeRegister(unsigned int address, uint32_t data)
{
    CVMUSBReadoutList list;
    uint32_t          rdstatus;
    size_t            rdcount;
    list.addRegisterWrite(address, data);

    int status = executeList(list,
			     &rdstatus, 
			     sizeof(rdstatus),
			     &rdcount);

    if (status == -1) {
	char message[100];
	sprintf(message, "CVMUSBRemote::writeRegister USB write failed: %s",
		strerror(errno));
	throw string(message);
    }
    if (status == -2) {
	char message[100];
	sprintf(message, "CVMUSBRemote::writeRegister USB read failed %s",
		strerror(errno));
	throw string(message);

    }
    
}


/**
 *  Marshall a CVMEReadoutList into a string that is a Tcl list of integers representing the
 *  contents of the list.
 *  @param list  - The CVMEReadoutList object.
 *  @return string
 *  @retval the Tcl list constructed from the vme readout list.
 */
string
CVMUSBRemote::marshallList(CVMUSBReadoutList& list)
{
  vector<uint32_t> listVect = list.get();
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
CVMUSBRemote::marshallOutputData(void* pOutputBuffer, const char* reply, size_t maxOutputSize)
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
    throw m_lastError;		// Error in reply.
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
