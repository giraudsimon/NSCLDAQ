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
# @file   Actions.cpp
# @brief  Implementation of the actions class.
# @author <fox@nscl.msu.edu>
*/
#include "Actions.h"

#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

// Define the static data:

CBufferQueue<Actions::ActionItem> Actions::m_actionQueue;
CSynchronizedThread*              Actions::m_pOutputThread(0);  // not initialzied.


/*------------------------------------------------------------------------------
 * Public interfaces:
 */


/**
 * Error
 *    Display an error message on our ReadoutGUI tab.
 *
 *  @param message - the message to display
 */
void
Actions::Error(std::string message)
{
    queueMessage(ErrorMessage, message);
}
/**
 * Log
 *    Display a log message on our readout gui tab.
 *
 *   @param message - message to display.
 */
void
Actions::Log(std::string message)
{
    queueMessage(LogMessage, message);
}
/**
 * Warning
 *    Display a warning message on our readout gui tabl
 *
 *  @param message - message to display.
 */
void
Actions::Warning(std::string message)
{
    queueMessage(WarningMessage, message);
}

/**
 * Output
 *    Output a normal message to our output tab.
 *
 *  @param message - the message to to output.
 */
void
Actions::Output(std::string message)
{
    queueMessage(OutputMessage, message);
}
/**
 * Debug
 *    Output a debug log message to our output tab.
 *
 *    @param message - message to output.
 */
void
Actions::Debug(std::string message)
{
    queueMessage(DebugMessage, message);
}

/**
 * TCLCommand
 *    Request the execution of a Tcl command by the ReadoutGui.
 *
 *  @param command - command to execute.
 */
void
Actions::TCLCommand(std::string command)
{
    queueMessage(TclCommand, command);
}
/**
 * BeginRun
 *    Try to start the run::
 */
void
Actions::BeginRun()
{
    TCLCommand("begin");
}
/**
 * PauseRun
 *  Attempt to pause an active run.,
 *
 */
void
Actions::PauseRun()
{
    TCLCommand("pause");
}
/**
 * ResumeRun
 *    Attempt to resume a paused run,.
 */
void
Actions::ResumeRun()
{
    TCLCommand("resume");
}
/**
 * EndRun
 *    Attempt to end a run.
 *
 *  @param propagate - propagate end run upwards.
 */
void
Actions::EndRun(bool propagate)
{
    if (propagate) {
        TCLCommand("end");
        
    } else {
        TCLCommand("local_end");
    }
}
/*-----------------------------------------------------------------------------
 *  Private utilities for the Actions class.
 */

/**
 * initialize
 *    If the actions system has not been initialized it will be here.
 *    At this point initialization only consists of
 *    - Creating the output thread.
 *    - Storing a pointer to the output thread in m_pOutputThread
 *    - Starting the output thread.
 */
void
Actions::initialize()
{
    if (! m_pOutputThread) {
        m_pOutputThread = new  COutputThread(&m_actionQueue);
        m_pOutputThread->start();
    }
}
/**
 * queueMessage
 *    Create an action message and queue it to the action queue.
 *    Note that initialize is invoked to ensure the output thread is running.
 *
 * @param acType -- The type of action requested.
 * @param payload -- The paylod for the action.
 */
void
Actions::queueMessage(ActionType actType, std::string payload)
{
    initialize();
    
    // Make/fill in the action item.
    
    ActionItem item;
    item.s_type = actType;
    item.s_pMessage = reinterpret_cast<char*>(malloc(payload.size() + 1));
    strcpy(item.s_pMessage, payload.c_str());
    
    // Queue  it for the output thread:
    
    m_actionQueue.queue(item);           // Shallow copy is just fine.
}

/*----------------------------------------------------------------------------
 * Implementation of the internal COutputThread Class.
 */

/**
 * @class Actions::COutputThread
 *    The output thread class ia a thread that is responsible for managing
 *    - message aggregation; that is turning several consecutive identical
 *      messages into "n occurences of: the original messge"
 *    - Ensuring that if there are aggregated messages and nothing has gone
 *      out recently those messages get flushed.
 *    - Flushing all aggregated messages when a TclCommand is requested.
 *
 *  Actions are queued to us via a CBufferQueue.  See the Actions class
 *  for the involved data/message structures.
 */



/**
 *  constructor
 *    - Fills in the map relating action types to action name strings.
 *    - Saves the action queue pointer.
 *
 *  @param pAQ  - Pointer to the action queue.
 *
 */
