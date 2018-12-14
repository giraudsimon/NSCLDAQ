#!/bin/sh
# \
exec tclsh "$0" "$@"

package require tcltest
#
::tcltest::configure -testdir [file dirname [file normalize [info script]]] \
    -notfile tclCCUSB.test 
#::tcltest::configure -file *.test
tcltest::testConstraint false 0
eval ::tcltest::configure $argv

# add hook to access whether tests failed for generating a meaningful return code
proc ::tcltest::cleanupTestsHook {} {
    variable numTests
  set ::exitCode [expr {$numTests(Failed)  >0}]
}

::tcltest::runAllTests 
puts "Note - it's normal for wait.test to claim to exit with errors because"
puts "       package require Wait outputs to stderr"
puts "       Check the test counters instead."
exit $::exitCode
