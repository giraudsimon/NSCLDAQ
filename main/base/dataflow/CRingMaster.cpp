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

#include <errno.h>
#include <config.h>
#include "CRingMaster.h"

#include "CRingBuffer.h"
#include <CPortManager.h>
#include <ErrnoException.h>

#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>


#include <iostream>

using namespace std;

static const string localhost("127.0.0.1");
static const string ringMasterAppName("RingMaster");

/////////////////////////////////////////////////////////////////////////////////////////
//  Constructors and implemented canonicals:


/*!
   Construct the object.  This involves connecting to the specified ring buffer.
   This in turn, requires using the port manager to determine which port the RingMaster
   is listening on.
  
   \param host  - The name of the host whose ring master we'll connect to.  Note that
                  remote hosts, for the purpose of this class, anything but localhost
                  or 127.0.0.1 have only limited functionality with the RingMaster
                  (for example you can't register or unregister rings in remote systems).
   
*/
CRingMaster::CRingMaster(string host) :
  m_socket(-1),
  m_isConnected(false),
  m_isDataConnection(false),
  m_isLocal(false)
{
  std::string ip=ipAddress(host);
  if (ip == localhost) m_isLocal = true;

  int ringMasterPort = getPort(host);

  // Now connect to the ring master:

  m_socket = socket(PF_INET, SOCK_STREAM, 0);
  if (m_socket == -1) {
    throw CErrnoException("CRingMaster::CRingMaster - could not create socket");
  }

  // Connect to the ring master:


  sockaddr_in peer;
  peer.sin_port   = htons(ringMasterPort);
  peer.sin_family = AF_INET;
  inet_aton(ip.c_str(), &(peer.sin_addr));

  try {
    if (connect(m_socket, (sockaddr*)&peer, sizeof(peer)) == -1) {
      throw CErrnoException("CRingMaster::CRingMaster - connecting");
    }
  }
  catch (...) {
    close(m_socket);
    throw;
  }
   

  m_isConnected  = true;
  
  // We don't want ringmaster connections to be inherited across execs..
  // bad enough they may be inherited across forks:

  long flags = fcntl(m_socket, F_GETFD, &flags);
  flags |= FD_CLOEXEC;
  fcntl(m_socket, F_SETFD, flags);

}

/*!
  The destructor closes the socket.
*/
CRingMaster::~CRingMaster() 
{
  if (m_isConnected && !m_isDataConnection) {
    shutdown(m_socket, SHUT_RDWR);
    close(m_socket);
  }
}

/*!

   Notify the RingMaster of a consumer  connection to a ring.  This must be a
   local connection on which transactions can be done.

   \param ringname - name of the ring being attached to.
   \param id       - id of the consumer.
   \param comment  - Comment that describes the connection.
*/
void
CRingMaster::notifyConsumerConnection(string ringname, int id, string comment)
{
  transactionOk();
  if (!m_isLocal) {
    errno = ENOTSUP;
    throw CErrnoException("CRingMaster::notifyConsumerConnection non local");
  }
  char type[64];
  sprintf(type, "consumer.%d", id);


  string message = formatConnection(ringname, type, comment);
  simpleTransaction(message);

}

/*!
  Notify the ring master of a producer connection to a ring.
  this must be a local connection on which transactions can be done:
  \param ringname - Name of the ring.
  \param comment  - Comment logged in the connection.

*/
void
CRingMaster::notifyProducerConnection(string ringname, string comment)
{
  transactionOk();
  if (!m_isLocal) {
    errno = ENOTSUP;
    throw CErrnoException("CRingMaster::notifyProducerConnection non local");
  }

 
  string message = formatConnection(ringname, "producer", comment);
  simpleTransaction(message);
}

/*!
  Notify the ring master of a disconnection from a producer ring.
  The object must be connected and transactions allowed.

  \param  name   - Name of the ring.
*/
void
CRingMaster::notifyProducerDisconnection(string name)
{
  transactionOk();
  if (!m_isLocal) {
    errno = ENOTSUP;
    throw CErrnoException("CRingMaster notify producer disconnection non local");
  }

  string message = formatDisconnection(name, "producer");
  simpleTransaction(message);
}


/*!
  Notify the ring master of a consumer disconnection.
  \param name - Name of the ring.
  \param slot - Id of the slot on which we are connected.
*/
void
CRingMaster::notifyConsumerDisconnection(string name, int slot)
{
  transactionOk();
  if (!m_isLocal) {
    errno = ENOTSUP;
    throw CErrnoException("CRingMaster notify consumer disconnection nonlocal");
  }
  char type[64];
  sprintf(type, "consumer.%d", slot);

  string message = formatDisconnection(name, type);
  simpleTransaction(message);
}

