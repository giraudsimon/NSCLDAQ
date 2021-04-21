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
# @file  ReadoutRESTUI.tcl
# @brief GUI elements to control/display a Readout program via REST.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide ReadoutRESTUI 1.0
package require Tk
package require snit



##
# @class ReadoutStatistics
#    Provide statistics widget for READOUT
#
# OPTIONS
#   -data   - a statistics dict from getStatistics.
#
snit::widgetadaptor  ReadoutStatistics {
    option -data -configuremethod _cfgData
    
    ##
    # Construct the widgets in a ttk::frame hull and lay them out.
    #
    constructor args {
        installhull using ttk::frame
        
        # Top Row:
        
        ttk::label $win.statistics -text Statistics
        ttk::label $win.triggers   -text Triggers
        ttk::label $win.accepted   -text {Accepted Triggers}
        ttk::label $win.bytes      -text {Bytes of Data}
        
        #   Per-run row:
        
        ttk::label $win.perrun    -text {Per run}
        ttk::label $win.pertrigs
        ttk::label $win.peraccepts
        ttk::label $win.perbytes
        
        #  Cumulative
        
        ttk::label $win.cumulative -text {Cumulative}
        ttk::label $win.cumtrigs
        ttk::label $win.cumaccepts
        ttk::label $win.cumbytes
        
        grid $win.statistics $win.triggers $win.accepted $win.bytes -sticky e -padx 5
        grid $win.perrun $win.pertrigs $win.peraccepts $win.perbytes -sticky e -padx 5
        grid $win.cumulative $win.cumtrigs $win.cumaccepts $win.cumbytes -sticky e -padx 5
        
        $self configurelist $args
    }
    #---------------------------------------------------------------------
    # Configuration management
    
    ##
    # _cfgData
    #    Configure the -data option.
    #
    # @param optname - option name.
    # @param value   - data dict.
    #
    method _cfgData {optname value} {
        
        $win.pertrigs configure -text [dict get $value perRun triggers]
        $win.peraccepts configure -text [dict get $value perRun acceptedTriggers]
        $win.perbytes  configure -text [dict get $value perRun bytes]
        
        $win.cumtrigs configure -text [dict get $value cumulative triggers]
        $win.cumaccepts configure -text [dict get $value cumulative acceptedTriggers]
        $win.cumbytes  configure -text [dict get $value cumulative bytes]
        
        set options($optname) $value
    }
}
##
# @class ReadoutParameters
#    Provides display/setting of the Readout title and run number.
#
# OPTIONS:
#   -title     - Run title.
#   -run       - Run number.
#   -titlecommand -called when Return is hit in the title field.
#   -runcommand- Called whn Return is hit in the run number spinbox.
#               or the increment/decrement buttons are clicked.
#
snit::widgetadaptor ReadoutParameters {
     option -title
     option -run -configuremethod _cfgRun -cgetmethod _cgetRun
     option -titlecommand -default [list]
     option -runcommand   -default [list]
     
     constructor {args} {
        installhull using ttk::frame
        
        ttk::label $win.tlabel -text {Title: }
        ttk::entry $win.title -textvariable [myvar options(-title)] -width 72
        
        ttk::label $win.rlabel -text {Run: }
        ttk::spinbox $win.run -from 0 -to 1000000 -command [mymethod _onSetRun]
        
        grid $win.tlabel $win.title -padx 3 -sticky w
        grid $win.rlabel $win.run   -padx 3 -sticky w
        
        #  Bindings:
        
        bind $win.title <Return> [mymethod _onSetTitle]
        bind $win.run   <Return> [mymethod _onSetRun]
        
        $self configurelist $args
        
     }
     #------------------------------------------------------------------
     # Configuration management.
     
     ##
     # _cfgRun
     #   Called when the run number is being configured.
     #
     # @param name - option name.
     # @param value - option value - must be a postive integer.
     #
     method _cfgRun {name value} {
        if {![string is integer -strict $value]} {
            error "Run number must be an integer but was: '$value'"
        }
        if {$value < 0} {
            error "Run number must be a positive integer but was: '$value'"
        }
        $win.run set $value
     }
     ##
     # _cgetRun
     #   Retrieve the run number.
     #
     # @param optname -  option  name.
     # @return string - value in the run number spinbox.
     #
     method _cgetRun {optname} {
        return $win.run get
     }
     
     #------------------------------------------------------------------
     # Event handling and command dispatching.
     #
     
     ##
     # _dispatch
     #
     #  Dispatch a user script.
     #
     # @param optname - name of option holding the script.
     # @param value   - value to append to the user script.
     #
     method _dispatch {optname value} {
        set script $options($optname)
        if {$script ne ""} {
            lappend script $value
            uplevel #0 $script
        }
     }
     
     ##
     # _onSetRun
     #    Called either if the spinbox is +/- used or a Return is typed in
     #   the spinbox.
     #
     #  - Ensure the value is a valid run number else messageBox
     #  - Dispatch to the -runcommand script.
     #
     method _onSetRun {} {
        set value [$win.run get]
        if {(![string is integer -strict $value]) || ($value < 0)} {
            tk_messageBox -title {Bad run} -icon error -type ok \
                -message "'$value' must be a positive integer to be a valid run number"
        } else {
            $self _dispatch -runcommand $value
        }
     }
     ##
     # _onSetTitle
     #    A new title has been set.
     #    invoke the -titlecommand script with the title as a value.
     #
     method _onSetTitle {} {
        $self _dispatch -titlecommand $options(-title)
     }
}
##
# @class ReadoutState
#   State display/control
#
# OPTIONS:
#   -state   - Set the current state.
#   -command - A button that can cause a change was clicked.
#
snit::widgetadaptor ReadoutState {
    option -state -configuremethod _cfgState
    option -command [list]
    
    constructor {args} {
        installhull using ttk::frame
        
        #We have state information,
        # an init button, a shutdown button and a BEGIN/END button
        #
        ttk::label $win.statelabel -text {State }
        ttk::label $win.state -textvariable [myvar options(-state)]
        ttk::button $win.beginend -text {} -command [mymethod _onBeginEnd]
        ttk::button $win.init     -text Initialize -command [mymethod _onInit]
        ttk::button $win.shutdown -text Shutdown   -command [mymethod _onShutdown]
        
        grid $win.statelabel $win.state $win.shutdown
        grid $win.beginend - $win.init
        
        
        $self configurelist $args
    }
    #----------------------------------------------------------------------
    # Configuration handling
    
    ##
    # _cfgState
    #    Called for a new state
    #    - Validate the state.
    #    - Set the state variables.
    #    - Set the button faces and states.
    #
    # @param optname - option name
    # @param value   - value.
    #
    method _cfgState {optname value} {
        if {$value in [list idle active paused]} {
            set options($optname) $value ;     # cget now works.
            
            if {$value eq "idle"} {
                
                $win.beginend configure -text Begin -state normal
                $win.init configure -state normal
                
            } elseif {$value eq "active"} {
                
                $win.beginend configure -text End -state normal
                $win.init     configure -state disabled
                
            } elseif {$value eq "paused"} {
                $win.beginend configure -text End -state disabled
                $win.init     configure -state disabled
            }
            
        } else {
            error "'$value' is not a valid state name"
        }
    }
    #----------------------------------------------------------------------
    # Event handling
    

        
    
}
