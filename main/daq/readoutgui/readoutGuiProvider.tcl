#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
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


##
# @file readoutGuiProvider.tcl
# @brief layer on top of an s800Provider that gives remote control of a ReadoutGUI
# @author Ron Fox <fox@nscl.msu.edu>

package provide RemoteGUI_Provider 1.0
package require S800_Provider
package require ui
package require ReadoutGuiClient
package require RunstateMachine


##
# This version only supports a single server
#
#

namespace eval ::RemoteGUI {
    
    
    #  Each data source has a dict that contains the following
    #  -  host - host we're connected to.
    #  -  user - User we're connected to.
    #  -  controlService - the remote control service name.
    #  -  outputService - The output forwarding service name.
    #  -  outputMonitor  - The output monitor object.
    #  -  id             - the source id.
    #   These dicts are kept in the dict below, indexed
    #   by source id:
    
    variable sourceInfo [dict create]
}

##
# parameters
#     Returns the set of parameters that are required to create the source
#    - host  - the host in which the readoutGUI is running
#    - user  - Username under which the readoutGUI is running
#
proc ::RemoteGUI::parameters {} {
    puts "Parameters called."
    return [dict create \
        host [list {Host Name}] user [list {User name}]  \
        remoteControlService [list {Remote control service} ReadoutGUIRemoteControl]    \
        remoteOutputService  [list {Remote output Relay} ReadoutGUIOutput]      \
    ]
}
##
# start
#    Start the data source.  This means
#    - Locating the run control port
#    - Locating the monitor port.
#    - Setting up to monitor output
#    - delegating the start to the s800::start.
#
# @param params - Parameterization of the source as a dict
# @note  The code starting us also sets a sourceid parameter
#        which identifies which source is starting.
#
proc ::RemoteGUI::start params {
    variable sourceInfo
    
    ::ReadoutGUIPanel::Log RemoteGUI output "start '$params'"
    
    # Extract the parameters:
    
    set sid  [dict get $params sourceid]
    set host [dict get $params host]
    set user [dict get $params user]
    set controlService [dict get $params remoteControlService]
    set outputService  [dict get $params remoteOutputService]
    errorIfDuplicate $sid;           # Sids must be unique.
    
    # Locate the s800 data source port, and attempt to start the data source
    # that will complain if there's already one.
                  
    set port [readoutGUIControlPort $host $controlService $user ]
    if {$port eq ""} {
        error "Unable to find a readout GUI in $host run by $user advertising on $controlService"
    }
    S800::start [dict create \
            port $port host $host sourceid $sid    \
    ]
    
    # Attempt to locate the output window and connect to it as well:

    
    set outputMonitor [ReadoutGUIOutputClient %AUTO% -host $host -user $user \
                        -outputcmd ::RemoteGUI::_handleOutput \
                        -closecmd [list ::RemoteGUI::_outserverClosed $sid] \
                        -service $outputService]
    $outputMonitor connect
    
    # Create the source description dict and add it to the
    # set we know about.
    
    set sourceDict [dict create \
        host $host user $user controlService $controlService \
        outputService $outputService outputMonitor $outputMonitor \
        id $sid                                              \
    ]
    dict append sourceInfo $sid $sourceDict
}
##
# check - See if we are still alive:
#
# @param id - the source id.
#
proc ::RemoteGUI::check id {
    set info [getSourceInfo $id];   # Errors if invalid sourceid.
    return [S800::check $id];             # sid will be the same.
}
##
# stop - stop the source by stopping the s800 and destroying the output
#        monitor.
# @param id - source id.
#
proc ::RemoteGUI::stop id {
    variable sourceInfo
    set info [getSourceInfo $id]
    
    # Stop output monitoring:
    
    set outSvc [dict get $info outputMonitor]
    ::ReadoutGUIPanel::Log RemoteGUI output "stop $id"
    catch {$outSvc destroy};   # Might have been destroyed already 
    
    # If still alive and necessary stop the run.
    set rctl [::S800::_getConnectionObject $id]
    set state [::S800::_getState $id]
    set status [::S800::check $id]

    if {($state ni {"halted"}) && $status} {
        
        set status [$rctl getState]
        if {$status ne "NotReady"} {
            $rctl masterTransition NotReady
        }
        # Regardless, _failed will run down the rest of this.
        

        ::S800::_failed $id;    # Will do all the right stuff.
                                # including destroying the connection object
        
    }

    # because ::S800::_failed could have been called in the conditional,
    # we need to make sure that the connection object has not been destroyed
    # already before we destroy it ourselves.
    if {$rctl in [::s800rctl info instances]} {
      $rctl destroy
    }
    
    # Now remove our source from the source dictionary.
    # - Forgetting the state of the source and
    # - Allowing the re-use of the source id by our clients.
    
    set sourceInfo [dict remove $sourceInfo $id]
}
##
# begin - start a run.
# @param id - source id
# @param run - run numer
# @param title
#
proc ::RemoteGUI::begin {id run title} {
    set info [getSourceInfo $id];    # Error if bad id.
    
    ::ReadoutGUIPanel::Log RemoteGUI output "begin $id $run $title"
    ::S800::begin $id $run $title

}
##
# end
#   End the run
# @param id - the source id
#
proc ::RemoteGUI::end id {
    set info [getSourceInfo $id]
    ::ReadoutGUIPanel::Log RemoteGUI output "End $id"
    ::S800::end $id
}

