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
# @file   logbookBundle.tcl
# @brief  Provide/register a bundle to interface with the logbook application
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide logbookbundle 1.0
package require logbookadmin
package require StateManager
package require RunstateMachine
package require ReadoutGUIPanel
package require Tk;               # Ensure we can pop up dialogs of various types.


##
# This package provides a logbook bundle that will record the run state
# transition that occur in any recorded run.
#  Note that:
#    1. A current logbook database must have been previously estblished e.g.
#       by using lg_current
#    2. If on entry the current logbook _has_ been established and there's an
#       active current run, it will be given an emergency end so that a new run
#       can be started without an error being thrown.
#
namespace eval ::LogBookBundle  {
    variable registered 0
    
    ##
    # MustLogTransition
    #   Given a from/to combination determines if the transition requires logging.
    #   some don't (e.g. NotReady -> Starting, Starting->Halted and Halted->NotReady)
    #   Note that Active/Paused -> NotReady merits an emergency end and therefore
    #   requires a transition.
    #  
    # @param from - current state.
    # @param to   - State we're transitioning to.
    # @return bool - True if this state transition must be logged.
    # @note we also only log if recording is enabled.
    #
    proc MustLogTransition {from to} {
        if {![::ReadoutGUIPanel::recordData] } {
            return 0
        }
        if {$from in [list NotReady Starting]} {
            return 0
        }
        if {$from in [list Active Paused]} {
            return 1;                # This is a state transition.
        }
        #
        #  Halted requires a transition if it's transitioning to Active.
        #  everything else does not require logging.
        
        if {($from eq "Halted") && ($to eq "Active")} {
            return 1
        }
        return 0
    }
    ##
    # ShiftSelected
    #   Ensure there's a current shift prior to trying to log a state change.
    #
    proc ShiftSelected { } {
        while {[currentShift] eq ""} {
            tk_MessageBox -title "Need a shift" -icon info -type ok \
                -message {Logging this state transition requires that a current shift be selected.  Use e.g. lg_selshift to do so}
        }
    }
    ##
    # LogTransition
    #   Actually logs a transition given that one must be logged
    #   and that we can log it.
    #
    # @param from - state we're leaving.
    # @param to   - state we're about to enter.
    #
    proc LogTransition {from to} {
        #
        # Transitions to NotReady with from other than Halted
        #  Require an emergency end:
        
        if {($from ne "Halted")  && ($to eq "NotReady")} {
            emergencyEndRun {Unexpected state transtion to  not ready in the middle of an active run}
        }
        #
        #  Unfortunately all the rest must be individually handled:
        #
        if { ($from eq "Halted") && ($to eq "Active")} {
            # Begin run:
            set number [::ReadoutGUIPanel::getRun]
            set title  [::ReadoutGUIPanel::getTitle]
            
            beginRun $number $title {Logged automatically by LogBookBundle}
            
        } elseif {($to eq "Halted")} {
            # end run:
            
            endRun {Logged automatically by LogBookBundle}
            
        } elseif {($to eq "Paused")} {
            # Pause run:
            
            pauseRun {Logged automatically by LogBookBundle}
            
        } elseif {($from eq "Paused") && ($to eq "Active")} {
            # Resume run:
            
            resumeRun {Logged automatically by LogBookBundle}
            
        } else {
            # Should not get here
            
            tk_messageBox -title {LogBookBundle error?!?} -icon error -type ok \
                -message "LogBookBundle::LogTransition called but I don't know how to log $from -> $to let developers know there's an error and include this message in your report"
        }
    }
}
namespace eval ::LogBookBundle {
    namespace export attach enter leave
    
}

##
# LogBookBundle::attach
#
#     Called when the bundle is attached to the state machine.
#     - The state had better not be one in the middle of a run.
#     - There better be a current log book selected.
#     - If the logbook has a current run, that run is given an emergency end.
#
# @param state - the current state.  This had better be
#                one of NotReady, Starting or Halted.
#                ... else we're joining in the middle of a run and that we cannot
#                do.
proc ::LogBookBundle::attach {state} {
    puts "Logbook bundle attach $state called."
    currentLogBookOrError;         # Just to make the error.
    if {$state ni [list NotReady Starting Halted]} {
        tk_messageBox -title {Attach too late} -type ok -icon error \
            -message {It's not legal to register the logbook bundle in the middle of a run}    
        exit -1
    }
    # If there's a current run we need to do an emergency end:
    
    if {[currentRun] ne ""} {
        emergencyEndRun {LogBookBundle detected startup after run ended imporoperly. Forcing log book to end the run}
    }
}
##
#  We're going to do all our work on leave.
#
proc ::LogBookBundle::enter {from to} {
    
}
##
# LogBookBundle::leave
#    Called when a state is about to be left.  If the to state is anything
#    that requires logging, we need to ensure that a current shift is select
#    If not, use promptForShift to ensure the user selects one.
#
proc ::LogBookBundle::leave {from to} {
    if {[::LogBookBundle::MustLogTransition $from $to]} {
        LogBookBundle::ShiftSelected
        
        LogBookBundle::LogTransition $from $to
    }
}

##
#  Register the logbook bundle.  If there's an active run,
#  force an emergency end to it.  We protect against multiple registration
#  so that we don't wind up forcing accidental transitions.
#
if {$::LogBookBundle::registered == 0} {
    set stateMachine [RunstateMachineSingleton %AUTO%]
    $stateMachine addCalloutBundle LogBookBundle
    set ::LogBookBundle::registered 1
    puts "Logbook  bundle registered"
}

    
