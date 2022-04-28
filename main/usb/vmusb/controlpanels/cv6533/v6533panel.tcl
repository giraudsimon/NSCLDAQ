#!/usr/bin/tclsh
#
#   This is the control panel application for the
#  CAEN V6533 power supply.
# Usage:
#    panel name
# Where
#    name - is the name of the device as known to 
#           Readout.
#



package require Tk

set here [file dirname [info script]]
set libdir [file join $here .. TclLibs]
set auto_path [concat $libdir $auto_path]

package require v6533Channel
package require v6533ChannelParams
package require v6533Driver

package require usbcontrolclient

#
#  Print the program usage:
#

proc usage {} {
    puts stderr "Usage:"
    puts stderr "   panel name"
    puts stderr "Where:"
    puts stderr "   name - the name of a V6533 device in controlconfig.tcl"

}

# 
#  Produce a row label. This serves to 
#  describe the contents of the channel widgets:
#
# Parameters:
#   wid  - widget name of containing frame.
# Returns:
#   wid
#
proc rowLabel wid {
    frame $wid -relief groove -borderwidth 2
    label $wid.name -text name
    label $wid.set  -text setpoint
    label $wid.v    -text voltage
    label $wid.i    -text current

    grid $wid.name
    grid $wid.set
    grid $wid.v
    grid $wid.i

    return $wid
}
#  Loads the setpoints from the devices
#  into the channel widgets.  This allows
#  the panel to be dropped and then picked up
#  again with no loss of information.
# Parameters:
#   device - Proxy for the server device driver.
#   wid    - base widget name. The actual channel i
#            widget name is $wid$i
proc loadSetpoints {device wid} {
    set setpoints [$device getSetpoints]

    for {set i 0} {$i < 6} {incr i} {
	set setpt [lindex $setpoints $i]
	set widget $wid$i
	$widget configure -setpoint $setpt
    }
}
#
#  Update the channel state periodically:
#  - Request the monitored data
#  - Update each widget from that data.
#  - Schedule the next run
#
# Parameters:
#   device   - Device object (v5633 snit type.).
#   widget   - The base widget name for the channels.
#              To get each channel widget the channel
#              number must be appended.
#   resched  - Number of seconds between reschedules.
#
proc updateChannels {device widget resched} {
    global statusBits
    global lastStatus

    # Get most recent monitored data
    #  - Report errors.
    #  - break apart the lists into stuff we need.
    #

    set data [$device monitor]
    if {[lindex $data 0] ne "OK"} {
	puts stderr "monitor failed: $data"
    } else {
	set channelStatuses [lindex $data 2]

	# Set the channel currents and voltages to have only 1 digit
	# past the decimal.

	set channelVoltages [list]
	foreach voltage [lindex $data 3] {
	    lappend channelVoltages [format %.1f $voltage]
	}
	set channelCurrents [list]
	foreach current [lindex $data 4] {
	    lappend channelCurrents [format %.1f $current]
	}


	# Update each channel widget:

	for {set i 0} {$i < 6} {incr i} {
	    set cw $widget$i
	    $cw configure -actualv [lindex $channelVoltages $i]
	    $cw configure -actuali [lindex $channelCurrents $i]
	    
	    #Status as color
	    # - Off green  Button label: On state normal
	    # - On  amber  Button label: Off state normal
	    # - Problem Red Button label On state disabled.


	    set stat [lindex $channelStatuses $i]
	    if {[expr $stat & 1] != 0} {
		$cw configure -bg gold -blabel Off \
		    -buttonstate normal
	    } elseif {[expr $stat & 0x1ff8] != 0} {

		$cw configure -bg red -blabel On \
		    -buttonstate disabled

	    } else {
		$cw configure -bg green -blabel On \
		    -buttonstate normal
	    }
	    # If there is a status widget for this
	    # Channel update it:

	    set topStatus .status$i
	    if {[winfo exists $topStatus] && ($stat ne [lindex $lastStatus $i])} {
		onStatusUpdate $topStatus $stat
		lset lastStatus $i $stat
	    }
    	}
    }

    after [expr {$resched * 1000}] \
	"updateChannels $device $widget $resched"
}
# Given the output of one of the simple get
# commands from v6533 return an appropriate output list
# The list will be either the values from the device
# or, if there was an error, the reason is output
# to stderr and the list is filled with 'undef'
# Parameters:
#   result  - The result of the get operation
# Returns:
#   a 6 element list.
proc getListValue {result} {
    if {[lindex $result 0] ne "OK"} {
	puts stderr "Get request failed: $result"
	for {set i 0} {$i < 6} {incr i} {
	    lappend retval "undef"
	}
	return  $retval
    } else {
	return [lrange  $result 1 end]
    }
}
#
#  Report errors if any from a setting:
#  Parameters:
#    Result of a set operation on the device.
#
proc reportSetError msg {
    if {[lindex $msg 0] eq "ERROR"} {
	puts stderr $msg
    }
}
#
#  Action proc that is called when a channel button
#  is pushed action is as follows:
#  If the label is "On", the channel is turned on.
#  If the label is "Off" the channel is turned off.
#  Actual widget appearance changes are handled when
#  the periodic update notices the change in status.
# Parameters:
#   widget  - The channel widget.
#   chan    - The channel number affected.
# Implicit inputs:
#   device - global variable that contains the
#            object that is a proxy to the server's
#            driver.
#
proc onButton {widget chan} {
    global device

    set state [$widget cget -blabel]
    if {$state == "On"} {
      set result [$device on $chan]
    } else {
	set result [$device off $chan]
    }
}
#
#  Action function called when the setpoint changes on a
#  channel.  The new value will be taken care of by the
#  widget. What is left for us to do is to set the
#  new value in the device.
# Parameters:
#   wid  - Channel control widget.
#   val  - New setpoint value.
#   chan - Channel number
# Implicit inputs:
#   device - proxy object for the server's device driver
#            for us.
proc onNewSetpoint {wid val chan} {
    global device
    $device setpoint $chan $val
}
#
#  Action function called when a properties request
#  is made.  A properties non-modal dialog is raised
#  for that channel
# Parameters:
#    widget   - The channel widget for which the dialog
#               is requested.
#    channel  - The channel for which the widget is
#               requested
#
proc onProperties {widget channel} {
    global Ilimit
    global Ttime
    global RupRate
    global RdnRate
    global PoffMode

    # Properties dialog if not yet up:

    if {![winfo exists .properties] } {
	
	toplevel .properties
	label         .properties.title -text "Properties for channel $channel"

	# Work area is a channel params widget

	channelParams .properties.controls \
	    -ilimit     [format %.1f [lindex $Ilimit $channel]] \
	    -triptime   [format %.1f [lindex $Ttime  $channel]] \
	    -rampup     [format %.1f [lindex $RupRate $channel]] \
	    -rampdown   [format %.1f [lindex $RdnRate $channel]] \
	    -offmode    [lindex $PoffMode $channel]
	
	# Action area has ok apply cancel buttons:

	set action [frame .properties.action]
	button $action.ok -text {Ok} \
	    -command [list onOkProperties .properties $channel]
	button $action.apply -text {Apply} \
	    -command [list onApplyProperites .properties $channel]
	button $action.cancel -text {Cancel} \
	    -command [list destroy .properties]
	grid $action.ok $action.apply $action.cancel


	grid .properties.title    -sticky ew
	grid .properties.controls -sticky ew
	grid $action              -sticky ew
    } else {
	tk_dialog .properties "Props up" \
	    {Please dismiss the properties dialog that is already up first} \
	    info 0 Ok
    }
 
}
#
#   Action proc to handle the Ok hit on the properties dialog
# Parmaeters:
#   widget - top level widget of the dialog.
#   channel - which channel the properties dialog is operating on.
#
proc onOkProperties {widget channel} {
    onApplyProperties $widget $channel;	# does the actual settings.
    destroy $widget;		# Ok dismisses as well as applies.
}
# Action proc to handle the Apply hit on the properties dialog
# Parameters
#   widget    - Top level of the apply dialog.
#   channel   - Which channel is affected:
#
proc onApplyProperties {widget channel} {
    global Ilimit
    global Ttime
    global RupRate
    global RdnRate
    global PoffMode
    global device

    # Get the current values 

    set ilimit  [lindex $Ilimit $channel]
    set ttime   [lindex $Ttime  $channel]
    set ruprate [lindex $RupRate $channel]
    set rdnrate [lindex $RdnRate $channel]
    set poff    [lindex $PoffMode $channel]

    # for each of the requested values,
    # if there is a change request it.

    set work $widget.controls

    set reqIlimit [$work cget -ilimit]
    if {$reqIlimit != $ilimit} {
	reportSetError [$device setIlimit $channel $reqIlimit]
	lset Ilimit $channel $reqIlimit
    }
    
    set reqTime [$work cget -triptime]
    if {$reqTime != $ttime} {
	reportSetError [$device setTripTime $channel $reqTime]
	lset Ttime $channel $reqTime
    }

    set rupReq [$work cget -rampup]
    if {$rupReq != $ruprate} {
	reportSetError [$device setRupRate $channel $rupReq]
	lset RupRate $channel $rupReq
    }

    set rdnReq [$work cget -rampdown]
    if {$rdnReq != $rdnrate} {
	reportSetError [$device setRdnRate $channel $rdnReq]
	lset RdnRate $channel $rdnReq
    }

    set poffReq [$work cget -offmode]
    if {$poffReq != $poff} {
	reportSetError [$device setOffMode $channel $poffReq]
	lset PoffMode $channel $poffReq
    }
}
#
#  Invoked if channel status was requested.
#  Parameters:
#    widget    - Widget whose status is being
#                requested.
#    channel   - Channel number requested.
#  Implicit inputs:
#
# NOTE:
#  We are just going to layout the widget.  The update
#  proc will determine if each channel has a displayed
#  status and update it accordingly.
#  
proc onStatus {widget channel} {
    global lastStatus

    set toplevel .status$channel
    
    # Don't double instantiate.

    if {![winfo exists $toplevel]} {
	toplevel $toplevel
	label $toplevel.title -text "Channel $channel status"
	grid $toplevel.title -columnspan 2
	lset lastStatus $channel ""; # force update.

	# All other widgets are dynamic on update.
    }
}
# Called to update a status widget.
# - The existing status items are destroyed (except of
#   course the title.
# - New status items are computed..one for each
#   important item.
# Parameters
#   top  - Top level widget containing status.
#   stat - Detailed status as per 3.2.2.6 of the
#          v6533 manual.
#
proc onStatusUpdate {top stat} {

    set statusNames [list                              \
		     Power Ramp Ramp "Over Current"    \
		     "Over Voltage"  "Under Voltage"  \
		     "Max Voltage"   "Max Current"     \
		     "Trip" "Over Power"               \
		     "Disabled" "Interlocked"]
    set statusOn [list              \
		      "On green" "up green"            \
		      "down green" "Set red" "Set red" \
		      "Set red" "Set red" "Set red"    \
		      "tripped red" "Set red"          \
		      "true gold" "true gold"]
    set statusOff [list    \
		       "Off green" "" "" "" "" "" "" "" \
		       "" "" "" ""]

    # Get rid of old status

    foreach widget [winfo children $top] {
	if {$widget ne "$top.title"} {
	    destroy $widget
	}
    }

    # Create new status:

    for {set i 0} {$i < [llength $statusNames]} {incr i} {
	set bit [expr {$stat & (1 << $i)}]
	if {$bit == 0} {
	    set value [lindex $statusOff $i]
	} else {
	    set value [lindex $statusOn $i]
	}
	# only display stuff if value not empty
	
	if {$value ne ""} {
	    set text [lindex $value 0]
	    set color [lindex $value 1]
	    set label [lindex $statusNames $i]

	    label $top.${i}l -text $label
	    label $top.$i    -text $text -fg $color
	    grid $top.${i}l $top.$i
	}
	
    }

}

