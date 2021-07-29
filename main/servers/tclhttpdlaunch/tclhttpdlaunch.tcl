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
# @file tclhttpdlaunch.tcl
# @brief Launcher for TCLHTTPD REST servers.
# @author Ron Fox <fox@nscl.msu.edu>
#

lappend auto_path $::env(DAQTCLLIBS)

package require portAllocator


##
# Usage:
#   $DAQBIN/tclhttpdlaunch   service-name libdir ?docroot?
#
# service-name - is the port manager service used to allocate a port
#                on which the tclhttpd will listen for connections.
# libdir       - Where the custom scripts live (these are the REST targets).
# docroot      - Optional document root for static web pages the server exports.
#

##
# _usage
#    Prints a message and usage to stderr and then exits with an error status.
#
# @param msg  - Message prior to the usage.
#
proc _usage {msg} {
    puts stderr $msg
    puts stderr "\nUsage:\n"
    puts stderr "   \$DAQBIN/tclhttpdlaunch service-name libdir ?docroot?"
    puts stderr "\nSimplified launcher for tclhttpd as a custom REST server."
    puts stderr "\nWhere:"
    puts stderr "     service-name - Service name on which to run the server"
    puts stderr "     libdir       - Directory with server extension scripts"
    puts stderr "     docroot      - Optional html document root."
    exit -1
}

#---------------------------------------------------------------------------
# Entry point:

if {[llength $argv] < 2} {
    _usage {Insufficient command line parameters}
}
if {[llength $argv] > 3} {
    _usage {Too many command line parameters}
}
#
#  Allocate a port to the service:
#
set service [lindex $argv 0]
set allocator [portAllocator %AUTO%]
set port [$allocator allocatePort $service]
$allocator destroy

set env(SERVICE_NAME) $service;   # So app can see it.

set portarg $port
set libarg [lindex $argv 1];    
    
#Optional -docroot:

if {[llength $argv] == 3} {
    set docrootarg  [lindex $argv 2]
} else {
    set docrootarg ""
}

set command [list exec [file join $::env(DAQSHARE) tclhttpd bin httpd.tcl] \
    -port $portarg  -library $libarg ]
if {$docrootarg ne ""} {
    lappend command -docRoot $docrootarg
}
puts stderr  "Running '$command'"
{*}$command




