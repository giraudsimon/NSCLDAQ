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
#ifndef CDATALATESTATISTICS_H
#define CDATALATESTATISTICS_H

/**
 * @file CDataLateStatistics.h
 * @brief Defines an observer class for accumulating
 *        statistics on data late event fragments.
 */

#include "CFragmentHandler.h"
#include <vector>
#include <map>
#include <stdint.h>

/**
 * @class CDataLateStatistics
 *
 * Derived from a CFragmentHandler::DataLateObserver base class
 * Objects of this type hook themselves into the 
 * fragment handler's data late event observer chain.
 * As such our operator() is called for each data late event.
 * All we do is produce statistics that describe the data lates
 * seen so far.
 */
class CDataLateStatistics : public CFragmentHandler::DataLateObserver
{
  // Public data types:
public:
  typedef struct _StatisticsElement {

    uint32_t     s_nSourceId;
    uint32_t     s_nLateFragmentCount;
    uint64_t     s_nWorstTimeDifference;
    _StatisticsElement() :
      s_nSourceId(0),
      s_nLateFragmentCount(0),
      s_nWorstTimeDifference(0) {}

  } StatisticsElement, *pStatisticsElement;

  typedef struct _Statistics {

    StatisticsElement s_summary; // s_sourceId not used
    std::vector<StatisticsElement> s_perSource;

  } Statistics, *pStatistics;

  /*
   * private data types.  The statistics are exported as above
   * but maintained internally in a manner that makes it much easier
   * to update.
   */

private:
  typedef struct _Counters {
    StatisticsElement s_summary; // s_sourceId not used.
    std::map<uint32_t, StatisticsElement> s_perSource;
  } Counters, *pCounters;

  // Member data:
private:
  Counters m_Statistics;

  // Canonical members (allowed)

public:
  CDataLateStatistics();
  virtual ~CDataLateStatistics();
  
  // Undesirable canonicals:

private:
  CDataLateStatistics(const CDataLateStatistics&);
  CDataLateStatistics& operator=(const CDataLateStatistics&);
  int operator==(const CDataLateStatistics&) const;
  int operator!=(const CDataLateStatistics&) const;

  // Virtual member overrides:

public:
  virtual void operator()(const ::EVB::Fragment& fragment,  uint64_t newest);

  // Need to provide some access to the statistics too:

  Statistics getStatistics() const;
  void clear();
};


#endif
