/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include "CEventOrderClient.h"
#include <ErrnoException.h>
#include <CSocket.h>
#include <CTCPConnectionFailed.h>

#include <CPortManager.h>
#include <os.h>
#include <io.h>
#include <errno.h>
#include <stdio.h>

#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <fragment.h>
#include <unistd.h>
#include <fcntl.h>
#include <new>
#include <system_error>


static const std::string EventBuilderService("ORDERER"); // Advertised service name.

/**
 * Construct the object.
 * @param host - The host on which the event builder is listening for client
 *               connections.
 * @param port - The port on which the event builder is listening for client connections.
 */
CEventOrderClient::CEventOrderClient(std::string host, uint16_t port) :
  m_host(host),
  m_port(port),
  m_pConnection(0),
  m_fConnected(false),
  m_nIovecSize(0),
  m_pIovec(nullptr)
{
   m_nIovecMaxSize = sysconf(_SC_IOV_MAX);   // Maximum number of iovs for write.
   if ( m_nIovecMaxSize == -1) {
    throw std::system_error(errno, std::generic_category(), "Trying to get _SC_IOV_MAX"); 
  }
}

/**
 * Destroy the object. Any existing connection to the event builder is dropped.
 */
CEventOrderClient::~CEventOrderClient()
{
  delete m_pConnection;
  free(m_pIovec);
}

/**
 * Locate the event builder on the specified host and return
 * the port on which its server is listening for connections.
 * 
 * @param host - the host in which to perform the inquiry.
 * @param pName - If not null, points to the name of the event builder.
 * 
 * @return uint16_t
 * @retval - the port on which the event builder is listening for our username.
 *
 * @note for name-less event builders, the name is qualified by the username.
 *       for named it is qualified by the username and the evbname.
 *
 */
uint16_t
CEventOrderClient::Lookup(std::string host, const char* pName)
{
  CPortManager manager(host);
  std::vector<CPortManager::portInfo> services = manager.getPortUsage();
  std::string me  = Os::whoami();

  std::string serviceName = EventBuilderService;
  serviceName += ":";
  serviceName += me;

  if (pName) {
    serviceName += ":";
    serviceName += pName;
  }

  // Look for the first match for my username and the correct service.

  for (int i =0; i < services.size(); i++) {
    if (services[i].s_Application == serviceName &&
	services[i].s_User        == me) {
      return services[i].s_Port;
    }
  }
  // Not running.
  // Use errno = ENOENT

  errno = ECONNREFUSED;
  throw CErrnoException("Looking up event builder service");

}
/**
 * Connect to a server.
 *
 * See eventorderer(5daq) for protocol information.
 *
 * @param[in] description - the description used in the CONNECT message
 *                      to describe the client to the server.
 * @param[in] sources - list of source ids that will be generated by this
 *                      client program.  
 */
void
CEventOrderClient::Connect(std::string description, std::list<int> sources)
{
  char portNumber[32];
  uint8_t* connectionBody(0);
  sprintf(portNumber, "%u", m_port);
  m_pConnection = new CSocket();
  try {
    m_pConnection->Connect(m_host, std::string(portNumber));
    
    // Now ensure the socket does not get propagated to anything we exec
    // (or fork/exec for that matter).  This ensures that if we
    // access a remote ring, the socket won't get duplicated into a
    // stdintoring causing no harm but consternation.
    
    int fd = m_pConnection->getSocketFd();
    int fdFlags;
    fdFlags = fcntl(fd, F_GETFD, NULL);
    fdFlags |= FD_CLOEXEC;                    // Close on exec.
    fcntl(fd, F_SETFD, fdFlags);

    // Message header:
    
    EVB::ClientMessageHeader hdr = {0 , EVB::CONNECT};
    
    // Figure out the total body size, allocate it, fill in hdr.s_bodySize:
    
    uint32_t bodySize = EVB_MAX_DESCRIPTION + (sources.size() + 1)*sizeof(uint32_t);
    EVB::pConnectBody pBody = static_cast<EVB::pConnectBody>(malloc(bodySize));
    if (!pBody) {
      throw CErrnoException("Unable to allocated connect msg body");
    }
    hdr.s_bodySize = bodySize;
    memset(pBody->s_description, 0, EVB_MAX_DESCRIPTION);
    strncpy(pBody->s_description, description.c_str(), EVB_MAX_DESCRIPTION);
    pBody->s_nSids = sources.size();
    int i(0);
    for (auto p = sources.begin(); p != sources.end(); p++) {
      pBody->s_sids[i] = *p;
      i++;
    }
    // We'll need to I/O vectors, one for the header, one for the body:
    
    iovec d[2];
    d[0].iov_base = &hdr;
    d[0].iov_len  = sizeof(EVB::ClientMessageHeader);
    
    d[1].iov_base = pBody;
    d[1].iov_len  = bodySize;
    
    message(2, d);          // Send and process reply.
    free(pBody);
  }
  catch (CTCPConnectionFailed& e) {
    errno = ECONNREFUSED;
    throw CErrnoException("Failed connection to server");
  }
  catch (std::string msg) {
    delete m_pConnection;
    m_pConnection = nullptr;
    m_fConnected = false;
    throw;
  }
  m_fConnected = true;

}
/**
 * Disconnect from the server.
 * If we are not connected this should throw a CErrnoException with
 * ENOTCONN as the ReasonCode.
 */