Actions::COutputThread::COutputThread(CBufferQueue<Actions::ActionItem>* pQ) :
    m_pActionQueue(pQ)
{
    // Fill in the map relating action types to their strings:
    
    Actions::ActionType acts[] = {
        Actions::ErrorMessage, Actions::LogMessage, Actions::WarningMessage,
        Actions::OutputMessage, Actions::DebugMessage, Actions::TclCommand
    };
    const char* actnames[] = {
         "ERRMSG ", "LOGMSG ", "WRNMSG ", "OUTPUT ", "DBGMSG ", "TCLCMD " , 0
    };
     
    Actions::ActionType* pActs = acts;
    const char**         pName = actnames;
    while (*pName) {
        m_actionTypesToNames[*pActs] = std::string(*pName);
        
        pName++;
        pActs++;
    }
    
}

/**
 * operator()
 *    This is the entry point of the code.
 *    - Save the current time in s_lastAdded
 *    - Process action items aggregating and flushing as time advances.
 *
 *    For now we'll just flush as time changes (each second).
 */
void
Actions::COutputThread::operator()()
{
    time_t lastTime = time(NULL);
    while(1) {
        // Try to get data from the queue...if not available, wait
        // for up to a second for data:
        
        Actions::ActionItem item;
        while (!m_pActionQueue->getnow(item)) {
            // See if we should flush all
            
            time_t now = time(NULL);
            if (now != lastTime) {
                flushMessages();              // Flush all consolidations.
                lastTime = now;
            }
            m_pActionQueue->wait(1000);
        }
        // We have an action item now.
        
        processItem(item);
        

    }
}
/**
 * flushMessages
 *    Flush all messages in he map.  Note we flush in increasing
 *    Severity.
 */
void
Actions::COutputThread::flushMessages()
{
    Actions::ActionType actionOrder[] = {
        Actions::DebugMessage, Actions::OutputMessage,  Actions::LogMessage,
        Actions::WarningMessage, Actions::ErrorMessage
    };
    unsigned numActions = sizeof(actionOrder)/sizeof(Actions::ActionType);
    
    for (int  i = 0; i < numActions; i++) {
        flushItem(actionOrder[i]);
    }
}

/**
 * flushItem
 *    Flush consolidated messages for a single item type.
 *
 *  @param itemType  - Type of item to flush.
 *
 *   @note - if there are no consolidated messages for this type obviously
 *           nothing is emitted.
 *   @note - If there is only one message it is emitted unmodified.
 *   @note - If there are multiple messges, the message is preceded
 *           with count information.
 *   @note - Regardless after all of this is done, the item is removed from
 *           the map.
 */
void
Actions::COutputThread::flushItem(Actions::ActionType itemType)
{
    std::map<Actions::ActionType, ActionInfo>::iterator p =
        m_ConsolidatedActions.find(itemType);
    
    if (p != m_ConsolidatedActions.end()) {
        std::string baseMessage = p->second.s_message;
        unsigned    count       = p->second.s_messageCount;
        if(count == 1) {
            outputItem(itemType, baseMessage);
        } else {
            std::stringstream s;
            s << count << " repetitions of: " << baseMessage;
            outputItem(itemType, s.str());
        }
        m_ConsolidatedActions.erase(p);         // Remove the consolidation entry.
    }
}
/**
 * outputItem
 *    Output an item to cerr:
 *    @param type - the type of action item.
 *    @param msg  - the associated message.
 */
void
Actions::COutputThread::outputItem(Actions::ActionType type, std::string msg)
{
    std::string actionName = m_actionTypesToNames[type];
    
    std::cerr
        << actionName << msg.size() << " " << msg << "\n" << std::flush;
}

/**
 * processItem
 *    - If this is a TclCommand - flush all the consolidations and output.
 *    - If there's already a consolidation for  this type if the message is the
 *      same just count it, otherwise flush and set a new consolidation.
 *    - If there's not an existing consolidation, make one.
 *    - free storage associated with the action item's payload.
 *
 *    @param action - action message.
 */
void
Actions::COutputThread::processItem(Actions::ActionItem item)
{
    Actions::ActionType type = item.s_type;
    std::string        msg  = item.s_pMessage;
    free(item.s_pMessage);                 // Free storage!'
    
    if (type == Actions::TclCommand) {
        flushMessages();                // Flush All messages before commanding.
        outputItem(type, msg);
    } else {
        //  Message, not an actual 'action'.
        
        std::map<Actions::ActionType, ActionInfo>::iterator p =
            m_ConsolidatedActions.find(type);
        if (p == m_ConsolidatedActions.end()) {     // No prior message.
            createConsolidation(type, msg);
        } else {
            if(msg == p->second.s_message) {          // If repetition
                p->second.s_messageCount++; // count.
            } else {
                flushItem(type);                     // otherwise flush  that item.
                createConsolidation(type, msg);    // start a new one.
            }
        }
    }
}
/**
 * createConsolidation
 *     Create a new consolidation entry.
 *
 *  @param type - Action type.
 *  @param msg  - Action message body.
 */
void
Actions::COutputThread::createConsolidation(
    Actions::ActionType type, std::string msg
)
{
    ActionInfo info = {msg, 1};              // First occurence.
    m_ConsolidatedActions[type] = info;
}
