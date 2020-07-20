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
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

##
#  Requires daqsetup.tcl to have been run

lappend auto_path [file join $::env(DAQROOT) TclLibs]
package require Utils

##
# @file samplextension.tcl
# @brief  Sample extension file for scaler display.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
# @note while tested at the time it was written, this sample file is not
#       warranted to function properly nor is it a supported part of the
#       NSCLDAQ software.
#
#  This is meant to be sourced into a scaler configuration file.
#  Could also be made a package and incorporated via package require.


proc formatElapsed {secs} {
    
    set seconds [expr {$secs %60}]
    
    set mins [expr {int($secs/60)}]
    set minutes [expr {$mins %60}]
    
    set hours [expr {int($mins/60)}]
    set hrs   [expr {$hours % 24}]
    
    set days [expr {int($hours/24)}]
    
    return [format "%d %02d:%02d:%02d" $days $hrs $minutes $seconds]
}

##
# UserBegin
#    Called at the beginning of a run:
#

proc UserBeginRun {} {
    set title [getTitle]
    set run   [getRunNumber]
    set stime [clock format [getStartTime]]
    
    puts "New run : $run started at $stime:"
    puts $title
}


##
# UserEnd called at the end of the run:
#
proc UserEndRun {} {
    set run [getRunNumber]
    set rtime [getElapsedTime]

    puts $rtime
    
    # Note the line below assumes the run is no longer than a day:
    
    puts "Run $run ended at [clock format [clock seconds]] after running for [Utils::formatDeltaTime $rtime]"
    
}

##
# UserUpdate
#   Called when some scaler was updated:
#
proc UserUpdate {} {
    set rtime [getElapsedTime]
    set names [lsort [getScalerNames]]

    puts "Scaler update at [formatElapsed $rtime] in to the run"
    puts "    Name           Rate       Counts"
    foreach name $names {
        set rate   [getRate $name]
        set counts [getTotal $name]
        
        puts [format "%15s %7f %12f" $name $rate $counts]
    }
}