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
# @file   mg_cfgEvlog.tcl
# @brief  Program, installed as $DAQBIN/mg_cfgEvlog to configure event loggers.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names env DAQTCLLIBS] ne ""} {
    lappend auto_path $env(DAQTCLLIBS)
}

package require Tk
package require sqlite3
package require evLogEditViews
package require evlogEditController
package require evlogeditmodel

##
#  This program melds together an event log editor view, an event log edit model
#  and an event log editor controller to create an editor of event log definitions
#  in an experiment database.
#   See _usage to see how to invoke the program.
#

#-------------------------------------------------------------------------------
#  Utility functions.
#

##
# _usage
#   Output an error message and the program usage to stderr before exiting.
#
# @param msg  - error message
# @return - this proc does not return but exits instead,
#
proc _usage {msg} {
    puts stderr $msg
    puts stderr "Usage\n"
    puts stderr "    \$DAQBIN/mg_cfgEvlog  exp-database"
    puts stderr "Edit the event log definitions in an experiment configuration"
    puts stderr "database\n"
    puts stderr "Where:"
    puts stderr "   exp-database is the path to an experimental database to edit."
    exit -1
}


#---------------------------------------------------------------------------
# Entry


# ensure we have a filename and that it's a writable file.

if {[llength $argv] != 1} {
    _usage "Incorrect number of program command line arguments"
}
set fname $argv

if {![file writable $fname]} {
    _usage "'$fname' must be a writable, pre-existing experiment database file"
}

if {[catch {sqlite db $fname} msg]} {
    _usage "'$fname' could not be opened as a database file: $msg"
}

set model [evlogEditModel %AUTO% -database db]
set view  [evlogEditorView .v]
set controller [evlogEditController %AUTO% -model $model -view $view]

pack .v -expand 1 -fill both

    