void
CEventOrderClient::disconnect()
{
  if (!m_fConnected) {
    errno = ENOTCONN;
    throw CErrnoException("Disconnect from server");
  }
  
  try {
    // Disconnect message has no body.
    
    EVB::ClientMessageHeader hdr = {0, EVB::DISCONNECT};
    iovec d;
    d.iov_base = &hdr;
    d.iov_len  = sizeof(EVB::ClientMessageHeader);
    message(1, &d);
  }
  catch (...) {
    delete m_pConnection;
    m_pConnection = nullptr;
    m_fConnected = false;
    throw;
  }
  delete m_pConnection;
  m_pConnection = nullptr;
  m_fConnected = false;
}
/**
 * Submits a chain of fragments.  (FragmentChain).  The chain is marshalled into 
 * a body buffer, and submitted to the event builder.
 * via 'message'.
 *
 * @param pChain - Pointer to the first element of the chain.
 *
 *  @exception CErrnoException if we are not connected or some other error occurs.
 *
 */
void
CEventOrderClient::submitFragments(EVB::pFragmentChain pChain)
{
  if (m_fConnected) {
    iovec* pDescription(nullptr);
    EVB::ClientMessageHeader hdr;
    hdr.s_msgType = EVB::FRAGMENTS;
    
    size_t bodyBytes = bytesInChain(pChain);
    hdr.s_bodySize = bodyBytes;
    size_t nIovsInChain = iovecsInChain(pChain);
    pDescription = static_cast<iovec*>(malloc((nIovsInChain+1)*sizeof(iovec)));
    
    pDescription[0].iov_base = &hdr;
    pDescription[0].iov_len = sizeof(EVB::ClientMessageHeader);
    fillFragmentDescriptors(pDescription+1, pChain);
    

    try {


      // The -1 below is because we don't relay the null terminator on the strings.

      message(nIovsInChain+1, pDescription);
      free(pDescription);
    
    }
    catch (...) {
      free(pDescription);
      throw;
    }
    // get the reply and hope it's what we expect.

    
  } else {
    errno = ENOTCONN;		// Not connected.
    throw CErrnoException ("submitting fragment chain");
  }
}

/**
 * Given a pointer to an array of fragments, and the number of fragments,
 * submits them to the event builder.  This is done copy free and amortized
 * dynamic memory free by building a set of I/O vectors
 * for the writev method.  
 *
 * @param nFragments - Number of fragments in the array.
 * @param ppFragments - Pointer to the first fragment in the array.
 */
void
CEventOrderClient::submitFragments(size_t nFragments, EVB::pFragment ppFragments)
{
  // Just make a fragment chain out of the fragments and use the previous
  // method:
  if (nFragments == 0) return;
  EVB::pFragmentChain pChain = new EVB::FragmentChain[nFragments];
  for (size_t i = 0; i < nFragments; i++) {
    pChain[i].s_pFragment = &(ppFragments[i]);
    if (i != nFragments-1) {
      pChain[i].s_pNext = &(pChain[i+1]);
    } else {
      pChain[i].s_pNext = nullptr;
    }
  }
  try {
    submitFragments(pChain);
  }
  catch (...) {
    delete []pChain;
    throw;
  }
  delete []pChain;
}
/**
 * Given an STL list of pointers to events:
 * - Marshalls these into an event fragment chain
 * - submits those to the event builder.
 *
 * @param fragments - the list of fragments to send.
 *
 * @note Only the fragment nodes are created dynamically.
 *       this minimizes data movement.
 */
void
CEventOrderClient::submitFragments(EVB::FragmentPointerList& fragments)
{
  size_t nFrags = fragments.size();
  if (nFrags == 0) return; // degenerate edge case...empty list...don't send.
  
  
  EVB::pFragmentChain pChain = new EVB::FragmentChain[nFrags];
  auto p = fragments.begin();
  size_t i(0);
  while (p != fragments.end()) {
    pChain[i].s_pFragment = *p;
    if (i != nFrags-1) {
      pChain[i].s_pNext  = &(pChain[i+1]);
    } else {
      pChain[i].s_pNext = nullptr;
    }
    p++;
    i++;
  }
  try {
    submitFragments(pChain);
  }
  catch (...) {
    delete []pChain;
  }
  delete []pChain;
  
}

/*-------------------------------------------------------------------------------------*/
// Private methods 

