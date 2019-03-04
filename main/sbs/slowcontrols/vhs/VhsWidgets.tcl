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

#  Some widgets for the vhs modules

package provide VhsWidgets 1.0
package require Tk
package require snit
package require iSegVhs
package require PollManager
package require meter

namespace eval VhsWidgets {
}


#======================= environment ========================================
#
#  Environment monitor widget.  This is a frame that shows all
#  the environmental voltages on the module.
#
#  OPTIONS:
#     -device  object    - The device to monitor; a vhs object,
#                          required but is dynamic.
#  METHODS:
#     update             - updates the display
#
# Layout:
#    +------------------------------------------+
#    |  5V supply     +12V supply   -12V supply |
#    |  [value] V     [value] V     [value] V   |
#    |                                          |
#    |  Temperature:  [value]   ()C ()F         |
#    +------------------------------------------+


snit::widget  VhsWidgets::environment {
    option -device  
    delegate option * to hull;               # Allow frame options to work.
    
    variable temperatureUnits C;             # Default is hardware units.
    #----------------------------------------------------------------------------------
    #
    #  Construct the widget:
    #
    constructor args {
        $self configurelist $args
        if {$options(-device) eq ""} {
            error "Constructing a VhsWidget::environment needs a -device option"
        }
        
        #  Create and layout the widgets:
        #
        label $win.p5label   -text {5V Supply (V)}
        label $win.p12label  -text {+12V Supply (V)}
        label $win.n12label  -text {-12V Supply (V)}
        grid $win.p5label $win.p12label $win.n12label
        
        label $win.p5value   -text {}
        label $win.p12value  -text {}
        label $win.n12value  -text {}
        grid $win.p5value $win.p12value $win.n12value
        
        label $win.templabel  -text {Temperature:}
        label $win.tempvalue  -text {}
        radiobutton $win.c    -variable ${selfns}::temperatureUnits  -value C -text C
        radiobutton $win.f    -variable ${selfns}::temperatureUnits  -value F -text F
        grid $win.templabel $win.tempvalue $win.c $win.f
        
        
        $self update
    }
    #-------------------------------------------------------------------------------
    #  Update the widget (could be placed in a pollmaster
    #
    method update {} {
        set dev $options(-device)
        
        # Voltages.
        
        $win.p5value configure -text                                 \
            [format "%0.2f V" [$dev get5VLevel]]
        $win.p12value configure -text                               \
            [format "%0.2f V" [$dev getP12VLevel]]
        $win.n12value configure -text                              \
            [format "%0.2f V" [$dev getN12VLevel]]
        
        # Temp in appropriate units:
        
        set temp  [$dev getAmbientTemperature]
        if {$temperatureUnits eq "F"} {
            set temp [expr 32 + 9.0*$temp/5.0]
        }
        $win.tempvalue configure -text                           \
            [format %0.2f $temp]
    }
}
#=================== channel ==========================================================
#
#  Widget to control/monitor a single channel of the supply.
#
# OPTIONS:
#    -device       - vhs device object required but dynamic.
#    -channel      - Channel to show... This is dyamic and defaults to 0.
#    -statuscommand- Associated a command with the status button.
# METHODS:
#    update        - Updates the display of the device.  This
#                    could be placed in a pollmanager thingy.
#
# LAYOUT:
#    +---------------------------------------+
#    |      Channel [number]                 |
#    |     nn V                nn mA         |
#    |  +-------+            +-------+       |
#    |  | volt  |            |current|       |
#    |  | meter |            | meter |       |
#    |  +-------+            +-------+       |
#    |set  [volts]^V           [ma]^V        |
#    |incr [incr]]^V           [incr]^V      |
#    |                                       |
#    |   [on/off]   [kill]   [Reset] [status]|
#    +---------------------------------------+
#
# Some explanation may be required:
#      [incr] sets how much the volts/ma/ramp spinboxes increment/decrement by
#      [on/off] does ramped on /off of the channel
#      [kill]   does an emergency off of the channel
#      [Reset]  Attempts to reset fault conditions on the channel
#      [status] Displays green ok if there are no faults, and red fault if
#               faults are reported.
#
snit::widget VhsWidgets::channel {
    option -device
    option -channel 0
    option -statuscommand
    delegate option * to hull
    
    constructor args {
        $self configurelist $args
        
        if {$options(-device) eq ""} {
            error "VhsWidgets::channel needs a -device option"
        }
        
        #  Set up the GUI.
        
        # First row of the thing.
        
        label $win.chanlabel   -text {Channel: }
        label $win.chanvalue   -textvariable ${selfns}::options(-channel)
        
        # second row.
        
        label $win.chanvolts   -text {}
        label $win.chancurrent -text {}
        
        # third row:
        
        set maxVolts [$options(-device) getVoltageLimit]
        set maxAmps  [$options(-device) getCurrentLimit]
        set maxMa    [expr $maxAmps * 1000.0]
        # Meter row: 
        
        controlwidget::meter $win.voltmeter  -from 0.0 -to $maxVolts -majorticks 500
        controlwidget::meter $win.ampmeter   -from 0.0 -to [expr $maxMa] -majorticks 1.0
        
        # Settings row for volts and current:
        
        label $win.setlabel    -text {Settings}
        spinbox $win.setvolts  -from 0 -to $maxVolts -increment 1.0   -width 6 -command [mymethod reassertSettings]
        spinbox $win.setcurrent -from 0 -to $maxMa    -increment 0.1  -width 4 -command [mymethod reassertSettings]
        
        # Settings row for spinbox increments.
        
        label $win.incrlabel   -text {Increment}
        spinbox $win.incrvolts -values {1.0 10.0 100.0 1000.0} -width 6 -command [list $win.setvolts configure -increment %s]
        spinbox $win.incramps  -values {.01 .1 1.0} -width 4            -command [list $win.setcurrent configure -increment %s]
        

        # Finally,  the buttons and status at the bottom:
        
        button $win.onoff  -text On   -command [mymethod onOnOffButton]
        button $win.kill   -text Kill -command [mymethod onKillButton]
        button $win.reset  -text Reset -command [mymethod onResetButton]
        button $win.status -text Status -background green -command [mymethod onStatusButton]
        
        # Now do the layout.
        
        grid   x     $win.chanlabel $win.chanvalue   x
        grid   x     $win.chanvolts   x     $win.chancurrent
        grid   x     $win.voltmeter     x            $win.ampmeter   
        grid   $win.setlabel $win.setvolts   x       $win.setcurrent
        grid   $win.incrlabel $win.incrvolts x      $win.incramps
        grid   $win.onoff $win.kill $win.reset $win.status
        
        
        # We want to set some events so that user typing in the various spinbox entries
        # will not be ignored.
        
        
        
        # Update the voltage/current targets if typed...but require a return,
        # focus out will refresh from the hardware.
        #
        
        bind $win.setvolts   <Return>     [mymethod reassertSettings]
        bind $win.setvolts   <FocusOut>   [mymethod getSettings]
        bind $win.setcurrent <Return>     [mymethod reassertSettings]
        bind $win.setcurrent <FocusOut>   [mymethod getSettings]
        
        # And update the increments too:
        
        bind $win.incrvolts <Return>     [mymethod setIncrement $win.incrvolts $win.setvolts]
        bind $win.incramps  <Return>     [mymethod setIncrement $win.incramps  $win.setcurrent]
        bind $win.incrvolts <FocusOut>   [mymethod setIncrement $win.incrvolts $win.setvolts]
        bind $win.incramps <FocusOut>   [mymethod setIncrement $win.incramps  $win.setcurrent]
        
        # Get the stuff that doesn't get updated:

	$self getSettings

        # update the gui the first time.
        
        $self update
    }
    #-------------------------------------------------------------------
    #
    # Update the display.  This could be put in a pollmgr
    # so that it gets done periodically.
    #
    method update {} {
        
	set device $options(-device)
	set chan   $options(-channel)
	set volts  [$device getMeasuredVoltage $chan]
	set amps   [$device getMeasuredCurrent $chan]
	set ma     [expr $amps * 1000.0];   # Milliamps.
	
	# The textual value labels..with units.

	$win.chanvolts   configure -text [format "%.2f %s" $volts V]
	$win.chancurrent configure -text [format "%.2f %s" $ma mA]
	
	$win.voltmeter set $volts
        $win.ampmeter  set $ma

	# Figure out the state of the on/off button.

	set status [$device getChannelStatus $chan]
	if {[lsearch $status "On"] == -1} {
	    $win.onoff configure -text {Turn On}
	} else {
	    $win.onoff configure -text {Turn Off}
	}

	# Update the status/fail button.
	# Fail is considered any event other than
        #    On, VoltageControlActive and Ramping.
        #
	# 
	set eventList [$device getChannelEvents $chan]
        set eventList [removeItems $eventList [list On VoltageControlActive Ramping]]
	if {[llength $eventList] != 0} {
	    $win.status configure -text Fail   -background red   -activebackground red
	} else {
	    $win.status configure -text Status -background green -activebackground green
	}
	
    }
    #-----------------------------------------------------------------------------
    #
    #  Processes clicks of the on/off button.  If the state of the supply is
    #  off, we try to turn the supply on.  If the state of the channel is on
    #  we try to turn it off.  Note that since the actual ability to turn the
    #  supply on/off will depend on external circumstances the update method
    #  must be called in a timely way as it is what will update the on/off button face.
    #
    method onOnOffButton {} {
        
        set device $options(-device)
	set chan   $options(-channel)

	# Figure out the state of the on/off button and issue the appropriate
        # device method call depending on the state.

	set status [$device getChannelStatus $chan]
        $self reassertSettings;            # Refresh the targets  from spinboxes.
        
	if {[lsearch $status "On"] == -1} {
            # On ... turn the channel off.

            $device channelOn $chan
	} else {
            # Off... turn the channel on

	    $device channelOff $chan
	}
    }
    #------------------------------------------------------------------------------
    #
    #  Does a kill on the channel's supply.  If the channel is on,
    #  the voltage will go to zero without a ramp.  How fast it goes to zero will
    #  most likely depend on the load to ground.
    #
    method onKillButton {} {
        
        set device $options(-device)
	set chan   $options(-channel)
        $device emergencyOff $chan
    }
    #------------------------------------------------------------------------------------
    #
    #   Processes the reset button.  This attempts to reset all events.
    #
    method onResetButton {} {
     
        set device $options(-device)
	set chan   $options(-channel)
        $device resetAllChannelEvents $chan
    }
    #------------------------------------------------------------------------------------
    #
    #   Processes the status button.. in this case the status button will
    #   dispatch to user code which, presumably would get the status of the
    #   channel and display it to the user.
    #
    method onStatusButton {} {
        set script $options(-statuscommand)
        if {$script ne ""} {
            eval $script
        }
    }
    #-------------------------------------------------------------------------------------
    #
    #   Gets the current channel settings and loads them into the GUI.
    #   By current channel settings we mean the set voltage, the set current.
    #
    method getSettings {} {
        set device $options(-device)
        set chan   $options(-channel)
        
        set v [$device getChannelVoltageSet $chan]
        set a [$device getChannelCurrentTrip $chan]
        set ma [expr $a * 1000.0]
        
        # Set the GUI elements:
        
        $win.setvolts set $v
        $win.setcurrent set $ma
 
    }
    #------------------------------------------------------------------------------
    #
    #   Reassert the voltage target, the current target from
    #   the associated GUI elements.
    #
    method reassertSettings {} {
        set device $options(-device)
        set chan   $options(-channel)
        
        set v    [$win.setvolts get]
        set a    [$win.setcurrent get]


        $device setChannelVoltageSet $chan $v
        $device setChannelCurrentTrip $chan $a
        
    }
    #==============================================================================
    #
    #  Remove items from a list.
    #
    # Parameters:
    #    list     - The list to process.
    #    items    - A list of items to remove if present in list.
    # Returns:
    #    list edited as directed by items.
    #
    proc removeItems {list items} {
        foreach remove $items {
            set rindex [lsearch $list $remove]
            if {$rindex != -1} {
                set list [lreplace $list $rindex $rindex]
            }
        }
        return $list
    }
    #-----------------------------------------------------------------------------
    #
    #  Set a new increment for a spinbox widget from the value of another widget
    #  the other widget must support get.
    #
    #  Parameters:
    #    incrwidget   - The widget that supplies the increment.
    #    target       - The spinbox whose increment will be set.
    #
    method setIncrement {incrwidget target} {
        $target configure -increment [$incrwidget get]
    }
}

