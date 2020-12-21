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
# @file evbui.tcl# @brief Provide an event builder user interface (e.g.glom parameters()).
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide evbui 1.0
package require Tk
package require snit
package require nsclspinbox
package require NSCLBgerror
package require EVBUtilities
package require textprompter

namespace eval ::EVBC {
    
}


##
#  ::EVBC::tsselector
#     Megawidget to allow users to select glom's timestamp policy.
#
# OPTIONS
#    -policy  - reflects the current policy and sets the current radiobutton
#               if configured.
#    -state   - Widget state, e.g. disabled
#    -command - Command to invoke when a radiobutton is clicked.
#                %P is substituted with the policy selected.
#
snit::widgetadaptor ::EVBC::tsselector {
    option -policy -default earliest -configuremethod _configPolicy
    option -state  -default normal   -configuremethod _configState
    option -command [list]
    
    variable policies [list earliest latest average]
    
    ##
    # constructor
    #   Constructs the megawidget:
    #   -   Hull is a ttk::frame
    #   -   At the left is a label.
    #   -   To the right are a set of radio buttons, one for each policy.
    #
    constructor args {
        installhull using ttk::frame
        
        ttk::label $win.l -text {Ts is: }
        grid $win.l -row 0 -column 0 -sticky w
        set c 1
        foreach policy $policies {
            ttk::radiobutton $win.$policy                                       \
                -text $policy -value $policy -variable [myvar options(-policy)] \
                -command [mymethod _onChanged]                   
            grid $win.$policy -row 0 -column $c -sticky w
            incr c
        }
        $self configurelist $args
    }
    #---------------------------------------------------------------------------
    #  Configuration handling methods.
    #
    
    ##
    #  _configPolicy
    #    Configure the -policy option.  Throw an error if the value is not
    #    in policies.
    #
    # @param optname  - option name (-policy e.g.).
    # @param value    - proposed value.
    #
    method _configPolicy {optname value} {
        if {$value ni $policies} {
            error "-policy ($value) must be in {$policies} "
        } else {
            set options($optname) $value;    # this updates the radiobuttons too.
        }
    }
    ##
    # _configState
    #    propagates the requested state to all of the radioubuttons.
    # @param optname - option name being configured.
    # @param value   - new value
    #
    #
    method _configState {optname value} {
        
        #  Ensure that this is a valid state:
        
        if {$value ni [list normal disabled]} {
            error "-state ($value) must be in {normal disabled}"
        }
        foreach policy $policies {
            $win.$policy configure -state $value
        }
        set options($optname) $value
              
    }
    #---------------------------------------------------------------------------
    #   Action handlers:
    #
    
    ##
    # _onChanged
    #   Called when the radio buttons change state.
    #
    method _onChanged {} {
        set cmd $options(-command)
        if {$cmd ne ""} {
            set cmd [string map [list %P $options(-policy)] $cmd]
            uplevel #0 $cmd
        }
    }
    
}
##
# EVBC::buildparams
#    Megawidget that sets the build params.  These include
#    whether or not glom is building and, if so, the coincidence
#    interval.
#
#  OPTIONS
#     -build  - boolean true if building.
#     -dt     - Time interval if building.
#     -state  - Widget state (note that the dt widget is also disabled if
#               building is off).
#     -maxfragments
#     -command - Script to invoke on UI changes:
#                 - %B - build boolena.
#                 - %T - coincidence interval.
#
snit::widgetadaptor ::EVBC::buildparams {
    option -build -default 1  -configuremethod _config10
    option -dt    -default 1     -type [list snit::integer -min 0] \
            -configuremethod _configdt
    option -maxfragments -default 1000 -type [list snit::integer -min 1] \
        -configuremethod _configmaxfrag
    option -command -default [list]
    option -state -default normal -configuremethod _configState
    
    constructor args {
        installhull using ttk::frame
        
        ttk::checkbutton $win.build -text Build \
            -variable [myvar options(-build)] -onvalue 1 -offvalue 0  \
            -command [mymethod _onCheckChanged]
        nscl::spinbox $win.dt -text {coincidence interval} \
            -command [mymethod _spinChanged ] \
            -validatecommand [mymethod _validateSpin %P 0] -validate all          \
            -from 0 -to 100000 -width 5 -state disabled
        nscl::spinbox $win.maxfrag -text {max frags} \
            -command [mymethod _maxChanged] \
            -validatecommand [mymethod _validateSpin %P 1] -validate all \
            -from 1 -to 100000 -width 6
        $win.dt set 1
	    ttk::label $win.dtlabel -text {Coincidence interval}
        ttk::label $win.maxlabel -text {Max Frags}
        grid $win.build $win.dt $win.dtlabel -sticky w
        grid $win.maxfrag $win.maxlabel -sticky w
	
	$self configurelist $args
    }
    #---------------------------------------------------------------------------
    # Configuration handlers:
    #
    
    ##
    # _config10
    #    Validate the -build option which must be a zero or a one.
    #    We can't use a bool type validator because the values of a checkbox
    #    are restricted to single values and there are several legal true/false
    #    Tcl bool values.
    #
    #  @param optname - name of the option being configured.
    #  @param value   - proposed value.
    #
    method _config10 {optname value} {
        if {$value ni [list 0 1]} {
            error "$optname must be either 1 or 0 was $value"
        }
        ##
        #  If we're not building, dt can't be set -- even if the state is normal
        #
        
        if {$options(-state) eq "normal"} {
            EVB::utility::_setStateFromBool $win.dt $value
        }
        set options($optname) $value
    }
    ##
    # _configdt
    #   Process changes in the -dt configuration.  Type checking has ensured
    #   that the new value is ok but we need to update the widget contents:
    #
    # @param optname - name of the option modified.
    # @param value   - new value.
    #
    method _configdt {optname value} {
        set $options($optname) $value
        $win.dt set $value
    }
    ##
    # _configmaxfrag
    #    Change the max fragments.
    # @param optname
    # @param optval
    #
    method _configmaxfrag {optname optval} {
        set options($optname) $optval
        $win.maxfrag set $optval
    }
    ##
    # _configState
    #    Validate that the state is either normal or disabled and
    #    set the widgets to the proper state:
    #
    # @param optname - name of the option being configured.
    # @param value   - Proposed value.
    #
    method _configState {optname value} {
        if {$value ni {normal disabled} } {
            error "$optname's value must be normal or disabled, was $value"
        }
        set options($optname) $value
        foreach w [list build dt maxfrag] {

            $win.$w configure -state $value
            
            #  This adjusts the state of the det widget sothat it's going to be
            #   off if build is disabled.
            
            if {$value eq "normal"} {
                $self _config10 -build $options(-build);   # Adjust
            }
        }
    }
    #--------------------------------------------------------------------------
    # Action handlers
    
    ##
    # _onChanged
    #   Notifies the user of a change in the user interface values.
    #
    method _onChanged {} {
        set cmd $options(-command)
        if {$cmd ne ""} {
            set cmd [string map [list %B $options(-build) %T $options(-dt) %M $options(-maxfragments)] $cmd]
            uplevel #0 $cmd
        }
    }
    ##
    # _onCheckChanged
    #
    #    Called when the checkbutton changes state
    #    - Get dt in the correct -state
    #    - Call the user's command handler.
    #
    method _onCheckChanged {} {
      $win configure -build $options(-build);    # To change state of dt.
      $self  _onChanged
    }

    ##
    # _spinChanged
    #    Let the user know the spinbox changed value.
    #    Update the options(-dt) from the spinbox current value
    #    and dispatch to _onChanged.
    #    the validation handler has already ensured the value is legal.
    #
    method _spinChanged {} {
        set options(-dt) [$win.dt get]
        $self _onChanged
    }
    ##
    # _maxChanged - max fragments changed.
    #
    method _maxChanged {} {
        set options(-maxfragments) [$win.maxfrag get]
        $self _onChanged
    }
    #---------------------------------------------------------------------------
    #  Validation handlers:
    
    ##
    #  _validateSpin
    #
    #    Ensures the spin box has a valid integers (0 or more).
    #
    # @param proposed - the new proposed value.
    # @param minval   - minimum allowed value (introduced to allow -dt = 0)
    # @return boolean - true if proposed is an integer >=1.
    #
    method _validateSpin {proposed minval} {
        if {![string is integer -strict $proposed]} {
            bell
            return 0
        }
        if {$proposed < $minval} {
            bell
            return 0
        }
        return 1
    
    }
    
}
##
# ::EVBC::glomparams
#     label frame that contains the UI elements for the glom parameters.
#     this is essentially a vertical stack of tssselector and buildparams
#
#     options get delegated and -command gets delegated to a unique handler.
#     We also expose the -title option of the labeled frame and =relief.
#
snit::widgetadaptor ::EVBC::glomparams {
    component tsselector
    component buildparams

    option -state -default normal -configuremethod _configState;      # handle this ourself.

    delegate option -policy to tsselector
    delegate option -tscommand to tsselector as -command

    delegate option -build to buildparams
    delegate option -dt    to buildparams
    delegate option -buildcommand to buildparams as -command
    delegate option -maxfragments to buildparams

    delegate option -title to hull as -text
    delegate option -relief to hull

    constructor args {
	installhull using ttk::labelframe 

	install tsselector using ::EVBC::tsselector $win.tsselector
	install buildparams using ::EVBC::buildparams $win.buildparams


	grid $tsselector -sticky w
	grid $buildparams -sticky ew

	$self configurelist $args
    }
    
    ##
    # _configState
    #    Ensure that -state is one of normal or disabled.
    #     propagate that on to the subwidgets.
    #
    # @param optname - name of the option being configured.
    # @param value   - proposed value.
    #
    method _configState {optname value} {
        if {$value ni [list normal disabled]} {
            error "$optname must have a value in {normal, disabled} was $value"
        }
        foreach w [list $tsselector $buildparams] {

            $w configure -state $value
        }
        set options($optname) $value
    }

}

