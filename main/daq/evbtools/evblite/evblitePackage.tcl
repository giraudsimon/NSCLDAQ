#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file evblite.tcl
# @brief Package that provides event builder-lite access to ReadoutCallouts and manager.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide evblite 1.0
package require snit


# hide everything in the evblite namespace

namespace eval evblite {

##
# we assume that the DAQ environment variables have been set up.
# If not, we attempt to figure them out as DAQTCLLIBS = script evbliteLocation ..
# and DAQBIN as script evbliteLocation ../../bin
#
set evbliteLocation [info script]
if {[array names env DAQTCLLIBS] ne ""} {
    set tcllibs $env(DAQTCLLIBS)
}  else {
    set tcllibs [file normalize [file join $evbliteLocation ..]]
}

lappend auto_path $tcllibs

if {[array names env DAQBIN] ne ""} {
    set bindir $env(DAQBIN)
} else {
    set bindir [file normalize [file join $evbliteLocation .. .. bin]]
}
#
#  Program paths we'll need:

set ringtostdout [file join $bindir ringtostdout]
set stdintoring  [file join $bindir stdintoring]
set evbtagger    [file join $bindir evbtagger]
set glom         [file join $bindir glom]

##
# evblite
#   Encapslates an event builder lite instance.
# METHODS:
#   start   - start the event builder lite.
#   stop    - Stop the instance.
#   help    - Returns human readable option documentation.
#
# OPTIONS
# *    -dt   - Glom build window.
# *   -nobuild - boolean flag - if true, glom won't build -defaults false.
# *   -timestamp-policy - glom's timestamp policy, defults to earliest may be
#                earliest, latest, average.
# *   -outsid  - Source id that Glom will emit in built event.s
# *   -maxfragments - largest number of fragments in an emitted event. Defaults to 1000.
# *   -inbuffersize - input buffersize in kbytes defaults to 1024 (1Megabyte).
#                   must be large enough to accomodate the largest ring item.
# *   -resetts - If an item specifies that it has a null timestamp, a timestamp is
#                assigned to its fragment header that is the timestamp  of the most
#                recent item.  This specifies that the most recent timestamp variable
#                be zeroed when a begin run item is seen.
# *   -insid   - For items with no body header (note PHYSICS_EVENT items must
#                have a body header).
# *   -inring  - URI of the ringbuffer that will be used as the data source.
# *   -outring - name of the output ringbuffer.  Will be created if needed.
#
snit::type evblite {
    option -dt -default 0 -type [list snit::integer -min 0]
    option -nobuild -default 0 -type [list snit::boolean]
    option -timestamp-policy -default earliest  \
        -type [list snit::enum -values [list earliest latest average]]
    option -outsid -default 0 -type [list snit::integer -min 0]
    option -maxfragments -default 1000
    option -inbuffersize -default 1024 -type [list snit::integer -min 1]
    option -resetts -default 1 -type [list snit::boolean]
    option -insid -default 0 -type [list snit::integer -min 0]
    option -inring
    option -outring
    
    #  These options must be nonempty:
    
    variable requiredOptions [list -dt -insid -inring -outring]
    
    # pipeline pids
    
    variable pidList -1;          
    ##
    # constructor
    #   @param args - command line options.
    #   @note we can't check for the presence of all required options at this time
    #        as the caller can config them into existence.
    #
    constructor args {
        $self configurelist $args
    }
}


};                    # End of evblite namespace.