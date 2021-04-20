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
# @file ReadoutREST.tcl
# @brief Package to bolt a REST server to any Readout framework.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names ::env DAQTCLLIBS] eq "DAQTCLLIBS"} {
    lappend auto_path $::env(DAQTCLLIBS)
}
package provide ReadoutREST 1.0
package require portAllocator
package require EmbeddedHttpdServer


##
#  Doing a package require ReadoutREST will allocate/advertise a service port
#  and start a REST server to control this Readout program.
#  Here's how the service name is derived:
#  - If ::env(SERVICE_NAME) exists and is nonempty, it's the service name.
#  - If ::env(SERVICE_NAME) is not defined but the Tcl global ServiceName
#     is then that's the service name.
#  - A final fallback is to use the service ReadoutREST
#


#  Figure out the service and allocate a port.

set serviceName ReadoutREST
if {[array names ::env SERVICE_NAME] eq "SERVICE_NAME"} {
    set serviceName $::env(SERVICE_NAME)
} elseif {[info globals ServiceName] eq "ServiceName"} {
    set serviceName $::ServiceName
}

set allocator [portAllocator %AUTO%]
set port   [$allocator allocatePort $serviceName]
$allocator destroy

#  Figure out where our libraries are:

set libraries [file join [file dirname [info script]] server]

# Start the server 


startHttpdServer $port "" $libraries


