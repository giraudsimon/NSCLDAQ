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
# @file   CStateMonitorThread.cpp
# @brief  Implement state monitor in autonomous thread.
# @author <fox@nscl.msu.edu>
*/
#include <CStateMonitorThread.h>
#include <SyncGuard.h>
#include <stdio.h>

/**
 *  The following is a private derivationof CStateMonitor which can
 *  synchronize with its containing thread.
 *
 */

class CThreadEncapsulatedStateMonitor : public CStateMonitor
{
private:
    Synchronizable& m_monitor;            // Shared with thread..hence reference.
    
public:
    CThreadEncapsulatedStateMonitor(
        std::string reqUri, std::string pubUri, Synchronizable& monitor
        ) : CStateMonitor(reqUri, pubUri),
            m_monitor(monitor)
    {}
    
  
   // Protected overrides
protected:
    virtual void initialState(std::string state);
    virtual void transition(std::string transition);
    virtual void runNumMsg(std::string body);
    virtual void titleMsg(std::string body);
    virtual void recordMsg(std::string body);  
};

// Each of the protected members just locks the m_monitor Sync object
// and calls the base class's member


void
CThreadEncapsulatedStateMonitor::initialState(std::string state)
{
    SyncGuard monitor(m_monitor);
    CStateMonitor::initialState(state);
}
void
CThreadEncapsulatedStateMonitor::transition(std::string transition)
{
    SyncGuard monitor(m_monitor);
    CStateMonitor::transition(transition);
}
void
CThreadEncapsulatedStateMonitor::runNumMsg(std::string body)
{
    SyncGuard monitor(m_monitor);
    CStateMonitor::runNumMsg(body);
}
void
CThreadEncapsulatedStateMonitor::titleMsg(std::string body)
{
    SyncGuard monitor(m_monitor);
    CStateMonitor::titleMsg(body);
}
void
CThreadEncapsulatedStateMonitor::recordMsg(std::string body)
{
    SyncGuard monitor(m_monitor);
    CStateMonitor::recordMsg(body);
}
/*-----------------------------------------------------------------------------

/**
 * constructor
 *    Creates the underlying state monitor.
 *
 *   @param requestUri - URI to which requests are directed (REQ/RES).
 *   @param publishUri - URI that publishes state changes (PUB/SUB).
 */
CStateMonitorThread::CStateMonitorThread(
    std::string requestUri, std::string publishUri
):
    Thread("State monitor"),
    m_pStateMonitor(0)
{
    m_pStateMonitor =  new CStateMonitor(requestUri, publishUri);
}
/**
 * destructor
 *    Better only call this if the thread has exited and you joined it
 *    because we're going to wipe out the monitor.
 */
CStateMonitorThread::~CStateMonitorThread()
{
    delete m_pStateMonitor;    
}
/**
 * Register
 *
 *   Registers a transition callback handler for transitions into the
 *   specified state.  Note that the callback handler will be invoked in
 *   the thread of the state monitor.  Furthermore it will run in a
 *   critical setion.
 *
 * @param state - the desired state.
 * @param cb    - The funtion to call back.
 * @param cd    - optional callback data.
 */
void
CStateMonitorThread::Register(
    std::string state, CStateMonitor::Callback cb, void* cbarg
)
{
    SyncGuard monitor(m_monitor);       // Lock until destruction
    m_pStateMonitor->Register(state, cb, cbarg);
    
}
/**
 * unregister
 *   Removes callback registration for a state.
 *
 *  @param state - the state that will become callback free
 */
void
CStateMonitorThread::unregister(std::string state)
{
    SyncGuard monitor(m_monitor);
    m_pStateMonitor->unregister(state);
}
/**
 * setTitleCallback
 *    Set a callback to fire when the title is changed.
 *    Note that the callback handler will be invoked in
 *   the thread of the state monitor.  Furthermore it will run in a
 *   critical setion.
 *
 *  @param cb - Pointe to the callback function.
 *  @param cd - Callback data.
 */
