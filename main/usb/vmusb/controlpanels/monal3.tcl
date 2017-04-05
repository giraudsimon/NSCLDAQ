#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec wish "$0" ${1+"$@"}

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
# @file MoNaL3.tcl
# @brief MoNA l3 trigger gui.
# @author Ron Fox <fox@nscl.msu.edu>
#

# Set up auto path

##
# removeTclLibPath
#   Removes the directories in TCLLIBPATH from the front of auto_path.
#   this keeps the user from screwing us over by defining TCLLIBPATH
#   in a way that pulls in old directories.
#
proc removeTclLibPath {} {
    if {[array names ::env TCLLIBPATH] ne ""} {
	set badDirs $::env(TCLLIBPATH)
	set path    $::auto_path

	foreach dir $badDirs {
	    set i [lsearch -exact $path $dir]
	    set path [lreplace $path $i $i]
	}

	set ::auto_path $path
    }
}

#  Add TclLibs for stuff relative to us

removeTclLibPath

set here [file dirname [info script]]
set tcllibs [file normalize [file join $here .. TclLibs]]
lappend auto_path $tcllibs



package require cmdline

package require ALevel3XLM72GUI
package require AXLM72GenericProxy
package require icon
package require Tk



# hide the user's command line options from package require Tk because
# we don't want to mess with those parameters. Just let the user configure
# the parameters that are presented by this code
set argv2 $argv
set argv {}

#set up the options
set options {
  {module.arg ""          "name of module registered to slow-controls server \[MANDATORY\]"}
  {host.arg   "localhost" "host running VMUSBReadout slow-controls server" }
  {port.arg   27000       "port the slow-controls server is listening on" }
  {ring.arg   ""          "name of ring VMUSBReadout is filling" }
  {configfile.arg ""      "name of configuration file to read from to initialize the GUI" }
}

set usage "-module name ?option value?"
if {("-help" in $argv2) || ("-?" in $argv2) || ("-h" in $argv2)} {
  puts [cmdline::usage $options $usage]
  exit
}

if {[catch {array set params [::cmdline::getoptions argv2 $options]} result]} {
  puts "Failed to parse command line arguments!"
  puts $result
  exit 1
}

set module     [lindex [array get params module] 1]
set host       [lindex [array get params host] 1]
set port       [lindex [array get params port] 1]
set ring       [lindex [array get params ring] 1]
set configFile [lindex [array get params configfile] 1]

if {$module eq {}} {
  puts "Error! The name of the slow controls module was not specified."
  puts [cmdline::usage $options $usage]
  exit 1 
}

if {$ring eq {}} {
  set ring tcp://localhost/$::tcl_platform(user)
}

setIcon l3trigicon.png

ALevel3XLM72 level3 $module $host $port $ring


level3 SetupGUI .

