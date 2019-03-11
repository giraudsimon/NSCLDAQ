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

#ifndef __CEVENTORDERCLIENT_H
#define __CEVENTORDERCLIENT_H

#include <string>
#include <stdint.h>
#include <list>
#include <sys/uio.h>


namespace EVB {
  typedef struct _Fragment Fragment, *pFragment;
  typedef struct _FragmentChain FragmentChain, *pFragmentChain;
  typedef std::list<pFragment> FragmentPointerList;
}

// Forward definition:

class CSocket;

/**
 * Class responsible for client interaction with the event orderer.
 */
class CEventOrderClient {
private:
  std::string m_host;		// Host running the event builder.
  uint16_t    m_port;		// port on which the event builder is running.
  CSocket*    m_pConnection;	// Connectionto the server.
  bool        m_fConnected;	// True if connection is alive.
  size_t      m_nIovecSize; // Number of elements allocated below.
  iovec*      m_pIovec;     // Pre-allocated iovector for writev.
  
  // construction/destruction/canonicals
public:
  CEventOrderClient(std::string host, uint16_t port);
  virtual ~CEventOrderClient();

private:
  CEventOrderClient(const CEventOrderClient&);
  CEventOrderClient& operator=(const CEventOrderClient&);
  int operator==(const CEventOrderClient&) const;
  int operator!=(const CEventOrderClient&) const;
  

  // Static members:
public:
  static uint16_t Lookup(std::string host, const char* pName=0);

  // Object operations:
public:
  void Connect(std::string description, std::list<int> sources);
  void disconnect();
  void submitFragments(EVB::pFragmentChain pChain);
  void submitFragments(size_t nFragments, EVB::pFragment ppFragments);
  void submitFragments(EVB::FragmentPointerList& fragments);

  // Utility functions:

private:
  static size_t message(void** msg, const void* request, size_t requestSize, const  void* body, size_t bodySize);
  std::string getReplyString();	
  static void freeChain(EVB::pFragmentChain pChain);
};


#endif
