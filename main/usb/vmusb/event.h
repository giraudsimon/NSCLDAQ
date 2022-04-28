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
# @file   event.h
# @brief  Tcl Event extension definitions.
# @author <fox@nscl.msu.edu>
*/

/**
 * This extension is added to events that carry strings:
 */
typedef struct _StringPayload {
    char*    pMessage;
} StringPayload, *pStringPayload;