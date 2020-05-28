#!/bin/sh
# \
exec tclsh "$0" "$@"

package require tcltest
##
#  Common code:
##
# Testing utility to update a widget option from a trace on a variable
# that has a name matching the option:
#
# @param widget - the widget to modify.
# @param name1  - name of the variable (and option)
# @param name2  - index of the variable if an array.
# @param op     - operation performed.
#
namespace eval EVB {
	namespace eval test {
		
	}
}
if {[info proc ::EVB::test::updateWidgetOption] eq ""} {
    proc ::EVB::test::updateWidgetOption {widget name1 name2 op} {
        upvar #0 $name1 value
        $widget configure $name1 $value
        return ""
    }
}

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
