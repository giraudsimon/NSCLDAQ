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

/** @file:  Worker.cpp
 *  @brief: Implement the main flow of control for a worker.
 */
#include "Worker.h"
#include "MessageTypes.h"

/**
 * constructor
 *    We just need to construct the thread base class.
 * @param name - name of the thread.
 */
Worker::Worker(const char* pName) :
    Thread(std::string(pName))
{}

/**
 * Destructor
 *    chains to the base class.
 */
Worker::~Worker() {}

/**
 * run
 *    Entry point for the thread.
 *    -  Connect to the producer and consumer.
 *    -  Get messages and dispatch appropriately until END_ITEM
 *    -  Close the producer and consumer.
 */
void
Worker::run()
{
    connectSource();
    connectSink();
    
    bool done = false;
    while(!done) {
        Message msg = getMessage();
        switch (msg.s_messageType) {
        case MessageType::PASSTHROUGH_ITEM:
            sendProcessedMessage(msg, nullptr);   // no processing.
            break;
        case MessageType::DROP_ITEM:
            break;                             // drop on the floor.
        case MessageType::END_ITEM:
            sendProcessedMessage(msg, nullptr);
            onEnd(msg);
            done = true;
            break;
        case MessageType::PROCESS_ITEM:
            {
                void* pData = processMessage(msg);
                sendProcessedMessage(msg, pData);
                freeProcessedMessage(pData);
            }
            break;
        default:
            onUnknownType(msg);
            break;
        }
        FreeMessage(msg);
    }
    
    disconnectSource();
    disconnectSink();
}