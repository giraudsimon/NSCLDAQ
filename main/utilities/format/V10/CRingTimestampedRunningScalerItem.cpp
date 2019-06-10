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
#include <V10/CRingTimestampedRunningScalerItem.h>
#include <time.h>
#include <string.h>

namespace DAQ {
  namespace V10 {

////////////////////////////////////////////////////////////////////////////////////
//
// Construtors and other canonicals.

/**
 * Regular constructor.  Builds a fully fledged ring item from its parameters.
 *
 * @param eventTimestamp - Timestamp that is synchronized to the event clock.
 *                         This timestamp is what gives this item its name.
 * @param startTime      - Start time of the increment interval.
 * @param endTime        - End time of the increment interval
 * @param intervalDivisor - Divisor that, when done as a floating point gets 
 *                          either the start or end time in seconds.
 * @param unixTimestamp  - Time of day the buffer was emitted. Seconds past the epoch.
 * @param scalers        - Vector of scaler values.
 */
CRingTimestampedRunningScalerItem::CRingTimestampedRunningScalerItem(uint64_t eventTimestamp,
								     uint32_t startTime,
								     uint32_t stopTime,
								     uint32_t intervalDivisor,
								     time_t   unixTimestamp,
								     std::vector <uint32_t> scalers) :
  CRingItem(TIMESTAMPED_NONINCR_SCALERS, bodySize(scalers.size()))
{
  init(scalers.size());;

  // Fill in the stuff fixed size stuff:

  m_pItem->s_eventTimestamp      = eventTimestamp;
  m_pItem->s_intervalStartOffset = startTime;
  m_pItem->s_intervalEndOffset   = stopTime;
  m_pItem->s_intervalDivisor     = intervalDivisor;
  m_pItem->s_clockTimestamp      = unixTimestamp;
  m_pItem->s_scalerCount         = scalers.size();

  for (int i=0; i < scalers.size(); i++) {
    m_pItem->s_scalers[i] =  scalers[i];
  }
  
}
/**
 * Upcast constructor from a ring item to this.
 *
 * @param rhs - CRingItem that will be up cast to us.
 *
 * @throw bad_cast - if the ring item is not the right type.
 */

CRingTimestampedRunningScalerItem::CRingTimestampedRunningScalerItem(const CRingItem& rhs) 
  throw(std::bad_cast) : CRingItem(rhs)
{
  if (type() != TIMESTAMPED_NONINCR_SCALERS) {
    throw std::bad_cast();
  }

  m_pItem = reinterpret_cast<pNonIncrTimestampedScaler>(getItemPointer());
  init(m_pItem->s_scalerCount);

}
/**
 * Copy constructor
 * 

 * @param rhs - The source of the copy.
 */
CRingTimestampedRunningScalerItem::CRingTimestampedRunningScalerItem(const CRingTimestampedRunningScalerItem& rhs) :
  CRingItem(rhs)
{

  m_pItem = reinterpret_cast<pNonIncrTimestampedScaler>(getItemPointer());
  init(m_pItem->s_scalerCount);
}
/**
 * Destructor chains to the base class:
 */

CRingTimestampedRunningScalerItem::~CRingTimestampedRunningScalerItem()
{}

/**
 * Assignment: Base class assignment with m_pItem getting set and init called
 * Very much like copy construction.
 *
 * @param rhs - The ring item we are assigning to this.
 */
CRingTimestampedRunningScalerItem&
CRingTimestampedRunningScalerItem::operator=(const CRingTimestampedRunningScalerItem& rhs)
{
  if (&rhs != this) {
    CRingItem::operator=(rhs);
    m_pItem = reinterpret_cast<pNonIncrTimestampedScaler>(getItemPointer());
    init(m_pItem->s_scalerCount);
  }

  return *this;
}
/**
 * Equality comparison.
 *  For now the base class operator==
 *
 * @param rhs - the item being compared to this.
 * 
 * @return int
 * @retval ==: 1
 * @retval !=: 0
 */
int
CRingTimestampedRunningScalerItem::operator==(const CRingTimestampedRunningScalerItem& rhs) const
{
  return CRingItem::operator==(rhs);
}
/**
 * Inequality comparison.. the inverse of equality.
 */
int
CRingTimestampedRunningScalerItem::operator!=(const CRingTimestampedRunningScalerItem& rhs) const
{
  return !(*this == rhs);
}

/*------------------------------------------------------------------------------------
**
** Accessors:
*/

/**
 * Get the event timestamp.
 *
 * @return uint64_t
 */
uint64_t
CRingTimestampedRunningScalerItem::getTimestamp() const
{
  return m_pItem->s_eventTimestamp;
}
/**
 * Get the start time in floating point seconds.  This is done by dividing the
 * start time given us by the interval divisor.
 *
 * @return float
 */
float
CRingTimestampedRunningScalerItem::getStartTime() const
{
  float start   = m_pItem->s_intervalStartOffset;
  float divisor = m_pItem->s_intervalDivisor;
  return start / divisor;
}
/**
 * Get the end time in floating point seconds.  This is done by dividing the
 * end time given us by the interval divisor.
 *
 * @return float
 */
float
CRingTimestampedRunningScalerItem::getStopTime() const
{
  float end     = m_pItem->s_intervalEndOffset;
  float divisor = m_pItem->s_intervalDivisor;
  return end / divisor;
}
/**
 * Get the unix timestamp for the event.
 *
 * @return time_t
 */
time_t
CRingTimestampedRunningScalerItem::getCalendarTime() const
{
  return static_cast<time_t>(m_pItem->s_clockTimestamp);
}

/**
 * Get the scalers as a vector:
 *
 * @return std::vector<uint32_t>
 */
std::vector<uint32_t>
CRingTimestampedRunningScalerItem::getScalers() const
{
  std::vector<uint32_t> result;

  for (int i = 0; i < m_pItem->s_scalerCount; i++) {
    result.push_back(m_pItem->s_scalers[i]);
  }
  return result;
}

uint32_t
CRingTimestampedRunningScalerItem::getScalerCount() const
{
    return m_pItem->s_scalerCount;
}

uint32_t
CRingTimestampedRunningScalerItem::getOffsetStart() const
{
   return m_pItem->s_intervalStartOffset;
}

uint32_t
CRingTimestampedRunningScalerItem::getOffsetEnd() const
{
   return m_pItem->s_intervalEndOffset;
}

uint32_t
CRingTimestampedRunningScalerItem::getIntervalDivisor() const
{
    return m_pItem->s_intervalDivisor;
}

/**
 * TODO: After merge need to add the formatting method
 *       need to add to ring item factory.
 */

/*------------------------------------------------------------------------------------
*
* Private utilities:
*/

/**
 * init
 *   Common initialization once the number of scalers is known
 *
 * @param nScalers - number of scalers.
 */
void
CRingTimestampedRunningScalerItem::init(uint32_t nScalers)
{
  m_pItem = reinterpret_cast<pNonIncrTimestampedScaler>(getItemPointer());

  size_t  size = bodySize(nScalers);
  uint8_t* pCursor = reinterpret_cast<uint8_t*>(getBodyPointer());
  pCursor         += size;
  setBodyCursor(pCursor);
  updateSize();
}
/**
 * bodySize
 *   Given the number of scalers determines the size of the body.
 * 
 * @param nScalers - Number of scalers in the body.
 *
 * @return size_t size in bytes of the item body.
 */
size_t
CRingTimestampedRunningScalerItem::bodySize(uint32_t nScalers)
{
  size_t size = sizeof(NonIncrTimestampedScaler) 
    + (nScalers-1)*sizeof(uint32_t) - sizeof(RingItemHeader);
  return size;
}


  } // end of V10 namespace
} // end DAQ
