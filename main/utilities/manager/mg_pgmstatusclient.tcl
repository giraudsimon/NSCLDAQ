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
# @file   mg_pgmstatusclient.tcl
# @brief API for program status client.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide programstatusclient 1.0

lappend auto_path $::env(DAQTCLLIBS)
package require http
package require snit
package require clientutils


namespace eval programstatusclient {
    variable base "/Programs"
    variable status "/status"
}

##
# @class ProgramClient
#
#   Provides a client to the manager server domain that hides the
#   nitty gritty details of making requests and decoding the results of
#   those requests.
#
#  OPTIONS:
#     -host   - host on which the server is running
#     -user   - user that started the server.
#
snit::method ProgramClient {
    option -host
    option -user
    
    constructor {args} {
        $self configurelist $args
    }
}