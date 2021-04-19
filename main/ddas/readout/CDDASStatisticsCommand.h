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

/** @file:  CDDASStatisticsCommand.h
 *  @brief: Provide DDAS specific 'statistics' command for getting statistics.
 */



#ifndef CDDASSTATISTICSCOMMAND_H
#define CDDASSTATISTICSCOMMAND_H
#include <TCLObjectProcessor.h>
#include <vector>

class CMyEventSegment;
class CMyScaler;

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CDDASStatisticsCommand
 *     Provides a statistics command processor that overrides the SBS one.
 *     We need to override because the concept of a trigger within DDAS is
 *     completely different than the triggering used to invoke the readouts.
 *     For DDAS readout, the triggering information comes from the module
 *     pseudo scalers.  Thus we'll function by grabbing byte statistics from
 *     CMyEventSegment and trigger information from the collection of
 *     CMyScaler objects.
 *
 *     @note 
 */
class CDDASStatisticsCommand : public CTCLObjectProcessor
{
private:
    CMyEventSegment*        m_pEventSegment;
    std::vector<CMyScaler*>& m_Scalers;
public:
    CDDASStatisticsCommand(
        CTCLInterpreter& interp, const char* command, CMyEventSegment* pSeg,
        std::vector<CMyScaler*>& scalers
    );
    virtual ~CDDASStatisticsCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
private:
    void formatResult(
        CTCLInterpreter& interp, CTCLObject& result,
        size_t bytes, size_t runBytes
    );
    void formatCounters(
        CTCLObject& result, size_t triggers, size_t accepted, size_t bytes
    );
};

#endif
