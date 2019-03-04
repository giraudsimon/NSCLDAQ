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
#include "V10/CRingScalerItem.h"
#include <time.h>
#include <string.h>
#include <sstream>
#include <stdio.h>

using namespace std;

namespace DAQ
{
  namespace V10 {

///////////////////////////////////////////////////////////////////////////////////////
//
// Constructors and other canonicals.
//

/*!
  Construct a scaler item with the scalers all zeroed and the times not set...
  except for the absolute timestamp which is set to now.
  \param numScalers - Number of scalers that will be put in the item.
*/
CRingScalerItem::CRingScalerItem(size_t numScalers) :
  CRingItem(INCREMENTAL_SCALERS,
	    bodySize(numScalers))
{
  init(numScalers);
  m_pScalers->s_intervalStartOffset = 0;
  m_pScalers->s_intervalEndOffset   = 0;
  m_pScalers->s_timestamp = static_cast<uint32_t>(time(NULL));
  m_pScalers->s_scalerCount = numScalers;
  memset(m_pScalers->s_scalers, 0, numScalers*sizeof(uint32_t));
}
/*!
  Construct a scaler item with all the info specified.
  \param startTime - incremental scaler interval start time.
  \param endTime   - incremental scaler interval end time
  \param timestamp - Absolute system time at which the item was created.
  \param scalers   - Vector of scaler values.

*/
CRingScalerItem::CRingScalerItem(uint32_t startTime,
				 uint32_t stopTime,
				 time_t   timestamp,
				 std::vector<uint32_t> scalers) :
  CRingItem(INCREMENTAL_SCALERS, bodySize(scalers.size()))
{
  init(scalers.size());

  m_pScalers->s_intervalStartOffset = startTime;
  m_pScalers->s_intervalEndOffset   = stopTime;
  m_pScalers->s_timestamp           = timestamp;
  m_pScalers->s_scalerCount         = scalers.size();
  for (int i = 0; i  < scalers.size(); i++) {
    m_pScalers->s_scalers[i] = scalers[i];
  }
}

/*!
  Casting construction. 
  \param rhs - const reference to a RingItem.  We are doing an upcast to 
  a scaler buffer.
*/ 
CRingScalerItem::CRingScalerItem(const CRingItem& rhs) throw(bad_cast) :
  CRingItem(rhs)
{
  if (type() != INCREMENTAL_SCALERS) {
    throw bad_cast();
  }
  pScalerItem pItem = reinterpret_cast<pScalerItem>(const_cast<CRingItem&>(rhs).getItemPointer());
  init(pItem->s_scalerCount);
  
}

/*!
  Ordinary copy construction
  \param rhs - The item we are copying into *this
*/
CRingScalerItem::CRingScalerItem(const CRingScalerItem& rhs) :
  CRingItem(rhs)
{
  init(rhs.m_pScalers->s_scalerCount);
}

/*!
  Destruction just delegates.
*/
CRingScalerItem::~CRingScalerItem()
{}

/*!
   Assignment.. just the base class assignment with init():

  \param rhs - The item being assigned to *this.
  \return CRingScalerItem&
  \retval *this
*/
CRingScalerItem&
CRingScalerItem::operator=(const CRingScalerItem& rhs) 
{
  if (this != &rhs) {
    CRingItem::operator=(rhs);
    init(rhs.m_pScalers->s_scalerCount);
  }
  return *this;
}
/*!
    Equality comparison.. there's no point in looking a the pointers.
    so the base class comparison is just fine.
    \param rhs - Item being compared to *this.
    \return int
    \retval 0 - Not equal
    \retval 1 - Equal
*/
int
CRingScalerItem::operator==(const CRingScalerItem& rhs) const
{
  return CRingItem::operator==(rhs);
}
/*!
   Inequality is the logical inverse of equality.
   \rhs - Item being compared to *thisl
   \return int
   \retval 0 - not unequal
   \retval 1 - unequal
*/
int
CRingScalerItem::operator!=(const CRingScalerItem& rhs) const
{
  return !(*this == rhs);
}
    
/////////////////////////////////////////////////////////////////////////////////////
//
//  Accessors - selectors and mutators.
//

/*!
   Set the scaler interval start time.

   \param startTime - the new interval start time.
*/
void
CRingScalerItem::setStartTime(uint32_t startTime)
{
  m_pScalers->s_intervalStartOffset = startTime;
}
/*!
   \return uint32_t
   \retval the current interval start time.
*/
uint32_t
CRingScalerItem::getStartTime() const
{
  return m_pScalers->s_intervalStartOffset;
}

/*!
   Set the interval ent time
   \pram endTime - the new interval end time.
*/
void
CRingScalerItem::setEndTime(uint32_t endTime) 
{
  m_pScalers->s_intervalEndOffset = endTime;
}
/*!
   \return uint32_t
   \retval The current interval end time.
*/
uint32_t
CRingScalerItem::getEndTime() const
{
  return m_pScalers->s_intervalEndOffset;
}


/*!
   set the absolute timestamp of the event.

   \param stamp - the time stampe.
*/
void
CRingScalerItem::setTimestamp(time_t  stamp)
{
  m_pScalers->s_timestamp = stamp;
}
/*!
   \return time_t
   \retval the current timestamp.
*/
time_t
CRingScalerItem::getTimestamp() const
{
  return m_pScalers->s_timestamp;
}


/*!
  Set a scaler value.
  \param channel - Number of the channel to modify.
  \param value   - The new value for that channel.
  \throw CRangError if channel is too big.
*/
void
CRingScalerItem::setScaler(uint32_t channel, uint32_t value) throw(CRangeError)
{
  throwIfInvalidChannel(channel, "Attempting to set a scaler value");
  m_pScalers->s_scalers[channel] = value;
}

/*!
  Set a scaler values.
  \param values  - The new values for that channel.
  \throw CRangError if there are too many channels
*/
void
CRingScalerItem::setScalers(const std::vector<uint32_t>& values)
{
    size_t size = values.size();
    for ( size_t ch=0; ch<size; ++ch) {
        setScaler(ch, values.at(ch));
    }
}

/*!

  \param channel - Number of the channel we want.
  \return uint32_t
  \retval Value of the selected scaler channel
  \throw CRangeError - if the channel number is out of range.
*/
uint32_t
CRingScalerItem::getScaler(uint32_t channel) const throw(CRangeError) 
{
  throwIfInvalidChannel(channel, "Attempting to get a scaler value");
  
  return m_pScalers->s_scalers[channel];
}
/*!
    \return std::vector<uint32_t>
    \retval - the scalers.
*/
vector<uint32_t>
CRingScalerItem::getScalers() const
{
  vector<uint32_t> result;
  for (int i =0; i < m_pScalers->s_scalerCount; i++) {
    result.push_back(m_pScalers->s_scalers[i]);
  }
  return result;
}

/*!
   \return uint32_t
   \retval number of scalers
*/
uint32_t
CRingScalerItem::getScalerCount() const
{
  return m_pScalers->s_scalerCount;
}
  
/////////////////////////////////////////////////////////////////////////////////////
//
// Private utilities


/*
 * Do common initialization.. setting m_pScalers and the cursor.
 */
void
CRingScalerItem::init(size_t n)
{
  m_pScalers       = reinterpret_cast<pScalerItem>(getItemPointer());
  size_t   size    = bodySize(n); 
  uint8_t* pCursor = reinterpret_cast<uint8_t*>(getBodyPointer());
  pCursor         += size;
  setBodyCursor(pCursor);
  updateSize();
    
}
/*-------------------------------------------------------
** Virtual method overrides
*-------------------------------------------------------*/

/**
 * typeName
 *
 *  Returns the string associated with this type of
 *  ring item ("Scaler").
 *
 * @return std::string - the item type name string.
 */
std::string
CRingScalerItem::typeName() const
{
  return std::string(" Scaler: ");
}
/**
 * toString
 *
 *  Return a textual human readable version of the data
 *  in this item.
 *
 * @return std::string - the text string.
 */

std::string
CRingScalerItem::toString() const
{
  std::ostringstream out;

  uint32_t end   = getEndTime();
  uint32_t start = getStartTime();
  string   time  = timeString(getTimestamp());
  vector<uint32_t> scalers = getScalers();

  float   duration = static_cast<float>(end - start);

  out << time << " : Incremental scalers:\n";
  out << "Interval start time: " << start << " end: " << end << " seconds in to the run\n\n";
  

  out << "Index         Counts                 Rate\n";
  for (int i=0; i < scalers.size(); i++) {
    char line[128];
    double rate = (static_cast<double>(scalers[i])/duration);

    sprintf(line, "%5d      %9d                 %.2f\n",
	    i, scalers[i], rate);
    out << line;
  }


  return out.str();
  
}

/*-------------------------------------------------------
** Private utilities:
*---------------------------------------------------------
/*
 *  Given the number of scalers determine the size of the body.
 */
size_t
CRingScalerItem::bodySize(size_t n)
{
  size_t  size    = sizeof(ScalerItem) + (n-1)*sizeof(uint32_t) - sizeof(RingItemHeader);
  return  size;
}
/*
** Throws a range error if the channel number is invalid.
*/
void
CRingScalerItem::throwIfInvalidChannel(uint32_t channel,
				       const char* message) const throw(CRangeError)
{
  if (channel >= m_pScalers->s_scalerCount) {
    throw CRangeError(0, m_pScalers->s_scalerCount, channel,
		      message);
  }
}

 } // end V10
} // end DAQ