/*!
  Notify the ring master a new ring has been created.

  \param ringname name of the new ring.
*/
void
CRingMaster::notifyCreate(string ringname)
{
  transactionOk();
  if (!m_isLocal) {
    errno = ENOTSUP;
    throw CErrnoException("CRingMaster register new ring nonlocal");
  }


  string message;
  message += "REGISTER ";
  message += ringname;
  message += "\n";
  simpleTransaction(message);
}
/*!
  Notify the ring master of ring destrution (unregistry).
  \param ringname  - ring being destroyed.

*/
void
CRingMaster::notifyDestroy(string ringname)
{
  transactionOk();
  if (!m_isLocal) {
    errno = ENOTSUP;
    throw CErrnoException("CRingMaster notify ring remove nonlocal");
  }
  
  string message;
  message += "UNREGISTER ";
  message += ringname;
  message += "\n";
  simpleTransaction(message);

}

/*!
   Request data from the ring master.  This asks the ring master to spin off a TCP hoiste
   that will send binary data from the ring down the socket to us.  After getting an
   OK BINARY FOLLOWS indication from the ring master, the socket is no longer any good
   for transaction.

   \param ringname - name of the ring from which we want data.

   \return int
   \retval the socket on which data will be recieved.

   \note errors are reported with exceptions after which the object is still available
   for transactions. Only on success is the object marked not for transactions.
   Don't destroy the object as that will close/shutdown the socket.

*/

int
CRingMaster::requestData(string ringname)
{


  transactionOk();		// need not be and usually is not localhost.

  string message;
  message += "REMOTE ";
  message += ringname;
  message += "\n";
  sendLine(message);
  string reply  = getLine();

  if (reply == "OK BINARY FOLLOWS\r\n") {
    m_isDataConnection = true;
    return m_socket;
  } 
  else {
    string exception;
    exception += "On request data transaction, expected reply OK got : ";
    exception += reply;
    throw exception;
  }
 
}
/**
 * requestUsage
 *    Return the usage string.  This is the output of the LIST command to the
 *    RingMaster.
 *
 * @return std::string - A Tcl list which contains the usage information string
 *                       from the RingMaster process.   This is a single line containing
 *                       A list of lists.  The first element of each list is the name of a
 *                       ringbufferwhiel the second element is a list containing:
 *                       *  Size of the ring in bytes.
 *                       *  Free space of the ring in bytes.
 *                       *  Maximum number of consumers.
 *                       *  PID of the producer or -1 if there is no current producer.
 *                       *  Maximum number of bytes in a consumer 'queue'.
 *                       *  Minimum number of bytes in a consumer 'queue'.
 *                       *  List of client information.  For each client this is a
 *                          sublist of the client PID, the maximum number of bytes it could
 *                          'get' without blocking.
 * @throws std::string - errorm essage on problems.
 */
std::string
CRingMaster::requestUsage()
{
    transactionOk();                         // Throw if we can't do a transaction.
    std::string message = "LIST\n";          // Send the request.
    sendLine(message);
    
    std::string ok = getLine();             // SB OK\r\n
    if(ok != "OK\r\n") {
        std::string msg = "LIST command did not return OK but: ";
        msg += ok;
        throw msg;
    }
    std::string reply = getLine();                   // Usage list.
    return reply;
}
/****************************************************************************************/
/* Locate the ring Master's port.                                                       */
/****************************************************************************************/

int 
CRingMaster::getPort(string host)
{
  int          port(-1);
  CPortManager manager(host);
  vector<CPortManager::portInfo> servers = manager.getPortUsage();
  for (int i=0; i < servers.size(); i++) {
    if(servers[i].s_Application == ringMasterAppName) {
      port = servers[i].s_Port;
      break;
    }
  }
  if (port == -1) {
    errno = ECONNREFUSED;
    throw CErrnoException("CRingMaster::getPort - no ringmaster port");
  }

  return port;
}
/**************************************************************************************/
/*     Test if it's ok to do a transaction and throw an exception if not.             */
/*     the exception thrown will be an EACCES errno exception                         */
/**************************************************************************************/
void
CRingMaster::transactionOk()
{
  if ((!m_isConnected) || m_isDataConnection) {
    errno = EACCES;
    throw CErrnoException("CRingMaster - cannot do transactions on data or disconnected objects");
  }
}
/**************************************************************************************/
/*  Send a line to the RingMaster.  This requires that transactions be ok, since      */
/*  1. We clearly can't send if we've lost the connection                             */
/*  2. If we've become a data connection, we're technically not talking to the        */
/*     ringmaster anymore.                                                            */
/**************************************************************************************/