/**
 * Return a message consisting of a request header and a body.
 * The message is dynamically allocated and must be freed by the caller as
 * delete []message.
 *
 * @param msg     - Pointer to a pointer that will hold the message.
 * @param request - Pointer to the request part of the message.
 * @param requestSize - number of bytes in the request part of the message.
 * @param body    - Pointer to the bytes of data in the body.
 * @param bodySize - number of bytes in the body.
 *
 * @return size_t
 * @retval  size of the message.
 */
size_t
CEventOrderClient::message(void** msg,
			   const void*  request, size_t requestSize, const void* body , size_t bodySize)
{
  // figure out the size of the message:

  //  std::cerr << "Message req: " << requestSize << " body " << bodySize
  //	    << std::endl;

  uint32_t rsize = requestSize;
  uint32_t bsize = bodySize;
  size_t totalSize = rsize + bsize + 2*sizeof(uint32_t);

  void* pMessage = malloc(totalSize);
  if (!pMessage) {
    throw CErrnoException("Allocating buffer");
  }
  // There must always be a request:
  char* p = reinterpret_cast<char*>(pMessage);
  memcpy(p, &rsize, sizeof(uint32_t));
  p += sizeof(uint32_t);
  memcpy(p, request, rsize);
  p += rsize;

  // If bsize == 0 or body == NULL, don't put them in the message (request only msg).

  if (body && bsize) {
    memcpy(p, &bsize, sizeof(uint32_t));
    p += sizeof(uint32_t);
    memcpy(p, body, bsize);
  }
  *msg = pMessage;
  return totalSize;


	
  
}
/**
 * Get a reply string from the server.
 * Reply strings are fully textual lines.  This just means
 * reading a character at a time until the newline.
 *
 * @return std::string.
 */
std::string
CEventOrderClient::getReplyString()
{
  std::string reply;
  while(1) {
    char c;
    m_pConnection->Read(&c, sizeof(c));
    if (c == '\n') return reply;
    reply += c;
  }
}
/**
 * Free a fragment chain (the fragments themselves are not freed by this).
 *
 * @param pChain - Pointer to the first chain element.
 */
void
CEventOrderClient::freeChain(EVB::pFragmentChain pChain)
{
  while (pChain) {
    EVB::pFragmentChain pNext = pChain->s_pNext;
    delete pChain;
    pChain = pNext;
  }
}
/**
 * makeIoVec
 *   Givena fragment pointer, create the two I/O vectors elements for it,
 *   First for the fragment header, second for the fragment payload.
 *
 * @param Frag - refernce to the fragment.
 * @param pVecs - Pointer to at least two I/O vector structs.
 * @return iovec* - pointer just past the set of I/O vectors filled in by this
 *                  method.
 */
iovec*
CEventOrderClient::makeIoVec(EVB::Fragment& Frag, iovec* pVecs)
{
  pVecs->iov_base = &Frag;
  pVecs->iov_len = sizeof(EVB::FragmentHeader);
  pVecs++;
  pVecs->iov_base = Frag.s_pBody;
  pVecs->iov_len  = Frag.s_header.s_size;
  
  return ++pVecs;
}
/**
 * message
 *    Sends a message and gets a reply.  If the reply is not
 *    the OK string we expect, then we throw it as an std::string exception.
 *
 *  @param nItems  - Number of iovec structs used to describe the message.
 *  @param parts   - Pointer to the iovecs.
 */
void
CEventOrderClient::message(size_t nItems, iovec* parts)
{
  int fd = m_pConnection->getSocketFd();
  io::writeDataVUnlimited(fd, parts, nItems);
  std::string reply = getReplyString();
  if (reply != "OK") {
    throw reply;
  }
}
/**
 * bytesInChain
 *   @param pFrags   - a fragment chain.
 *   @return size_t  - number of bytes needed to write the chain.
 */
size_t
CEventOrderClient::bytesInChain(EVB::pFragmentChain pFrags)
{
  EVB::pFragmentChain p = pFrags;
  size_t result(0);
  while (p) {
    result += sizeof(EVB::FragmentHeader) + p->s_pFragment->s_header.s_size;
    p      = p->s_pNext;
  }
  
  return result;
}
/**
 * iovecsInChain
 *   @param pFrags - fragment chain to size.
 *   @return size_t  - number of iovects needed to describe the whole chain.
 */
size_t
CEventOrderClient::iovecsInChain(EVB::pFragmentChain pFrags)
{
  size_t result(0);
  
   while (pFrags) {
    result += 2;                    // Each chain entry needs 2 iovecs.
    pFrags      = pFrags->s_pNext;
  }
  
  return result;
}
/**
 * fillFragmentDescriptors
 *    Fills in the fragment descriptors for a chain.
 * @param pVec - the vectors to fill - caller must have ensured it's big enough.
 * @param pFrags - Fragment chain to describe.
 */
void
CEventOrderClient::fillFragmentDescriptors(iovec* pVec, EVB::pFragmentChain pFrags)
{
    while (pFrags) {
      pVec = makeIoVec(*pFrags->s_pFragment, pVec);
      pFrags = pFrags->s_pNext;
    }
}