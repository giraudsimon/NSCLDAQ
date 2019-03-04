/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

#include <CProcessCountPredicate.h>
#include <CRunStatistics.h>


CProcessCountPredicate::CProcessCountPredicate(size_t nToProcess, CRunStatistics& stats)
  : m_stats(stats), m_toProcess(nToProcess)
{}


bool CProcessCountPredicate::operator()()
{
  return (m_stats.getProcessCount() < m_toProcess);
}
