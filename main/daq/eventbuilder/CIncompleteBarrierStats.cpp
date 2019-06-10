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
 * @file CIncompleteBarrierStats.cpp
 * @brief Implementation of the CIncompleteBarrierStats class.
 */

#include "CIncompleteBarrierStats.h"


/**
 *-----------------------------------------------------------------------------------
 * Implementatiuons of canonicals.
 */


/**
 * constructor
 *   Regiseter with the fragment handler singelton
 */
CIncompleteBarrierStats::CIncompleteBarrierStats()
{
  CFragmentHandler* pHandler = CFragmentHandler::getInstance();
  pHandler->addPartialBarrierObserver(this);
}
/**
 * destructor
 *   Unregister this.
 */
CIncompleteBarrierStats::~CIncompleteBarrierStats()
{
  CFragmentHandler* pHandler = CFragmentHandler::getInstance();
  pHandler->removePartialBarrierObserver(this);
  
}
/*
**----------------------------------------------------------------------------------
* Implementation of normal public methods.
*/

/** 
 * operator()
 *
 *   This  method is called when there is an incomplete barrier.
 *   The actual statistics are maintained here.
 *
 * @param barrierTypes - for each fragment that _was_ received, this
 *                       vector contains the source id and barrier type.
 * @param missingSources - Vector of the sources that did not contribute
 *                       a barrier fragment in time to make this barrier work.
 */
void
CIncompleteBarrierStats::operator()(const std::vector<std::pair<uint32_t, uint32_t> >& barrierTypes, 
		  const std::vector<uint32_t>& missingSources)
{
  m_stats.s_totalIncomplete++;
  bool     heterogeneous = false;
  uint32_t firstType     = barrierTypes[0].second;

  // Iterate over barrier types to determine if this was a heterogenous barrier

  for (int i = 0; i < barrierTypes.size(); i++) {
    if (barrierTypes[i].second != firstType) {
      heterogeneous = true;
    } 
  }
  if (heterogeneous) {
    m_stats.s_homogeneous++;
  } else {
    m_stats.s_heterogeneous++;
  }

  // process missing sources to maintain the missing count histogram.
  // and number of times each source was gone:

  m_stats.s_missingHistogram[missingSources.size()]++;
  for (int i = 0 ; i < missingSources.size(); i++) {
    m_stats.s_sourceMissingCount[missingSources[i]]++;
  }
     
}
/**
 * getStatistics
 *
 *   Return a copy of the current statistics struct.
 *
 * @return CIncompleteBarrierStats::Statistics
 */

CIncompleteBarrierStats::Statistics
CIncompleteBarrierStats::getStatistics() const
{
  return m_stats;
}
/**
 * clear
 *
 *  Clears the statistics arrays and clear()'s the maps.
 */
void
CIncompleteBarrierStats::clear()
{
  m_stats.s_totalIncomplete = 0;
  m_stats.s_homogeneous     = 0;
  m_stats.s_heterogeneous   = 0;
  m_stats.s_missingHistogram.clear();
  m_stats.s_sourceMissingCount.clear();
}
