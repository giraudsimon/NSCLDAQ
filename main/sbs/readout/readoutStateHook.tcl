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
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file readoutStateHook.tcl
# @brief Hook the readout program into the NSCL State manager.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide ReadoutStateHook 1.0


##
# The big assumption for this file is that Readout is being run from
# the boot program.  Under that assumption, the environment variables
# 'TRANSITION_REQUEST_URI'  and  'TRANSITION_SUBSCRIPTION_URI' are
# defined to be the URI's that get us connected to the state manager.
# In addtion, 'DAQROOT' is defined to point at the top level of the
# NSCLDAQ installation directory.
#


#  Pull in the state manager:


set nsclTclLibs [file join $env(DAQROOT) TclLibs]
lappend auto_path $nsclTclLibs
package require   statemanager


#-----------------------------------------------------------------------
# Script procs...they're all going to be in the StateAwareRdo namespace:

namespace eval ::StateAwareRdo {
    set active 0    
}

##
# ::StateAwareRdo::toActive
#
#   Handle the toActive transition.  This is only done if the prior state
#   was Ready so that we don't jump in in the middle of a run.
#
# @param prior - previous state.
# @param state - Current state (Active).
#
proc ::StateAwareRdo::toActive {prior state} {
    if {[string toupper $prior] eq "READY"} {

	# Update the title/run number from the state manager:
	# doing it this way insulates us from the effect of the run/title
        # changing in the midst of a run.

	set ::title [::statemanager::statemonitor gettitle]
	set ::run   [::statemanager::statemonitor getrun]

        #
        #  If begin fails... go to not ready
        #
        if {[catch begin msg]} {
            puts "Begin failed: '$msg'"
            ::statemanager::statemonitor transition FAIL
            return
        }
        set ::StateAwareRdo::active 1;               # Run in progress
    }
}
##
# ::StateAwareRdo::toReady
#
#   Handle transitions to the Ready state.  We only act if we are
#   known to be coming from the active state...and the run is in progress
#   This prevents us from stopping a run that is not actually in progress.
#   and from acting when whe are just getting an initial state (probably
#   we don't actually have to factor the prior state into this calculation).
#
# @param prior - prior state.
# @param state - Current state (Ready).
#
proc ::StateAwareRdo::toReady {prior state} {
    if {([string toupper $prior] eq "ACTIVE") && ($::StateAwareRdo::active)} {
        if {[catch end msg]} {
            puts "End failed: '$msg'"
            ::statemanager::statemonitor transition FAIL
            return
        }
        set ::StateAwareRdo::active 0
    }
}
##
# ::StateAwareRdo::toNotReady
#
#  Handles transitions to the NotReady State.  NotReady is entered only
#  if the program is supposed to exit:
#  * If data taking is in progress, the run is ended.
#  * We exit.
#
# @param prior - the prior state.
# @param state - The current state (NotReady).
#
proc ::StateAwareRdo::toNotReady {prior state} {
    if {$::StateAwareRdo::active} {
        catch end msg;                    # Don't let failure kill us.
    }
    exit 0;                           # exit the program
}
##
# onTriggerFail
#   Called if the trigger loop failed.  In this case:
#   * End our run cleanly.
#   * Fail the system.
#
# @param msg - reason for the failure.
#
proc onTriggerFail msg {
    puts "The trigger loop failed: $msg"
    catch end
    statemanager::statemonitor transition FAIL
}
#------------------------------------------------------------------------
# Script main entry point
#

#  Start the state manager thread/event-loop pump.

set transURI $env(TRANSITION_REQUEST_URI)
set stateURI $env(TRANSITION_SUBSCRIPTION_URI)

statemanager::statemonitor start $transURI $stateURI

# Register interest in state
# We care about Active, Ready and NotReady
#

statemanager::statemonitor register Active   ::StateAwareRdo::toActive
statemanager::statemonitor register Ready    ::StateAwareRdo::toReady
statemanager::statemonitor register NotReady ::StateAwareRdo::toNotReady




# Falling through enters the readout program's event loop allowing poked commands
# to work too.
