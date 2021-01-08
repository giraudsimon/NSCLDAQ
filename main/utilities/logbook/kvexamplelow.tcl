#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}


##
#  kvexample for low level bindings.
#
lappend auto_path [file join $::env(DAQROOT) TclLibs]
package require logbook

if {[llength $argv] ne 2} {
    puts stderr "kvexamplelow.tcl filename key"
    exit -1
}

set book [lindex $argv 0]
set key  [lindex $argv 1]

if {[catch {logbook::logbook open $book} instance]} {
    puts stderr "Could not open logbook $book : $instance"
    exit -1
}
    
if {[$instance kvExists $key]} {
    puts "$key : [$instance kvGet $key]"
    set status 0
} else {
    puts stderr "No such key: $key in $book"
    set status -1
}

$instance destroy
exit $status


