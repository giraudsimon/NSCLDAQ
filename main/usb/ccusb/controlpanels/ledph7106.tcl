#!/bin/sh
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
# the next line restarts using tclsh \
              exec tclsh "$0" "$@"


##
# @file ledph7106.tcl
# @brief Control panel full application for the PH7106 LED camac module.
#

# Add TclLibs to the auto_path:

set bindir [file dirname [info script]]
set libdir [file join $bindir .. TclLibs]
lappend auto_path $libdir

package require Tk
package require ph7106Widget
package require usbcontrolclient

set serverSelected 0
set serverPort    -1
set serverHost   ""
set serverSocket ""

##
# Usage:
#   ledph7106.tcl module-name
#
#  Where:
#    module-name is the name of the module in the controlconfig.tcl file.
#
#


##
# selectServer
#
#   Called when the user has made a choice in the server selection.
#  - Fishes the host and port out of the server and sets them in
#    the server{Host,Port} variables.
#  - increments serverSelected to allow the vwait in mainline code to complete.
#
# @param widget - Widget path to the slowControlsPrompter widget.
#
proc selectServer widget {
    set ::serverPort [$widget cget -port]
    set ::serverHost [$widget cget -host]
    incr ::serverSelected
}

#-----------------------------------------------------------------------------
#
#  Entry point:

# Ensure there's a module-name and prompt for the server:

if {$argc != 1} {
    puts stderr "Usage:"
    puts stderr "   ledph7106.tcl module-name"
    puts stderr "Where:"
    puts stderr "   module-name is the name of the module to control as it"
    puts stderr "   appears inthe controlconfig.tcl file."
    exit -1
}


wm withdraw .

toplevel .dialog
slowControlsPrompter .dialog.prompt \
    -okcmd [list selectServer %W] -cancelcmd exit -type CCUSBReadout
pack .dialog.prompt

vwait serverSelected
destroy .dialog

wm deiconify .

#-----------------------------------------------------------------------------
#
# Open a socket on the server/port as seleted above:
# Make the control panel and pack it into .


ph7106Widget .control -name [lindex $argv 0] -host $serverHost -port $serverPort
pack .control


