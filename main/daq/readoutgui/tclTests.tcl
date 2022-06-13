#!/bin/sh
# \
exec tclsh "$0" "$@"

package require tcltest
set testmanager 1
#
#  sshpipe.test is too finicky - can't get it to run quite right yet because
#  timing is not-deterministic.
#
::tcltest::configure -testdir [file dirname [file normalize [info script]]] 
#::tcltest::configure -testdir [file dirname [file normalize [info script]]] \
    -notfile sshpipe.test
#::tcltest::configure -file *.test
tcltest::testConstraint false 0
eval ::tcltest::configure $argv

# add hook to access whether tests failed for generating a meaningful return code
proc ::tcltest::cleanupTestsHook {} {
  variable numTests
  set ::exitCode [expr {$numTests(Failed) >0}]
}


::tcltest::runAllTests 

exit $::exitCode
