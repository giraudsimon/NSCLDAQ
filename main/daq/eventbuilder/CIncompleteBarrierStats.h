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

#ifndef __CINCOMPLETEBARRIERSTATS_H
#define __CINCOMPLETEBARRIERSTATS_H

/**
 * @file CIncompleteBarrierStats.h
 * @brief Define CIncompleteBarriers statistics observer class.
 */

#ifndef __CFRAGMENTHANDLER_H
#include <CFragmentHandler.h>
#endif

/**
 * @class CIncompleteBarrierStass
 *
 *  Observer class which maintians statistics about the incomplete barriers that
 *  were encounted in the event orderer.  An incomplete barrier is one where
 *  barriers were not received.
 *  This class maintains the following statistics on incomplete barriers:
 *  - Number of barriers that were incomplete.
 *  - Number of homogeneous incomplete barriers
 *  - Number of heterogeneous incomplete barriers
 *  - histogram of the number of missing sources.
 *  - For each source, then number of times it was missing from a barrier.
 *
 * @note This class self-registers its objects as observers with the
 *       CFragmentHandler singleton.
 */
class CIncompleteBarrierStats : public CFragmentHandler::PartialBarrierObserver
{
  // public data types:

public:
  typedef struct _Statistics {
    size_t s_totalIncomplete;
    size_t s_homogeneous;
    size_t s_heterogeneous;
    std::map<uint32_t, size_t> s_missingHistogram;
    std::map<uint32_t, size_t> s_sourceMissingCount;
    
    _Statistics() :
      s_totalIncomplete(0), s_homogeneous(0), s_heterogeneous(0) {}
    
    
  } Statistics, *pStatistics;

  // Private data:

private:
  Statistics m_stats;

  // Implemented canonicals
public:
  CIncompleteBarrierStats();
  virtual ~CIncompleteBarrierStats();

  // unimplemented canonicals.

private:
  CIncompleteBarrierStats(const CIncompleteBarrierStats&);
  CIncompleteBarrierStats& operator=(const CIncompleteBarrierStats&);
  int operator==(const CIncompleteBarrierStats&) const;
  int operator!=(const CIncompleteBarrierStats&) const;

  // public methods, including the observer method:

public:
  void operator()(const std::vector<std::pair<uint32_t, uint32_t> >& barrierTypes, 
		  const std::vector<uint32_t>& missingSources);
  Statistics getStatistics() const;
  void clear();
  
};
#endif
