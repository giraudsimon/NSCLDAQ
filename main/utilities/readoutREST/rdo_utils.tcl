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
# @file  rdo_utils.tcl
# @brief Utility package used by rdo_commands.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide rdoutils 1.0
if {[array names env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}
package require programstatusclient

##
# getProgramHost
#  @param host - host the manager is running in.
#  @param user - user the manager is running as.
#  @param program - name of managed program.
#  @return host the program is running in.
#  @note - errors are thrown if there is no such program or the program is
#          not running
#
proc getProgramHost {host user program} {
# Does the program exist and is it running?  If so we need its host:

ProgramClient p -host $host -user $user
    set pgmInfo [p status]
    p destroy
    set programs [dict get $pgmInfo programs]
    
    set programInfo [dict create]
    foreach p $programs {
        if {[dict get $p name] eq $program} {
            set programInfo $p
            break
        }
    }
    if {$programInfo eq [dict create]} {
        error "There is no program '$program' defined."
    }
    if {![dict get $programInfo active] } {
        error "Program '$program' is not active."
    }
    
    
    set programHost [dict get $programInfo host];   
    return $programHost
}