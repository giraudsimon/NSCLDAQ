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
* @file CRingFragmentSource.cpp
* @brief Implement the main logic of the ring fragment source.
*/
#include "CRingFragmentSource.h"
#include <CEventOrderClient.h>
#include <CRingBuffer.h>

#include <iostream>
#include <stdexcept>
#include <dlfcn.h>
#include <system_error>
#include <errno.h>
/**
 * constructor:
 *    Just sets up the data - the only interesting logic
 *    is turning a nonempty string of a timestamp extractor into a
 *    function pointer to the timestamp extraction function.
 *
 * @param client - Event builder client object.
 * @param dataSource - Ring buffer from which we're going to get data.
 * @param validIds   - List of ids we told the event builder we'll expect.
 * @param tsExtractorLib - Path to the shared object that has the timestamp
 *                     extraction function to be used if no header is seen.
 *                     This is an empty string? null pointer? if not used.
 * @param haveHeaders - The ring items have body headers.
 * @param endRunsExpected - number of end runs after which to exit
 *                       note that zero means don't exit.
 * @param endTimeoutSeconds -number of seconds after the first end run at which
 *                      to give up waiting for more.
 * @param timestampOffset - Offset to add to timestamps when creating the event builder
 *                      headers.  This allows for startup skew compensation while
 *                      keeping the glom --dt small.
 */
CRingFragmentSource::CRingFragmentSource(
    CEventOrderClient& client, CRingBuffer& dataSource, std::list<int> validIds,
    const char* tsExtractorLib, int haveHeaders, int endRunsExpected,
    int endTimeoutSeconds, int timestampOffset
) :
    m_client(client), m_dataSource(dataSource), m_tsExtractor(nullptr),
    m_expectBodyHeaders(haveHeaders != 0), m_isOneShot(endRunsExpected != 0),
    m_endsExpected(endRunsExpected), m_endRunTimeout(endTimeoutSeconds),
    m_timestampOffset(timestampOffset)
{
    setValidIds(validIds);
    setTsExtractor(tsExtractorLib);
}

/**
 * operator()
 * The application logic.
*/
void
CRingFragmentSource::operator()()
{
    
}
//////////////////////////////////////////////////////////////////////
// Private methods.

/**
 * setValidIds
 *    given the list of valid source ids, creates the m_validSids set
 * @param ids - list of valid sourceids.
 */
void
CRingFragmentSource::setValidIds(std::list<int>& ids)
{
    for (auto i :ids ) {
        m_validSids.insert(i);
    }
}
/**
 * setTsExtractor
 *   If the timestamp extractor library is supplied, load it and
 *   locate the timestamp function.  If that's found set m_tsExtrator.
 *   If not an error is emitted and we throw an exception.
 *
 *   Note that it's not an error not to supply a timestamp extractor unless
 *   m_expectBodyHeaders is false.
 *
 * @param tsExtractorLib -path to the .so that has the timestamp extractor.
 */
void
CRingFragmentSource::setTsExtractor(const char* tsExtractorLib)
{
    if (tsExtractorLib == nullptr) {
        if (!m_expectBodyHeaders) {
            std::cerr << "If --expectbodyheaders is not supplied --timestampextractor is required\n";
            throw std::logic_error(
                "If --expectbodyheaders is not supplied --timestampextractor is required"
            );
        }
        return;                      // NO timestamp extractor is ok if we have body headers.
    }
    std::string tsLib(tsExtractorLib);
    if (tsLib == "") {
        if (!m_expectBodyHeaders) {
           std::cerr << "If --expectbodyheaders is not supplied --timestampextractor is required\n";
            throw std::logic_error(
                "If --expectbodyheaders is not supplied --timestampextractor is required"
            );
        }
        return;   
    }
    // Access the timestamp extractor.  From here on in,
    // all errors are also fatal:
    
    void* pDll = dlopen(tsLib.c_str(), RTLD_NOW);
    if(!pDll) {
        std::string msg("Failed to open shared timestamp extractor library: ");
        msg += tsLib;
        msg += " ";
        msg += dlerror();
        std::cerr << msg << std::endl;
        throw std::logic_error(msg);        
    }
    void* timestamp = dlsym(pDll, "timestamp");
    if (!timestamp) {
        std::string msg(
            "Failed to find the 'timestamp' function in the extractor library: ");
        msg += tsLib;
        msg += " ";
        msg += dlerror();
        throw std::logic_error(msg);
    }
    
    // Set m_tsExtractor:
    
    m_tsExtractor = reinterpret_cast<timestampExtractor>(timestamp);
}