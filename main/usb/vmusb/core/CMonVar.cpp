/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CMonVar.cpp
 *  @brief: Provide support for monitored variables in the style of SBS Readout.
 */
#include "CMonVar.h"
#include <TCLObject.h>
#include <TCLInterpreter.h>
#include <TCLVariable.h>
#include <DataBuffer.h>
#include <DataFormat.h>

#include <stdexcept>
#include <algorithm>
#include <vector>
#include <DataBuffer.h>
#include <string.h>
#include <time.h>


/*-----------------------------------------------------------------------------
 *   CMonvarDictionary implementation.
 */

CMonvarDictionary* CMonvarDictionary::m_pInstance(0);

/**
 * constructor
 *    we need to implement this since bothered to declare
 *    it private so that we can make this a singleton.  It's implementation is
 *    empty.
 */
CMonvarDictionary::CMonvarDictionary()
{}

/**
 * getInstance
 *     Return a pointer to the object singleton instance.
 *     Creates it the at the first call.
 *
 * @return CMonvarDictionary*
 */
CMonvarDictionary*
CMonvarDictionary::getInstance()
{
    if (!m_pInstance) {
        m_pInstance = new CMonvarDictionary;
    }
    
    return  m_pInstance;
}
/**
 * add
 *    Adds a  new variable to the list.  The list is maintained in
 *    sorted order std::list::merge (a list of one is added).
 *
 *  @param variableName new variable to be added.
 *  @throw std::logic_error if variableName is a dup.
 */
void
CMonvarDictionary::add(const::std::string& variableName)
{
    // Does this entry exist in the list?
    
    if(std::find(
        m_monitoredVariableNames.begin(), m_monitoredVariableNames.end(),
        variableName) != m_monitoredVariableNames.end()) {
        throw std::logic_error(variableName + " Duplicate variable name");
    }
    // Make a single element list with variableName and merge the two
    
    std::list<std::string> newItem = {variableName};
    m_monitoredVariableNames.merge(newItem);
}
/**
 * remove
 *    Remove an item from the list.
 *
 *  @param variableName - string to remove.
 *  @throw std::logic_error if the variableName isn't in the list.
 */
void
CMonvarDictionary::remove(const std::string& variableName)
{
    std::list<std::string>::iterator p =
        std::find(
            m_monitoredVariableNames.begin(),
            m_monitoredVariableNames.end(),
            variableName
        );
    if (p == m_monitoredVariableNames.end()) {
        throw std::logic_error(variableName + " not in list, can't delete");
    }
    m_monitoredVariableNames.erase(p);
}
/**
 * get
 *    @return const std::list<std::string>&  - the list of variables.
 */
const std::list<std::string>&
CMonvarDictionary::get() const
{
    return  m_monitoredVariableNames;
}
/**
 * size
 *    Returns the number of elements in the dictionary.
 *
 *  @return size_t
 */
size_t
CMonvarDictionary::size() const
{
    return m_monitoredVariableNames.size();
}
/**
 * begin
 *    @return std::list<std::string>::iterator - 'pointing' to the first element
 *          of the list.
 */
std::list<std::string>::iterator
CMonvarDictionary::begin()
{
    return m_monitoredVariableNames.begin();
}
/**
 * end
 *   @return std::list<std::string>::iterator - 'pointing' past the end of the
 *              list (end of iteration sentinell).
 */
std::list<std::string>::iterator
CMonvarDictionary::end()
{
    return m_monitoredVariableNames.end();
}

/*--------------------------------------------------------------------------
 * CMonitorVariables class implementation.
 */

/**
 * constructor
 *   @param interp - references the interpreter that runs the timer and
 *                   in which the variable names will be looked up.
 *   @param ms     - Number of ms the timer runs for.
 */
CMonitorVariables::CMonitorVariables(CTCLInterpreter& interp, TCLPLUS::UInt_t msec) :
    CTCLTimer(&interp, msec)
{}
/**
 * operator()
 *    Called when the timer expires.  Note there's an interesting timing
 *    hole we're careful about.  
 *    -   We create the items
 *    -   Repropagate the timer.
 */
void
CMonitorVariables::operator()()
{
    createItems();
    Set();
}
/**
 * createItems
 *     Creates items into databuffers.  First we create a vector of
 *     scriptlets.  Scriptlets are produced using CTCLObject lists so that
 *     Tcl quoting is done right.  The scriptlets are set commands that
 *     would recreate the value of each variable.
 *     Once that's created we fill in as many TYPE_STRINGS data items
 *     as needed and queue them up for COutputThread to emit as ring items.
 */
