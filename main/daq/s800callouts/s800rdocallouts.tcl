#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2009.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

package provide s800callouts 1.0


puts "---------------------- S800 readout callouts script -------------"

#
#  Provides readout callout methods for the s800.
#
#  These just match the regular readoutcallouts methods but are in an
# s800 namespace.
# The assumption is that the s800 package is in our search path:
#
package require s800
package require ReadoutGUIPanel

namespace eval s800 {
    variable initialized false;	# If initialization has not taken place.
    variable s800        ""
    variable record
    variable unexpectedEnd [list]; # Command to execute on unexpected run end.
    variable runState      {unknown}
    variable pipelineRunning 0;	# 'true' if the catcher/feeder pipeline is running.
    variable here          [file dirname [info script]]; # Locate ourself.
    variable s800Host      spdaq48;			 #  Default s800 spdaq
    variable s800Port      9002;			 # Default s800 port.
    variable s800Ring      "s800_$::tcl_platform(user)";			 # Default s800 destination ring
    variable feederFd     -1;				 # If open fd on pipe.
    variable pollId       -1
    variable disconnected 0;	# Disconnected message given...
}


##
# Self rescheduling proc that monitors the state of the S800.
# for now the entire body is in a catch loop and errors
# result in the state being set to 'unknown'.
#
# @param ms - Milliseconds of periodicity.
#
proc s800::monitorState ms {
    set ::s800::pollId [after $ms [list ::s800::monitorState $ms]]
    set status [catch {
	set s800State [$::s800::s800 getState]


    } msg]

    # On failure, set state to unknown.

    if {$status} {
	set s800State Unknown
	
	if {!$::s800::disconnected} {
	    set ::s800::disconnected 1
	    tk_messageBox -title {S800 monitor failed} \
		-message "monitor state failed: $msg will try to reconnect periodically."
	}
    } else {
	# Re-assert slave mode:
	# if necessasry
	if {$s800::disconnected} {
	    $::s800::s800 setSlave
	}
	set s800::disconnected 0; # Mark connected.
    }

    # Set the s800 state only if it changed.  Since there may
    # be traces on it we only want to fire them if necessary:

    if {$s800State ne $::s800::runState} {
	puts "S800 state was $s800::runState -> $s800State"
	set ::s800::runState $s800State
    }

}

##
# Called in response to  a change in run state.  If the state change is
# out of 'active', and there's an unexpectedEnd handler it is called.
#
# @param name1 - Name of the state variable.
# @param name2 - array index or empty string if not an array.
# @param op    - Operation .. should be 'write'.
proc s800::stateChange {name1 name2 op} {
    set newState [set $name1]
    if {($newState ne "active") && ($::s800::unexpectedEnd ne "")} {

	uplevel #0 $::s800::unexpectedEnd $newState
	
    }
}

##
#   Called in respnose to destruction of the . widget...
#   or any widget in its tree.
#   Force the exit to go through our clean up handling.
#
# @param widget - the widget being destroyed.
#
proc s800::DestroyHandler {widget} {
    if {$widget eq "."} {
	exit
    }
}
#
#  Exit handler - puts the s800 back in master mode, if the socket
# is working then call the real exit
#
# @param status - the status of the real exit.
proc ::s800::Exit {{status 0}} {
    proc bgerror {msg} {
	puts "background error: $msg"
	::s800::real_exit 0;			# Disable background errors.
    }
    puts "S800 exit"
    if {$::s800::s800 ne ""} {
	if {[catch {
	    if {[$::s800::s800 getState] eq "active"} { # end any active run
		puts "ending active run"
		$::s800::s800 end
	    }
	    puts "Setting active"
	    $::s800::s800 setMaster
	    destroy $::s800::s800
	    set ::s800::s800 ""
	} msg]} {
	    puts "Failed to end s800 run: $msg"
	}
    }
    # Kill off the feeder if it's alive.  Note that this
    # is unix specific because it execs 'kill.'
    #

    if {$::s800::pipelineRunning} {
	catch {
	    puts "Killing pipeline"
	    set pids [pid $::s800::feederFd]
	    fileevent $::s800::feederFd readable ""
	    foreach pid $pids {
		puts "Killing $pids"
		catch {exec kill -9 $pid} msg
		puts "Kill $pid - $msg"
	    }
	    close $::s800::feederFd
	}
    }
    after cancel $::s800::pollId
    catch {destroy .}
    ::s800::real_exit $status
}

## Get a value from its default and env name.
#
#  @param default - default value.
#  @param envName - Name of env variable that overrides.
# 
# @return value - the final value.
#
proc s800::getValue {default envName} {
    if {[array names ::env $envName] eq "" } {
	return $default
    }
    return $::env($envName)
}

##
#  If the feeder pipeline gets readable, we read 
#  if the pipe got closed, close our end and set piplineRunning false.
#
# @param fd - File descriptor open on the pipe.
#
proc s800::pipeReadable fd {
    if {[eof $fd]} {
	close $fd
	set ::s800::pipelineRunning 0
    } else {
	ReadoutGUIPanel::outputText "s800: [gets $fd]"
    }
} 

