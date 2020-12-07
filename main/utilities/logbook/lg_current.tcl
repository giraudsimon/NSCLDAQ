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
# @file lg_current.tcl
# @brief Set the current logbook.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names env DAQROOT] eq "DAQROOT"} {
    set tcllibs [file join $env(DAQROOT) TclLibs]
} else {
    set tcllibs [file normalize [file join [file dirname [info script]] .. TclLibs]]
}
lappend auto_path $tcllibs

package require logbookadmin

##
#   lg_current
#    Set the current logbook database;
#
#   If there are any command line parameters, there must be only one and it
#   must be the name of an existing database file which will be set current.
#   If not filename argument is given, Tk will be used to prompt for one
#   via tk_getOpenFile.
#


#------------------------------------------------------------------------------
# entry point

if {[llength $argv] > 1} {
    puts stderr "Usage:"
    puts stderr "   lg_current ?databasefile?"
    puts stderr
    puts stderr "If the database filename is omitted it will be graphically"
    puts stderr "prompted for."
    exit -1
}

if {[llength $argv] == 1} {
    set filename [lindex $argv 0]
} else {
    package require Tk
    wm withdraw .;                # Only want to show the dialog.
    set filename [tk_getOpenFile -defaultextension .logbook \
        -title {Choose logbook database file} \
        -filetypes [list                                        \
            [list {LogBook Files} {.logbook}  ]                 \
            [list {All files} *]                                \
        ]
    ]
}
if {$filename eq ""} {
    puts stderr "No file selected, doing nothing"
    exit 0
}
if {![file readable $filename]} {
    puts stderr "'$filename' is not readable"
}
setCurrentLogBook $filename
exit 0

    
