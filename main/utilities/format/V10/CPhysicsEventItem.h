#ifndef NSCLDAQ10_CPHYSICSEVENTITEM_H
#define NSCLDAQ10_CPHYSICSEVENTITEM_H
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

#include <V10/CRingItem.h>		/* Base class */

#include <ByteBuffer.h>

#include <unistd.h>

#include <stdint.h>

#include <string>
#include <vector>

namespace DAQ {
  namespace V10 {

struct _RingItem;

/**
 *  This class is a wrapper for physics events.
 *  It's mainly provided so that textual dumps
 *  can be performed as typeName and toString
 *  are the only substantive methods...everything
 *  else just delegates to the base class.
 */

class CPhysicsEventItem : public CRingItem
{
public:
  CPhysicsEventItem(uint16_t type, size_t maxBody=8192);
  CPhysicsEventItem(const CPhysicsEventItem& rhs);
  CPhysicsEventItem(const CRingItem& rhs);
  virtual ~CPhysicsEventItem();

  CPhysicsEventItem& operator=(const CPhysicsEventItem& rhs);
  int operator==(const CPhysicsEventItem& rhs) const;
  int operator!=(const CPhysicsEventItem& rhs) const;

  // public utility methods
public:

  /*!
   * \returns copy of the body data
   */
  DAQ::Buffer::ByteBuffer getBodyData() const;
  // Virtual methods that all ring items must provide:

  virtual std::string typeName() const;	// Textual type of item.
  virtual std::string toString() const; // Provide string dump of the item.

 
  
};

} // end of V10
}// end of DAQ

#endif
