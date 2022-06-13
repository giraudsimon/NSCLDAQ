#!/bin/sh
# \
exec tclsh "$0" "$@"

package require tcltest

::tcltest::configure -testdir [file dirname [file normalize [info script]]]

#::tcltest::configure -outfile test.log

set ::failures 0
proc ::tcltest::cleanupTestsHook {} {
  variable numTests
  incr ::failures $numTests(Failed)
}


tcltest::testConstraint false 0
eval ::tcltest::configure $argv
::tcltest::runAllTests

if {$::failures} {
    exit 1
}