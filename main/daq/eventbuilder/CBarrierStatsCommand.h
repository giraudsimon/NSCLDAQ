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

#ifndef CBARRIERSTATSCOMMAND_H
#define CBARRIERSTATSCOMMAND_h

/**
 * @file CBarrierStatsCommand.h
 * @brief Header that defines the CBarrierStatsCommand class
 *        Tcl command to get barrier statistics.
 */


#include <TCLObjectProcessor.h>


// These headers must be included because C++ does not allow
// nested class typedefs.


#include "CBarrierStats.h"
#include "CIncompleteBarrierStats.h"

// Forward class/type definitions:

class CTCLInterpreter;
class CTCLObject;


/**
 * @class CBarrierStatsCommand
 *
 *   This class implements the 'barrierstats' command. 
 *   The command fetches statistics about the barriers fragments
 *   that have been processed by the event orderer.  The output of this
 *   is a two element list.  The first element of the list provides
 *   the statistics for complete barriers.  The second element provides
 *   the statistics for incomplete barriers.
 *
 *   Command format:
 *
 * \code{.tcl}
 *     barrierstats
 * \endcode
 *   Complete barrier statistics are in turn a list with the following elements:
 *   - number of complete barriers processed.
 *   - number of complete homogeneous barriers.
 *   - number of complete heterogeneous barriers.
 *   - Number of barrier fragments of each type.  This is a list where each element contains
 *     + A barrier type.
 *     + The number of barriers of that type seen.
 *   - Statistics by event source, which is a list, where each element is:
 *     + Source id.
 *     + Barrier fragments from that source.
 *     + List describing the fragments of each type from this source:
 *        - Barrier fragment type.
 *        - Number of that type
 *  
 *  Incomplete barrier statistics are also a list containingthe following elements:
 *  - Number of incomplete barriers.
 *  - Number of homogeneous incomplete barriers.
 *  - Number of heterogeneous incomplete barriers.
 *  - histogram showing how many sources were missing, this is represented as a list where each
 *    list element is a pair:
 *    +  n
 *    +  Number of times n sources were missing from the barrier.
 *  - Source statstics.  This is a list containing:
 *    + Source id.
 *    + Number of times the source was missing from a barrier.
 */
class CBarrierStatsCommand : public CTCLObjectProcessor
{
  // Private data:

private:
  CBarrierStats*            m_pCompleteStats;
  CIncompleteBarrierStats*  m_pIncompleteStats;

  // Canonicals for which there are implementations.

public:
  CBarrierStatsCommand(CTCLInterpreter& interp, std::string commandName);
  virtual ~CBarrierStatsCommand();

  // Canonicals for which there will not be implementations.

private:
  CBarrierStatsCommand(const CBarrierStatsCommand&);
  CBarrierStatsCommand& operator=(const CBarrierStatsCommand&);
  int operator==(const CBarrierStatsCommand&) const;
  int operator!=(const CBarrierStatsCommand&) const;

  // public entries.

public:
  int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

  // utilities:

private:
  CTCLObject* completeStats(CTCLInterpreter& interp);
  CTCLObject* incompleteStats(CTCLInterpreter& interp);
  CTCLObject* compileBarrierStats(CTCLInterpreter& interp, CBarrierStats::BarrierTypeStats& typeStats);

  void freeObjects();
};


#endif
