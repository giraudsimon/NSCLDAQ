#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#

package provide gdgwidget 1.0

package require Tk
package require snit
package require BWidget

#  This package supplies a widget to control jtec/Wiener gate and delay 
#  generator modules.  The widget is independent of the
#  actual control mechanism.  It is supposed to interact with some
#  connection dependent contoller to operate.
#  Layout of the widget is as follows:
#
#   +------------------------------------------------+
#   | Title                                          |
#   |         Channel [   ] +/-                      |
#   |  Width: <value>          Delay <value>         |
#   |         ^   V             ^   V                |
#   |               [ Set All ]                      |
#   +------------------------------------------------+
#
#    ^ - an up arrow button   V  - a down arrow button.
#
# Options:
#    Option     Type   Default     Purpose 
#    -title     string  -          Provides a title string for the widget.
#    -upwidth   script  -          Script to process width + button.
#    -downwidth script  -          Script to process width - button.
#    -updelay   script  -          Script to process delay + button.
#    -downdelay script  -          Script to process delay - button.
#    -setdelay  script  -          Script to set a delay value (channel value)
#    -setwidth  script  -          Script to set a width value (channel value)
#
#  All scripts have the current channel number and the parameter value
#  appended to them... so scripts in general should be proc calls.
#
# Methods:
#   getWidth  n         - Retrieves the width value for channel n
#   setWidth  n v       - Sets the width value for channel n to v.
#   getDelay  n         - Retrieves the delay value for channel n
#   setDelay  n v       - Sets the delay value for channel n to v.
#

snit::widget gdgwidget {
    option -title      ""
    option -upwidth    ""
    option -downwidth  ""
    option -updelay    ""
    option -downdelay  ""
    option -setdelay   ""
    option -setwidth   ""

    variable lastchan       7
    variable currentChannel 0
    variable widths
    variable delays

    constructor args {
	$self configurelist $args

	# Create the widths/delays arrays:

	for {set channel 0} {$channel <= $lastchan} {incr channel} {
	    set widths($channel) 0
	    set delays($channel) 0
	}
	#  Create the widgets.. the whole thing can easily be gridded.
	#
	label    $win.title   -textvariable $options(-title)
	label    $win.chanlbl -text Channel
	spinbox  $win.chan    -from 0 -to $lastchan -incr 1 -width 1 \
	                      -command [mymethod onChannelChange]
	label    $win.widlbl  -text Width
	label    $win.width   -text 0  -width 5
	label    $win.dellbl  -text Delay
	label    $win.delay   -text 0  -width 5

	frame $win.widcontrol
        ArrowButton $win.widcontrol.widplus -dir top     -helptext {Increase width} \
	                                    -command [mymethod onWidthUp]
	ArrowButton $win.widcontrol.widdown -dir bottom  -helptext {Decrease width} \
	                                    -command [mymethod onWidthDown]
	pack $win.widcontrol.widplus $win.widcontrol.widdown -side left

	label $win.delcontrol
	ArrowButton $win.delcontrol.delplus -dir top     -helptext {Increase delay} \
	                                    -command [mymethod onDelayUp]
	ArrowButton $win.delcontrol.deldown -dir bottom  -helptext {Decrease delay} \
	                                    -command [mymethod onDelayDown]
	pack $win.delcontrol.delplus $win.delcontrol.deldown -side left

	button $win.setall -text {Set All} -comman [mymethod onSetAll]

	# Layout the widgets on the grid.

	grid $win.title       -            -        -        -      -      
	grid     x        $win.chanlbl $win.chan
	grid $win.widlbl  $win.width      $win.dellbl $win.delay
	grid     x        $win.widcontrol     x       $win.delcontrol
	grid     x        $win.setall         -           x

    }


    #   Public methods




    #  Return the width of channel n.

    method getWidth n {
	return $widths($n)
    }
    # Set the width of channel n.  If n is displayed it will be updated
    # in the widget.

    method setWidth {n v} {
	set widths($n) $v
	if {$n == $currentChannel} {
	    $win.width configure -text "$v ns"
	}
    }
    # Get the delay of channel n.

    method getDelay n {
	return $delays($n)
    }
    # Set the delay of channel n if channel n is displayed, it will be updated.

    method setDelay {n v} {
	set delays($n) $v
	if {$n == $currentChannel} {
	    $win.delay configure -text "$v ns"
	}
    }

    #--------------------------------------------------------------------------
    #   'private' methods
    
    method onWidthUp {} {
	$self dispatch $options(-upwidth) $widths($currentChannel)
    }

    method onWidthDown {} {
	$self dispatch $options(-downwidth) $widths($currentChannel)
    }

    method onDelayUp {} {
	$self dispatch $options(-updelay) $delays($currentChannel)
    }

    method onDelayDown {} {
	$self dispatch $options(-downdelay) $delays($currentChannel)
    }

    #  Process channel changes.  Set the current channel and
    #  adjust the label appropriately.
    #
    method onChannelChange {} {
	set currentChannel [$win.chan get]
	$self setWidth $currentChannel $widths($currentChannel)
	$self setDelay $currentChannel $delays($currentChannel)
    }
    #
    #   Process the set all button... all channel widths/delays
    #   are set to match the current one
    #
    method onSetAll {} {
	set width $widths($currentChannel)
	set delay $delays($currentChannel)

	for {set i 0} {$i <= $lastchan} {incr i} {
	    $self dispatchSet -setdelay $i $delay
	    $self dispatchSet -setwidth $i $width
	}
    }

    #  Dispatches to a script if one exists.
    #
    method dispatch {script currentValue} {
	if {$script ne ""} {
	    eval $script $currentChannel $currentValue
	}
    }
    # Dispatches to a -set* script...
    
    method dispatchSet {option channel value} {
	set script $options($option)
	if {$script ne ""} {
	    eval $script $channel $value
	}
    }
}