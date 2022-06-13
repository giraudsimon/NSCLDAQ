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
# @file  ringutils.tcl
# @brief ringutils package provides ring buffer utility methods.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide ringutils 1.0
package require portAllocator



##
# getRingUsage
#
#  Get the rings used/known by a ringmaster.
#
# @param host - defaults to localhost Host for which to ask for this
#               information
#
# @return list - Returns the list from the LIST command to that ringmaster.
# @note factored out from eventlogBundle.
# @note Unlike ringbuffer usage in CRingCommand.cpp, this can contact a remote
#       ringmaster and therefore return information about the ring buffers in
#       other hosts.
#
proc getRingUsage {{host localhost}} {
    portAllocator create manager -hostname $host
    set ports [manager findServerAllUsers RingMaster]
    manager destroy

    if {[llength $ports] == 0} {
      error "No RingMaster server  on $host"
    }
    if {[llength $ports] !=1} {
      error "Multiple ring masters are running chaos!!"
    }
    
    set service [lindex $ports 0]
    set port    [lindex $service 1]

    
    set sock [socket $host $port]
    fconfigure $sock -buffering line
    puts $sock LIST
    gets $sock OK
    if {$OK ne "OK"} {
        error "Expected OK from Ring master but got $OK"
    }
    gets $sock info
    close $sock
    return $info
}
