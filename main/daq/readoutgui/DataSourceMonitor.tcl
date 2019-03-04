#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file DataSourceMonitor.tcl
# @brief Monitor liveness of data sources
# @author Ron Fox <fox@nscl.msu.edu>

package provide DataSourceMonitor 1.0
package require RunstateMachine
package require ReadoutGUIPanel
package require DataSourceManager

##
# This file provides a run state machine callout bundle.
# When data sources are supposed to be active (when the state is other than
# NotReady or Starting), the data source manager
# is periodically polled for the states of all data sources.
#
# If any data source drops out:
#  *   A message is logged indicating this.
#  *   If the state is Active or Paused it is forced to the Halted state.
#  *   All data sources are stopped
#  *   The state is forced to NotReady.
#
#  This puts the system in a state where the data sources can be restarted
#  when either circumstances or fixes to the failing data source warrant.
#




namespace eval ::DataSourceMonitor {
    variable afterId          -1;
    variable monitorInterval  1000; # ms between checks.
    variable dsm              [DataSourcemanagerSingleton %AUTO%]
    
    # Mandatory bundle exports.
    
    namespace export attach enter leave
}
##
# ::DataSourceMonitor::_getSourceInfo
#
# Return the dict of source information from the list of known sources:
#
# @param id      - Source id.
# @param sources - list of sources.
#
proc ::DataSourceMonitor::_getSourceInfo {id sources} {
    foreach sourceDict $sources {
        if {[dict get $sourceDict sourceid] == $id} {
            return $sourceDict
        }
    }
    error "::DataSourceMonitor::_getSourceInfo - source not in sources list!"
}

##
# ::DataSourceMonitor::_checkSources
#
#   Auto-rescheduling proc that checks the liveness of the data sources
#   and handles the case where a data source is no longer running.
#
# @param interval - ms between reschedule.
#
proc ::DataSourceMonitor::_checkSources ms {
     
    set results [$::DataSourceMonitor::dsm check]
    set sources [$::DataSourceMonitor::dsm sources]
    set aok      1
    #
    #  We're always going to interate over all sources because
    #  we want to log messages for all sources tht died:
    #
    dict for {sourceId status} $results {
        if {!$status} {
            set aok 0;           # Remember that at least one source failed.
            
            set sourceInfo [::DataSourceMonitor::_getSourceInfo $sourceId $sources]
            set provider [dict get $sourceInfo provider]
            #
            #  Make the pure parameterization dict
            #
            dict unset sourceInfo sourceid
            dict unset sourceInfo provider
            #
            # Dicts have a string rep like a list of key value so:
            #
            set parameterization [join [string trim $sourceInfo] " = "] 
            ::ReadoutGUIPanel::Log DataSourceMonitor error \
                "Data source $sourceId exited ($provider $parameterization)"
            
        }
    }
    #
    #  If at least one source failed, stop them all and transition to
    #  NotReady:
    #
    if {!$aok} {
        ReadoutGUIPanel::Log DataSourceMonitor error "At least one data source has exited"
        #
        #  If the run is active try to stop it so that event files get properly
        #  closed.. note that the event logger will likely time out
        #
        #  Note not at all sure why this is being done but it takes much too long
        #  tentatively commented out and we do a quick transition to not ready
        #  putting the onus on the bundles to do the right thing when/if this
        #  happens
        
        if {0} {
            if {[$::DataSourceMonitor::sm getState] ne "Halted"} {
                ReadoutGUIPanel::Log DataSourceMonitor warning "Stopping run - if recording data this could take some time"
                $::DataSourceMonitor::sm transition Halted
            }
        }
        
        #  -> not ready
        
        ReadoutGUIPanel::Log DataSourceMonitor warning "Emergency transition to not ready:"
        set sm [::RunstateMachineSingleton %AUTO%]
        $sm transition NotReady
        destroy $sm

    }
 
    #
    #  May as well cancel self now if a source failed:
    #
    if {$aok}  {
        set ::DataSourceMonitor::afterId \
            [after $ms [list ::DataSourceMonitor::_checkSources $ms]]
    } else {
        set ::DataSourceMonitor::afterId -1
    }
}

#------------------------------------------------------------------------------
#  Bundle procs

##
# ::DataSourceMonitor::attach
#
#   Called when the bundle is registered with the state manager.
#   not used.
proc ::DataSourceMonitor::attach {state} {}

proc ::DataSourceMonitor::precheckTransitionForErrors {from to} {
	return [list]
}

##
# ::DataSourceMonitor::leave
#
#  Called when the state machine leaves a state
#  Not used.
#
proc ::DataSourceMonitor::leave {from to} {}
##
# ::DataSourceMonitor::enter
#    Called when a state is entered.  In our case:
#    *  If we transitioned from the Starting to the Halted state
#       we start monitoring the data sources.
#    *  If we transitioned to the NotReady state and we were monitoring
#       data sources (::DataSourceMonitor::afterId != -1), monitoring
#       is turned off.
#
# @param from  - The state we just left.
# @param to    - The state we are now in.
proc ::DataSourceMonitor::enter {from to} {
    if {$from eq "Starting" && $to eq "Halted"} {
        
        # Start monitoring:
        
        ::DataSourceMonitor::_checkSources $::DataSourceMonitor::monitorInterval
        
    } elseif {$to eq "NotReady" && $::DataSourceMonitor::afterId != -1} {
        
        # Stop monitoring by cancelling the after and marking it cancelled:
        
        after cancel $::DataSourceMonitor::afterId
        set ::DataSourceMonitor::afterId -1
    }
}

##
# ::DataSourceMonitor::register
#
#  Register with the statemachine:
#
proc ::DataSourceMonitor::register {} {
  set sm [::RunstateMachineSingleton %AUTO%]
  $sm  addCalloutBundle DataSourceMonitor
  $sm destroy
}



