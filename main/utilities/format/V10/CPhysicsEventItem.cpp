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

#include "V10/CPhysicsEventItem.h"
#include <V10/DataFormatV10.h>
#include <sstream>
#include <stdio.h>
#include <vector>
#include <typeinfo>

namespace DAQ {
  namespace V10 {

/*
 * All the canonical methods just delegate to the base class
 */


CPhysicsEventItem::CPhysicsEventItem(uint16_t type, size_t maxBody) :
  CRingItem(type, maxBody) {}

CPhysicsEventItem::CPhysicsEventItem(const CPhysicsEventItem& rhs) :
  CRingItem(rhs) {}

CPhysicsEventItem::~CPhysicsEventItem() {}

CPhysicsEventItem::CPhysicsEventItem(const CRingItem& rhs) 
  : CRingItem(rhs)
{
  if (type() != PHYSICS_EVENT) {
    throw std::bad_cast();
  }
}

CPhysicsEventItem& 
CPhysicsEventItem::operator=(const CPhysicsEventItem& rhs) 
{
  CRingItem::operator=(rhs);
  return *this;
}

int 
CPhysicsEventItem::operator==(const CPhysicsEventItem& rhs) const
{
  return CRingItem::operator==(rhs);
}

int 
CPhysicsEventItem::operator!=(const CPhysicsEventItem& rhs) const
{
  return CRingItem::operator!=(rhs);
}


/*--------------------------------------------------
 *
 * Virtual method overrides.
 */

/**
 * typeName
 *    Returns the type name associated with the item.
 * 
 * @return std::string  - "Event ".
 */
std::string
CPhysicsEventItem::typeName() const
{
  return "Event ";
}

/**
 * toString
 *
 *  Convert the event to a string.
 *
 * @return std::string - stringified versino of the event.
 */
std::string
CPhysicsEventItem::toString() const
{
  std::ostringstream out;
  uint32_t  bytes = getBodySize();
  uint32_t  words = bytes/sizeof(uint16_t);
  const uint16_t* body  = reinterpret_cast<const uint16_t*>((const_cast<CPhysicsEventItem*>(this))->getBodyPointer());

  out << "Event " << bytes << " bytes long\n";

  int  w = out.width();
  char f = out.fill();

  
  for (int i =1; i <= words; i++) {
    char number[32];
    sprintf(number, "%04x ", *body++);
    out << number;
    if ( (i%8) == 0) {
      out << std::endl;
    }
  }
  out << std::endl;
  
  
  return out.str();

}

DAQ::Buffer::ByteBuffer CPhysicsEventItem::getBodyData() const
{
  auto pBegin = reinterpret_cast<const char*>(getBodyPointer());
  auto pEnd   = pBegin + getBodySize();

  return DAQ::Buffer::ByteBuffer(pBegin, pEnd);
}

  } // end V10
} // end DAQ