#
#------------------------------------------------

#  If the parameter count is not right, error exit

if {[llength $argv] != 1} {
    usage
    exit -1
}
set name $argv

#-------------------------------------------------

# Gui layout.


label .l -text "V6533 control for $argv"
grid .l -columnspan 3

#  Layout the main part of the screen:
#  there are channel widgets for each of the
#  6 channels of the module.
#

for {set i 0} {$i < 6} {incr i} {
    v6533Channel .c$i -label "Ch $i" -blabel On -bg green \
	-setpoint 0 -actualv 0 -actuali 0 \
	-command [list onButton %W $i]   \
	-setchanged [list onNewSetpoint %W %V $i] \
	-properties [list onProperties %W $i] \
	-statuscmd  [list onStatus  %w $i]

}
#  Each row has three widgets:

set row 1
set col 1
grid [rowLabel .r$row] -row $row -column 0 -sticky n
for {set i 0} {$i < 6} {incr i} {
    grid .c$i -row $row  -column $col
    incr col
    if {($col > 3) && ($i < 5)} {
	set col 1
	incr row
	grid [rowLabel .r$row] -row $row -column 0 -sticky n

    }
}
##
# setConnectionInfo
#
#  Updates the connection information from the connection prompter.
#
# @param widget - Control prompter widget.
#
# @note - the widget is also destroyed.
#
proc setConnectionInfo widget {
    set ::host [$widget cget -host]
    set ::port [$widget cget -port]
    set parent [winfo parent $widget]
    bind $parent <Destroy> ""
    destroy $parent
    incr ::prompted
}
##
# cancelPrompt
#
# handles various forms of cancellation of the prompting widget.
#
# @param widget - promting widget.
#
proc cancelPrompt widget {
    set parent [winfo parent  $widget]
    bind $parent <Destroy> ""
    exit
}
#------------------------------------------------

