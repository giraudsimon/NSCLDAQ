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

#include "CBarrierStats.h"

/**
 * @file CBarrierstats.cpp
 * @brief Implementation of the CBarrierStats clas - observer that accumulates barrier statistics.
 */

/*----------------------------------------------------------------------------------------
** Canonicals
*/

/**
 * constructor
 *
 *  Registers us with the fragment handler.   Data default constructos ensure
 *  we have zeroed statistics to start with.
 */
CBarrierStats::CBarrierStats()
{
  CFragmentHandler* pHandler = CFragmentHandler::getInstance();
  pHandler->addBarrierObserver(this);
}
/**
 * destructor
 *
 *  Unregister this from the fragment handler.
 */
CBarrierStats::~CBarrierStats()
{
  CFragmentHandler* pHandler = CFragmentHandler::getInstance();
  pHandler->removeBarrierObserver(this);
}

/*-------------------------------------------------------------------------------------------
** Public methods.
*/

/**
 * operator()
 *
 *  This method is called on observation.  It is what actually accumulates
 *  the statistics
 *
 * @param typelist - vector containin the types of barrier fragments received
 *                   from each data source.
 */
void
CBarrierStats::operator()(const std::vector<std::pair<uint32_t, uint32_t> >& typeList)
{
  bool homogeneous = true;
  uint32_t firstType = typeList[0].second; // TODO: Empty barrier.

  for (int i =0; i < typeList.size(); i++) {
    uint32_t source = typeList[i].first;
    uint32_t type   = typeList[i].second;

    if (type != firstType) homogeneous = false;

    m_statistics.s_barriers.s_typeCount[type]++;
    m_statistics.s_sourceCounts[source].s_barrierCount++;
    m_statistics.s_sourceCounts[source].s_typeCount[type]++;

  }
  if (homogeneous) {
    m_statistics.s_homogeneous++;
  } else {
    m_statistics.s_heterogeneous++;
  }
  m_statistics.s_barriers.s_barrierCount++;
  
}
/**
 * getStatistics
 *
 *  Return a copy of the current statistics struct.
 *
 * @return BarrieStats statistics struct.
 */
CBarrierStats::BarrierStats
CBarrierStats::getStatistics() const
{
  return m_statistics;
}
/**
 * clear
 *
 * Clears the statistics data.  Scalers are cleared but the std::maps also have all
 * elements cleared as well.
 */
void
CBarrierStats::clear()
{
  m_statistics.s_homogeneous    = 0;
  m_statistics.s_heterogeneous = 0;

  m_statistics.s_barriers.s_barrierCount = 0;
  m_statistics.s_barriers.s_typeCount.clear();

  m_statistics.s_sourceCounts.clear();

}

