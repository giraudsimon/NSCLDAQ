#
#   This file contains a megawidget that allows
#   The client to modify detailed channel parameters
#   these parameters are the sort of thing that
#   typically get set infrequently and ignored,
#   unlike the voltage set point and current limits
#   which might get adjusted more frequently
#   The parameters managed include
#   Trip time
#   Software voltage limit
#   Ramp up/down rates.
#   power on/off mode
#  
#  The shape of the GUI is:
#
#  +--------------------------+
#  |  Ilimit        Trip Time |
#  |  [     ]      [        ] |
#  | Ramp up       Ramp Down  |
#  | [     ]       [        ] |
#  |      Power off mode      |
#  | ( ) Off    ( ) Ramp Down |
#  +--------------------------+
#
#   The Gui is a pure view object it does not
#   provide any control.  It is expected that will
#   be provided by the client.
#
# Options:
#   -ilimit     get/set the value of the current limit
#   -triptime   get/set the value of the trip time
#   -rampup     get/set the value of the ramp up rate.
#   -rampdown   get/set the value of the ramp down rate.
#   -offmode    get/set the value of the on/off mode
#               This value must either be
#               kill      - V is abrubptly set to zero.
#               ramp     - V ramps at rampdown rate to zero.
#

package provide v6533ChannelParams 1.0

# TODO: units on entries.
#       numeric validations.


package require Tk
package require snit


snit::widget channelParams {
    option -ilimit
    option -triptime
    option -rampup
    option -rampdown
    option -offmode -default ramp

  

    constructor args {
	$self configurelist $args
	# Create the widget components.

	label $win.ilimitl -text "I limit"
	label $win.ttimel  -text "Trip Time"
	
	entry $win.ilimit -textvariable [myvar options(-ilimit)]
	entry $win.ttime  -textvariable [myvar options(-triptime)]

	label $win.rupl   -text "Ramp up"
	label $win.rdnl   -text "Ramp Down"

	entry $win.rup    -textvariable [myvar options(-rampup)]
	entry $win.rdn    -textvariable [myvar options(-rampdown)]

	label $win.pofl   -text "Power off mode"
	radiobutton $win.poff -value "kill" -variable [myvar options(-offmode)] -text off
	radiobutton $win.pramp -value "ramp" -variable [myvar options(-offmode)] -text ramp

	# Layout the components in $win

	grid $win.ilimitl $win.ttimel
	grid $win.ilimit  $win.ttime

	grid $win.rupl    $win.rdnl
	grid $win.rup     $win.rdn

	grid $win.pofl    -columnspan 2
	grid $win.poff   $win.pramp

    }
}
