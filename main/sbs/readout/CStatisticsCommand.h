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

/** @file:  CStatisticsCommand.h
 *  @brief: Provide the Tcl 'statistics' command.
 */
#ifndef CSTATISTICSCOMMAND_H
#define CSTATISTICSCOMMAND_H
#include "TCLObjectProcessor.h"
#include "CExperiment.h"

class CInterpreter;
class CTCLObject;


/**
 * @class CStatisticsCommand
 *    Provides the 'statistics' command.  This command returns the
 *    Readout's statistics.  This consists of a two element list. The
 *    first element of the list are the cumulative statistics (across all runs).
 *    the second element of the list are the per run statistics for the current
 *    or, if the run is halted most recent run.
 *    Each of those sublists has three elements containing in order:
 *    -   The number of times the trigger fired.
 *    -   The number of times the trigger resulted in an even (e.g. the user
 *        code did not reject the event).
 *    -   The number of bytes of event data read.  Note that this is number of
 *        body bytes and, while it counts the word count longword, it does not
 *        count either the body header or the ring item header.
 */
class CStatisticsCommand : public CTCLObjectProcessor
{
private:
    CExperiment* m_pExperiment;
public:
    CStatisticsCommand(
        CTCLInterpreter* pInterp, const char* name, CExperiment* pExperiment
    );
    virtual ~CStatisticsCommand();
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
private:
    void formatCounters(CTCLObject& result, const CExperiment::Counters& counters);
};

#endif