#!/usr/bin/env tclsh

#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#      NSCL DAQ Development Team 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#
# @author Jeromy Tompkins

set here [file normalize [file dirname [info script]]]
lappend auto_path [file join $here .. TclLibs]
lappend auto_path [file join $here .. lib]
puts $auto_path

package require InstallRoot
package require cmdline
package require Itcl 

set options {
  {module.arg ""         "name of module registered to slow-controls server \[MANDATORY\]"}
  {host.arg "localhost"  "host running VMUSBReadout slow-controls server" }
  {port.arg  27000       "port the slow-controls server is listening on" }
  {ring.arg $::tcl_platform(user) "name of ring VMUSBReadout is filling" }
}

set usage " --module value ?option value? :"
array set params [::cmdline::getoptions argv $options]
foreach key [array names params] {
  set params(-$key) $params($key)
  array unset params $key
}


if {[lindex [array get params -module] 1] eq ""} {
  puts [::cmdline::usage $options $usage]
  puts "User must pass a valid value for the --module option"
  exit
}

set module [lindex [array get params -module] 1]
set host [lindex [array get params -host] 1]
set port [lindex [array get params -port] 1]
set ring [lindex [array get params -ring] 1]

lappend auto_path [file join [::InstallRoot::Where] TclLibs] 
package require xlm72scalerpanel


set parent .

# Create the widget
::XLM72ScalerGUI aPanel $parent $module $host $port 

# Make the background color lightblue
. configure -background lightblue

## Grid the frame
set top [aPanel GetTopFrame]
grid $top -sticky nsew  -padx 6 -pady 6
grid columnconfigure $top 1 -weight 1
grid rowconfigure $top 1 -weight 0

# Configure some window manager details 
wm protocol $parent WM_DELETE_WINDOW { aPanel OnExit } 
wm title $parent "AXLM72Scaler Control Panel: ::aPanel"
wm resizable $parent false false

