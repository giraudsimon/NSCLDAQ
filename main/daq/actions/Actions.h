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
# @file   Actions.h
# @brief  Define the Actions class -mechanisms for children to get readoutGUi to do stuff.
# @author <fox@nscl.msu.edu>
*/

#ifndef ACTIONS_H
#define ACTIONS_H

#include <CBufferQueue.h>
#include <CSynchronizedThread.h>
#include <map>
#include <string>
#include <time.h>

/**
 * @class Actions
 *     This class contains a bunch of static methods that provide
 *     the ability for children of ReadoutGUI to get the GUI to do stuff
 *     for them.  'stuff' is defined as:
 *     - Logging a message
 *     - Performing a Tcl command on their behalf.  This is normally, but not
 *       exclusively used to affect the run state.
 *
 *  @note Message consolidation is used to avoid saturating the event loop
 *        of the readout gui.  It is that message consolidation that makes
 *        this class more than a simple namespace.  This class will have
 *        entries that queue messages to a thread which consolidates them and
 *        then, only every 'now and then' outputs those messages on stderr
 *        where the ReadoutGui interprets them....Tcl operations will
 *        result in a flush of any queued output prior to performing the
 *        command.
 */
class Actions
{
    // Internal data structures:
private:
    // Types of actions clients can request:
    
    typedef enum _ActionType {
        ErrorMessage, LogMessage, WarningMessage, OutputMessage, DebugMessage,
        TclCommand
    } ActionType;
    
    // Struct queued between the threads:
    
    
    typedef struct _ActionItem {
        ActionType           s_type;
        char*                s_pMessage;
    } ActionItem, *pActionType;
    
    // Internal data:
    
private:
    static CBufferQueue<ActionItem>  m_actionQueue;
    static CSynchronizedThread*      m_pOutputThread;
    
    
    // Public actions:

public:
    static void Error(std::string message);
    static void Log(std::string message);
    static void Warning(std::string message);
    static void Output(std::string message);
    static void Debug(std::string message);

    static void TCLCommand(std::string command);
    static void BeginRun();
    static void EndRun(bool propagate=true);
    static void PauseRun();
    static void ResumeRun();
    
    
    // Private Utilities:
    
private:
    static void initialize();
    static void queueMessage(ActionType type, std::string payload);
    
    
    // The output thread:
    
    class COutputThread : public CSynchronizedThread
    {
        // Private data structures.
        typedef struct _ActionInfo {
            std::string    s_message;
            unsigned       s_messageCount;
            
        } ActionInfo, *pActionInfo;
        

        // Private data:
    private:
        CBufferQueue<Actions::ActionItem>*        m_pActionQueue;
        std::map<Actions::ActionType, ActionInfo> m_ConsolidatedActions;
        std::map<Actions::ActionType, std::string> m_actionTypesToNames;
    public:
        COutputThread(CBufferQueue<Actions::ActionItem>* queue);
        void operator()();
    private:
        void flushMessages();
        void flushItem(Actions::ActionType type);
        void outputItem(Actions::ActionType type, std::string msg);
        void processItem(Actions::ActionItem item);
        void createConsolidation(Actions::ActionType type, std::string msg);
        
        
    };
    
};

#endif
