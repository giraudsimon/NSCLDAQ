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
#ifndef CDLATESTATSCOMMAND_H
#define CDLATESTATSCOMMAND_H


#include <TCLObjectProcessor.h>



#include <stdint.h>
#include <map>

// forward definitions:

class CTCLObjectProcessor;
class CTCLObject;
class CDataLateStatistics;

/**
 * @class CDLateStatsCommand
 *   
 * This class provides the ability to monitor data late events, maintain statistics
 * and fetch them at the Tcl level.
 *
 * Data late information is maintained as:
 *  - A total number of data lates.
 *  - The worst case data late (largest time difference between newest fragment and
 *    late fragment.
 *  - The per data source data late counts (for data sources with data lates).
 *  - The worst case data late per source.
 *
 * The command provided is:
 *
 * \verbatim
 *   dlatestats
 * \endverbatim
 */
class CDLateStatsCommand : public CTCLObjectProcessor
{
  // Private data:

private:
  CDataLateStatistics* m_pStatistics;

  // Legal canonical methods:

public:
  CDLateStatsCommand(CTCLInterpreter& interp, std::string command);
  virtual ~CDLateStatsCommand();

  // illegal canonical methods:

private:
  CDLateStatsCommand(const CDLateStatsCommand&);
  CDLateStatsCommand& operator=(const CDLateStatsCommand&);
  int operator==(const CDLateStatsCommand&) const;
  int operator!=(const CDLateStatsCommand&) const;

  // virtual method overrides:

public:
  virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

  
};


#endif
