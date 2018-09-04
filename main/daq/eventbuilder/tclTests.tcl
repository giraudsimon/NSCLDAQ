#!/bin/sh
# \
exec tclsh "$0" "$@"

package require tcltest
::tcltest::configure -testdir [file dirname [file normalize [info script]]] \
		     -skip rdo10*
tcltest::testConstraint false 0
eval ::tcltest::configure $argv

set ::exitCode 0
proc ::tcltest::cleanupTestsHook {} {
  variable numTests
  set ::exitCode [expr {$numTests(Failed) >0}]
}

::tcltest::runAllTests

exit $:::exitCode
