#!/usr/bin/tclsh

set host [lindex $argv 0]
set port [lindex $argv 1]

set fd [socket $host $port]
fconfigure stdout -translation binary -encoding binary -buffering full
fconfigure $fd    -translation binary -encoding binary

while {![eof $fd]} {
#    puts -nonewline  [read $fd 8192]
    chan copy $fd stdout -size 8192
    flush stdout
}