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
#include "CDataLateStatistics.h"
#include "fragment.h"

/** 
 * @file CDataLateStatistics.cpp 
 * @brief Implementation of the CDataLateStatistics class.
 */

/*-----------------------------------------------------------------------
** Implementation of canonical methods:
*/

/**
 * constructor:
 *
 *   By providing a default constructor for the StatisticsElement there's no need
 *   to do any data initialization.  We do, however add this to the data late 
 *   observer list.
 */
CDataLateStatistics::CDataLateStatistics()
{
  CFragmentHandler* pHandler = CFragmentHandler::getInstance();

  pHandler->addDataLateObserver(this);
}
/**
 * destructor:
 *   The map will destroy itself.  We do need to remove this
 *   from the data late observer list.
 */
CDataLateStatistics::~CDataLateStatistics()
{
  CFragmentHandler* pHandler = CFragmentHandler::getInstance();
  pHandler->removeDataLateObserver(this);
}
/*---------------------------------------------------------------------
** Public member methods:
*/

/**
 * operator()
 *   This is the observation method.  It is called each time the
 *   fragment handler discovers a data late fragment.
 *   We update the m_Statistics struct elements as appropriate.
 *
 * @param fragment - Reference to the late fragment.
 * @param newest   - Timestamp of the newest event the handler has.
 */
void
CDataLateStatistics::operator()(const ::EVB::Fragment& fragment, uint64_t newest)
{
  // update the summary elements:

  m_Statistics.s_summary.s_nLateFragmentCount++;
  
  uint64_t timestamp = fragment.s_header.s_timestamp;
  uint64_t difference = newest - timestamp;

  if ((difference) > m_Statistics.s_summary.s_nWorstTimeDifference) {
    m_Statistics.s_summary.s_nWorstTimeDifference = difference;
  }

  // update the per source statistics.. once more the default constructor
  // for StatisticsElements makes this straightforward:

  uint32_t sourceId = fragment.s_header.s_sourceId;

  StatisticsElement& sourceStats(m_Statistics.s_perSource[sourceId]);
  sourceStats.s_nSourceId = sourceId; // in case this is a new source element.
  sourceStats.s_nLateFragmentCount++;
  if (difference > sourceStats.s_nWorstTimeDifference) {
    sourceStats.s_nWorstTimeDifference = difference;
  }
  
}
/**
 * getStatistics
 *
 * Marshall the data from the internal m_Statistics counters into a
 * Statstics struct for external consumption.
 * 
 * @return Statistics
 */
CDataLateStatistics::Statistics
CDataLateStatistics::getStatistics() const
{
  Statistics result;
  
  // fill in the global summary:

  result.s_summary = m_Statistics.s_summary; // Bitwise copy is fine for this.

  // Walk the map adding elements to the vector.

  std::map<uint32_t, StatisticsElement>::const_iterator p = m_Statistics.s_perSource.begin();
  while (p != m_Statistics.s_perSource.end()) {
    result.s_perSource.push_back(p->second);

    p++;
  }

  return result;
}

/**
 * clear
 *
 *   Clear the statistics.   The summary stats are Zeroed, the map that makse up
 *   the per source elements is cleared.
 */
void
CDataLateStatistics::clear()
{
  m_Statistics.s_summary.s_nLateFragmentCount = 0;
  m_Statistics.s_summary.s_nWorstTimeDifference         = 0;

  m_Statistics.s_perSource.clear();
}
