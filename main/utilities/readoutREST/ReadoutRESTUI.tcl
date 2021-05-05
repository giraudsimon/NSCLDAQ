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
    delegate option * to hull
    
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
#   -nexttitle - Next used title - passed to -titlecommand.
#   -nextrun   - Next used run - passed to -runcommand.
#   -titlecommand -called when Return is hit in the title field.
#   -runcommand- Called whn Return is hit in the run number spinbox.
#               or the increment/decrement buttons are clicked.
#
snit::widgetadaptor ReadoutParameters {
     option -title
     option -run -default 0
     option -nexttitle
     option -nextrun      -configuremethod _cfgRun
     option -titlecommand -default [list]
     option -runcommand   -default [list]
     option -state -default normal -configuremethod _cfgState
     
     variable lastTitle ""
     
     
     delegate option * to hull
     
     constructor {args} {
        installhull using ttk::frame
        
        ttk::frame $win.title
        
        ttk::label $win.title.ctlabel -text {Current Title: }
        ttk::label $win.title.ctitle -textvariable [myvar options(-title)]
        ttk::label $win.title.tlabel -text {Next Title: }
        ttk::entry $win.title.title -textvariable [myvar options(-nexttitle)] -width 72
        
        ttk::frame $win.run
        
        ttk::label $win.run.crlabel -text {Current Run: }
        ttk::label $win.run.crun    -textvariable [myvar options(-run)]
        ttk::label $win.run.rlabel -text {Next Run: }
        ttk::spinbox $win.run.run -from 0 -to 1000000 -command [mymethod _onSetRun] \
            -textvariable [myvar options(-nextrun)]
        $win.run.run set 0
        
        grid $win.title.ctlabel $win.title.ctitle -padx 3 -sticky w
        grid $win.title.tlabel $win.title.title -padx 3 -sticky w
        grid $win.run.crlabel $win.run.crun $win.run.rlabel $win.run.run   \
            -padx 3 -sticky w
        grid $win.title -sticky w
        grid $win.run -sticky w
        
        #  Bindings:
        
        bind $win.title.title <Return> [mymethod _onSetTitle]
        bind $win.run.run   <Return> [mymethod _onSetRun]
        
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
        $win.run.run set $value
     }
     ##
     # _cfgState
     #   Configures the widget state:
     # @param optname - option name.
     # @param value   - Must be one of 'normal' or 'disabled'
     #
     method _cfgState {optname value} {
        if {$value eq "normal"} {
            $win.title.title configure -state normal
            $win.run.run  configure -state normal
        } elseif {$value eq "disabled"} {
            $win.title.title configure -state disabled
            $win.run.run  configure -state disabled
        } else {
            error "'$value' is a valid state. Must be 'normal' or 'disabled'"
        }
        
        set options($optname $value)
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
        set value [$win.run.run get]
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
        $self _dispatch -titlecommand $options(-nexttitle)
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
    delegate option * to hull
    
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
    # @note  The pseudo state 'inconsistent' can be set to indicate
    #        that, if we are managing several Readouts they are not all in the
    #        same state.
    #
    method _cfgState {optname value} {
        if {$value in [list idle active paused inconsistent]} {
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
            } elseif {$value eq "inconsistent"} {
                $win.beginend configure -state disabled
                $win.init     configure -state disabled
            }
            
        } else {
            error "'$value' is not a valid state name"
        }
    }
    #----------------------------------------------------------------------
    # Event handling
    #
    
    ##
    # _dispatch
    #    Generic dispatch of the -command once the new state has been determined.
    #
    # @param change - the change desired. e.g. begin end init shutdown.
    # @note The script, if any, is in options(-command)
    method _dispatch {change} {
        set script $options(-command)
        if {$script ne ""} {
            lappend script $change
            uplevel #0 $script
        }
    }
    ##
    # _onBeginEnd
    #   Dispatches depending on the state of the Begin/End button.
    #
    method _onBeginEnd {} {
        set text [$win.beginend cget -text]
        if {$text eq "Begin"} {
            $self _dispatch begin
        } elseif {$text eq "End" }  {
            $self _dispatch end
        } else {
            error "Begin/end button text is '$test' and that's no a legal value"
        }
    }
    ##
    # _onInit
    #   Reacts to the init button
    method _onInit {} {
        $self _dispatch init
    }
    ##
    # _onShutdown
    #   Reacts to the shutdown button.
    #
    method _onShutdown {} {
        $self _dispatch shutdown
    }
}

