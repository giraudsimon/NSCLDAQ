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
# @file   CStateMonitorThread.h
# @brief  Defines mechanisms for putting a state monitor in a thread
# @author <fox@nscl.msu.edu>
*/
#ifndef CSTATEMONITORTHREAD_H
#define CSTATEMONITORTHREAD_H

#include <Thread.h>
#include <Synchronizable.h>
#include <string>
#include <CStateMonitor.h>



/**
 * @class CStateMonitorThread
 *   A CStateMonitorThread object is a thread that runs a state monitor
 *   event loop.  This class is useful if you need to interact with the
 *   State manager in an application that either has no event loop or,
 *   alternatively, whose event loop cannot easily be extended to incorporate
 *   the event loop needed to run a CStateMonitor directly.
 *
 * @note that all callbacks that can be registered are:
 *  *  Run in the state monitor thread and responsible for doing their own
 *     inter-thread synchronization.
 *  *  Run with the state monitors synchronizable held.
 */

class CStateMonitorThread : public Thread
{
private:
    Synchronizable     m_monitor;
    CStateMonitor*     m_pStateMonitor;
    
public:
    CStateMonitorThread(std::string requestUri, std::string publishUri);
    virtual ~CStateMonitorThread();
    
public:
    virtual void Register(std::string state, CStateMonitor::Callback cb, void* cbarg);
    virtual void unregister(std::string state);
    virtual void         setTitleCallback(CStateMonitor::TitleCallback cb, void* cd = 0);
    virtual void         setRunNumberCallback(CStateMonitor::RunNumberCallback cb, void* cd = 0);
    virtual void         setRecordingCallback(CStateMonitor::RecordingCallback cb, void* cd = 0);
    
    std::string getState();
    std::string getTitle();
    int         getRunNumber();
    bool        getRecording();
    
    std::string requestTransition(std::string transition);
    std::string setTitle(std::string title);
    std::string setRun(int run);
    std::string setRecording(bool state);
    
public:
    virtual void run();            // Start the monitor.
    
 
        
    
};
#endif