/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CDuplicateTimeStatCommand
# @brief  defines a class for a command that returns info about duplicate timestamps.
# @author <fox@nscl.msu.edu>
*/

#ifndef CDUPLICATETIMESTATCOMMAND_H
#define CDUPLICATETIMESTATCOMMAND_H


#include <TCLObjectProcessor.h>
#include <stdint.h>


// Forward definitions:

class CTCLInterpreter;
class CTCLObject;
class CDuplicateTimestampStatsObserver;

/**
 * @class CDuplicateTimeStatCommand
 *    Implements a command that obtains statistics about cases where a duplicate
 *    timestamp was observed coming from a single queue in consecutive fragments.
 *    The command returns a list containing:
 *    *  The number of duplicates seen
 *    *  A list for each data source that had a duplicate that contains
 *    *  the source id, a count of duplicates and the timestamp most recently duplicated.
 */
class CDuplicateTimeStatCommand : public CTCLObjectProcessor
{
private:
    CDuplicateTimestampStatsObserver* m_pObserver;
    
    // Legal canonicals
    
public:
    CDuplicateTimeStatCommand(CTCLInterpreter& interp, std::string command);
    virtual ~CDuplicateTimeStatCommand();
    
    // Forbidden canonicals.

private:
    CDuplicateTimeStatCommand(const CDuplicateTimeStatCommand& rhs);
    CDuplicateTimeStatCommand& operator=(const CDuplicateTimeStatCommand& rhs);
    int operator==(CDuplicateTimeStatCommand& rhs) const;
    int operator!=(CDuplicateTimeStatCommand& rhs) const;
    
    // command entry point:
    
public:
  int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

  // command execution methods:

private:
  void clear(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
  void get(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
  CTCLObject uint64Object(CTCLInterpreter& interp, uint64_t value);
    
};

#endif


