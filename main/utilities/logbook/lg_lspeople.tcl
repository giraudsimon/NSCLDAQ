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
# @file lg_lspeople.tcl
# @brief List the people in the current database.
# @author Ron Fox <fox@nscl.msu.edu>
#
if {[array names env DAQROOT] eq "DAQROOT"} {
    set tcllibs [file join $env(DAQROOT) TclLibs]
} else {
    set tcllibs [file normalize [file join [file dirname [info script]] .. TclLibs]]
}
lappend auto_path $tcllibs

package require logbookadmin
package require lg_utilities

##
# Usage:
#    lg_lspeople
#  Outputs a report describing the people known to the logbook.
#
    


if {[currentLogBook] eq ""} {
    puts stderr {There is no current logbook, select one with $DAQBIN/lg_current}
    exit -1
}

set people [listPeople]
set report [reportPeople $people]
puts $report

exit 0

    
