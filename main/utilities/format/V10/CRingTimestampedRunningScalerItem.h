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


/** 
 * @file CRingTimestampedRunningScalerItem.h
 * @brief ring item class definition for time stamped non-incremental scaler data
 * @author Ron Fox (fox@nscl.msu.edu)
 */

#ifndef DAQ10_CRINGTIMESTAMPEDRUNNINGSCALERITEM_H
#define DAQ10_CRINGTIMESTAMPEDRUNNINGSCALERITEM_H

#include "V10/CRingItem.h"
#include "V10/DataFormatV10.h"

#include <vector>
#include <typeinfo>

namespace DAQ {
  namespace V10 {

/**
 * @class CRingItemTimestampedRunningScalerItem
 *
 *  Encapsulates scaler data that has a timestamp synchronized to the
 * event data and is not incremental.
 */
class CRingTimestampedRunningScalerItem : public CRingItem
{
private:
  pNonIncrTimestampedScaler m_pItem;

  // Canonical operations:
public:
  CRingTimestampedRunningScalerItem(uint64_t eventTimestamp,
				    uint32_t startTime,
				    uint32_t stopTime,
				    uint32_t intervalDivisor,
				    time_t   unixTimestamp,
				    std::vector <uint32_t> scalers);
  CRingTimestampedRunningScalerItem(const CRingItem& rhs) throw(std::bad_cast);
  CRingTimestampedRunningScalerItem(const CRingTimestampedRunningScalerItem& rhs);
  virtual ~CRingTimestampedRunningScalerItem();
  CRingTimestampedRunningScalerItem& operator=(const CRingTimestampedRunningScalerItem& rhs);
  int operator==(const CRingTimestampedRunningScalerItem& rhs) const;
  int operator!=(const CRingTimestampedRunningScalerItem& rhs) const;

  // Accessors:

  uint64_t getTimestamp() const;
  float    getStartTime() const;
  float    getStopTime() const;
  time_t   getCalendarTime() const;
  std::vector<uint32_t> getScalers() const;
  uint32_t getScalerCount() const;

  uint32_t getOffsetStart() const;
  uint32_t getOffsetEnd() const;
  uint32_t getIntervalDivisor() const;
  // Utility methods:

private:
  void init(uint32_t nScalers);
  size_t bodySize(uint32_t nScalers);
  
};

} // end of V10 namespace
} // end DAQ

#endif
