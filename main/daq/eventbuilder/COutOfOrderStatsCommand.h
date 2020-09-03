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

/** @file:  OutOfOrderStatsCommand.h
 *  @brief: Get statistics for out of order events.
 */
#ifndef OUTOFORDERSTATSCOMMAND_H
#define OUTOFORDERSTATSCOMMAND_H
#include <TCLObjectProcessor.h>
#include "CFragmentHandler.h"
#include <TCLObject.h>
#include <map>
/**
 * @class COutOutOfOrderStatsCommand
 *    This class uses a non-monotonic timestamp observer to
 *    gather out of order event statistics that can be fetched
 *    at he Tcl script level.  The following are maintained:
 *
 *    - Number of out of order fragments
 *    - Prior timestamp of most recent of these.
 *    - Timestamp of the most recent of these.
 *    - Same statistics organized by source id.
 * @note These statistics represent out of order fragments
 *      received on a specific source (sources are supposed to be
 *      time ordered).
 *      
 */
class COutOfOrderStatsCommand : public CTCLObjectProcessor {
private:
    struct OutOfOrderRecord {
        size_t   s_count;
        uint64_t s_priorTs;
        uint64_t s_Ts;
    };
    struct OutOfOrderStatistics {
        OutOfOrderRecord s_totals;
        std::map<uint32_t, OutOfOrderRecord> s_bySource;
    };
    
    // We'll establish this observer o maintain statistics.
    
    class MyObserver : public CFragmentHandler::NonMonotonicTimestampObserver {
    private:
        COutOfOrderStatsCommand* m_pCommand;
    public:
        MyObserver(COutOfOrderStatsCommand* pCommand);
        virtual void operator()(
            unsigned sourceid, std::uint64_t priorTimestamp, std::uint64_t thisTimestamp
        );
        
    };
    
    OutOfOrderStatistics m_Statistics;
    MyObserver           m_Observer;
public:
    COutOfOrderStatsCommand(CTCLInterpreter& interp, const char* command);
    virtual ~COutOfOrderStatsCommand();
    
    virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    void logOutOfOrder(
        unsigned sourceid, std::uint64_t priorTimestamp,
        std::uint64_t thisTimestamp
    );
    CTCLObject makeStatsList(CTCLInterpreter& interp, const OutOfOrderRecord& stats);
    CTCLObject uint64Obj(CTCLInterpreter& interp, uint64_t value);
};


#endif