##
# @class ReadoutUI
#    Combines the classes above to create an integrated UI:
#
#  +----------------------------------------------+
#  | Parameters UI                                |
#  +--------------------+-------------------------+
#  | statistics         |  control/state          |
#  +--------------------+-------------------------+
#
# OPTIONS
#   See component classes that's all there are.
#
snit::widgetadaptor ReadoutUI {
    component parameters
    component statistics
    component control
    
    delegate option -statistics to statistics as -data
    
    delegate option -title        to parameters
    delegate option -nexttitle     to parameters
    delegate option -run          to parameters
    delegate option -nextrun       to parameters
    delegate option -titlecommand to parameters 
    delegate option -runcommand   to parameters
    delegate option -state        to parameters
    
    delegate option -currentstate to control as -state
    delegate option -statecommand to control as -command
    
    ##
    #  The constructor assembles the hull and components and
    #  the components do the rest.
    #
    constructor {args} {
        installhull using ttk::frame
        install parameters using ReadoutParameters $win.params \
            -borderwidth 4 -relief groove
        install statistics using ReadoutStatistics $win.stats \
            -borderwidth 4 -relief groove
        install control    using ReadoutState      $win.state \
            -borderwidth 4 -relief groove
        
        grid $parameters -columnspan 2 -sticky nsew
        grid $statistics $control -sticky nsew
        $self configurelist $args
    }
    
    
}
#-------------------------------------------------------------------------------
#
#  Test code.  Requires a running serving Readout.
#
# @param host - host in which readout is running.
# @param user - user the readout is running under.
# @param service - service advertised - defaults to ReadoutREST
proc test {host user {service ReadoutREST}} {
    package require ReadoutRESTClient
    ReadoutRESTClient readout -host $host -user $user
    set ::afterid -1
    ##
    # updateUI
    #   Maintain the readonly state of the user interface.
    # @param interval - ms between updates.
    # @param w        - widget to maintain
    # @param c        - client command.
    #
    proc updateUI {interval w c} {
        set stats [$c getStatistics]
        set currentTitle [$c getTitle]
        set currentRun   [$c getRunNumber]
        set state        [$c getState]
        
        if {$state eq "active"} {
            $w configure -state disabled
        } else {
            $w configure -state normal
        }
        
        
        $w configure -title \
            $currentTitle -run $currentRun -currentstate $state -statistics $stats
        
        set ::afterid [after $interval updateUI $interval $w $c]
    }
    ##
    # _newRunNumber
    #    Called when a new run number must be set.
    #
    # @param c   - Readout client command.
    # @param rno - new run number.
    #
    proc _newRunNumber {c rno} {
    
        $c setRunNumber $rno
    }
    ##
    # _newTitle
    #    Called to set a new title value
    #
    # @param c  - readout client command.
    # @param t  - New title value.
    #
    proc _newTitle {c t} {
        
        $c setTitle $t
    }
    ##
    # _changeState
    #    Set the readout state.
    #
    # @param c - client command.
    # @param s - New state.
    #
    proc _changeState {c s} {
        $c $s
        if {$s eq "shutdown"} {
            after cancel $::afterid
            tk_messageBox -parent . -title "Exiting --" -icon info -type ok \
                -message  {Shutting down Readout -- also shutting down test ui}
            exit
        }
    }
    
    ReadoutUI .ui \
        -runcommand [list _newRunNumber readout] \
        -titlecommand [list _newTitle readout]   \
        -statecommand [list _changeState readout]        \
        -nextrun [readout getRunNumber] -nexttitle [readout getTitle]
    
    pack .ui -fill both -expand 1
    
    set ::afterid [updateUI 1000 .ui readout]
    
}
