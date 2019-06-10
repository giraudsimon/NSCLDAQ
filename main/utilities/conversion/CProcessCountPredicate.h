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

#ifndef CPROCESSCOUNTPREDICATE_H
#define CPROCESSCOUNTPREDICATE_H

#include <CPredicate.h>

class CRunStatistics;

class CProcessCountPredicate: public CPredicate
{
  private:
    CRunStatistics& m_stats;
    size_t m_toProcess;
  public:
    CProcessCountPredicate(size_t nToProcess, CRunStatistics& stats);

    bool operator()();
};

#endif
