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
#include "CEventPacket.h"


using namespace std;

/*!
    Construct the object.  
    \param seg   - Reference to the event segment to embed in the packet.
    \param tag   - Integer tag describing the packet in the event stream.
    \param name  - Short name for the packet (e.g. "sega").
    \param description - Long description of the packet (e.g. "Segmented Ge detector data").
    \param version - Packet version string.
*/
CEventPacket::CEventPacket(CEventSegment& seg,
			   short          tag,
			   string         name,
			   string         description,
			   string         version) :
  m_Packet(tag, name, description, version),
  m_Segment(seg)
{}

/*!
   Destructor
*/
CEventPacket::~CEventPacket() {}

/*!
  Delegate initialization to our encapsulated segment 
*/
void
CEventPacket::initialize()
{
  m_Segment.initialize();
}
/*!
  Delegate clear to the encapsulated segment:
*/
void
CEventPacket::clear()
{
  m_Segment.clear();
}
/*!
  Delegate disable to the encapsulated segment:
*/
void
CEventPacket::disable()
{
  m_Segment.disable();
}
/*!
   Encapsulate the segment in our packet.
   \param pBuffer - pointer to the output buffer.
   \param maxwords - Maximum number of words left in that output buffer.

   \return size_t
   \retval number of 16 bit words inserted into the buffer.

*/
size_t
CEventPacket::read(void* pBuffer, size_t maxwords)
{
  unsigned short* p     = reinterpret_cast<unsigned short*>(pBuffer);
  unsigned short* pBody = m_Packet.Begin(p);
  size_t bodySize = m_Segment.read(pBody,
				   maxwords - 
				   (pBody - p));

  // End the packet and compute the final event size:

  pBody += bodySize;
  pBody = m_Packet.End(pBody);

  return (pBody - p);
}
