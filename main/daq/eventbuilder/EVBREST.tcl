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
# @file  EVBREST.tcl
# @brief Script to start a REST server that provides event builder statistics.
#
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names ::env DAQTCLLIBS] eq "DAQTCLLIBS"} {
    lappend auto_path $::env(DAQTCLLIBS)
}
package provide EVBREST 1.0
package require portAllocator
package require EmbeddedHttpdServer

#  The Rest library is in the REST directory below this directory.

set restLibraryDir [file join [file dirname [info script]] REST]

##
#  During a package require EVBREST this file will allocate/advertise
#  a service port and start a REST server to monitor the event builder
#  on that port.  The service name is derived by looking for it in
#  the following places in order:
#  -  ::env(SERVICE_NAME)
#  -  The Tcl global Variable ServiceName.
#  -  The literal value ORDERER_REST
#

#  Figure out a service name:

if {[array names ::env SERVICE_NAME] ne ""} {
    set service $::env(SERVICE_NAME)
} elseif {[info globals ServiceName] ne ""} {
    set service $::ServiceName
} else {
    set service "ORDERER_REST"
}


#  Alloate a listener port:

set allocator [portAllocator %AUTO%]
set port [$allocator allocatePort $service]
$allocator destroy

#  Start the server:

puts "Starting server with library in $restLibraryDir"

startHttpdServer $port "" $restLibraryDir

#  At some point the caller needs to enter the event loop e.g. with
#
#  vwait forever