void
CStateMonitorThread::setTitleCallback(CStateMonitor::TitleCallback cb, void* cd)
{
    SyncGuard monitor(m_monitor);
    m_pStateMonitor->setTitleCallback(cb, cd);
}
/**
 * setRunNumberCallback
 *   sets a callback to be invoked when the run number changes
 *   Note that the callback handler will be invoked in
 *   the thread of the state monitor.  Furthermore it will run in a
 *   critical setion.
 *
 *  @param cb - Pointe to the callback function.
 *  @param cd - Callback data.
 */
void
CStateMonitorThread::setRunNumberCallback(CStateMonitor::RunNumberCallback cb, void* cd)
{
    SyncGuard monitor(m_monitor);
    m_pStateMonitor->setRunNumberCallback(cb, cd);
}
/**
 * setRecordingCallback
 *    Sets a callabck to be invoked when the recording state changes.
 *    Note that the callback handler will be invoked in
 *   the thread of the state monitor.  Furthermore it will run in a
 *   critical setion.
 *
 *  @param cb - Pointe to the callback function.
 *  @param cd - Callback data.
 */
void
CStateMonitorThread::setRecordingCallback(CStateMonitor::RecordingCallback cb, void* cd )
{
    SyncGuard monitor(m_monitor);
    m_pStateMonitor->setRecordingCallback(cb, cd);
}

/**
 * getState
 *    Get the current state from the monitor.
 *
 * @return std::string current state.
 * @note If the state is not yet known this will return an empty string.
 */
std::string
CStateMonitorThread::getState()
{
    SyncGuard monitor(m_monitor);
    return m_pStateMonitor->getState();
}
/**
 * getTitle
 *     Returns the current title or an empty string if the title is not yet
 *     known or not yet set.
 *
 * @return std::string
 */
std::string
CStateMonitorThread::getTitle()
{
    SyncGuard monitor(m_monitor);
    return m_pStateMonitor->getTitle();
}
/**
 * getRunNumber
 *    Return the current run number or -1 if the run number is not yet
 *    known.
 *
 * @return int
 */
int
CStateMonitorThread::getRunNumber()
{
    SyncGuard monitor(m_monitor);
    return m_pStateMonitor->getRunNumber();
}
/**
 * getRecording
 *
 *  @return Boolean - State of the recording variable.
 *  
 */
bool
CStateMonitorThread::getRecording()
{
    SyncGuard monitor(m_monitor);
    return m_pStateMonitor->getRecording();
}
/**
 * RequestTransition
 *
 * @param transition - name of the transition to request.
 * @return std::string - response from the server.
 */
std::string
CStateMonitorThread::requestTransition(std::string transition)
{
    SyncGuard monitor(m_monitor);
    return m_pStateMonitor->requestTransition(transition);
}
/**
 * setTitle
 *    Requests that a new title be set for the run.
 *
 *  @param title - the title string.
 *  @return std::string  - the response from the server.
 */
std::string
CStateMonitorThread::setTitle(std::string title)
{
    std::string msg = "TITLE:";
    msg += title;
    SyncGuard monitor(m_monitor);
    m_pStateMonitor->sendRequestMessage(msg);
}
/**
 * setRun
 *   Set the run number
 *
 *  @param run - The new run number.
 *  @return std::string - server response.
 */
std::string
CStateMonitorThread::setRun(int run)
{
    char buffer[1000];
    sprintf(buffer, "RUN:%d", run);
    SyncGuard monitor(m_monitor);
    m_pStateMonitor->sendRequestMessage(std::string(buffer));
}
/**
 * setRecording
 *   Set the state of the recording variable.
 *
 *   @param state - requested state.
 *   @return std::string - The server's reply
 */
std::string
CStateMonitorThread::setRecording(bool state)
{
    std::string msg = "RECORD:";
    if (state) {
        msg += "True";
    } else {
        msg += "False";
    }
    SyncGuard monitor(m_monitor);
    m_pStateMonitor->sendRequestMessage(msg);
    
}
/**
 * run
 *    Entry point for the thread, just start the monitor.
 */
void
CStateMonitorThread::run()
{
    m_pStateMonitor->run();
}


