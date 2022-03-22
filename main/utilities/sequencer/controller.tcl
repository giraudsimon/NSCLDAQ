#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#

package provide controller 1.0
package require ReadoutGUIPanel
package require ReadoutGui
package require ReadoutControl
package require ExpFileSystem


# sample controller for starting/stopping runs on behalf of the
# sequencer:
set ::runFlag 0
set ::runNumber 0;   #Initial run of the plan.

namespace eval ::seqController {
    variable table
    variable button
    variable plannedRun 0
}

#
#  Return the event file in the complete dir for a run/seq
#  pair.
proc eventFile {run seq} {
   set evtfile [ExpFileSystem::WhereisRunFile $run $seq]

    # evt file is the name in the run dir .. need to locate
    # the complete dir and graft that on instead.
    set tail    [file tail $evtfile]
    set dir     [ExpFileSystem::WhereareCompleteEventFiles]
    set evtfile [file join $dir $tail]


    return $evtfile
}
#
#  Deletes data associated with a run
# run - The run number to delete data for.
#
proc deleteData run {
    puts -nonewline "Deleting data for run $run..."
    flush stdout
    set seq 0

    set evtfile [eventFile $run $seq]

    while {[file exists $evtfile] } {
        exec rm -f $evtfile
        incr seq
        set evtfile [eventFile $run $seq]
    }
    exec rm -rf [ExpFileSystem::WhereisRun $run]
    puts "done"

}

# This is our hook on the end of run.
# we increment the runFlag which in turn will
# bounce startPlannedRun out of vwait if it's there..
# allowing the next step of the run plan to be called.
#

proc OnEnd run {
    global runFlag

    # Only do any of this, of course if this is a planned run:
    #
    if {$::seqController::plannedRun} {
      incr runFlag
      after 500 {nextStep $seqController::button $seqController::table}
    }
}


# Called when a run plan is being started.  We just remember
# the first run number.
#
proc startingPlan {} {
    global runNumber
    set runNumber  [ReadoutControl::GetRun]
    set ::seqController::plannedRun 1;	# Runs are planned.

}
#
#  Called when a run plan has been aborted.
#  If event recording was enabled, ask the user if they
#  want to trash the runs they took:
#
proc planAborted {} {
    global runNumber
    #  with 11.x we can't as easily find our recorded data after the fact...
}


#
# Start a run from the run plan

#   button           - is the button widget controlling the run plan
#   table            - is the table widget controlling the run plan
#
proc startPlannedRun {button table} {
    global runFlag
    set seqController::table $table
    set seqController::button $button
      
    if {[ReadoutControl::getState] ne "NotRunning"} {
        tk_messageBox -icon error -title "Run active" \
            -message {The Readout GUI is not in a state that allows a run to be started. Correct that.}
        abortPlan $button $table 0
    } else {

        # Force the timed run on.

        ReadoutGUIPanel::setTimed 1

        # Start the run.  vwait for the
        # run to end so that we can call
        # nextStep.. note that vwait will wait in the
        # event loop keeping the GUI alive.
	
         
         ReadoutControl::Begin

    }

}
#
#  Called to abort an in-flight run plan.
#  Just end the run.
#
proc stopPlannedRun {} {
    ReadoutControl::End

}
