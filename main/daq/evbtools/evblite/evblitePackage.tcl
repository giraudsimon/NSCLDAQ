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
package require ringutils


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
set ringbuffercmd [file join $bindir ringbuffer]

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
    
    variable requiredOptions [list -inring -outring]
    
    # pipeline pids
    
    variable pidList [list]
    ##
    # constructor
    #   @param args - command line options.
    #   @note we can't check for the presence of all required options at this time
    #        as the caller can config them into existence.
    #
    constructor args {
        $self configurelist $args
    }
    ############################################################################
    #  Public methods
    
    ##
    # start evb-lite
    #   - Make sure the inring and the outring have been set.
    #   - If we think we're already running then stop
    #   - If our output ringbuffer does not yet exist, make it.
    #   - Construct the pipeline command
    #   - exec it inthe background.
    # @note that since this is a pipe grounded in ringbuffers on bothends,
    #    we can't capture the output/errors.
    method start {} {
        if {![$self _ringsSpecified]} {
            puts stderr "evblite: -inring and -outring must have been set and at least one of them has not been"
            exit 1
        }
        if {[$self _isRunning]} {
            $self stop
        }
        $self _makeOutRingIfNeeded
        
        set command [$self _constructPipeCommand]
        # puts $command;# debugging.
        set pidList [exec {*}$command &]
    }
    ##
    # stop
    #   Stops each PID in the pidList.  This is done with kill -9 inside
    #   catch blocks that ignore the errors since killing one might kill all.
    #
    method stop {} {
        foreach pid $pidList {
            catch {exec kill -9 $pid}
        }
    }
    
    ###########################################################################
    #  Private utility methods.
    
    ##
    # _ringsSpecified
    #   @return bool - false if one of -inring or -outring is an empty string.
    #
    method _ringsSpecified {} {
        if {[$self cget -inring] eq ""} {
            return 0
        }
        if {[$self cget -outring] eq ""} {
            return 0
        }
        return 1
    }
    ##
    # _makeOutRingIfNeeded
    #   If the output ring does not exist, create it with the default parameters.
    #
    method _makeOutRingIfNeeded {} {
        set usage [getRingUsage]
        set outring [$self cget -outring]
        set index [lsearch -index 0 $usage $outring]
        if {$index == -1} {
            exec $::evblite::ringbuffercmd create $outring
        }
    }
    ##
    # _constructPipeCommand
    #   @return list - which makes up the pipeline command.
    #
    method _constructPipeCommand {} {
        set result [list]
        
        #  First pipeline element is a ringtostdout on the source ring:
        
        lappend result $::evblite::ringtostdout [$self cget -inring] |
        
        # Second element is the evbtagger with appropriate options added:
        
        lappend result $::evblite::evbtagger --buffersize [$self cget -inbuffersize] \
            --sourceid [$self cget -insid]
        if {[$self cget -resetts]} {
            lappend result --resetts;     # toggles it to off.
        }
        lappend result |
        
        #  Third element is glom:
        
        lappend result $::evblite::glom --dt [$self cget -dt] \
            --timestamp-policy [$self cget -timestamp-policy] \
            --sourceid [$self cget -outsid] --maxfragments [$self cget -maxfragments]
        
        if {[$self cget -nobuild]} {
            lappend result --nobuild
        }
        lappend result |
        
        # Final element is stdintoring
        
        lappend result $::evblite::stdintoring [$self cget -outring]
        
        return $result
    }
    ##
    # _isRunning
    #   @return bool - true if we think we're already runnin.
    #   @note This can have the side-effect of setting the pidList instance
    #         variable if there's already a producer on the out ring and
    #         we have no record of starting it.
    #   @note regarding the note above, what can happen is that we start the
    #         the event builder, exit, and run again with the old event builder
    #         still running.
    #
    method _isRunning {} {
        #  IF we have a pid list we're running:
        
        if {[llength $pidList] > 0} {
            return 1
        }
        # But someone else might have started so look for a producer
        # on our output ring:
            
        # if so we'll put it's pid in our pid list..._probably_ stopping it
        # will stop the whole pipeline in the [stop] method...unless some other
        # _user_ stopped it in which case we're pretty much unable to stop.
        
        set usage [getRingUsage]
        
        # What we have is a list of elements like {ringname {- - - producer - - -}}
        # where - means we don't care what it is.
        
        set index [lsearch -index 0 $usage [$self cget -outring]]
        if {$index == -1} {
            return 0
        }
        set ringInfo [lindex $usage $index]
        set ringUsage [lindex $ringInfo 1]
        set producerPid [lindex $ringUsage 3]
        
        if {$producerPid == -1} {
            return 0;      # no producer.
        } else {
            set pidList $producerPid ;  # so in stop we can try to kill it.
            return 1;     # Assume we're the producer.
        }
    }
}


};                    # End of evblite namespace.