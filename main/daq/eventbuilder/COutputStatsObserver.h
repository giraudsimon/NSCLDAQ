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

#ifndef __COUTPUTSTATSOBSERVER_H
#define __COUTPUTSTATSOBSERVER_H

#ifndef __CFRAGMENTHANDLER_H
#include "CFragmentHandler.h"
#endif

#ifndef __TCL_H
#include <tcl.h>
#ifndef __TCL_H
#define __TCL_H
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#endif
#endif


#ifndef __STL_MAP
#include <map>
#ifndef __STL_MAP
#define __STL_MAP
#endif
#endif

#include <CMutex.h>

/**
 * @class COutputStatsObserver
 *
 * The output statistics observer is instantiated to provide
 * statistics counters for the output stages.
 * The instantiation is normally by the COutputStatsCommand class which
 * provides the Tcl command to get the statistics.
 * 
 * This class:
 * # Gathers statistics from the flow of output data.
 * # Provides a method to allow the statistics to be fetched.
 */
class COutputStatsObserver : public CFragmentHandler::Observer
{

public:
  typedef struct _statistics {
    uint64_t                                    s_nTotalFragments;
    std::vector<std::pair<uint32_t, uint64_t> > s_perSourceFragments;

  } Statistics, *pStatistics;


  // private types/data:
private:
  typedef std::map<uint32_t, uint64_t> PerSourceStats, *pPerSourceStats;
private:
 
  uint64_t                     m_nTotalFragments;
  
  CMutex                       m_perSourceStatGuard;
  PerSourceStats               m_perSourceStatistics;

  // Public type definitions:


  // Canonical member methods:

public:
  COutputStatsObserver();
  virtual ~COutputStatsObserver();

  // forbidden canonicals:

private:
  COutputStatsObserver(const COutputStatsObserver&);
  COutputStatsObserver& operator=(const COutputStatsObserver&) const;
  int operator==(const COutputStatsObserver&) const;
  int operator!=(const COutputStatsObserver&) const;

  // Object methods:
public:
  virtual void operator()(const std::list<std::pair<time_t, EVB::pFragment> >& event);
  void clear();
  Statistics getStatistics() const;
};


#endif