#============================ channelStatus ===================================
#
#   This widget displays the status and event status from a channel of a vhs
#   module.  A reset button at the bottom allows the user to attempt to reset
#   the event status bits... which will turn off the fail light on any
#   corresponding channel widget.
#
# OPTIONS:
#    -device          - Device to monitor required at construction time, but dynamic
#    -channel         - channel to monitor within the device (defaults to 0), dynamic
# METHODS:
#    update           - Updates the status display.. this can be put in a
#                       Poll manager or it can be called in some timely manner after you
#                       have reason to believe the status has changed.
# Layout:
#
#   +--------------------------------------------------+
#   | +----------------------+  +------------------+   |
#   | |  listbox of status   |  | listbox of events|   |
#   | +----------------------+  +------------------+   |
#   |                                [Reset]           |
#   +--------------------------------------------------+
#
#  Listboxes are 12 elements tall (maximum number of status items).
#
snit::widget  VhsWidgets::channelStatus {
    option    -device  ""
    option    -channel 0
    
    constructor args {
        $self configurelist $args
        
        if {$options(-device) eq ""} {
            error "VhsWidgets::channelStatus widget must have a device to monitor"
        }
        
        label     $win.title      -text {Status for channel: }
        label     $win.channel    -textvariable ${selfns}::options(-channel)
        label     $win.statlabel  -text {Status}
        label     $win.evtlabel   -text {Latched events}
        listbox   $win.status     -height 12
        listbox   $win.events     -height 12
        button    $win.reset      -text {Reset Events} -command [mymethod onReset]
        
        grid $win.title         $win.channel
        grid $win.statlabel     x   $win.evtlabel
        grid $win.status        x   $win.events
        grid      x             x   $win.reset

        $self update
    }
    #-----------------------------------------------------------
    #  Update the status and events window.
    #
    method update {} {
        set dev  $options(-device)
        set chan $options(-channel)
        
        $win.status delete 0 end
        $win.events delete 0 end
        
        foreach item [$dev getChannelStatus $chan] {
            $win.status insert end $item
        }
        
        foreach item [$dev getChannelEvents $chan] {
            $win.events insert end $item
        }
    }
    #----------------------------------------------------------
    #  Try to reset all the pending events.
    #
    #
    method onReset {} {
        set dev  $options(-device)
        set chan $options(-channel)
        
        $dev resetAllChannelEvents $chan
    }
}
