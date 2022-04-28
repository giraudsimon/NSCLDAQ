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

package provide gdgcontrol 1.0
package require snit
package require usbcontrolclient


#
#   This package provides a snit::type that ties together all
#   the stuff needed to make a jtec/wiener gate and delay generator
#   work correctly for the VMUSB readout based daq.  In this
#   daq we must ask the readout program to do all of the I/O for us
#   as the VMUSB is inherently not-sharable.
#   We do this through a connection object.
# 
#  Option      type        Purpose
#  -widget     gdgwidgt  - The widget providing the GUI must implement the
#                          interfaces of gdgwidget (probably is a gdgwidget).
#                          can only be configured at construction time.
#  -connection client    - The connection to the readout program
#  -name       string    - Name of the device in the readout program.
#  -onlost     script    - Script to call when the connection to the
#                          readout program is lost.  If the script is defined,
#                          communication failures don't throw errors. It is up to
#                          the application to recover from them (e.g. create a
#                          new connection and retry.
#
#                          If the script is empty, communication failures throw
#                          errors.
#
# Methods:
#   UpdateValues         - Does a complete update of the values from the hardware.
#   getAll                  - Retrieves a list of 2 elements.  The first element
#                          is the list of widths, the second, the list of delays.
#   setAll                  - Given a list of two elements, sets the widths from
#                          the first list, and the delays from the second.
#

snit::type gdgcontrol {
    option   -widget     "-"
    option   -connection "-"
    option   -name       ""
    option   -onlost     ""

    variable maxChannels 8
    variable minValue    0
    variable maxValue    0x7fffffff; #signed maxint.
    variable nsPerTick   8;          # what each register value is worth.

    constructor args {
	$self configurelist $args

	# Attach the widget to our callbacks.

	set w $options(-widget)

	$w configure -upwidth    [mymethod onWidthPlus]     \
	             -downwidth  [mymethod onWidthDown]     \
	             -updelay    [mymethod onDelayPlus]     \
	             -downdelay  [mymethod onDelayDown]    \
	             -setdelay   [mymethod onSetDelay]     \
	             -setwidth   [mymethod onSetWidth]
	
                    
    }
    # Public methods.

    # Update the widget to match the hardware.
    #

    method UpdateValues {} {
	
	for {set i 0} {$i < $maxChannels} {incr i} {
	    $self refreshWidth $i
	    $self refreshDelay $i
	}
    }
    # Set get all values.
    #

    method getAll {} {
	for {set i 0} {$i < $maxChannels} {incr i} {
	    lappend widths [$self Get width$i]
	    lappend delays [$self Get delay$i]
	}
	return [list $widths $delays]
    }
    # set all values.
    #
    method setAll {values} {
	set widths [lindex $values 0]
	set delays [lindex $values 1]

	set channel 0
	foreach value $widths {
	    $self Set width$channel $value
	    $self refreshWidth $channel
	    incr channel
	}
	set channel 0
	foreach value $delays {
	    $self Set delay$channel $value
	    $self refreshDelay $channel
	    incr channel
	}
    }


    #-----------------------------------------------------------------
    #
    # Callback methods

    method onWidthPlus {channel oldValue} {
	incr oldValue $nsPerTick
	if {$oldValue > $maxValue} {
	    set oldValue $maxValue
	}
	$self Set width$channel $oldValue
	$self refreshWidth $channel
	
    }
    method onWidthDown {channel oldValue} {
	incr oldValue -$nsPerTick
	if {$oldValue < $minValue} {
	    set oldValue $minValue
	}
	$self Set width$channel $oldValue
	$self refreshWidth $channel
    }
    method onDelayPlus {channel oldValue} {
	incr oldValue $nsPerTick
	if {$oldValue > $maxValue} {
	    set oldValue $maxValue
	}
	$self Set delay$channel $oldValue
	$self refreshDelay $channel
    }
    method onDelayDown {channel oldValue} {
	incr oldValue -$nsPerTick
	if {$oldValue < $minValue} {
	    set oldValue $minValue
	}
	$self Set delay$channel $oldValue
	$self refreshDelay $channel
    }
    method onSetDelay {channel value} {
	$self Set delay$channel $value
	$self refreshDelay $channel

    }
    method onSetWidth {channel value} {
	$self Set width$channel $value
	$self refreshWidth $channel
    }
    method refreshWidth channel {
	set value [$self Get width$channel]
	$options(-widget) setWidth $channel $value
    }
    method refreshDelay channel {
	set value [$self Get delay$channel]

	$options(-widget) setDelay $channel $value
    }

  
    # perform  transactions with the device.
    #

    method Get channel {
	set name $options(-name)
	set conn $options(-connection)
	set value  [$self transaction [list $conn Get $name $channel]]
	set value  [expr $value*$nsPerTick]
	return $value
    }
    method Set {channel value}  {
	set name $options(-name)
	set conn $options(-connection)
	set value [expr $value/$nsPerTick]
	
	return [$self transaction [list $conn Set $name $channel $value]]

    }
    method Update {} {
	set name $options(-name)
	set conn $options(-connection)

	return [$self transaction [list $conn Update $name]]
    }


    method transaction script {
	# loop until transaction works or error signalled.

	while {[catch {eval $script} msg] && ($msg ne "")} {
	    if {$options(-onlost) ne ""} {# comm fail handler..
		eval $options(-onlost) $options(-connection)
		set script [lreplace $script 0 0 $options(-connection)]
	    } else {			# no handler.
		error "Communication failure $msg"
	    }
	}
	if {[lindex $msg 0] eq "ERROR"} {
	    error $msg;			# Transaction error.
	}
	return $msg
    }
}
