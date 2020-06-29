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

#ifndef CBARRIERSTATS_H
#define CBARRIERSTATS_H


/**
 * @file CBarrierStats.h
 * @brief Observer to accumulate statistics on complete barriers.
 */

#include "CFragmentHandler.h"


/**
 * @class CBarrierStats
 *
 *  This class is an observer for complete barrier events. It
 *  auto-registers with ther fragment handler on construction.
 *  When a barrier is signalled it maintains the following
 *  statistics:
 *  - Number of barriers.
 *  - Number of homogeneous barriers (all fragments same barrier type).
 *  - Number of heterogeneous barriers.
 *  - Number of fragments of each barrier type
 *  - By each data source:
 *    +   Source id
 *    +   Total number of barrier fragments.
 *    +   Total number of fragments of each barrier type.
 *
 * Naturally a method is provided to fetch those statistics.
 */
class CBarrierStats : public CFragmentHandler::BarrierObserver
{
  // Public data types:
public:
  typedef struct _BarrierTypeStats {
    size_t                     s_barrierCount; // total barriers
    std::map<uint32_t, size_t> s_typeCount;    // frags by barrier type.
    _BarrierTypeStats() :		       // default constructor zeroes 
      s_barrierCount(0)	{}		       // all fields.
  } BarrierTypeStats, *pBarrierTypeStats;

  typedef struct _BarrierStats {
    size_t                               s_homogeneous;
    size_t                               s_heterogeneous;
    BarrierTypeStats                     s_barriers;
    std::map<uint32_t, BarrierTypeStats> s_sourceCounts;
    _BarrierStats() :		// Construction zeros.
      s_homogeneous(0),
      s_heterogeneous(0),
      s_barriers() {}

  } BarrierStats, *pBarrierStats;

  // Private data:

private:
  BarrierStats m_statistics;

  // Canonicals that are public:
public:
  CBarrierStats();
  virtual ~CBarrierStats();

  // Canonicals that are not supported.
private:
  CBarrierStats(const CBarrierStats&);
  CBarrierStats& operator=(const CBarrierStats&);
  int operator==(const CBarrierStats&) const;
  int operator!=(const CBarrierStats&) const;

  // Public interface (including observation method)
public:
  void          operator()(const std::vector<std::pair<uint32_t, uint32_t> >& typeList);
  BarrierStats  getStatistics() const;
  void          clear();
};

#endif
