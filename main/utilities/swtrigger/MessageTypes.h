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

// Message types:

namespace MessageType {
    static const uint32_t PASSTHROUGH_ITEM = 1;
    static const uint32_t PROCESS_ITEM     = 2;
    static const uint32_t DROP_ITEM        = 3;
    static const uint32_t END_ITEM         = 0x7fffffff;
    static const uint32_t BARRIER          = 0x80000000;
}
#endif