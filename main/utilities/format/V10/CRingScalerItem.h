#ifndef NSCLDAQ10_CRINGSCALERITEM_H
#define NSCLDAQ10_CRINGSCALERITEM_H
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



#include "V10/CRingItem.h"
#include "V10/DataFormatV10.h"

#include <RangeError.h>

#include <typeinfo>
#include <vector>
#include <string>

namespace DAQ
{
  namespace V10 {

/*!
   This class derived from CRingItem and represents a set of scalers that have been 
   formatted as a ring item.  
*/
class CRingScalerItem : public CRingItem
{
  // private object data:

private:
  pScalerItem       m_pScalers;

  // Constructors and canonicals.:

public:
  CRingScalerItem(size_t numScalers);
  CRingScalerItem(uint32_t startTime,
		  uint32_t stopTime,
		  time_t   timestamp,
		  std::vector<uint32_t> scalers);
  CRingScalerItem(const CRingItem& rhs) throw(std::bad_cast);
  CRingScalerItem(const CRingScalerItem& rhs);
  
  virtual ~CRingScalerItem();

  CRingScalerItem& operator=(const CRingScalerItem& rhs);
  int operator==(const CRingScalerItem& rhs) const;
  int operator!=(const CRingScalerItem& rhs) const;

  // Accessor member functions.

  void     setStartTime(uint32_t startTime);
  uint32_t getStartTime() const;

  void     setEndTime(uint32_t endTime);
  uint32_t getEndTime() const;

  void     setTimestamp(time_t stamp);
  time_t   getTimestamp() const;

  void     setScaler(uint32_t channel, uint32_t value) throw(CRangeError);
  uint32_t getScaler(uint32_t channel) const throw(CRangeError);

  void setScalers(const std::vector<uint32_t>& scalers);
  std::vector<uint32_t> getScalers() const;

  uint32_t getScalerCount() const;

  // Virtual methods overriddent:

  virtual std::string typeName() const;
  virtual std::string toString() const;

  // utility.

private:
  size_t bodySize(size_t n);
  void init(size_t n);
  void throwIfInvalidChannel(uint32_t channel, 
			     const char* message) const throw(CRangeError);
};
   

  } // end of V10 namespace
} // end of DAQ
#endif
