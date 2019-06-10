#!/bin/sh
#
#   Server to set up pipelines to hoist data to
#   external clients in compatibility mode.
#
#  Usage:
#     spectcldaq.server url port
#
#  Note:
#    The BUFFERSIZE env variable is honored if present.
#
#\
    exec tclsh ${0} ${@}

# spectcldaq is assumed to be here.
#

set here [file dirname [info script]]

#
#  Error message showing the script usage:
#
#  
proc usage {} {
    puts stderr "Usage:"
    puts stderr "    spectcldaq.server url port"
    puts stderr " url - URL of ring to hoist data from"
    puts stderr " port - TCP/IP port on which to listen for data."

    exit
}

#
#  Accept a connection by setting up the output pipeline.
#
#
# Parameters:
#   socket      - socket to use to communicate with the remote client.
#   remoteaddr  - Remote host ip.
#   remoteport  - Remote port ip.
proc accept {socket remoteaddr remoteport} {
    global here
    global env
    global url

    set selector  [file join $here ringselector]
    set formatter [file join $here compatibilitybuffer]
    set bsize ""
    if {([array names env BUFFERSIZE] ne "") && 
	[string is integer $env(BUFFERSIZE)]} {
	set bsize "--buffersize=$env(BUFFERSIZE)"
    }
    exec --  $selector --sample=PHYSICS_EVENT --source=$url | $formatter $bsize >@ $socket &
    
    # give the child process a chance to get going before closing the socket.

    after 500 "close $socket"
}

# Entry point check the paramters and pull them out:
#

if {[llength $argv] != 2} {
    usage
}

set url  [lindex $argv 0]
set port [lindex $argv 1]


set serverSocket [socket -server accept $port]

vwait forever