##
# Start the s800 catcher/ring feeder process.
#
proc s800::_startFeeder {} {
    set host [::s800::getValue $::s800::s800Host S800_HOST]
    set port [::s800::getValue  $::s800::s800Port S800_PORT]
    set ring [::s800::getValue $::s800::s800Ring  S800_RING]


    set bindir [file join $::s800::here .. .. bin]
    set netcat [file join $bindir netcat.tcl]
    set btoring [file join $bindir BufferToRing]
    set stdintoring [file join $bindir stdintoring]


    set shmbase [file join /dev shm]
    set shmname [file join $shmbase $ring]
    if {![file exists $shmname]} {
	exec $bindir/ringbuffer create $ring
    }

    set fd [open "| $netcat $host $port | $btoring | $stdintoring $ring " r]


    
#    set fd [open "| [file join $bindir s800toring] $host $port $ring 2>@1" r]
    set ::s800::pipelineRunning 1

    # Setup a filevent to catch output/errors and close:

    fconfigure $fd -buffering line
    fileevent $fd readable [list ::s800::pipeReadable  $fd]

    set ::s800::feederFd $fd
}


#------------------------------------------------------------------
#
#  This should be explicitly called to initialize thec connection with
#  the remote run control system.
#
# @param host - host running the s800 readout.
# @param port - Port on which it listens for run control connections;
#
# @note - The s800 is put in to slave mode.
# @note - exit is renamed to s800::exit and 
#         1. exit is defined here to set the server to master mode
#            and close the connection prior to doing an s800::exitr
#         2. has a destroy handler set to do the same thing.
proc s800::Initialize {{host localhost} {port 8000}} {
    if {[catch {

    set s800::s800 [s800rctl %AUTO% -host $host -port $port ]
    

    # Set the S800 in slave mode and, if it is running stop it to match
    # the current GUI state:



    set state [$::s800::s800 getState]
    if {$state eq "active"} {
	tk_messageBox -title  {s800 run active} \
	    -message {The S800 run is active and must be stopped first}
	exit -1
    }
    $::s800::s800 setSlave; 
} msg]} {
	tk_messageBox -title {S800 protocol error} -message "Error communicating with S800@$host: $msg"
	exit -1
    }

    # Remove the pause/resume button....once the GUI has time to start.

    after 1000 [list if {[winfo exists .pausres]} {grid forget .pauseres}]
    
    #  Set destroy handler:
    
    wm protocol . WM_DELETE_WINDOW exit; # Ensure exits on delete window are clean.
    
    # Rename exit.
    
    rename ::exit ::s800::real_exit
    rename ::s800::Exit ::exit
    
    toplevel .s800
    checkbutton .s800.record -text {Record Crate files} -variable ::s800::record \
	-onvalue 1 -offvalue 0
    pack .s800.record
    set ::s800::record 1


    # Status data for the s800:

    label .s800statel -text {S800 state: }
    label .s800state  -textvariable ::s800::runState


    # Append the s800 information to the bottom of the GUI;

    set gridDimensions [grid size .]
    set rows           [lindex $gridDimensions 1]
    incr rows

    grid .s800statel -row $rows  -column 1 -columnspan 2
    grid .s800state  -row $rows  -column 3

    s800::monitorState 2000;	# Monitor the s800 state every nms.
    
    
}

##
# onBegin - start the s800 run.
#
proc s800::OnBegin {} {

    # If necessary, start the feeder pipline assumed to be in 
    # here/../../bin

    if {!$::s800::pipelineRunning} {
	::s800::_startFeeder
    }

    set title [ReadoutState::getTitle]
    set run   [ReadoutState::getRun]
    set home  $::env(HOME)
    if {[catch {
	$::s800::s800 setTitle $title
	$::s800::s800 setRun   $run
	$::s800::s800 setDestination $home
	if {$::s800::record} {
	    $::s800::s800 setRecordingOn
	} else {
	    $::s800::s800 setRecordingOff
	}
	$::s800::s800 begin

	# On successful begin establish a state handler for runState
	# Since there can be a bit of time before the s800 transitions to active
	# and because of the way the event queue operates, the trace below gets scheduled
	# for 1 second in the future by which time (hopefully) the s800 is active.
	# if this does not work, then we'll have to do something a bit like the state machine
	# in gretina...watch the state change first to active and then only care about changes
	# from active -> inactive.
	#
	puts "Starting run"
	after 1000 {trace add variable ::s800::runState write ::s800::stateChange}

    } msg]} {
	tk_messageBox -icon error -title "S800 readout GUI failed" \
	    -message "BEGIN Error communicating with the s800 readout - $msg - restart the s800 readout and this GUI"
	::s800::real_exit -1
    }
} 

##
# onEnd - stop the s800 run.
#
proc s800::OnEnd {} {
    if {[catch {
	$::s800::s800 end

	# cancel the variable trace.

	trace remove variable ::s800::runState write ::s800::stateChange

    } msg]} {
	tk_messageBox -icon error -title "S800 readout GUI failed" \
	    -message "END Error communicating with the s800 readout - $msg - restart the s800 readout and this GUI"
	exit -1
    }
    puts "Recording state [ReadoutState::getRecording]"
    if {[ReadoutState::getRecording] == 0} {
	puts "Attempting to increment run."
	ReadoutGUIPanel::incrRun;	# change s800 run #

    }
}