void
CRingMaster::sendLine(std::string line)
{
  transactionOk();

  const char* pBuf       = line.c_str();
  size_t      remaining  = line.size();

  while (remaining > 0) {
    ssize_t sent = send(m_socket, pBuf, remaining, MSG_NOSIGNAL);
    if (sent >= 0) {
      remaining -= sent;
      pBuf      += sent;
    }
    else {
      throwIfBadErrno();
    }
  }
}

/**********************************************************************************/
/*  Get a line of text from a socket. Transactions must be allowed as lines       */
/*  represent replies from the ring master sever, not raw data                    */
/**********************************************************************************/

string
CRingMaster::getLine()
{
  transactionOk();

  string result;
  char   data;

  // Unfortunately, unless I want to do some buffering, the only way I know
  // to do this is a character at a time.  Since performance is not critical,
  // this is what we'll do.
  //

  while (1) {
    ssize_t status = recv(m_socket, &data, 1, MSG_WAITALL);
    if (status == 1) {
      result += data;
      if (data == '\n') return result;
    }
    else if (status < 0) {
      throwIfBadErrno();
    }
  }
}

/**********************************************************************************/
/* Performa a transaction with the server for which we expect as a result simply  */
/* OK\r\n                                                                         */
/**********************************************************************************/

void
CRingMaster::simpleTransaction(string line)
{
  sendLine(line);
  string reply = getLine();

  if (reply != string("OK\r\n")) {
    string exception;
    exception += "Invalid response: ";
    exception += reply;
    exception += " when expecting 'OK'";
    throw exception;
  }
}

/**********************************************************************************/
/* Returns the dotted ip address string that coresponds to a host.                */
/* It is acceptable for the input adress to already be in that form.              */
/* If the host cannot be transformed to an input address,  the appropriate        */
/* errno exception is thrown.                                                     */
/**********************************************************************************/

string
CRingMaster::ipAddress(string host)
{
  struct hostent* pEntry;
  struct hostent  entry;
  char            buffer[1024];
  int             e;
  if (!gethostbyname_r(host.c_str(), 
		       &entry, buffer, sizeof(buffer),
		       &pEntry, &e)) {
    // pEntry->h_addr_list[0] has a longword IP address in network byte order:
    //
 
    uint32_t ipaddr = ntohl(*((long*)(pEntry->h_addr_list[0])));
    
    char encodedIp[32];
    sprintf(encodedIp, "%d.%d.%d.%d",
	    (ipaddr >> 24) & 0xff,
	    (ipaddr >> 16) & 0xff,
	    (ipaddr >> 8)  & 0xff,
	    ipaddr         & 0xff);

    return string(encodedIp);
  }

  // Doesn't translate, see if it's a valid dotted address, in which case
  // we can just return it.

  in_addr encoded;
  if (inet_aton(host.c_str(), &encoded)) {
    return host;
  }
  errno = EHOSTUNREACH;
  throw CErrnoException("CRingMaster:ipAddress - no such host");
      
}

/**********************************************************************************/
/* Return true if the errno from a send or recv is likely means the socket        */
/* can't ever be sent to again.                                                   */
/**********************************************************************************/


bool
CRingMaster::badErrno()
{
  return !((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR));
}
/**
 * If badErrno is true, close the socket and throw an errno
 * exception.
 */
void
CRingMaster::throwIfBadErrno()
{
  if (badErrno()) {
    m_isConnected = false;	// Lost connection
    close(m_socket);
    throw CErrnoException("CRingMaster sending a message to the server");
  }
}

/**********************************************************************************/
/*  Format a connection notification string for any type of connection            */
/**********************************************************************************/
string
CRingMaster::formatConnection(string ringname, const char* type, string comment)
{
  pid_t pid = getpid();

  size_t messageSize = ringname.size() + comment.size() + strlen(type) + 100; // Conservatively good.
  char* line         = new char[messageSize];
  sprintf(line, "CONNECT {%s} %s %d {%s}\n",
	  ringname.c_str(), type, pid, comment.c_str());
  string message(line);
  delete []line;
  return message;
}
/*******************************************************************************/
/*  Format a disconnection notification string for any type of connection      */
/*  It is up to the calller to format the slot number in to the consumer       */
/*  type                                                                       */
/*******************************************************************************/

string
CRingMaster::formatDisconnection(string ringname, const char* type)
{
  pid_t  pid         = getpid();
  size_t messageSize = ringname.size() + strlen(type) + 100;
  char  *line        = new char[messageSize];
  sprintf(line, "DISCONNECT {%s} %s %d\n",
	  ringname.c_str(),
	  type,
	  pid);
  string message(line);
  delete [] line;
  return message;
}
