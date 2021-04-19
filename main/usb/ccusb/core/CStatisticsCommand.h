/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:   CStatisticsCommand.h
 *  @brief:  Provides the 'statistics' command.
 */
#ifndef CSTATISTICSCOMMAND_H
#define CSTATISTICSCOMMAND_H
#include <TCLObjectProcessor.h>
#include "COutputThread.h"

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CStatisticsCommand
 *    Creates on demand statistics of trigger and sizes:
 *    The command returns two lists. THe first list contains the
 *    cumulative statistics while the second list contains the statistics
 *    from the run in progress or most recent run if the run is not active.
 *    Each of the lists has three elements that are, in order:
 *    -   The number of triggers
 *    -   The number of accepted triggers. XXUSB devices have no method for
 *        rejecting triggers so this will be the same as the number of triggers.
 *    -   The number of bytes of event data put into the ring buffer.  This
 *        is exclusive of body headers and ring item headers.
 */
class CStatisticsCommand : public CTCLObjectProcessor
{
public:
    CStatisticsCommand(CTCLInterpreter& interp, const char* command);
    virtual ~CStatisticsCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
private:
    void formatCounters(CTCLObject& result, const COutputThread::Counters& c);
};
#endif