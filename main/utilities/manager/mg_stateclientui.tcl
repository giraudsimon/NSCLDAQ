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
# @file   mg_stateclientui.tcl
# @brief  Provide widget for state client.
# @note   The test proc if run gives a live ui.
#

# @author Ron Fox <fox@nscl.msu.edu>
#
package provide stateclientui 1.0
package require Tk
package require snit

##
# @class StateWidget
#    Widget for state management:
#
# OPTIONS
#   -current - current state.
#   -next    - List of allowed next states.
#   -command - Command executed if a transition is desired.
#
# APPEARANCE:
#    +---------------------------------------+
#    |  State <current-state>                |
#    |  [state selector]  [ Transition ]     |
#    +---------------------------------------+
#
#  The idea is that:
#     - Current-state is the value of the -current option.
#     - [state selector] is a ttk::combobox stocked with the states in
#       the -next option.
#     - Clicking the[ Transition] button with an item in [State selector]
#       selected invokes -command.
#
snit::widgetadaptor StateWidget {
    option -current -default ""
    option -next    -default "" -configuremethod _cfgNext 
    option -command -default [list]
    
    constructor {args} {
        installhull using ttk::frame
        
        ttk::label $win.curlabel -text "State: "
        ttk::label $win.current  -textvariable [myvar options(-current)]
        
        ttk::combobox $win.selector -width 10
        ttk::button  $win.select  -text Transition -command [mymethod _transition]
        
        grid $win.curlabel $win.current -sticky w
        grid $win.selector $win.select -sticky w
        
        $self configurelist $args
    
    }
    #-------------------------------------------------------------------------
    # Configuration management
    
    ##
    # _cfgNext
    #   Configure the set of next items in the combo box menu:
    #
    # @param optval - option value (-next)
    # @param val    - values
    #
    method _cfgNext {optval val} {
        $win.selector configure -values $val
        
        set options($optval) $val
    }
    
    #-------------------------------------------------------------------------
    # Event handling
    
    ##
    # _transition
    #   Invoked by the $win.select
    #   If there is a user script and if there's a nonempty value in the
    #   combobox, the user script is invoked with the requested state
    #   lappended to it.
    #
    method _transition {} {
        set script $options(-command)
        set req    [$win.selector get]
        
        if {($script ne "") && ($req ne "")} {
            lappend script $req
            uplevel #0 $script
        }
        $win.selector set ""
    }
    
}


#----------------------------------------------------------------------
#  The code below is intended to show how you can use the widget above
#  in conjunction with the stateclient package to get a live UI that
#  manages and displays state -- more than one can run simultaneously.
#
# @param user - user that runs the manager.
# @param host - host the manager is running in
proc test {user host} {
    lappend auto_path $::env(DAQTCLLIBS)
    package require stateclient
    
    set ::api [StateClient %AUTO% -user $user -host $host]
    
    #
    #  Called when the UI wants a transition
    #
    proc handleTransition {newstate} {
        set newstate [$::api transition $newstate]
        .ui configure -current $newstate -next [$::api nextStates]    
    }
    #
    #  refreshes state/next states every time interval:
    #
    proc updateUI {ms} {
        .ui configure -current [$::api currentState] -next [$::api nextStates]
        after $ms updateUI $ms
    }
    
    # The rest is trivial:
    
    StateWidget .ui -command handleTransition
    updateUI 1000
    pack .ui
}
    
    

    
