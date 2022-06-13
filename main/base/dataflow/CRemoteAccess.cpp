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

#include <config.h>
#include "CRemoteAccess.h"


#include "CRingBuffer.h"
#include "CRingMaster.h"
#include "ringbufint.h"

#include <URL.h>
#include <os.h>
#include <stdio.h>
#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


using namespace std;

///////////////////////////////////////////////////////////////////////////////////////
// Static member variables:

#ifndef K
#define K 1024
#endif

/*!
   This static member determines the size of the data area of a proxy ring buffer.
   The CRingAccess:set/getProxyRingSize members can modify/read this value.
*/
size_t CRingAccess::m_proxyRingSize(DEFAULT_DATASIZE);

/*!
   This static member determines the number of consumers that will be allowed to 
   attach to a proxy ring.   This can be modified/read by 
   set/getProxyMaxConsumers
*/
size_t CRingAccess::m_proxyMaxConsumers(DEFAULT_MAX_CONSUMERS);

/*!
   This value determines the chunk size for the reads stdintoring will use
   when getting remote data to stuff in the ring.  This can be modified/read via
   set/getMinData

*/
size_t CRingAccess::m_minData(32*K);

/*!
   This value determines the timeout after which stdintoring will just transfer the
data that can be gotten from the remote host, regardless of size.
*/
unsigned CRingAccess::m_Timeout(2);

///////////////////////////////////////////////////////////////////////////////////////
//  Member functions that manipulate the defaults.

/*!

  Set a new proxy ring size.  This only affects proxy rings that are created
in the future by this application.  This can be a bit tricky as 
- The proxy ring you are about to connect to may have already been brought into existence
  prior to this call by another application.
- This setting is application global so if your application sets this but another application
  brings the proxy ring into being, your setting is ignored.

  \parameter newSize - new size to use in future proxy ring creations.
  \return size_t
  \retval Prior size of this parameter.
*/
size_t
CRingAccess::setProxyRingSize(size_t newSize)
{
  size_t oldSize  = m_proxyRingSize;

  m_proxyRingSize = newSize;

  return oldSize;
}
/*!
   Return the current proxy ring data size.
   \return size_t
   \retval data size that will be given to all proxy rings that
           are created by this application until the next invocationof 
	   setProxyRingSize.

*/
size_t
CRingAccess::getProxyRingSize()
{
  return m_proxyRingSize;
}

/*!
   Set a new value for the maximum number of consumers that will be allowed
   to connect to new proxy rings.  See the comments  for setProxyRingSize, as the same caveats 
   apply.

   \param newMax  - new maximum consumer count.
   \return size_t
   \retval Old maximum consumer count.
*/
size_t
CRingAccess::setProxyMaxConsumers(size_t newMax)
{
  size_t oldMax = m_proxyMaxConsumers;

  m_proxyMaxConsumers = newMax;
  return oldMax;
}
/*!
   Return the current value for the limit on consumer connections allowed for 
   subsequent proxy ring buffer creations.
   \return size_t
   \retval Value of the parameter.
*/
size_t
CRingAccess::getProxyMaxConsumers()
{
  return m_proxyMaxConsumers;
}

/*!
   Set the chunk size used by stdintoring when hoisting data into proxy ring.
   \param minData - New value for the chunk size.
   \return size_t
   \retval Prior chunk size.
*/
size_t
CRingAccess::setMinData(size_t minData)
{
  size_t oldMinData = m_minData;
  m_minData         = minData;
  return oldMinData;

}

/*!
   Get the proxy's min data parameter.
   \return size_t
   \retval the parameter's value.
*/
size_t
CRingAccess::getMinData()
{
  return m_minData;
}
/*!
  Set the timeout on reads from stdin that stdintoring will use.
  \param newTimeout - the new timeout in seconds.
  \return unsigned 
  \retval prior value of the parameter.
*/
unsigned
CRingAccess::setTimeout(unsigned newTimeout)
{
  unsigned oldTimeout = m_Timeout;
  m_Timeout           = newTimeout;
  return oldTimeout;
}
/*!
   Return the timeout value
   \return unsigned
   \retval current valueo f the timeout parameter.
*/
unsigned
CRingAccess::getTimeout()
{
  return m_Timeout;
}
/*!
  This is the flagship entry of the class.  Connects to a ring buffer either local or remote,
  the ring buffer is designated by a URI of the form:

\verbatim
tcp://somehost/ringname
\endverbatim

  Where:
  - somehost is a host on which a ringmaster process is running and
  - ringname is the name of a ring buffer in that host.

  The function first checks to see if a local ring named somehost.ringname exists.
  if so, a connection if formed to that and the problem is solved.    If not,
  the function will contact the ringmaster server on somehost.  The ringmaster will
  be sent a REMOTE request we will creae the proxy ring here as described above, and
  create a deteched stdintoring process that will catch the remote data and feed it into the
  proxy ring from the socket the ringmaster (well really a ringtostdout instance) will use 
  to send data to this host.
  
  \param uri - The Universal Resource Identifier that identifies the ring from which
               we would like to take data.

  \return CRingBuffer*
  \retval Pointer to the ring buffer (proxy or normal) from which data can be
          gotten.

   \note All errors are signalled with exceptions.

*/
CRingBuffer*
CRingAccess::daqConsumeFrom(string uri)
{
  // Break the uri into its component pieces

  URL parsed(uri);
  string host = parsed.getHostName();
  string ring = parsed.getPath();

  // If the hostname is localhost, then the ring is local and it's simple:

  if (local(host)) {
    return new CRingBuffer(ring, CRingBuffer::consumer);
  }
  host = Os::getfqdn(host.c_str());
  // If the proxy ring exists..and has a feeder... we can just connect to it:

  string proxyRingName(host);
  proxyRingName += ".";
  proxyRingName += ring;
  if (CRingBuffer::isRing(proxyRingName)) {
    CRingBuffer* pRingBuffer =  new CRingBuffer(proxyRingName, CRingBuffer::consumer);
    CRingBuffer::Usage usage = pRingBuffer->getUsage();
    if (usage.s_producer != -1) {
      return pRingBuffer;
    }
    else {
      startPipeline(host, ring, proxyRingName);
      return pRingBuffer;
    }
  }

  CRingBuffer::create(proxyRingName, m_proxyRingSize, m_proxyMaxConsumers, true);
  startPipeline(host, ring,  proxyRingName);


  // - create the proxy ring.
  // - get our feeder process going.
  // - close the socket.
  // - Return a consumer connection to the proxy ring.

  CRingBuffer* proxy = new CRingBuffer(proxyRingName, CRingBuffer::consumer);


  return proxy;
}
//////////////////////////////////////////////////////////////////////////////
//  Utilities:

