#ifndef CREMOTEACCESS_H
#define CREMOTEACCESS_H
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
#include <unistd.h>
#include <string>


class CRingBuffer;

/*!
  Provides programmatic remote access to ring buffers.
  This software allows application to gain data from rings that live in remote systems.
  Rings are specified as URIs of the form:
  tcp://hostname/ringname

  There are two major cases; local and remote rings.  A local ring is defined by
  hostname == localhost.  In that case, a client connection to the local ring is made
  via the CRingBuffer class directly.  If the hostname is not localhost, 
  Proxy rings are created with names of the form hostname.ringname.

  If hostname.ringname exists, it is assumed to be a proxy for ringname in the host
  hostname, and a CRingBuffer connection is used to directly connect to the ring.

  If hostname.ringname does not exist, the proxy ring is created and the remote system's
  RingMaster process is contacted to get a REMOTE connection started.  This is connected
  to a stdintoring process that will be run locally to inject data into the ring.
  The entire affair looks a lot like a cross system pipeline:

  remotering -> ringtostdout |network| stdintoring -> proxyring.

  The effect is for there to aggregate the transfer of data from a remote system
  to all the clients of a ring in this system.

*/
class CRingAccess {
  // Static class members. These define the parameters for stdintoring and the proxy
  // ring actually created.
private:
  static size_t   m_proxyRingSize;
  static size_t   m_proxyMaxConsumers;
  static size_t   m_minData;
  static unsigned m_Timeout;
  // Public interface:
public:
  static size_t setProxyRingSize(size_t newSize);
  static size_t getProxyRingSize();
  
  static size_t setProxyMaxConsumers(size_t newMax);
  static size_t getProxyMaxConsumers();

  static size_t setMinData(size_t minData);
  static size_t getMinData();

  static unsigned setTimeout(unsigned newTimeout);
  static unsigned getTimeout();


  static CRingBuffer* daqConsumeFrom(std::string uri);
  static bool local(std::string host);  

  // Utilities
private:
  static void startFeeder(std::string proxyName, int socket);
  static void startPipeline(std::string hostName, 
			    std::string remoteRingname, 
			    std::string localRingname);

};



#endif
