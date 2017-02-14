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
# @file MoNaL3.tcl
# @brief MoNA l3 trigger gui.
# @author Ron Fox <fox@nscl.msu.edu>
#

set here [file dirname [info script]]
set tcllibs [file normalize [file join $here .. TclLibs]]
lappend auto_path $tcllibs

puts $auto_path

package require ALevel3XLM72GUI
package require AXLM72GenericProxy


proc Usage {} {
    puts stderr "Usage:"
    puts stderr "   MonaL3  name widget"
    puts stderr "      name - the name of a mon alevel 3 trigger gui."
    puts stderr "      widget - Name of the gui Widge to create"
}

if {[llength $argv] != 2} {
    Usage
    exit 1
}
set name [lindex $argv 0]
set widget [lindex $argv 1]


ALevel3XLM72 level3 $name

level3 SetupGUI $widget