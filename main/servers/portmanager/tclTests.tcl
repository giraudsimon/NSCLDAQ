package require tcltest
::tcltest::configure -testdir \
    [file dirname [info script]]

puts "'$argv'"

eval tcltest::configure $argv
# Capture the exit code.

proc tcltest::cleanupTestsHook {} {
    variable numTests
    set ::exitCode [expr {$numTests(Failed) > 0}]

}



::tcltest::runAllTests
exit $::exitCode
