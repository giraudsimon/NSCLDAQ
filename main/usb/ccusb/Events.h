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
# @file   Events.h
# @brief  Defines the extended Tcl_Event structures
# @author <fox@nscl.msu.edu>
*/
#ifndef __EVENTS_H
#define __EVENTS_H
/**
 * Events are sometimes passed from threads to threads running interpreter
 * event loops via Tcl_ThreadQueueEvent.  These events are defined by a
 * Tcl_Event struct with additional event specific data appended.
 * This file defines the additional event data.  Note that only the additional
 * data are defined here in order to minimize the dependence on other
 * types (e.g. Tcl_Event).
 * 
 */



/**
 * AcquisitionFailed is an event used to indicate the acquisition thread
 * failed.
 */
typedef struct _AcquisitionFailedEvent {
    char*    pMessage;                /* Human readable error message */   
} AcquisitionFailedEvent, *pAcquisitionFailedEvent;

#endif