# Setup communication.  Connect to 
# localhost unless $env(DAQHOST) is defined and
# port 27000 unless $env(CONTROLPORT) is defined.
#

set host localhost
set port 27000

if {[array names env DAQHOST] ne ""} {
    set host $env(DAQHOST)
}
if {[array names env CONTROLPORT] ne ""} {
    set port $env(CONTROLPORT)
}

##
# Prompt for the connection parameters.
#
wm withdraw .
toplevel .prompter
slowControlsPrompter .prompter.p -host $host -port $port \
    -type VMUSBReadout -okcmd [list setConnectionInfo %W] \
    -cancelcmd [list cancelPrompt %W]
set prompted 0
pack .prompter.p
bind .prompter <Destroy> [list cancelPrompt .prompter.p]
vwait prompted

wm deiconify .

#  Make the socket connection with the control server
#  If the connection fails, readout is not running
#  as specified:

set failed [catch {set sock [socket $host $port]}]
if {$failed} {
    error "Failed to connect to control server@$host:$port"
}
fconfigure $sock -buffering line

# Create an object to communicate with the server
# on behalf of this device:

set device [v6533 %AUTO% -socket $sock  -name $name]

#  Get the setpoints from the device and load them into
#  the widgets.

loadSetpoints  $device .c

#
#  Starts the update process.  The update process
#  periodically requests the monitored data from the
#  the device and updates the GUI using that data:
#

updateChannels $device .c 2;	

#
#  Retrieve the channel parameters that can be 
#  adjusted by the pop up...we get them for all
#  channels (lists indexed by channel number
#  so that we don't need to interact with the server
#  to populate the popup.
#  Globals are:
#   Ilimit  - channel current limits.
#   Ttime   - channel tript times.
#   RupRate - Channel ramp up rates.
#   RdnRate - Channel Ramp down rates.
#   PoffMode - Channel power off modes.

set Ilimit   [getListValue [$device getIlimit]]
set Ttime    [getListValue [$device getTripTimes]]
set RupRate  [getListValue [$device getRupRate]]
set RdnRate  [getListValue [$device getRdnRate]]
set PoffMode [getListValue [$device getOffMode]]
set lastStatus [list "" "" "" "" "" ""]; # Ensure this is wrong
