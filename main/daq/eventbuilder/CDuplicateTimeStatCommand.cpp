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
# @file   CDuplicateTimeStatCommand.cpp
# @brief  Implementation of the CDuplicateTimeStatCommandn class.
# @author <fox@nscl.msu.edu>
*/

#include "CDuplicateTimeStatCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CFragmentHandler.h"
#include <map>
#include <vector>

// Internal class; an observer that accumulates statistics.

class CDuplicateTimestampStatsObserver : public CFragmentHandler::DuplicateTimestampObserver
{
public:
    typedef struct _SourceStatistics {
        uint32_t s_sourceId;
        size_t   s_counter;
        uint64_t s_lastDupTimestamp;
      _SourceStatistics() :
        s_sourceId(0), s_counter(0), s_lastDupTimestamp(0) {}
    } SourceStatistics, *pSourceStatistics;
    
private:
    size_t                               m_totalDupCounter;
    std::map<uint32_t, SourceStatistics> m_perSourceStatistics;
public:
    void operator()(uint32_t sourceId, uint64_t timestamp);
    
    size_t                        getTotalDupCount()      const;
    std::vector<SourceStatistics> getPerSourceStatistics() const;
    void clear();
};

// Implementationof CDuplicateTimeStatsObjserver:

// Observer function update the statistics:

void
CDuplicateTimestampStatsObserver::operator()(uint32_t sourceId, uint64_t timestamp)
{
    m_totalDupCounter++;
    SourceStatistics& sourceStats(m_perSourceStatistics[sourceId]);
    sourceStats.s_sourceId = sourceId;                    // In case this is new.
    sourceStats.s_counter++;
    sourceStats.s_lastDupTimestamp = timestamp;
}

// Get the global count statistics:

size_t
CDuplicateTimestampStatsObserver::getTotalDupCount() const
{
    return m_totalDupCounter;
}
// Marshall the map into a vector of per source statistics:

std::vector<CDuplicateTimestampStatsObserver::SourceStatistics>
CDuplicateTimestampStatsObserver::getPerSourceStatistics() const
{
    std::vector<SourceStatistics> result;
    std::map<uint32_t, SourceStatistics>::const_iterator p =
        m_perSourceStatistics.begin();
    while (p != m_perSourceStatistics.end()) {
        result.push_back(p->second);
        
        p++;
    }
    return result;
}

void
CDuplicateTimestampStatsObserver::clear()
{
    m_totalDupCounter = 0;
    m_perSourceStatistics.clear();
}
/*-------------------------------------------------------------------------------
 * The main class implementation.
 */

/**
 * constructor
 *    Register the command, create our observer and register _that_ as well.
 *
 *  @param interp - refers to the interpreter we are going to register with.
 *  @param cmd    - the command name string we will register as.
 */
CDuplicateTimeStatCommand::CDuplicateTimeStatCommand(CTCLInterpreter& interp, std::string cmd) :
    CTCLObjectProcessor(interp, cmd, true), m_pObserver(0)
{
    m_pObserver = new CDuplicateTimestampStatsObserver;
    m_pObserver->clear();
    CFragmentHandler::getInstance()->addDuplicateTimestampObserver(m_pObserver);
}
/**
 * destructor
 *    Remove our observer and delete it.
 */
CDuplicateTimeStatCommand::~CDuplicateTimeStatCommand()
{
    CFragmentHandler::getInstance()->removeDuplicateTimestampObserver(m_pObserver);
    delete m_pObserver;
    m_pObserver = 0;
}


/**
 * operator()
 *    Gets control when the command is invoked in our interpreter.
 *    *  Ensure that we have have sufficient command parameters for the keyword.
 *    *  Dispatch to the command handler for the keyword used.
 *
 *  @note as we've been doing recently we'll use exception handling to simplify
 *        error return handling.  The exceptions thrown will be std::string messages
 *
 * @param interp - the interpreter running us.
 * @param objv   - The vector of command words.
 * @return int TCL_OK on success, TCL_ERROR on failure.
 */
int
CDuplicateTimeStatCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 2, "Insufficient command parameters");
        
        std::string subcommand = objv[1];
        if (subcommand == "get") {
            get(interp, objv);
        } else if (subcommand == "clear") {
            clear(interp, objv);
        } else {
            throw std::string(
                "Invalid subcommand keyword, must be either 'get' or 'clear'"
            );
        }
        
    } catch(std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    return TCL_OK;
}

/**
 * clear
 *   Clears the observer's statistics.
 * 
 * @param interp - the interpreter running us.
 * @param objv   - The vector of command words.
 */
void
CDuplicateTimeStatCommand::clear(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "Incorrect number of command parameters");
    m_pObserver->clear();
}
/**
 * get
 *    Get the current statistics.
 *    The result is set to a two element list.  The first element will be the
 *    total number of consecutive duplicate timestamps from a data source.
 *    The second will be a list contaning sublists that have sourceId, counter
 *    of duplicates and the most recent timestamp duplicated.
 *
 * @param interp - the interpreter running us.
 * @param objv   - The vector of command words.
 */
void
CDuplicateTimeStatCommand::get(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "Incorrect number of command parameters");
    
    // Create the result object 
    
    CTCLObject result;
    result.Bind(interp);
    
    
    // First list element is the total duplicate count:
    
    result += static_cast<int>(m_pObserver->getTotalDupCount());

    // Now build the per source statistics:
    
    CTCLObject perSourceStats;
    perSourceStats.Bind(interp);
    std::vector<CDuplicateTimestampStatsObserver::SourceStatistics> statVector =
        m_pObserver->getPerSourceStatistics();
        
    for (int i = 0; i < statVector.size(); i++) {
        CTCLObject statElement;
        statElement.Bind(interp);
        
        statElement += static_cast<int>(statVector[i].s_sourceId);
        statElement += static_cast<int>(statVector[i].s_counter);
        statElement += uint64Object(interp, statVector[i].s_lastDupTimestamp);
        
        
        perSourceStats += statElement;
    }
    
    result += perSourceStats;
    interp.setResult(result);
    
}
/**
 * uint64Object
 *    Create a CTCLObject from a uin64_t.  This is not (yet?) supported in
 *    CTCLObject::operator().  What we're going to do is create a WideInt
 *    Tcl_Obj* and then use _that_ to construct the CTCLObject we are returning.
 *    
 *  @param interp -references the interpreter to which the CTCLObject is bound.
 *  @param value  - the uint64_t we're binding.
 *  @return CTCLObject
 *
 */
CTCLObject
CDuplicateTimeStatCommand::uint64Object(CTCLInterpreter& interp, uint64_t value)
{
    Tcl_Obj* nativeObject = Tcl_NewWideIntObj(value);
    CTCLObject wrappedObject(nativeObject);
    wrappedObject.Bind(interp);
    
    return wrappedObject;
}