/****************************************************************************/
/* Contact a remote ring master and get a pipeline set up to a proxy ring.  */
/* Parameters:                                                              */
/*    hostName        - Name of host whose data we want.                    */
/*    remoteRingname  - Name of ring in remote host.                        */
/*    localRingname   - Name of local proxy ring.                           */
/****************************************************************************/
void
CRingAccess::startPipeline(string hostName, string remoteRingname, string localRingname)
{
  // The ring is remote.  We need help from the remote ringmaster:

  int socket;
  CRingMaster master(hostName);
  socket = master.requestData(remoteRingname);

  // We have a socket on which data will be sent.


  startFeeder(localRingname, socket); // do this now so the feeder doesn't inherit the
  close(socket);		// But don't shutdown.

}

/****************************************************************************/
/* Start the feeder process for the ringbuffer:                             */
/* - fork, parent returns, child does the work.                             */
/* - dup2 the socket into stdin, close stdout, stderr                       */
/* - start a new session                                                    */
/* - exec stdintoring (which lives in BINDIR) as appropriate.               */
/****************************************************************************/
void
CRingAccess::startFeeder(string proxyName, int socket)
{
  if (fork()) {
    return;
  }

  // Take care of our standard files...


  if (socket != STDIN_FILENO) {
    Os::checkNegativeStatus(close(STDIN_FILENO));
  }
  int status = dup2(socket, STDIN_FILENO);


  close(STDOUT_FILENO);
  //  close(STDERR_FILENO);

  // Detach from the current terminal

  if (daemon(1,1)) {
    int err = errno;
    cerr << "Unable to daemonize the feeder process. " << strerror(err) << endl;
    exit(-1);
  }			// TODO: Figure out posix way to do this.

  // Construct the path to stdintoring;

  string program(BINDIR);
  program += "/stdintoring";


  // Construct the switches:

  char mindataSw[100];
  char timeoutSw[100];
  sprintf(mindataSw, "--mindata=%d", m_minData);
  sprintf(timeoutSw, "--timeout=%d", m_Timeout);

  // build up and do the execve:

  char* const argv[10]  = {const_cast<char*>(program.c_str()), 
			   mindataSw, 
			   timeoutSw, 
			   const_cast<char*>(proxyName.c_str()), 
			 NULL};
  char* const env[1]  = {NULL};

  execve(program.c_str(), argv, env);

  exit(-1);			// should never happen!!
 
}
/**
 * Determine if a host is local.  We do the following steps:
 *   - If the host == localhost we are done and the domain is local.
 *   - get the fqdn of the local host
 *   - parse out the domain of the local host's fqdn.
 *   - If the host has no periods, add the domain name creating an fdqdn.
 *   - If the resulting FQDN for the host and locahost are the same
 *     we are local, otherwise we are remote.
 * @param host  - Host we are checking.
 * @return bool
 * @retaval true if the host is local, false otherwise.
 */

bool
CRingAccess::local(std::string host)
{
  // Check for explicit localhost:

  if (host == std::string("localhost")) return true;


  // Create the fqdn of the local host:
  // TODO: Error handling from gethostname and getaddrinfo
  //
  char hostname[HOST_NAME_MAX+1];
  gethostname(hostname, sizeof(hostname));

  if (host == std::string(hostname)) {
    return true;
  }
  std::string fqhostname = Os::getfqdn(hostname);
  

  // If the host has no periods append the domain name from
  // fqhostname.

  if (host.find(".") == std::string::npos) {
    
    // locate the start of the domain name in 
    // fqhostname..and append the domain to the host:

    size_t domainStartsAt = fqhostname.find(".");
    if (domainStartsAt != std::string::npos) {
      host += fqhostname.substr(domainStartsAt);
    }
  }

  return host == fqhostname;

	      

}
