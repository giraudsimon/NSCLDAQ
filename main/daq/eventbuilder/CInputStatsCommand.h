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

#ifndef CINPUTSTATSCOMMAND_H
#define CINPUTSTATSCOMMAND_H


#include <TCLObjectProcessor.h>
#include <string>


// Forward definitions:

class CTCLObject;


/**
 *  @class CInputStatsCommand
 * 
 *     This class provides a command that gets the input statistics from
 *     the event builder fragment queues.  The input statistics 
 *     describe the queues in a summary way.
 *     The command returns a list of the following form:
 * \verbatim
 *   {oldestTimestamp newestTimestamp totalFragcount queue-statistics}
 * \endverbatim
 *    Where:
 *    - oldestTimestamp is the timestamp of the oldest queued fragment and
 *    - newestTimestamp is similarly the timestamp of the newest queued fragment.
 *    - totalFragmentCount is the total number of fragments in all input queues.
 *    - queue-statistics is itself a list of detailed queue statistics. Each element
 *      is a sublist containing in order:
 *      # id    - the source id that is putting fragments in this queue.
 *      # depth - the number of fragments queued in this queue.
 *      # oldest- the timestamp of the fragment at the front of the queue.
 *                by the specifications of data sources, this is the
 *                oldest queued fragment from that data source.
 *      # bytes - the number of bytes in the queue.
 *      # dequeued -Number of bytes dequeued from the queue.
 *      # totalqueued - Cumulative bytes that have been put in the queue.
 *
 */
class CInputStatsCommand : public CTCLObjectProcessor 
{
  // Allowed canonicals.
public:
  CInputStatsCommand(CTCLInterpreter& interp, std::string command);
  virtual ~CInputStatsCommand();

  // Forbidden canonicals.
private:
  CInputStatsCommand(const CInputStatsCommand&);
  CInputStatsCommand& operator=(const CInputStatsCommand&);
  int operator==(const CInputStatsCommand&) const;
  int operator!=(const CInputStatsCommand&) const;

  // Virtual method overrides:

public:
  virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

};


#endif
