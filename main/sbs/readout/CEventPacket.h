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

#ifndef CEVENTPACKET_H
#define CEVENTPACKET_H

#include "CEventSegment.h"
#include "CDocumentedPacket.h"
#include <string>

/*!
   This class defines an event segment that is encapsulated in a
   documented packet.  This eases the burden of creating documented packets
   and gets the packetization logic right.
   Layering as well as inheritence are used.  The class is derived from and event
   segment, however it operates by containing both an event segment and a
   documentd packet.
*/
class CEventPacket : public CEventSegment
{
private:
  CDocumentedPacket   m_Packet;
  CEventSegment&       m_Segment;

  // constructors and other canonicals.  As we dont' know if the
  // segments will copy copy construction etc. are disallowed.
  //

public:
  CEventPacket(CEventSegment& seg,
	       short tag, std::string name, std::string description, std::string version);
  virtual ~CEventPacket();

private:
  CEventPacket(CEventPacket& rhs);
  CEventPacket& operator=(const CEventPacket& rhs);
  int operator==(const CEventPacket& rhs) const;
  int operator!=(const CEventPacket& rhs) const;

  // Virtual functions with the exception of read, these all delegate to 
  // m_Segment:
public:
  virtual void   initialize();
  virtual void   clear();
  virtual void   disable();
  virtual size_t read(void* pBuffer, size_t maxwords);


    
};


#endif

