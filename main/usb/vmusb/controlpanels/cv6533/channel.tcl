#
#  This snit widget represents the controls for
#  a single channel of a CAEN V6533 HV unit.
#  
#  The layout of the control is:
#
#    +-------------+
#    |   <label>   
#    |   Setpoint
#    |   Actual V
#    |   Actual I
#    |   [Button]
#    +-------------+
#
# OPTIONS:
#   -label   - Set the value of the label field.
#   -actualv - Set a new value for the actual voltage field.
#   -actuali - Set a new value for the actual current field.
#   -bg      - Set a new background color.
#   -blabel  - Set the button label.
#   -command - Attach a script to the button.
#   -setpoint - Set the value of the V setpoint
#   -setchanged - Set the value of the setpoint changed command.
#   -properties - Command executed on a right click on the
#                  widgets.
#   -buttoncolor - Color of the button.
#   -buttontstate - normal/disabled e.g.
#   -statuscmd    - Script executed on status request.
#
#
package require Tk
package require snit

package provide v6533Channel 1.0


snit::widget v6533Channel {

    option -label   -configuremethod setlabel
    option -actualv -configuremethod setActualV
    option -actuali -configuremethod setActualI
    option -bg      -configuremethod setBackground
    option -blabel  -configuremethod setButtonLabel
    option -command
    option -setpoint -configuremethod setSetPoint
    option -setchanged
    option -properties
    option -buttoncolor -configuremethod setButtonBg
    option -buttonstate -configuremethod setButtonState
    option -statuscmd
    #
    #  The constructor is going to follow
    #  a sliggtly different pattern than
    # our previous work.  This time we'll set
    # up the widgets and then configure ourselves
    # as that action will populate the contents of
    # the widgets.
    #
    constructor args {
	label $win.label   -relief groove -borderwidth 2
	entry $win.setpoint -validate focusout \
	    -vcmd [mymethod onSetpointChanged] \
	    -justify right
	label $win.actualv
	label $win.actuali
	button $win.button  -command [mymethod onButton]

	grid $win.label    -sticky ew
	grid $win.setpoint -sticky ew
	grid $win.actualv  -sticky ew
	grid $win.actuali  -sticky ew
	grid $win.button   -sticky ew

	$self configurelist $args

	#  Bindings:
	#
	#  b3 will invoke onProperties.
	#  dbl1 will invoke onStatus
	#
	foreach wid [list            \
			 $win.label \
			 $win.setpoint \
			 $win.actualv  \
			 $win.actuali  \
			 $win.button] {

	   bind $wid <Button-3>       \
		[mymethod onProperties]
	   bind $wid <Double-Button-1> \
	       [mymethod onStatus]
	   
	}
	bind $win.setpoint <Return> [mymethod onSetpointChanged]
	
    }
    #---------------------------------------------
    #
    #   Configuration handling.
    #

    #  Called when the -label is configured

    method setlabel {name value} {
	$win.label config -text $value
	set options($name) $value
    }
    # Called when the -actualv option is configurerd.
    
    method setActualV {name value} {
	$win.actualv configure -text $value
	set options($name) $value
    }
    #
    #  Set the value of the actual current
    #
    method setActualI {name value} {
	$win.actuali configure -text $value
	set options($name) $value
    }
    #
    # set the background color for all the widgets.
    # this is usually used to show status information.
    #
    method setBackground {name value} {
	foreach widget [list $win.label    \
			    $win.setpoint  \
			    $win.actualv   \
			    $win.actuali] {
	    $widget configure -background $value
	}
	set options($name) $value
    }
    #
    # Set the label of the button (configure method).
    #
    method setButtonLabel {name value} {
	$win.button configure -text $value
	set options($name) $value
    }
    #
    #  Called when the set point is configured:
    #  - Set the entry value
    #  - Set options(-setpoint)
    #  - Dispatchto onSetpointChanged.
    #
    method setSetPoint {name value} {
	$win.setpoint delete 0 end
	$win.setpoint insert 0 $value
	$self onSetpointChanged; # This will set option.
    }
    #
    # Called for the -buttoncolor option set:
    #
    method setButtonBg {name value} {
	$win.button configure -bg $value
	set options($name) $value
    }
    #
    #   Called form the -buttonstate optionset:
    #
    method setButtonState  {name value} {
	$win.button configure -state $value
	set options($name) $value
    }
    #---------------------------------------------
    #
    #  Event handling:
    #

    #  Handle button clicks.  If there is a 
    #  -command option specified and not null this is
    #  treated as a script and dispatched to.
    #  Substitutions supported:
    #    %W   - Widget identifier ($win)
    #
    # note: The script is evaluated in the top level.
    #
    method onButton {} {
	set command $options(-command)
	if {$command ne ""} {
	    regsub -all {%W} $command $win command
	    uplevel #0 $command
	}
    }
    
    #
    #  handle setpoint changes.  If there is a
    #  -setchanged option that is not null,
    #  it is executed at the top level with the folowing
    #  substitutions:
    #  %W   - The widget.
    #  %V   - The new setpoint value.
    #
    method onSetpointChanged {} {
	set value [$win.setpoint get]
	if {! [string is double $value]} {
	    $win.setpoint delete 0 end
	    $win.setpoint insert end $options(-setpoint)
	    return 0
	}
	set options(-setpoint) $value; # for cget.
	set command $options(-setchanged)
	if {$command ne ""} {
	    regsub -all {%W} $command $win command
	    regsub -all {%V} $command $value command
	    uplevel #0 $command
	}
	return 1
    }
    #
    # Handles right clicks on the widget.
    # this will invoke the -properties option
    # if defined.  
    # Substitutions:
    #   %W - widget name.
    #
    method onProperties {} {
	set command $options(-properties)
	if {$command ne ""} {
	    regsub -all {%W} $command $win command
	    uplevel #0 $command
	}
    }
    # Handles double clicks on the widget.
    # This will invoke the -statuscmd option
    # script if defined.
    # Substitutions are:
    #  %W  - Widget name.
    #
    method onStatus {} {
	set command $options(-statuscmd)
	if {$command ne ""} {
	    regsub -all {%W} $command $win command
	    uplevel #0 $command
	}
    }
}
