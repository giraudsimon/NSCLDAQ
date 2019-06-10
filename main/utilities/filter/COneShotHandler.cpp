
#include <COneShotHandler.h>
#include <CRingStateChangeItem.h>
#include <DataFormat.h>
#include <COneShotException.h>
#include <ErrnoException.h>
#include <limits>
#include <sstream>
#include <iostream>

static uint32_t defaultRunNumber = std::numeric_limits<uint32_t>::max();

COneShotHandler::COneShotHandler(unsigned int ntrans)
  : m_nExpectedSources(ntrans),
  m_stateCounts(),
  m_cachedRunNo(defaultRunNumber),
  m_complete(false)
{
  m_stateCounts[BEGIN_RUN] = 0;
  m_stateCounts[END_RUN] = 0;
  m_stateCounts[PAUSE_RUN] = 0;
  m_stateCounts[RESUME_RUN] = 0;
}


/**! Process a new item
*
* Check for error case:
* - the run number changes when in the middle of the run.
*
* @param pItem a state change item to process
* 
* @throws CErrnoException when run number changes unexpectedly
*/
void COneShotHandler::update(CRingItem* pItem)
{
  if (pItem==nullptr) {
    throw COneShotException("COneShotHandler::update(CRingStateChangeItem*)",
                            "Null pointer passed as argument.");
  }
  uint32_t type = pItem->type();

  // if we have already reached our limit, throw
  if (m_complete) {
    throw COneShotException("COneShotHandler::update(CRingStateChangeItem*)",
                            "Unexpected, extra state change item");
  }

  if (validType(type)) {
    updateState(static_cast<CRingStateChangeItem*>(pItem));
  }
}

void COneShotHandler::updateState(CRingStateChangeItem* pItem)
{
  // Check that the run number hasn't changed unexpectedly
  uint32_t run = pItem->getRunNumber();
  if (run != m_cachedRunNo  &&  m_cachedRunNo != defaultRunNumber) {
    throw COneShotException("COneShotHandler::update(CRingStateChangeItem*)",
        "More begin runs detected than expected");
  }

  // Only do something if we understand the state change
  uint32_t type = pItem->type();
  if (type==BEGIN_RUN) {
    if (waitingForBegin()) {
      // Handle the first begin run specially
      initialize(pItem);

      ++m_stateCounts[type];

    } else if (getCount(BEGIN_RUN) >= m_nExpectedSources) {
      // Handle if there are too many BEGIN_RUNS 
      std::ostringstream errmsg;
      errmsg << "Too many begin runs observed. Expecting only " 
        << m_nExpectedSources;
      throw COneShotException("COneShotHandler::update(CRingStateChangeItem*)",
          errmsg.str());

    }
  } else {
    if (! waitingForBegin()) { 
      ++m_stateCounts[type]; 
    }
  }

  m_complete = (getCount(END_RUN)==m_nExpectedSources);
}

void COneShotHandler::initialize(CRingStateChangeItem* pItem)
{
  // Set the run number
  m_cachedRunNo = pItem->getRunNumber();
  
  // Reset the counters 
  clearCounts();

}


/**! Check if the run has been completed */
bool COneShotHandler::complete() const
{
  return m_complete;
}

void COneShotHandler::reset()
{
  clearCounts();
  m_complete = false;
}

bool COneShotHandler::waitingForBegin() const
{
  return (getCount(BEGIN_RUN)==0);
}

/**! Get the number of state change items already seen
*
* @param key the type of the state change item
* @return the number of valid state changes of type key observed 
*/
uint32_t COneShotHandler::getCount(uint32_t key) const
{
  uint32_t count=0;
  try {
    std::map<uint32_t,uint32_t>::const_iterator it;

    it = m_stateCounts.find(key);
    count = it->second;

  } catch (std::exception&) {}

  return count;
}


/**! Verify that type is BEGIN_RUN, END_RUN, PAUSE_RUN, RESUME_RUN
*
* Also enforce that the order of the state changes is sensible.
* 
*
* @param type the ring item type
* @return true if type is BEGIN/END/PAUSE/RESUME_RUN
*/
bool COneShotHandler::validType(uint32_t type) const
{
  std::map<uint32_t,uint32_t>::const_iterator it, itend;
  it = m_stateCounts.begin();
  itend = m_stateCounts.end();
  
  // Check that this is in fact one of the 4 supported transitions
  bool foundMatch=false;
  while (it!=itend && !foundMatch) {
    foundMatch = (type == it->first);
    ++it;   
  }

  return foundMatch;
}

/**! Reset the counters */
void COneShotHandler::clearCounts()
{
  std::map<uint32_t,uint32_t>::iterator it, itend;
  it = m_stateCounts.begin();
  itend = m_stateCounts.end();
  while (it!=itend) {
    it->second = 0; 
    ++it;
  }
}
