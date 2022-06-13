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
# @file startServer
# @brief Start Tclhttpd on a specific port.
# @author Ron Fox <fox@nscl.msu.edu>
#


##
# Starts an httpd server given the following three positional parameters
# 
#  service-name - name of the service to register with the port allocator.
#  docroot      - name of the document root directory.
#  libdir       - name of the library directory (application scripts).
#

lappend auto_path [file join $::env(DAQROOT) TclLibs]
set server [file join $::env(DAQROOT) share tclhttpd bin httpd.tcl]

package require portAllocator

##
# Usage
#   Prints  usage text and exits with an error.
#  @param msg - error message
#
proc Usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "   startServer service-name docroot libdir"
    puts stderr "Where:"
    puts stderr "   service-name is the NSCLDAQ port manager service to register"
    puts stderr "   docroot      is the document root directory."
    puts stderr "   libdir       is the directory with the application tcl files"
    exit -1
}

#-------------------------------------------------------------------------------
#  Entry point
#
if {[llength $argv] != 3} {
    Usage "Incorrect command line parameter count"
}

set service  [lindex $argv 0]
set docroot  [lindex $argv 1]
set libdir   [lindex $argv 2]

set alloc [::portAllocator create $service]
set port [$alloc allocatePort $service]

exec $server -port $port -docRoot $docroot -library $libdir


