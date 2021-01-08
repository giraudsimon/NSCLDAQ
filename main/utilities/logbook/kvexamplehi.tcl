#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

lappend auto_path [file join $::env(DAQROOT) TclLibs]
package require logbookadmin

if {[llength $argv] != 1} {
    puts stderr "kvexamplehi.tcl  key"
    exit -1
}

set key [lindex $argv 0]
if {[kvExists $key]} {
    puts "$key : [kvGet $key]"
} else {
    puts stderr "There is no key named $key"
    exit -1
}
exit 0
