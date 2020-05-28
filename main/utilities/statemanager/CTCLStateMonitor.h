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
# @file   CTCLStateMonitor.h
# @brief  Implement Tcl state monitors.
# @author <fox@nscl.msu.edu>
*/


#ifndef CTCLSTATEMONITOR_H
#define CTCLSTATEMONITOR_H

#include "CStateMonitor.h"
#include <TCLObjectProcessor.h>
#include <tcl.h>
#include <map>



/**
 * Note that since the Tcl state monitor will require the monitor class itself
 * to live in a separate thread, we really need a couple of things here:
 * *  CTCLStateMonitor - derives from CStateMonitor and add synchronization so that
 *    registrations and unregistrations don't step on the asynchronous toes of
 *    dispatching state transitions and initial states.
 * *  CTCLStateMonitorCommand - Implements the statemon command for an interpreter
 *    which provides script level access to state monitoring as an event
 *    driven entity.
 * *  StateMonitor_Init  - package load handler to register the
 *    CTCLStateMonitorCommand with the interpreter that package requires it.
 *
 * What all this means is that CTCLStateMonitorCommand will create a thread
 * that will instantiate and run a CTCLStateMonitor.  CTCLStateMonitorCommand
 * will register handlers for states as directed by the script.  The handlers
 * will basically do a ThreadQueueEvent passing the script as a parameter.
 * The handler of that event will arrange execution of the script at global level.
 */



/**
 * @class CTCLStateMonitor
 *    This class wraps CStateMonitor calls in mutex lock/unlocks to make it
 *    thread-safe.
 */
class CTCLStateMonitor : public CStateMonitor {
private:
  TCL_DECLARE_MUTEX(m_lock);
public:
    CTCLStateMonitor(
        std::string transitionRequestURI, std::string statePublisherURI,
        CStateMonitorBase::InitCallback cb = 0
    );
    virtual ~CTCLStateMonitor();
    
public:
    virtual void Register(std::string state, Callback cb, void* cbarg);
    virtual void unregister(std::string state);
protected:
    virtual void initialState(std::string state);
    virtual void transition(std::string newState);
    
    
};

/**
 * @class CTCLStateMonitorCommand
 *
 * Provides the code for the statemonitor command.  This command
 * has several forms:
 *
 * statemonitor start  requestUri stateUri
 *
 * Starts the state manager thread using requestUri as the URI to the
 * state manager state change request port and stateUri as the state manager
 * state/transition publication port.
 * At this time only one state manager per interpreter is supported.  If this command
 * has been successfully invoked it is an error to invoke it again.
 *
 * statemonitor register state-name script
 *
 * Registers 'script' to be run if state-name is entered.  The script has
 * two parameters appended to it; the prior state and the state being entered.
 * If the prior state is not known and empty string is provided.
 *
 * statemonitor unregister state-name
 *
 * Unregisters the script associated with a state entry.
 *
 * statemonitor transition new-state
 *
 * Request the state manager to transition to new-state.
 *
 * statemonitor gettitle
 *
 * Get the current title. (empty string if there isn't one).
 *
 * satemonitor getrun
 *
 * Get the current run number (empty string if there isn't one).
 *
 * statemonitor titlecallback script
 *
 * Set the title callback.  An empty string removes any callback.
 *
 * statemonitor runnumcallback scripty
 *
 * Set the run number callback.  An empty string removes and callback.
 *
 * The title and run number callbacks have the new title/run number appended
 * to them when called.
 *  
 */
class CTCLStateMonitorCommand : public CTCLObjectProcessor
{
private:
    typedef struct _event {
        Tcl_Event                s_event;
        CTCLStateMonitorCommand* s_pCommand;
        char*                    s_pScript;
        char*                    s_prior;
        char*                    s_state;
    } event;
    
    typedef struct _RunEvent {
        Tcl_Event                s_event;
        int                      s_runNumber;
        CTCLStateMonitorCommand* s_this;
    } RunEvent, *pRunEvent;
    
    typedef struct _TitleEvent {
        Tcl_Event               s_event;
        char*                   s_pNewTitle;
        CTCLStateMonitorCommand* s_this;
    } TitleEvent, *pTitleEvent;
    
    typedef struct _RecordEvent {
        Tcl_Event              s_event;
        bool                   s_state;
        CTCLStateMonitorCommand* s_this;
    } RecordEvent, *pRecordEvent;
    
    typedef std::string *callbackInfo;
    
private:
    Tcl_ThreadId       m_monitorThread;
    Tcl_ThreadId       m_myThread;
    CTCLStateMonitor*  m_pStateMonitor;
    std::map<std::string, std::string*> m_allocationMap;
    
    // The data below are maintained here to avoid the need for
    // locks to acquire the data from the monitor thread.
    // We'll use our monitor events to maintain these.
    std::string        m_Title;    
    int                m_runNumber;
    bool               m_recording;
    
    // Scripts for run number and title changes:
    
    std::string       m_titleChangeScript;
    std::string       m_runNumberChangeScript;
    std::string       m_recordingChangeScript;
    
public:
    CTCLStateMonitorCommand(CTCLInterpreter& interp, std::string name);
    virtual ~CTCLStateMonitorCommand();
    
private:
    CTCLStateMonitorCommand(const CTCLStateMonitorCommand& rhs);
    CTCLStateMonitorCommand& operator=(const CTCLStateMonitorCommand& rhs);
    int operator==(const CTCLStateMonitorCommand& rhs);
    int operator!=(const CTCLStateMonitorCommand& rhs);
    
protected:
    virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    virtual void start(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    virtual void Register(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    virtual void unregister(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    virtual void requestTransition(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    virtual void getTitle(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    virtual void getRun(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    virtual void setTitleCallback(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    virtual void setRunNumCallback(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    virtual void getRecording(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    virtual void setRecordingCallback(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

    /* Private methods involved in hoisting the thread callback to a
       script invocation
    */
private:
    void  scriptCaller(std::string prior, std::string, std::string script);
    static int eventRelay(Tcl_Event* pEvPtr, int flags);
    static void callback(
        CStateMonitor* pMonitor, std::string prior, std::string state, void* cbParam
    );
    static void eventLoopThread(ClientData cd);
        
    bool canDispatch(std::string state, std::string script);
    static std::string toUpper(std::string in);

    // Mechanics for passing a new run number to us:
    
    static void queueRunNumberEvent(CStateMonitor* pMon, int run, void* pObj);
    static int  runNumberEvent(Tcl_Event* pEvent, int mask);
    void        runRunNumberScript(int runNumber);
    
    // Ditto for the new title:
    
    static void queueTitleEvent(CStateMonitor* pMon, std::string title, void *pObj);
    static int  titleEvent(Tcl_Event* pEvent, int mask);
    void        runTitleScript(std::string title);
    
    static void queueRecordingEvent(CStateMonitor* pMon, bool state, void* pObj);
    static int  recordingEvent(Tcl_Event* pEvent, int mask);
    void        runRecordingScript(bool state);
};

#endif
