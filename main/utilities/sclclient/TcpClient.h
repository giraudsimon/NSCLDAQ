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

 
// Class: TcpClientConnection                     //ANSI C++
//
// Represents a client connection to a Tcp/Ip socket.  These connections
// are characterized by a remote host and connection port.  The
// operating system construct which represents these items is a
// socket file descriptor.
//   Object oriented callback styles are supported via client object
// static function call relays and the Connection and Disconnect
// callbacks.  Key data type is:
//   typedef void TCPClientCallbackProc(TcpClientConnection&, void* );
//
// Author:
//     Ron Fox
//     NSCL
//     Michigan State University
//     East Lansing, MI 48824-1321
//     mailto: fox@nscl.msu.edu
// 
// (c) Copyright NSCL 1999, All rights reserved TcpClient.h
//

#ifndef TCPCLIENT_H  //Required for current class
#define TCPCLIENT_H


#include <libtclplusdatatypes.h>

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#ifndef SHUT_RDWR
#define SHUT_RDWR 3
#endif
#include <unistd.h>


class TcpClientConnection;

typedef void TCPClientCbProc(TcpClientConnection&, void*);

enum TCPConnectionState {
    Connected,
    Disconnected
};

class TcpClientConnection      
{                       
			
   TCPClientCbProc* m_ConnectionCallback; //Pointer to connection callback
				//  This member is NULL if there is
				// no callback.
   void* m_ConnectionCbData;	//Connection callback data.
   TCPClientCbProc* m_DisconnectCallback; //Pointer to callback function 
				// called if the connection to
				// the peer is lost.  If Null, there is 
				// no callback.
   void* m_DisconnectCbData;	//Disconnect callback client data.
   std::string m_sRemoteHost;	//Textual address of host with which we 
				// are connecting.
   unsigned int m_nRemotePort; //Remote port within the remote host at 
				// which the server is listening.
   int m_nFd;			//File id open on the socket.
   TCPConnectionState m_eConnectionState; //Current state of the connection 
				// can be either:
				// Connected or Disconnected

  // This is an enum rather than a bool to allow for future
  // expansion to a richer state machine should need arise.        

protected:

public:

   // Constructors and other cannonical operations:

  TcpClientConnection ()    : 
    m_ConnectionCallback(0),  
    m_ConnectionCbData(0),  
    m_DisconnectCallback(0),   
    m_DisconnectCbData(0),   
    m_sRemoteHost(std::string("localhost")),   
    m_nRemotePort(1024),   
    m_nFd(0),   
    m_eConnectionState(Disconnected) 
  { 
    
  } 
  TcpClientConnection(const std::string& rHost, int nPort=1024) : 
    m_ConnectionCallback(0),
    m_ConnectionCbData(0),
    m_DisconnectCallback(0),
    m_DisconnectCbData(0),
    m_sRemoteHost(rHost),
    m_nRemotePort(nPort),
    m_nFd(0),
    m_eConnectionState(Disconnected)
  {
  }
   virtual ~ TcpClientConnection ( )  // Destructor 
     {
       if(m_eConnectionState == Connected) {
	 Disconnect();
       }
     }  
   //Copy constructor and assignment are illegal since while the fd can
  // be duped, the connection state cannot be maintained consistently.
  // comparison is also disallowed because it doesn't make sense.
private:
  TcpClientConnection (const TcpClientConnection& aTcpClientConnection ) ;
  TcpClientConnection& 
  operator= (const TcpClientConnection& aTcpClientConnection);
  int operator== (const TcpClientConnection& aTcpClientConnection) const;
public:	
// Selectors:

public:

  TCPClientCbProc* getConnectionCallback() const
  { return m_ConnectionCallback;
  }
  void* getConnectionCbData() const
  { return m_ConnectionCbData;
  }
  TCPClientCbProc* getDisconnectCallback() const
  { return m_DisconnectCallback;
  }
  void* getDisconnecctCbData() const
  { return m_DisconnectCbData;
  }
  std::string getRemoteHost() const
  { return m_sRemoteHost;
  }
  unsigned int getRemotePort() const
  { return m_nRemotePort;
  }
  int getFd() const
  { return m_nFd;
  }
  TCPConnectionState getConnectionState() const
  { return m_eConnectionState;
  }
                       
// Mutators:

protected:

  void setConnectionCallback (TCPClientCbProc* am_ConnectionCallback)
  { m_ConnectionCallback = am_ConnectionCallback;
  }
  void setConnectionCbData (void* am_ConnectionCbData)
  { m_ConnectionCbData = am_ConnectionCbData;
  }
  void setDisconnectCallback (TCPClientCbProc* am_DisconnectCallback)
  { m_DisconnectCallback = am_DisconnectCallback;
  }
  void setRemoteHost (const std::string am_sRemoteHost)
  { m_sRemoteHost = am_sRemoteHost;
  }
  void setRemotePort (const unsigned int am_nRemotePort)
  { m_nRemotePort = am_nRemotePort;
  }
  void setFd (const int am_nFd)
  { m_nFd = am_nFd;
  }
  void setConnectionState (const TCPConnectionState am_eConnectionState)
  { m_eConnectionState = am_eConnectionState;
  }
       
public:

  TCLPLUS::Bool_t Connect ()    ;
  TCLPLUS::Bool_t Connect (const std::string& sRemoteHost, unsigned port) {
    m_sRemoteHost = sRemoteHost;
    m_nRemotePort = port;
    return Connect();
  }
   void Disconnect ()    ;
   int Send (void* pData, int nBytes)    ;
   int Receive (void* pData, int nBufferSize)    ;
   int GetLine (std::string& rBuffer)    ;
   int Poll ()    ;
   void Flush();		// Flush all pending input.
   TCPConnectionState GetConnectionState ()    const;
   TCPClientCbProc* 
   SetConnectCallback (TCPClientCbProc* pProc, void* pClientData = 0)    ;
   TCPClientCbProc* GetConnectCallback ()    const;
   TCPClientCbProc* 
   SetDisconnectCallback (TCPClientCbProc* pProc, void* pClientData = 0)    ;
   TCPClientCbProc* GetDisconnectCallback ()    const;
 
protected:

private:
  void DoDisconnect() {
    shutdown(m_nFd, SHUT_RDWR);
    close(m_nFd);
    m_eConnectionState = Disconnected;
  }
  sockaddr_in MakeSocketAddress();
  static void AppendString(std::string& rDest, char* pStart, char* pEnd);
  void DisconnectSensed();
};


#endif
