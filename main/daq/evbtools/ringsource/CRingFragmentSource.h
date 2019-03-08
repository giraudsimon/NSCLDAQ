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

/**
* @file CRingFragmentSource.h
* @brief application logic of ring fragment source.
*/
#ifndef CRINGFRAGMENTSOURCE_H
#define CRINGFRAGMENTSOURCE_H
#include <DataFormat.h>
#include <stdint.h>
#include <list>
#include <set>

class CEventOrderClient;
class CRingBuffer;

/**
 * @class CRingFragmentSource
 *    This class contains the main application logic of the ring
 *    fragment source application
 */
class CRingFragmentSource
{
public:
    typedef uint64_t  (*timestampExtractor)(pPhysicsEventItem);
private:
    CEventOrderClient& m_client;
    CRingBuffer&       m_dataSource;
    std::set<uint32_t> m_validSids;
    timestampExtractor m_tsExtractor;
    bool               m_expectBodyHeaders;
    bool               m_isOneShot;
    int                m_endsExpected;
    int                m_endRunTimeout;
    uint64_t           m_timestampOffset;
public:
    CRingFragmentSource(
        CEventOrderClient& client, CRingBuffer& dataSource, std::list<int> validIds,
        const char* tsExtractorLib, int haveHeaders, int endRunsExpected,
        int endTimeoutSeconds, int timestampOffset
    );
    
    void operator()();

    // private utilties
private:
    void setValidIds(std::list<int>& ids);
    void setTsExtractor(const char* tsExtractorLib);
};


#endif