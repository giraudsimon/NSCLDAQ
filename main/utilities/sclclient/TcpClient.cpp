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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";// Class: TcpClientConnection
// Represents a client connection to a Tcp/Ip socket.  These connections
// are characterized by a remote host and connection port.  The
// operating system construct which represents these items is a
// socket file descriptor.
//  Object oriented callback styles are supported via client object
// static function call relays and the Connection and Disconnect
// callbacks.  Key data type is:
//   typedef void TCPClientCallbackProc(TcpClientConnection&, void* );
//
// Author:
//    Ron Fox
//    NSCL
//    Michigan State University
//    East Lansing, MI 48824-1321
//
//
//////////////////////////.cpp file/////////////////////////////////////////////////////
#include <config.h>
#include "TcpClient.h"    				

#include <sys/poll.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>


using namespace std;


static const char* pCopyrightNotice = 
"(C) Copyright 1999 NSCL, All rights reserved TcpClient.cpp \n";

// Functions for class TcpClientConnection

//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Connect()
//  Operation Type: 
//     
TCLPLUS::Bool_t TcpClientConnection::Connect()  
{
  // Attempts to form a connection to the current host and port.
  //  If successful, the connection callback is called prior
  // to returning.
  if(m_eConnectionState == Connected) {// Error if already connected.
    return TCLPLUS::kfFALSE;
  }
  m_nFd = socket(AF_INET, SOCK_STREAM, 0);
  if(m_nFd < 0) {
    return TCLPLUS::kfFALSE;
  }
  try {
    sockaddr_in peer = MakeSocketAddress();
    if(connect(m_nFd, (sockaddr*)&peer, sizeof(peer))) {
      throw string(strerror(errno));
    }
  }
  catch(string& reason) {	// Error with the fd open.
    DoDisconnect();
    return TCLPLUS::kfFALSE;
  }
  // Success

  m_eConnectionState = Connected;
  if(m_ConnectionCallback) {
    m_ConnectionCallback(*this, m_ConnectionCbData);
  }
  return TCLPLUS::kfTRUE;

}