##
# ::EVBC::intermedRing
#
#  Provide the intermediate ring parameters:
#
# OPTIONS:
#   -tee   - boolean/checkbox enable teeing of output ring.
#   -ring  - Name of the ring to which the tee'd ordered fragments will
#            go.
#   -command - something changed.
#              %W  - widget.
#   -state - widget state
#
#   -title - Title string of ttk::lableframe.
#   -relief - Relief of ttk::labelframe.
snit::widgetadaptor ::EVBC::intermedRing {
    option -tee -default 0 -configuremethod _configTee
    option -ring -default ""
    option -command [list]
    option -state -default normal -configuremethod _configState

    delegate option -title to hull as -text
    delegate option -relief to hull

    constructor args {
	installhull using ttk::labelframe

	ttk::checkbutton $win.tee -variable [myvar options(-tee)] \
	    -onvalue 1 -offvalue 0 -text {Tee output to this ring} \
	    -command [mymethod _onCheckbox]
    textprompt $win.ring -text {Ring Name} \
        -textvariable [myvar options(-ring)] \
	    -state disabled
	#ttk::label    $win.rlabel -text {Ring Name} 
	#ttk::entry     $win.ring -textvariable [myvar options(-ring)] \
	#    -state disabled
	
	# Layout:

	grid $win.tee -sticky w -columnspan 2
	grid $win.ring -sticky w

	bind $win.ring <FocusOut> [mymethod _onRingChange]
	bind $win.ring <Return>   [mymethod _onRingChange]
	
	$self configurelist $args
    }
    #-----------------------------------------------------------------
    # Configuration methods:

    ##
    # _configTee
    #
    #    - proposed value must be boolean. The entry
    #    - The ring text entry is only enabled if the value is true.
    # 
    # @param optname - name of the option that's being configured (-tee).
    # @param value   - proposed value.
    #
    method _configTee {optname value} {
        if {$value ni [list 0 1]} {
            error "$optname value must be in {0,1} was $value"
        }
        set options($optname) $value
    
        # Handle the state of the entry widget:
        
        if {$options(-state) eq "normal"} {

            EVB::utility::_setStateFromBool $win.ring $value
        }
        
    }
    ##
    # _configState
    #    Process changes to -state:
    #    -   must be normal or disabled.
    #    -   If disabled, all input controls are disabled.
    #    -   If normal, the tee checkbox is enabled.
    #    -   If -tee is true, the ring entry is enbled too.
    #
    # @param optname - name of the option being modified.
    # @param value   - proposed new value.
    #
    method _configState {optname value} {
	if {$value ni [list disabled normal]} {
	    error "$optname value must be one of {disabled, normal} was $value"
	}
	set options($optname) $value

	if {$value eq "disabled"} {
	    foreach w  [list tee ring] {
		$win.$w config -state disabled
	    }
	} else {
	    $win.tee config -state normal
	    if {$options(-tee)} {
		$win.ring config -state normal
	    } else {
		$win.ring config -state disabled;   # we don't ask prior state.
	    }
	}
    }
    #-------------------------------------------------------------------
    #  Action handlers:

    ##
    # _onCheckbox
    #    Called when the tee checkbox was changed.  We need to
    #    -  Configure -state so that the enable/disable is normal
    #    -  invoke the _command method.
    #
    method _onCheckbox {} {
	#
	#  This looks a bit funky but it's right --- trust me.
	#  because changing the checkbutton does not invoke the
	#  configuration handler since Tk has no way to know
	#  the variable's semantics

	$self  _configTee -tee $options(-tee)

	$self _command
    }
    ##
    # _onRingChange
    #   Called when the text entry reflects a committed change to the
    #   ring name entry.   Invoke the _command method.  At present we don't
    #   have a configuration handler for the -ring option so we don't need
    #   to invoke it.
    #
    method _onRingChange {} {
	$self _command
    }
    ##
    # _command
    #   dispatch the -command script along with the substitutions it uses.
    #
    method _command {} {
        EVB::utility::_dispatch $options(-command) [list %W $win]
    }
}
##
# ::EVBC::destring
#    Prompt for information about the destination ring.  A text entry
#    and a checkbutton which indicates whether or not data from that
#    ring should be recorded.
#
# OPTIONS
#    -state - normal|disabled - state of widgets.
#    -command - Widget was changed. %W is substituted to be the widget.
#    -ring    - Name of ringbuffer.
#    -record  - 1|0  If the ring should have its data recorded.
#    -title   - The -text of the hull which is a ttk::labelframe
#    -relief  - The -relief of the hull which is a ttk::labelframe.
#
snit::widgetadaptor ::EVBC::destring {
    option -state -default normal -configuremethod _configState
    option -command [list]
    option -ring 
    option -record -default 0 -configuremethod _configRecord

    delegate option -title  to hull as -text
    delegate option -relief to hull
    
    constructor args {
	installhull using ttk::labelframe

	ttk::checkbutton $win.record -text {Use for recording} -onvalue 1 -offvalue 0 \
	    -variable [myvar options(-record)] -command [mymethod _onCommand]
    textprompt $win.ring -text {Output Ring} -textvariable [myvar options(-ring)]
	#ttk::label       $win.ringlabel -text {Output Ring}
	#ttk::entry       $win.ring      -textvariable [myvar options(-ring)]

	bind $win.ring <Return> [mymethod _onCommand]
	bind $win.ring <FocusOut> [mymethod _onCommand]

	# Layout the widgets:

	

	grid $win.ring -sticky w
	grid $win.record              -sticky w

	#  Process any construction configuration options:

	$self configurelist $args


    }
    #------------------------------------------------------------------
    #  Configuration handlers.

    ##
    # _configState 
    #       process -state configuration:
    #       - Ensure the value is one of {normal, disabled}
    #       - Propagate the new state down to the control widgets.
    #
    #  @param optname - Name of option being configured.
    #  @param value   - New proposed value.
    #
    method _configState {optname value} {
	if {$value ni [list normal disabled]} {
	    error "The value of $optname must be in {normal, disabled} was $value"
	}
	foreach w [list record ring] {
	    $win.$w configure -state $value
	}
	set options($optname) $value
    }
    ##
    # _configRecord
    #    Ensure the value for record is in 0,1.
    #
    #   @param optname - name of the option being configured (-record).
    #   @param value   - proposed new value.
    #
    method _configRecord {optname value} {
	if {$value ni [list 1 0]} {
	    error "Value for $optname should be in {1, 0} was $value"
	}
	set options($optname) $value
    }
    #--------------------------------------------------------------
    #   Option handlers.
    #

    ##
    # _onCommand
    #   Dispatch to the command handler since the
    #   widget control contents changed.
    #
    method _onCommand {} {
        EVB::utility::_dispatch $options(-command) [list %W $win]
    }

}
##
#  ::EVBC::eventbuildercp
#    Full event builder control panel.  Each of the megawidgets above
#    is gridded side-by-side with reasonalbe titles and we
#    delegate options to them in a reasonable way.
#    all of these are in a ttk::frame as a hull.
#
#  OPTIONS:
#    -tspolicy  - Timestamp policy.
#    -tscommand - Called if Ts policy changed.#
#    -glomtitle - Title for the glom control panel.
#    -build    - True if event building is requested.
#    -dt       - Coincidence interval in timestamp ticks if buildingh.
#    -glomcmd  - Called if these glom parameters change.
#
#   -teetitle  - Title for the tee out control panel
#   -tee       - True if we want to see the raw evb output in a ring.
#   -teering   - name of the ring if -tee.
#   -teecommand- called if the tee parameters above changed.
#  
#   -outtitle - TItle of the output control panel.
#   -oring    - Output ring.
#   -record   - record from the output ring?
#   -oringcommand - Records -oputputing.
#
#   -state    - state of the entire widget.
#   -relief   - relief of the outer frame.
#
snit::widgetadaptor EVBC::eventbuildercp {
    component glomparams
    component tee
    component outring


    option -state -default normal -configuremethod _configState

    delegate option -relief to hull



    delegate option -glomtitle to glomparams as -title
    delegate option -build     to glomparams
    delegate option -dt        to glomparams
    delegate option -glomcmd   to glomparams as -buildcommand
    delegate option -policy    to glomparams
    delegate option -tscommand to glomparams
    delegate option -tspolicy  to glomparams as -policy
    delegate option -maxfragments to glomparams


    delegate option -teetitle   to tee as -title
    delegate option -tee        to tee
    delegate option -teering    to tee as -ring
    delegate option -teecommand to tee as -command

    delegate option -oring    to outring as -ring
    delegate option -record     to outring
    delegate option -oringcommand to outring as -command

    ##
    # constructor - assemble the megawidget.
    #

    constructor args {
        installhull using ttk::frame
        ttk::frame $win.leftStretch
        ttk::frame $win.rightStretch
      
        install glomparams using ::EVBC::glomparams $win.build \
            -title {Event building parameters (vsn 11)} -relief groove
        install tee using ::EVBC::intermedRing $win.tee \
            -title {Ordered Fragment Ring} -relief groove
        install outring using ::EVBC::destring $win.oring \
            -title {Destination ring} -relief groove
    
        grid $glomparams -column 0 -row 0 -sticky w -pady 3 -pady 3
        grid $win.leftStretch -column 1 -row 0  -sticky nsew
        grid $tee -column 2 -row 0 -sticky ew -pady 3 -pady 3
        grid $win.rightStretch -column 3 -row 0  -sticky nsew
        grid $outring -column 4 -row 0 -sticky e -pady 3 -pady 3
      
        grid columnconfigure $win {1 3} -weight 1
      
        $self configurelist $args
    }
    #-------------------------------------------------------
    #  Configuration processors.

    ##
    # _configState
    #    Configure -state option.  We must ensure the proposed
    #    value is legal and then propagate it to all to widgets.
    #
    # @param optname  - name of option being configured.
    # @param value    - Proposed new value.
    #
    method _configState {optname value} {
        if {$value ni [list normal disabled]} {
            error "$optname value must be one of {norma, disabled} was $value"
        }
        foreach w [list $glomparams $tee $outring] {
            $w configure -state $value
        }
        set options($optname) $value
    }
    
}
