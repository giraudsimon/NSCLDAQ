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

/** @file:  MessageTypes.h
 *  @brief: Contains message type codes.
 */

#ifndef MESSAGETYPES_H
#define MESSAGETYPES_H
#include <stdint.h>
#include <list>

// Message types:

namespace MessageType {
    static const uint32_t PROCESS_ITEM     = 1;
    static const uint32_t REGISTRATION     = 2;
    static const uint32_t UNREGISTRATION   = 3;
    static const uint32_t EXIT_REQUEST     = 4;
    static const uint32_t END_ITEM         = 5;
    static const uint32_t DATA_REQ         = 6;

   // messages look like this:
    
    typedef struct _Message {
        uint32_t         s_messageType;
        std::list<std::pair<uint32_t, void*> > s_dataParts;
    } Message, *pMessage;

}
#endif