##
# init 
#   Initialize data sources 
# @param id - the source id
#
proc ::RemoteGUI::init id {
    set info [getSourceInfo $id]
    ::ReadoutGUIPanel::Log RemoteGUI output "Init $id"
    ::S800::init $id
}
##
# capbilities - the s800 capabilities:
#
proc ::RemoteGUI::capabilities {} {
    return [::S800::capabilities]
}

#------------------------------------------------------------------------------
#
#  Private methods:
#

##
# _handleOutput
#
#  Called to handle output from the remote Readout GUI ..just output it to our
#  display
#
# @param output - output from the remote
#
proc ::RemoteGUI::_handleOutput output {
    set ow [::Output::getInstance]
    $ow puts "Relayed: $output"
}
##
# _outserverClosed
#
#  Called when the output server closes...probably the ReadoutGUI has blown
#  away.
#   *  destroy the connection object and complain to the output log.
#   *  Start a transition to notready:
# @param id - id of the source who's output server closed.
#
proc ::RemoteGUI::_outserverClosed {id} {
    
    #  Log the disconnecty
    
    
    set sourceInfo [getSourceInfo $id]
    set outputMonitor [dict get $sourceInfo outputMonitor]
    
    $outputMonitor destroy
    
    set ow [::Output::getInstance]
    $ow log error "Lost connection to remote GUI output server."
    $ow log error "Probably the control server connection already was lost or soon will be"
    
    #  Transition to NotReady:
    
    forceFailure;            # in RunstateMachine pkg.

}
##
# errorIfDuplicate
#    Throws an errro if the source id given is already known
#
# @param id - source id to check
#
proc ::RemoteGUI::errorIfDuplicate {id} {
    variable sourceInfo
    if {[dict exists $sourceInfo $id]} {
        error "RemoteGUI source manager; duplicate source id $id"
    }
}
##
# getSourceInfo
#     Return the dict that provides information about the
#     specified data source. The error is controlled so that
#     any error text is meaningful rather than a dict key not found.
#
# @param id - id of the source to lookup.
# @return dict - the source description dict.
#
proc ::RemoteGUI::getSourceInfo {id} {
    variable sourceInfo
    if {[dict exists $sourceInfo $id]} {
        return [dict get $sourceInfo $id]
    } else {
        error "RemoteGUI Source manager; no such source id $id"
    }
}
