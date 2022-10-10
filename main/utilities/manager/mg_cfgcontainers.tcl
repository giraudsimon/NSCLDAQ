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

# Assume we're installed in DAQBIN and TclLibs is one dir above us

set libdir [file normalize [file join [file dirname [info script]] .. TclLibs]]
set bindir [file dirname [info script]]

lappend auto_path $libdir

##
# @file   mg_cfgcontainers.tcl
# @brief  Command to configure containers.
# @author Ron Fox <fox@nscl.msu.edu>
#
package require sqlite3
package require containers
package require containeredit
##
# Usage:
#    mg_cfgcontainers database-path
#
#  Where:
#    database-path is the path to an already created configuration database file.
#

proc Usage {msg} {
    puts stderr $msg
    
    puts "Usage:";
    puts "    mg_cfgcontainers database-path"
    puts "Where:"
    puts "   database-path is the path to a configuration database created with"
    puts "   \$DAQBIN/mg_mkconfig"
    
    exit -1
}

#------------------------------------------------------------------------------
#  Action handlers for editor:

##
# createContainer
#
#    Spin off the container creation wizard.
#
# @param - dbfile - database filename.
# @param - editor - top level editor widget.
# @param - db     - database handle.
#
proc createContainer {dbfile editor db} {
    exec [file join $::bindir mg_container_wizard] $dbfile
    
    #  Now update the container defs:
    
    $editor configure -containers [container::listDefinitions $db]
    
}
##
# replaceContainer
#    (potentially) Replace an existing container.  Note that the name of the
#    starting point container maight have been edited which would actually create
#    a new container.
#    - If the container name exists, remove it.
#    - Then call createContainer to finish the job.
#
# @param - editor -the editor widget.
# @param - db     - the database command.
# @param - def    - The new container defintion dict.
#
proc replaceContainer {editor db def} {
    set name [dict get $def name]
    
    if {[container::exists $db $name]} {
        container::remove $db $name
    }
    #   recreate the new container:
    
    container::add $db $name \
        [dict get $def image] [dict get $def init] [dict get $def bindings]
    
    $editor configure -containers [container::listDefinitions $db]
    
    
}
#------------------------------------------------------------------------------
#  Entry point.

##
#  Need a database name and the file must already exist:

if {[llength $argv] != 1} {
    Usage "Incorrect number of command line arguments"
}

#  The database file must be readable and writable:

set dbfile [lindex $argv 0]
if {![file readable $dbfile]} {
    Usage "$dbfile is not readable (may not exist)"
}
if {![file writable $dbfile]} {
    Usage "$dbfile cannot be written.  Check permissions."
}
##
# Set up the user interface:
#
sqlite3 db $dbfile
set existingContainers [container::listDefinitions db]
container::Editor .editor -containers $existingContainers \
    -newcommand [list createContainer $dbfile .editor db]        \
    -replacecommand [list replaceContainer .editor db]

ttk::frame .actions
ttk::button .actions.exit -text Exit -command exit
grid .actions.exit -sticky w

pack .editor -fill both -expand 1
pack .actions  -fill x -expand 1
