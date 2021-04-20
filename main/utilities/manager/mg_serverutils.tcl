#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file  mg_serverutils.tcl
# @brief Utilities used by the manager server.
# @author Ron Fox <fox@nscl.msu.edu>
#

# We make this a formal package because it has useful utilities all rest
# server may use.

package provide restutils 1.0

package require json::write


##
# GetRequestType
#    Returns the request type (e.g GET, POST etc).
#
# @param sock - the sock passed in to the request handler.
# @return string
#
proc GetRequestType {sock} {
    upvar #0 Httpd$sock data
    return $data(proto)
}
##
# GetPostedtData
#   Returns POST/GET-query data
#
# @param sock - sock passed in to the request handler.
# @return list that can shimmer to a dict - [list pname value ...]
#
proc GetPostedData {sock} {
    upvar #0 Httpd$sock data
    return [Url_DecodeQuery $data(query)]
}

##
# ErrorReturn
#    Return an error on the request:
#
# @param sock   - socket of the request.
# @param message - message part of returned object.
#
proc ErrorReturn {sock message} {
    Httpd_ReturnData $sock application/json [json::write object \
        status [json::write string ERROR]                       \
        message [json::write string $message]                   \
    ]
}
##
# MakeStringList
#    Given a list of strings returns them as a list of JSON encoded strings.
#
# @param  strings
# @return list
#
proc MakeStringList {strings} {
    set result [list]
    foreach string $strings {
        lappend result  [json::write string $string]
    }
    return $result
}