//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Disconnect()
//  Operation Type: 
//     
void TcpClientConnection::Disconnect()  
{
  if(m_eConnectionState != Connected) return;
  DoDisconnect();
  if(m_DisconnectCallback) {
    m_DisconnectCallback(*this, m_DisconnectCbData);
  }

  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Send(void* pData, int nBytes)
//  Operation Type: 
//     
int TcpClientConnection::Send(void* pData, int nBytes)  
{
  // Attempts to send  a block of data to the connection.
  //
  // Returns:
  //   nbytes if successful.
  //   0         if connection is not open.
  //   -1        for other errors.
  //   > 0       Number of bytes transmitted.
  ///

  if(m_eConnectionState != Connected) return 0;
  int nbytes =  send(m_nFd, pData, nBytes, 0);
  if(nbytes <= 0) DisconnectSensed();
  return nbytes;
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Receive(void* pData, int nBufferSize)
//  Operation Type: 
//     
int TcpClientConnection::Receive(void* pData, int nBufferSize)  
{
  // Recieves a buffer of data from the socket.
  // Returns:
  //    nbytes - number of bytes read.
  //    0         - End of file.
  //   -1         - Problem, see errno.
  //   >0         - Actual number of bytes received.
  //
  if(m_eConnectionState != Connected) return 0;
  int nbytes =  recv(m_nFd, pData, nBufferSize, 0);
  if(nbytes <= 0) {
    DisconnectSensed();
  }
  return nbytes;

}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     GetLine(string& rBuffer)
//  Operation Type: 
//     
int TcpClientConnection::GetLine(string& rBuffer)  
{
  // Reads a line of data from the socket.  A line of data
  // is text terminated by a \n.
  // The \n is not returned.
  //  The text is *appended* to the rBuffer parameter
  // Returns:
  //    Number of characters read.
  //    0  - End of file encountered at first read. -- or line consisting only of \n.
  //   -1  - errno based problem.
  //   >0  - Number of bytes read.

  if(m_eConnectionState != Connected) return 0;

  // Peek is used to look at the data to determine how many bytes to actually
  // read.

  TCLPLUS::Bool_t fDone = TCLPLUS::kfFALSE;
  int    nBytes= 0;
  char   Buffer[1000];		// Internal buffer for peekahead.
  TCLPLUS::Bool_t fGotBytes = TCLPLUS::kfFALSE;	// To deal with empty lines.
  while(!fDone) {
    int nAvail = recv(m_nFd, Buffer, sizeof(Buffer-1), MSG_PEEK);
    int nBytesAdded = 0;
    if(nAvail >0) fGotBytes = TCLPLUS::kfTRUE;
    if(nAvail ==0) return nBytes; // End of file (socket closed).
    if(nAvail < 0) return nAvail; // Error.
    Buffer[nAvail] = 0;		// Ensure null termination.
    char* pEol = index(Buffer, '\n');
    if(pEol && (pEol != Buffer)) {
      pEol--;			// Don't include the eol character.
      AppendString(rBuffer, Buffer, pEol);
      nBytesAdded = ((unsigned long)pEol - (unsigned long)Buffer);
      fDone = TCLPLUS::kfTRUE;		// Done at end of line.
    }
    else if (!pEol) {
      nBytesAdded  = nAvail;
      rBuffer += Buffer;
    }
    else {			// Bare end of line:
      fDone = TCLPLUS::kfTRUE;
    }
    nBytes += nBytesAdded;		// Update the number of bytes read.
    recv(m_nFd, Buffer, nAvail, 0); // Flush them from the input.
  }
  if((nBytes <= 0) && (!fGotBytes)) DisconnectSensed();
  if((nBytes == 0) && fGotBytes) { // Empty lines map to a space line.
    nBytes++;
    rBuffer += ' ';
  }
  return nBytes;
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Poll()
//  Operation Type: 
//     
int TcpClientConnection::Poll()  
{
  // Determines whether or not the connection has data:
  //
  // Returns:
  //    1   - Data can be read from the socket.
  //    0   - No data.
  //   -1   - errno based error condition.
  //   -2   - Socket closed.
  //

  struct pollfd  myfd;
  myfd.fd = m_nFd;
  myfd.events = POLLHUP | POLLIN;
  myfd.revents = 0;
  int result = poll(&myfd, 1, 0);
  if(result < 0)  return -1;
  if(result == 0) return 0;
  if(myfd.revents & POLLIN) return 1; // Input data preferred over hangup
  if(myfd.revents & POLLHUP) return -2;

  // BUGBUGBUG  - what to do if something happened but no bits?

  return 0;

}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     GetConnectionState()
//  Operation Type: 
//     
TCPConnectionState 
TcpClientConnection::GetConnectionState()   const
{
  // Returns the connection status of the socket.  

  return m_eConnectionState;

}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     SetConnectCallback(ClientCbProc* pProc, void* pClientData)
//  Operation Type: 
//     
TCPClientCbProc* 
TcpClientConnection::SetConnectCallback(TCPClientCbProc* pProc, 
					void* pClientData)  
{
  // Establishes a connection callback procedure.  If the callback is
  // null there will be no callback.
  //
  //  The connection callback is called when the client becomes 
  //  connected to a server.
  //
  // Returns:
  //    Pointer to any prior procedure or null if there was no prior procedure.
  
  TCPClientCbProc* pPrior=  GetConnectCallback();
  m_ConnectionCallback = pProc;
  m_ConnectionCbData   = pClientData;
  return pPrior;
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     GetConnectCallback()
//  Operation Type: 
//     
TCPClientCbProc* TcpClientConnection::GetConnectCallback()   const
{
  // Returns pointer to the current callback proc.
  // Null if not defined.

  return m_ConnectionCallback;
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     SetDisconnectCallback(TCPClientCbProc* pProc, void* pClientData)
//  Operation Type: 
//     
TCPClientCbProc* 
TcpClientConnection::SetDisconnectCallback(TCPClientCbProc* pProc, 
					   void* pClientData)  
{
  // Sets a new Disconnection callback.  The function is called
  // when a disconnect is detected, or forced.
  // 
  // Returns:
  //  Prior callback or null if not defined.
  
  TCPClientCbProc* pPrior = GetDisconnectCallback();
  m_DisconnectCallback = pProc;
  m_DisconnectCbData   = pClientData;
  return pPrior;
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     GetDisconnectCallback()
//  Operation Type: 
//     
TCPClientCbProc* TcpClientConnection::GetDisconnectCallback()   const
{
  // Returns the current value of the callback proc.
  
  return m_DisconnectCallback;
}
/////////////////////////////////////////////////////////////////////////////
//
// Function:
//   sockaddr_in MakeSocketAddress()
// Operation type:
//   Private utility>
//
sockaddr_in
TcpClientConnection::MakeSocketAddress()
{
  sockaddr_in result;

  // Takes the current hostname and port and creates a sockaddr_in struct
  // which represents them (suitable for connect(2)).  If there are errors,
  // they are thrown as string exceptions.

  // First try the address as a dotted number:
 
  if((result.sin_addr.s_addr = inet_addr(m_sRemoteHost.c_str())) == 
      0xffffffff) {
    
    // If that doesn't work use DNS to resolve as a hostname, or throw 
    // no such host exception.



    struct hostent* pHostEntry;
    struct hostent  entry;
    char            data[1024];
    int             error;
    if (gethostbyname_r(m_sRemoteHost.c_str(),
			&entry, data, sizeof(data),
			&pHostEntry, &error)) {
      throw (string("No such host: ") + m_sRemoteHost) ;
    }
    memcpy(&result.sin_addr.s_addr, entry.h_addr, sizeof (in_addr));
  }

  result.sin_port = htons(m_nRemotePort);
  result.sin_family = AF_INET;
 
  return result;
  
}
/////////////////////////////////////////////////////////////////////////////
//
// Function:
//   void AppendString(string& rDest, char* pStart, char* pEnd)
// Operation Type:
//    Private Utility
//
void
TcpClientConnection::AppendString(string& rDest, char* pStart, char* pEnd)
{
  while(pStart < pEnd) rDest += *pStart++;
}

////////////////////////////////////////////////////////////////////////////
//
// Function:
//    void DisconnectSensed()
// Operation Type:
//    Private utility.
//
void
TcpClientConnection::DisconnectSensed()
{
  DoDisconnect();
  if(m_DisconnectCallback) {
    m_DisconnectCallback(*this, m_DisconnectCbData);
  }
}
////////////////////////////////////////////////////////////////////////
//
// Function:
//   void Flush()
// Operation Type:
//   I/O
void
TcpClientConnection::Flush()
{
  // Remove pending input from the socket.
  //
  while(Poll() == 1) {
    char data;
    Receive(&data, 1);
  }
}
