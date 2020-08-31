#!/bin/sh
# start tclsh \
    exec tclsh ${0} ${@}


#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



#
#   This is the application itself.  We are just going to setup the
#   pieces and parts of the system, hook them together and enter the event loop.
#
#   Since we have no idea which version of tcl will be used to run us, we require that
#   the user run us as tclshx.y Application.tcl ...
#

#   Usage:
#      tclshx.y TclServer.tcl [-ports range] [-listen port] [-log file]
#
#  If being run from root, in general the defaults are correct for all of these:
#      -ports  30001-31000
#      -listen 30000
#      -log    /var/log/var/log/nscldaq/portmanager.log
#
 
# In order to find the packages we need to add the canonicalized .. to 
# auto_path if it isn't already.

set here [file dirname [info script]]
set wd [pwd]


cd [file join $here .. TclLibs]
set libDir [pwd]
cd $wd

if {[lsearch -exact $auto_path $libDir] == -1} {
    set auto_path [concat $libDir $auto_path]
}
package require removetcllibpath
package require snit
package require ConnectionManager
package require PortManager
package require Log

#  Global variables that hold the defaults:

set defaultListenPort   30000
set defaultPortRange   {30001 31000}
set defaultLogFile      /var/log/nscldaq/portmanager.log
set defaultUsageFile    /var/tmp/daqportmgr/ports.txt
set defaultPortFile     /var/tmp/daqportmgr/listen.port
set defaultPidFile      /var/run/nscldaq/daqportmgr.pid

#  These global variables hold the selected run parameter values:

set selectedListenPort  $defaultListenPort
set selectedPortRange   $defaultPortRange
set selectedLogFile     $defaultLogFile
set selectedUsageFile   $defaultUsageFile
set selectedPortFile    $defaultPortFile
set selectedPidFile     $defaultPidFile

# usage:
#    Produces a return value that contains the program
#    usage string.
#
proc usage {} {
    set self [file tail [info script]]
    set Usage         "$self ?-ports low-high? ?-listen port? ?-log filename?"
    append Usage "\n" {    -ports   - Specifies the range of ports to be managed}
    append Usage "\n" {    -listen  - Specifies the port on which the program listens for connections}
    append Usage "\n" {    -log     - Specifies the log file}
    
    return $Usage
}
##
# openFileWithBackup
#   Given a full filename path attempts to open that for write.
#   IF the open fails, a file with the same name is opened for write in ~
#
#  @param path -- full filename path to attempt.
#
proc openFileWithBackup {path} {
    if {[catch {open $path w} fd]} {
        #failed open
        
        set fname [file tail $path]
        set fname [file join ~ $fname]
        set fd [open $fname w]
    }
    return $fd
}
# ParseParameters:
#   Parse the command line parameters.  Since we don't support  anything
#   other than switches with parameters this is relatively easy.
# Parameters:
#   rawParameters   (list)
#       The list of raw command line parameters.
# SideEffects:
#   The global variables selected* get set according to the mix of default
#   values and switched values.
#
proc ParseParameters {rawParameters} {
    global  selectedListenPort
    global  selectedPortRange
    global  selectedLogFile
    global  selectedPidFile
    global  selectedPortFile
    
    # Parameters come in pairs:
    
    for {set i 0} {$i < [llength $rawParameters]} {incr i 2} {
        set Switch [lindex $rawParameters $i]
        set Value  [lindex $rawParameters [expr {$i+1}]]
        switch -exact -- $Switch {
            -ports {
                #  Ports must have a list with 2 numeric elements...
                
                if {[scan $Value "%d-%d" low high] != 2} {
                    error "Failed to parse -ports $Value  \n [usage]"
                }
                #  If necessary swap values to get low hi pairs:
                
                if {$low > $high} {
                    set tmp $high
                    set high $low
                    set low  $tmp
                }
                set selectedPortRange [list $low $high]
                
            }
            -listen {
                # must have a single numerical port value:
                
                if {[scan $Value "%d" port] != 1} {
                    error "Failed to parse -listen $Value \n [usage]"
                }
                set selectedListenPort $port
            }
            -log {
                # The value is a text string:
                
                set selectedLogFile $Value
            }
	    -portfile {			# Undocumented
		set selectedPortFile $Value
	    }
	    -pidfile {			# Undocumented
		set selectedPidFile $Value
	    }
            default {
                error "Invalid switch $Switch\n [usage]"
            }
        }
       
    }
    
}
# enterEventLoop
#   Enters the event loop:  This  program is built to run
#   from tclsh, rather than wish, but is event driven.
#   The simple code below makes tclsh process events
#   until the application is forced to exit:
#
set forever 0
proc enterEventLoop {} {
    global forever
    vwait  forever
}
# run:
#   Called to run the application.  Any initialization is done first,
#   then this proc is called; to instantiate instances of the components
#   and to stitch them together.
# Parameters:
#   argList (list)
#       Program parameter list e.g. from command line.
#
proc run {argList} {
    global selectedListenPort
    global selectedPortRange
    global selectedLogFile
    global selectedUsageFile
    global selectedPortFile
    global selectedPidFile
    
    if {[catch {ParseParameters $argList} msg]} {
	puts stderr $msg
	exit -1
    }
    
    # Create the directories needed by all the files:
    
    file mkdir [file dirname $selectedLogFile]
    file mkdir [file dirname $selectedUsageFile]
    file mkdir [file dirname $selectedPortFile]
    file mkdir [file dirname $selectedPidFile]
    
    # Set our port and pid files.
    # Note - first we try to use the selected file ... if that does not
    #        work we use a file with the same name but in ~.
    
    set fd [openFileWithBackup $selectedPortFile]
    puts $fd $selectedListenPort
    close $fd
    
    set   fd [openFileWithBackup $selectedPidFile]
    puts $fd [pid]
    close $fd
    
    #  Build the Log and port manager objects:
    
    
    
    set logger [Log create log -filename $selectedLogFile]
    set ports  [PortManager create ports -range $selectedPortRange]
    set cm     [ConnectionManager create cm -ports     $ports \
                                            -logger    $logger \
                                            -listen    $selectedListenPort \
                                            -usagefile $selectedUsageFile]
                                            
    # Start processing events:
    
    enterEventLoop
}

#  Entry point
if {[info exists argv]} {
    run $argv
}
# 