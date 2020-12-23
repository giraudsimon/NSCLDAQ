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
# @file lg_kvstore.tcl
# @brief Access the key value store
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names env DAQROOT] eq "DAQROOT"} {
    set tcllibs [file join $env(DAQROOT) TclLibs]
} else {
    set tcllibs [file normalize [file join [file dirname [info script]] .. TclLibs]]
}
lappend auto_path $tcllibs

package require logbookadmin

proc usage { } {
    puts stderr "Usage:"
    puts stderr "  lg_kvstore exists key"
    puts stderr "  lg_kvstore get    key"
    puts stderr "  lg_kvstore set    key value"
    puts stderr "  lg_kvstore create key value"
    puts stderr "Where:"
    puts stderr "   key - is a key in or about to be in the key value store"
    puts stderr "   value - is a value to be assigned to that key"
    puts stderr "The verbs:"
    puts stderr "   exists - tests for the presence of key in the kv store"
    puts stderr "   get    - retrieves the value of key from the kv store"
    puts stderr "   set    - Replaces the value of key with value in the kv store"
    puts stderr "            if key does not exist it is created"
    puts stderr "   create - Creates a new key value pair in the kv store. It"
    puts stderr "            is an error for key to already exist."
    exit -1
}

#----------------------------------------------------------------------------
#
# Entry point:

if {[llength $argv] < 1} {
    usage
}

set verb [lindex $argv 0]
set tail [lrange $argv 1 end]

if {$verb eq "exists"} {
    if {[llength $tail] != 1} {
        usage
    }
    set value [kvExists $tail]
    exit $value
} elseif {$verb eq "get"} {
    if {[llength $tail] != 1} {
        usage
    }
    puts [kvGet $tail]
} elseif {$verb eq "set"} {
    if {[llength $tail] != 2} {
        usage
    }
    set key [lindex $tail 0]
    set value [lindex $tail 1]
    kvSet $key $value
} elseif {$verb eq "create"} {
    if {[llength $tail] != 2} {
        usage
    }
    set key [lindex $tail 0]
    set value [lindex $tail 1]
    kvCreate $key $value
} else {
    usage
}
exit 0