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
#	     The FRIB
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file ReadoutControl.tcl
# @brief Provides the ReadoutControl that the sequencer used to depend on.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide ReadoutControl 2.0
package require ReadoutGUIPanel
package require RunstateMachine

##
# The sequencer package was originally written on NSCLDAQ-10.x
# In 11, the ReadoutGui was totally re-written and therefore we needed to
# adjust stuff works.
#   The package expected a ReadoutControl package.  This is gone in the Readout
# GUI but we're going to provide it here so the edits to controller.tcl are
# minimized.
#
#  We have namespaced commands (in ReadoutControl) and must implement:
#    GetRun - returns the current run number.
#    isTapeOn - returns the current recording state.
#    getState - Returns the state of the system. Note that
#               this used to be a variable and is now a proc and that the
#               actual textual returned values will not be mapped
#               into the old state names expected by controller.tcl:
#               Active, NotRunning, and NotReady.
#               NotReady is a new state, and Active includes the Paused state.
#    Begin   - Start a run (was in ReadoutGui).
#    End     - End a run (was in ReadoutGui).


namespace eval ReadoutControl {
    ##
    # GetRun
    #    @return the run number currently in the GUI.
    #
    proc GetRun { } {
        return [::ReadoutGUIPanel::getRun]
    }
    ##
    # isTapeOn
    #   @return true if recording is enabled.
    #
    proc isTapeOn { } {
        return [ReadoutGUIPanel::recordData]
    }
    ##
    # getState
    #   @return textual state name:
    #   @retval NotRunning - if the system in the Halted state.
    #   @retval Active     - If the system is Active or Paused.
    #   @retval NotReady   - If the system is in the NotReady state.
    #
    proc getState {} {
        set stateManager [RunstateMachineSingleton %AUTO%]
        set rawState [$stateManager getState]
        $stateManager destroy
        set result "Unknown"
        if {$rawState in [list Halted]} {
            set result NotRunning
        } elseif {$rawState in [list Active Paused]} {
            set result Active
        } elseif {$rawState in [list NotReady Starting]} {
            set result NotReady
        }
        return $result
    }
    ##
    # Begin
    #   start a run:
    #
    proc Begin { } {
        begin;                       # in state machine.
    }
    ##
    # End
    #    End a run
    #
    proc End { } {
        end;                        # in state machine.
    }
}

