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
# @file   mg_cfprogram.tcl
# @brief  Program to configure programs.
# @author Ron Fox <fox@nscl.msu.edu>
#

set libdir [file normalize [file join [file dirname [info script]] .. TclLibs]]
lappend auto_path $libdir

package require sqlite3
package require programedit
package require programs
package require dialogwrapper

#-----------------------------------------------------------------------------
# Global variables


#-----------------------------------------------------------------------------
# Utility procs.

##
# Usage
#   Print an error message and program usage:
#      mg_cfgprogram database-path
#
# @param msg - Error message.
#
proc Usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "   mg_cfgprogram  database-path"
    puts stderr "Where:"
    puts stderr "  database-path is the path to an already created configuration "
    puts stderr "  database path"
    exit -1
}
##
# _newDefinition
#    Creates a new program definition: Brings up a program::View with no
#    pre-loaded definition in a dialogwrapper with Ok and Cancel buttons.
#    On OK, we read the definition out of the dialog and try to create a new definition.
#

#------------------------------------------------------------------------------
# Entry:




if {[llength $argv] != 1} {
    Usage "Incorrect number of command line parameters"
}

set file [lindex $argv 0]

sqlite3 db $file

# Let's get the list of program definitions and put up a SelectProgram
# widget.
#
#  At the bottom of that we'll put
#   A "New..." Button that allows the creation of a new program.
#   We'll also establish a -command on the program definition list that
#   allows us to edit an existing definition.
#
set definitions [program::listDefinitions db]
program::SelectProgram .selection -command _editDefinition -programs $definitions
ttk::button .new -text New... -command _newDefinition

grid .selection -sticky nsew
grid .new       -sticky w