void
CMonitorVariables::createItems()
{
    std::vector<std::string> scriptlets;
    const std::list<std::string>& varlist(
        CMonvarDictionary::getInstance()->get()
    );
    
    // Create the scriptlets
    
    for (auto p = varlist.begin(); p != varlist.end(); p++) {
        std::string s = createScript(*p);
        scriptlets.push_back(s);
    }
    // Now pack them into the buffers.
    
    int n = 0;
    DataBuffer* pBuffer(0);
    pStringsBuffer pHeader(0);
    char* pCursor(0);
    for (int i = 0; i < scriptlets.size(); i++) {
        if (!pBuffer) {
            pBuffer = gFreeBuffers.get();
            pHeader = reinterpret_cast<pStringsBuffer>(pBuffer->s_rawData);
            pCursor = initBuffer(pBuffer);
        }
        // If a scriptlet will fit put it in the buffer and  do the
        // appropriate book keeping (s_bufferSize and string count etc.).
        // If not back out of this one (decrement i), send the buffer to
        // the output thread and indicate we need a new output buffer.
        //
        uint32_t spaceNeeded = scriptlets[i].size() + 1; // Trailing null too.
        if (spaceNeeded < (pBuffer->s_storageSize - pBuffer->s_bufferSize)) {
            strcpy(pCursor, scriptlets[i].c_str());
            pCursor += spaceNeeded;
            pHeader->s_stringCount++;
            pBuffer->s_bufferSize += spaceNeeded;
        } else {
            i--;                 // Need another loop pass for this.
            gFilledBuffers.queue(pBuffer);
            pBuffer = 0;                // Force new allocation.
            pHeader = 0;
        }
    }
    // It's possible there's still data in the buffer:
    // If there are no scriptlets, pBuffer is nullptr.
    
    if (pBuffer && pBuffer->s_bufferSize) {
        gFilledBuffers.queue(pBuffer);
    }
    
}
/**
 * createScript
 *    Given a variable, create a script that gives it its current value.
 *    If the variable does not exist, the script sets the variable to
 *    *UNDEFINED*
 *
 *  @param var -name of the variable.
 *  @return std::string - a set command to restore the variable's value.
 */
std::string
CMonitorVariables::createScript(const std::string& var)
{
    CTCLInterpreter* pInterp = getInterpreter();
    CTCLVariable     tclVar(pInterp, var, TCLPLUS::kfFALSE);
    const char* pValue = tclVar.Get();
    if (!pValue) {
        pValue = "*UNDEFINED*";           // no value for the variable.
    }
    CTCLObject obj;
    obj.Bind(*pInterp);
    obj = "set";
    obj += var;
    obj += std::string(pValue);
    
    std::string result(obj);
    return result;
}
/**
 * initBuffer
 *    Given a new DataBuffer initialize it as a string buffer.
 *
 * @param pBuffer - data buffer (passed in as void* so we don't have to
 *                 define it in the header),
 * @return char* - Point in which to insert data.
 */
char*
CMonitorVariables::initBuffer(void* pBuffer)
{
    DataBuffer* p = static_cast<DataBuffer*>(pBuffer);
    pStringsBuffer pBody = reinterpret_cast<pStringsBuffer>(p->s_rawData);
    
    p->s_bufferSize = 0;                // NO data yet.
    p->s_bufferType = TYPE_STRINGS;
    p->s_timeStamp.tv_sec = time(nullptr);
    p->s_timeStamp.tv_nsec = 0;
    pBody->s_stringCount = 0;
    pBody->s_ringType = MONITORED_VARIABLES;
    return pBody->s_strings;
    
}
/*------------------------------------------------------------------------------
 *  CMonvarCommand implementation.
 **/

/**
 * constructor
 *    @param interp - interpreter on which the command is registered.
 *    @param command - name of the command.
 */
CMonvarCommand::CMonvarCommand(
    CTCLInterpreter& interp, const char* command
)  :   CTCLObjectProcessor(interp, command, TCLPLUS::kfTRUE)
{
    
}

/**
 * destructor
 *   Empty for now
 */
CMonvarCommand::~CMonvarCommand()
{}
/**
 * operator()
 *    Command processor fish out the subcommand and dispatch
 *
 * @param interp - interpreter reference.
 * @param objv   - CTCLObject encapsulated command line parameters.
 * @return int  - TCL_OK - ok or else TCL_ERROR
 * @note exception processing is used to do error management.
 */
int
CMonvarCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 2, "Insufficient parameters");
        std::string subcommand = objv[1];
        
        if (subcommand == "add") {
            add(interp, objv);
        } else if (subcommand == "remove") {
            remove(interp, objv);
        } else if (subcommand == "list") {
            list(interp, objv);
        } else {
            std::string msg = "Invalid subcommand: '";
            msg += subcommand;
            msg += "'";
            throw msg;
        }
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    
    
    return TCL_OK;
}

/**
 * add
 *    Add a new string to the dictionary.
 *    -  Require exactly three parameters.
 *    -  Add the item to the dict - that'll throw on any error.
 *
 *  @throw if failed.
 */
void
CMonvarCommand::add(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 3, "add - invalid number of arguments");
    std::string name = objv[2];
    CMonvarDictionary::getInstance()->add(name);
}
/**
 * remove
 *    Remove the specified string from the dictionary.
 *
 */
void
CMonvarCommand::remove(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 3, "remove - invalid number of arguments");
    std::string name = objv[2];
    CMonvarDictionary::getInstance()->remove(name);
}
/**
 * list
 *    List the names in the dictionary the optional glob pattern is used
 *    to allow a filter.
 */
void
CMonvarCommand::list(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireAtMost(objv, 3, "Remove invalid number of parameters");
    
    // Figure out the match pattern:
    
    std::string pattern="*";
    if (objv.size() == 3) pattern = std::string(objv[2]);
    
    CTCLObject result;
    result.Bind(interp);
    
    for (
        auto p = CMonvarDictionary::getInstance()->begin();
        p != CMonvarDictionary::getInstance()->end(); p++) {
        
        std::string item = *p;
        if (Tcl_StringMatch(item.c_str(), pattern.c_str())) {
            result += item;
        }
        
    }
    
    interp.setResult(result);
}

