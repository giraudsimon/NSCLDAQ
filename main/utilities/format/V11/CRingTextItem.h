#ifndef CV11RINGTEXTITEM_H
#define CV11RINGTEXTITEM_H

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

#include "V11/CRingItem.h"

#include "V11/DataFormatV11.h"

#include <RangeError.h>
#include <stdint.h>
#include <time.h>
#include <string>
#include <vector>
#include <typeinfo>

namespace DAQ {
  namespace V11 {

/*!
  The text ring item provides a mechanism to put an item in/take an item out of 
  a ring buffer that consists of null terminated text strings.  
*/
class CRingTextItem : public CRingItem
{
  // Private data:


public:
  // Constructors and other canonicals:

  CRingTextItem(uint16_t type,
		std::vector<std::string> theStrings);
  CRingTextItem(uint16_t type,
		std::vector<std::string> theStrings,
		uint32_t                 offsetTime,
		time_t                   timestamp) ;
  CRingTextItem(
    uint16_t type, uint64_t eventTimestamp, uint32_t source, uint32_t barrier,
    std::vector<std::string> theStrings, uint32_t offsetTime, time_t timestamp,
    int offsetDivisor = 1
  );
  CRingTextItem(const CRingItem& rhs) ;
  CRingTextItem(const CRingTextItem& rhs);

  virtual ~CRingTextItem();

  CRingTextItem& operator=(const CRingTextItem& rhs);
  int operator==(const CRingTextItem& rhs) const;
  int operator!=(const CRingTextItem& rhs) const;

  // Public interface:
public:
  std::vector<std::string>  getStrings() const;
  uint32_t getStringCount() const;

  void     setTimeOffset(uint32_t offset);
  uint32_t getTimeOffset() const;
  float    computeElapsedTime() const;
  uint32_t getTimeDivisor() const;
  void     setTimeDivisor(uint32_t divisor);
  void     setTimestamp(time_t stamp);
  time_t   getTimestamp() const;

  // Virtual methods all ring overrides.

  virtual std::string typeName() const;
  virtual std::string toString() const;

  //private utilities:
private:
  size_t bodySize(std::vector<std::string> strings) const;
  bool   validType() const;
  void   copyStrings(std::vector<std::string> strings);
  void   init();
};

} // end of V11 namespace
} // end DAQ

#endif
