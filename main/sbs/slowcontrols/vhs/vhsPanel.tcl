#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

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


# This application controls an iSeg VHS module. The Application
# presents a tabbed notebook as the main GUI.
# The front page of the tabbed notebook is a summary of all the channels.
# Requested voltages, actual voltages, are shown as well as buttons for
# turning on/off selected or all channels.
# Channel actual values are red if there is a 'bad status' event in them.
# The next set of pages are detailed channel pages where the Status button
# will bring up a pop up window showing the detailed status of that channel
# The last page is a panel showing the environmental characteristics of the
# module (temperature and input voltages e.g.
#
# Usage:
#    vhsPanel  baseAddress
#
# Where
#    baseAddress   - Is the base address of the module and defaults to
#                    0x4000, the factory default address.
#
#

#  Append the script directory to the library search path:

set here [file dirname [info script]]
set libdir [file normalize [file join $here .. TclLibs]]
lappend auto_path $libdir

puts "$here"
puts $auto_path

package require Tk
package require iSegVhs
package require VhsWidgets
package require PollManager

package require snit

#===========================Startup Initialization===========================
#
#  Let's create the device, and a poll manager to handle
#  the page updates:

set baseAddress       0x4000;                # Default base address.
set defaultUpdateRate 500;                   # Default ms between updates.

if {[info vars args] ne ""} {
    if {[llength $args] != 0} {
        set baseAddress [lindex $args 0]
    }
}

set module [vhs create %AUTO% -base $baseAddress]
set poll   [pollManager create %AUTO% -period $defaultUpdateRate]
#
#==========================Internal procs types and megawidgets===============
#
#
#************************Megawidget for summarizing a channel*****************
#
# OPTIONS:
#    -device       - Device to summarize
#    -channel      - Channel inside the device to summarize.
#    -oncolor      - Color of the channel if it's on (defaults to green).
#    -offcolor     - Color of channel if it's off (defaults to yellow).
#    -failcolor    - Color of channel if it's posting an event (defaults to red).
#
#
# METHODS:
#    update        - Update the channel status.
#    get           - Get value of voltage entry field.
#
# LAYOUT:
#    +----------------+
#    | Channel n      |
#    |  Actual V      |
#    |  Requested V   |
#    |  [new Req V]   |
#    |  [On/Off]      |
#    +----------------+
snit::widget channelSummary {
    option -device    ""
    option -channel   0
    option -oncolor   green
    option -offcolor  yellow
    option -failcolor red
    option -command   ""
    delegate option * to hull
    
    constructor args {
        $self configurelist $args
        
        # Require a device
        
        if {$options(-device) eq ""} {
            error {channelSummary must have a -device specification}
        }
        
        # Layout the widget:
        
        label $win.title   -text Channel
        label $win.chan    -textvariable ${selfns}::options(-channel)
        
        label $win.actual
        label $win.requested
        entry $win.request   -width 6
        button $win.onoff    -text {Turn On} -command [mymethod onOnOffButton]
        
        grid $win.title $win.chan
        grid $win.actual    -  -sticky w
        grid $win.requested -  -sticky w
        grid $win.request   -  -sticky w
        grid $win.onoff     -
        
        # Set the initial values of all the stuff:
        
        $self update
        $self revertRequest

        bind $win <1> [mymethod onClick]
        bind $win.title <1> [mymethod onClick]
        bind $win.chan  <1> [mymethod onClick]
        bind $win.actual <1> [mymethod onClick]
        bind $win.requested <1> [mymethod onClick]
        
        # If <Return> is hit in the entry, it sets a new requested v.
        # If <FocusOut> the entry is restored from the current requested V.
        
        bind $win.request <Return>   [mymethod updateRequest]
        bind $win.request <FocusOut> [mymethod revertRequest]
        
    }
    #---------------------------------------------------------------
    #  Update the channel's requested voltage from the entry:
    #
    method updateRequest {} {
        $options(-device) setChannelVoltageSet $options(-channel) \
                [$win.request get]
    }
    #----------------------------------------------------------------
    #
    #  Revert the request widget to the value of the
    #  channel's requested voltage:
    #
    method revertRequest {} {
        $win.request delete 0 end
        $win.request insert end \
            [format %0.2f [$options(-device) getChannelVoltageSet $options(-channel)]]    }
    #----------------------------------------------------------------
    #  Update the values of the read-only fields:
    #
    method update {} {
        set device $options(-device)
        
        $win.actual configure -text \
                [format %0.2fV [$device getMeasuredVoltage $options(-channel)]]
        $win.requested configure -text \
                [format %0.2fV [$device getChannelVoltageSet $options(-channel)]]
        set status [$device getChannelStatus $options(-channel)]
        if {[lsearch $status On] ne -1} {
            $win.onoff configure -text {Turn Off}
            set color $options(-oncolor)
        } else {
            $win.onoff configure -text {Turn On}
            
            # See if we need to report fail or if we are just off:
            
            set events [$device getChannelEvents $options(-channel)]
            if {[goodStatus $events]} {
                set color $options(-offcolor)
            } else {
                set color $options(-failcolor)
            }
        }
        $win configure -background $color
        foreach widget [winfo children $win] {
            $widget configure -background $color
        }
    }
    #-------------------------------------------------------------------
    #
    #  The on/off button was clicked:
    #
    method onOnOffButton {} {
        set status [$options(-device) getChannelStatus $options(-channel)]
        if {[lsearch $status "On"] == -1} {
            $options(-device) setChannelVoltageSet $options(-channel) [$self get]
            $options(-device) channelOn $options(-channel)
        } else {
            $options(-device) channelOff  $options(-channel)
        }
    }

    #-------------------------------------------------------------------
    #
    # Get the value of the settings entry:
    #
    method get {} {
        return [$win.request get]
    }
    #--------------------------------------------------------------------
    #
    #   Process mouse clicks in the field:
    #
    method onClick {} {
        set command $options(-command)
        if {$command ne ""} {
            eval $command
        }
    }
}
#************************Megawidget for summarizing all channels**************
#
# OPTIONS:
#    -device     - Name of the device to monitor/control.
# METHODS:
#    update      - Update the state of the display
# LAYOUT:
#    +-------------------------------------------+
#    |  Rows of up to 4 channel summary widgets  |
#    |  that show all the supplies in a single   |
#    |   module                                  |
#    +-------------------------------------------+
#    |     [All On]    [All Off]   [Kill All]    |
#    +-------------------------------------------+
#
snit::widget vhsSummary {
    option -device
    option -notebook
    delegate option * to hull
    
    constructor args {
        $self configurelist $args
        
        if {$options(-device) eq ""} {
            error "vhsSummary widget creation requires a -device option ($self)"
        }
        
        #  Divide the wdiget into a top and bottom frame:
        
        set top     [frame $win.channels    -borderwidth 3 -relief groove]
        set bottom  [frame $win.controls]
        
        #  Fill the top part with the channels:
        
        set device $options(-device)
        set chans  [$device channelCount]
        set row    0
        set col    0
        for {set i 0} {$i < $chans} {incr i} {
            set w [channelSummary $top.chan$i  -device $device -channel $i -relief groove -borderwidth 3]
            $w configure -command [mymethod selectTab [expr $i + 1]]
            grid $w -row $row -column $col
            incr col
            if {$col >= 4} {
                set col 0
                incr row
            }
        }
        grid $top -row 0 -column 0
        
        # Fill the bottom part with some control buttons and the ramp speed contol:
        
        set maxvolts [$options(-device) getNominalVoltage 0]
        set maxramp  [expr 0.2 * $maxvolts];                   # 20% of nominal voltage.
        label  $bottom.ramplbl -text {Ramp Rate (V/S)}
        spinbox $bottom.ramp   -from 1.0  -to $maxramp -increment 1.0 \
            -command [mymethod setRamp] -width 5
        label  $bottom.rampinclbl -text {Ramp Increment}
        spinbox $bottom.rampinc -values {1.0 5.0 10.0 50.0 100.0}  \
            -command [list $bottom.ramp configure -increment %s] -width 5
        bind $bottom.ramp <Return> [mymethod setRamp]
        bind $bottom.ramp <FocusOut> [mymethod restoreRamp]
        $self restoreRamp
        
        
        button $bottom.allon   -text {All On}   -command [mymethod onAllOnButton]
        button $bottom.alloff  -text {All Off}  -command [mymethod onAllOffButton]
        button $bottom.killall -text {Kill All} -command [mymethod onKillAllButton]
        button $bottom.resetall -text {Reset All} -command [mymethod onResetAllButton]
        
        grid $bottom.ramplbl $bottom.ramp $bottom.rampinclbl $bottom.rampinc
        grid $bottom.allon $bottom.alloff $bottom.killall $bottom.resetall
        grid $bottom -row 1 -column 0        
    }
    #---------------------------------------------------------------------------------
    #  Select tab n in the notebook
    #  if the notebook is defined.
    #
    method selectTab num {
        set notebook $options(-notebook)
        if {$notebook ne ""} {
            $notebook select $num
        }
    }
    #---------------------------------------------------------------------------------
    #
    #   Set the ramp from the value in $win.controls.ramp
    #
    method setRamp {} {
        set voltsPerSecond [$win.controls.ramp get]
        set maxvolts [$options(-device) getNominalVoltage 0]
        
        set pctPerSecond [expr 100.0*$voltsPerSecond/$maxvolts]
        
        $options(-device) setRampSpeed $pctPerSecond
    }
    #---------------------------------------------------------------------------------
    #
    #  restore the $win.controls.ramp value from the current ramp speed register.
    #
    method restoreRamp {} {
        set pctPerSecond [$options(-device) getRampSpeed]
        set maxvolts     [$options(-device) getNominalVoltage 0]
        set voltsPerSecond [expr $pctPerSecond*$maxvolts/100.0]
        
        $win.controls.ramp set $voltsPerSecond
    }
    #---------------------------------------------------------------------------------
    #
    # Update is just a matter of updating the children:
    #
    method update {} {
        set chans [$options(-device) channelCount]
        for {set i 0} {$i < $chans} {incr i} {
            $win.channels.chan$i update
        }
    }
    #--------------------------------------------------------------------------------
    #
    #  Try to reset all fault conditions in all channels
    #
    method onResetAllButton {} {
        set dev $options(-device)
        set chans [$dev channelCount]
        
        for {set i 0} {$i < $chans} {incr i} {
            $dev resetAllChannelEvents $i
        }
    }
    #--------------------------------------------------------------------------------
    #
    #   onAllOnButton - turns on all HV channels:
    #                   seems like for some firmware the All on function does not work.
    #
    method onAllOnButton {} {
        set dev $options(-device)
        set chans [$dev channelCount]
        for {set i 0} {$i < $chans} {incr i} {
            $dev setChannelVoltageSet $i [$win.channels.chan$i get]
            $dev channelOn $i
        }
    }
    #-------------------------------------------------------------------------------
    #
    #  onAllOffButton  - Turn all hv channels off.
    #
    method onAllOffButton {} {
        set dev $options(-device)
        set chans [$dev channelCount]
        for {set i 0} {$i < $chans} {incr i} {
            $dev channelOff $i
        }
    }
    #------------------------------------------------------------------------------
    #
    #  Emergency off all channels
    #
    method onKillAllButton {} {
        set dev $options(-device)
        set chans [$dev channelCount]
        for {set i 0} {$i < $chans} {incr i} {
            $dev emergencyOff $i
        }
    }
}
#-------------------------------------------------------------------
# saveFile - Prompt for a file in which to save the current settings:
#
# Parameters:
#   module   - The module that we are saving.
#
proc saveFile {module} {
    set file [tk_getSaveFile    -defaultextension .vhq    \
                                -filetypes                  \
            {{{Vhq  Files} .vhq } {{All Files} * }}          \
            -title {Select file}]
    if {$file ne ""} {
        set fd [open $file w]
        puts $fd "ramp [$module getRampSpeed]"
        set channels [$module channelCount]
        for {set i 0} {$i < $channels} {incr i} {
            puts $fd "voltage $i [$module getChannelVoltageSet $i]"
            puts $fd "current $i [$module getChannelCurrentTrip $i]"
        }
        close $fd
    }
}
#-------------------------------------------------------------------
# readFile:
#   - Warn the user that we'll ramp the voltages  down first.
#   - Prompt for a file name.
#   - source the file in to do the settings.
#   - Destroy and recreate the GUI so that we can
#    get stuff re-initialized.
#
proc readFile {module} {
    global poll
    global defaultUpdateRate
    
    set answer [tk_messageBox -title {Continue?} -icon warning -type okcancel  \
                -message {Before restoring the file we are going to ramp the voltages down}]
    if {$answer eq "cancel"} return
    
    # Prompt for the file.. cancel will still leave us powered up.
    #
    
    set file [tk_getOpenFile    -defaultextension .vhq    \
                                -filetypes                  \
            {{{Vhq  Files} .vhq } {{All Files} * }}          \
            -title {Select file}]
    if {$file eq ""} return
    
    # Ramp the module down...
    
    powerDown $module
    
    # Read the file:
    
    source $file
    
    #  Recreate the gui and poll manager.
    
    $poll destroy
    foreach widget [winfo children .] {
        destroy $widget
    }
    set poll [pollManager create %AUTO% -period $defaultUpdateRate]
    
    setupGui $module $poll
    $poll start
}

#-------------------------------------------------------------------
# Power the module down.
#
proc powerDown module {
    set channels [$module channelCount]
    for {set i 0} {$i < $channels} {incr i} {
        $module channelOff $i
    }
}
#-------------------------------------------------------------------
#
#  exitProgram - Exit the program. We ask if the user wants the
#  voltages ramped down first.
#
# Parameters:
#   module   - The hardware we're controlling
proc exitProgram {module} {
    set answer [tk_messageBox -title {Ramp Down}  -icon question \
                -type yesnocancel                                \
                -message {Do you want to ramp down the supply before exiting?}]
    
    if {$answer eq "cancel"} return
    
    if {$answer eq "yes"} {
        powerDown $module
    }
    
    exit
                
}
#-------------------------------------------------------------------
#
#  Determine if the channel is in some fail state:
#
#  Returns true if channel state is good.
#
proc goodStatus {events} {
    # Remove the On, Ramping, VoltageControlActive
    # event words and if there's anything left we have a problem.
    #
    set idx [lsearch $events VoltageControlActive]
    if {$idx != -1} {
        set events [lreplace $events $idx $idx]
    }
    set idx [lsearch $events Ramping]
    if {$idx != -1} {
        set events [lreplace $events $idx $idx]
    }
    set idx [lsearch $events On]
    if {$idx != -1} {
        set events [lreplace $events $idx $idx]
    }
    return [expr [llength $events] == 0]
}
#
#-----------------------------------------------------------------------------
#
#  resetAll:
#     Attempts to set all events, both the module wide and per-channel
#     events.
# Parameters:
#    device    - The name of the device object to reset.
#
proc resetAll device {
    set channels [$device channelCount]
    
    # Try to clear the module wide events:
    
    $device clearTempNoGoodEvent
    $device clearSupplyNotGoodEvent
    $device clearSafetyLoopOpenEvent
    
    # Try to clear the per channel events:
    
    for {set chan 0} {$chan < $channels} {incr chan} {
        $device resetAllChannelEvents $chan
    }
}

#-----------------------------------------------------------------------------
#
#  setupModule:
#     Initialize the iSeg module the way we want it:
#     - All Module level events are allowed.
#     - Kill is enabled, as are set adjustment
#     - Each channel in the module is allowed to set its bit in the event channel
#       mask if something goes wrong.
#     - Each module is enabled to set the following events:
#       VoltageLimit, CurrentLimit, Trip, ExtInhibit, VoltageBounds,
#       CurrentBounds, EmergencyOff, InputError
#     - We try to reset all of the channel events and the module events.
# Everything else is left alone.. This allows a module that has been left on
# with an interface that exits in error to have the control panel re-attach
# with no break in HV service.
#
# Parametrs:
#    device    - Name of the device object that we are controlling.
#
proc setupModule device {
    set channels [$device channelCount];       #We'll need this too no doubt.
    
    #  Setup the module wide event mask.
    
    $device allowTemperatureEvent true
    $device allowPowerEvent       true
    $device allowSafetyEvent      true
    
    # Enable kill and turn on fine adjust.
    
    $device killEnable true
    $device fineAdjust true
    
    # Allow events from all channels to post to the module event channel status:
    
    $device maskChannelEvents 0x0fff;            # This covers all potential channels.
    
    # Set up the initial event masks for each channel to allow only the events we want:
    
    set channelEventMask [expr  $vhs::ChannelEventVoltageLimit      | \
                                $vhs::ChannelEventCurrentLimit      | \
                                $vhs::ChannelEventTrip              | \
                                $vhs::ChannelEventInhibit           | \
                                $vhs::ChannelEventVoltageBounds     | \
                                $vhs::ChannelEventCurrentBounds     | \
                                $vhs::ChannelEventEmergencyOff      | \
                                $vhs::ChannelEventInputError]
    for {set chan 0} {$chan < $channels} {incr chan} {
        $device setChannelEventMask $chan $channelEventMask
    }
    
    #  Try to make the device usable by resetting all event conditions:
    
    resetAll $device
}
#-----------------------------------------------------------------------------
#
# deleteStatus
#    Protocol handler for window deletion for a channel status window:
#    - Cancels the udpate.
#    - Destroys the window
# Parameters:
#   widget   - The top level being destroyed.
#   poll     - The poll manager handling the update schedule.
#   id       - The id of the poll item to destroy.
#
proc deleteStatus {widget poll id} {
    $poll remove $id
    destroy $widget
}
#-----------------------------------------------------------------------------
# displayStatus
#    - Creates and displays a channel detailed status
#    - Arranges for the status to be updated continuously
#    - Arranges for the status update to stop when the window is destroyed.
#
# Parameters:
#   device         - The device for which we're displaying the status.
#   channel        - Number of the channel within the device.
#   poll           - Poll manager for the updates.
#
proc displayStatus {device channel poll} {
    
    # If the status page is already up, just transfer focus to it:
    
    if {[winfo exists .status$channel]}  {
        focus .status$channel
        return;  
    }
    # Create/display the widget.
        
    toplevel .status$channel
    VhsWidgets::channelStatus .status$channel.status -device $device -channel $channel
    pack .status$channel.status -fill both -expand 1
    
    #  Arrange for updates to be automatic
    
    set pollid [$poll add [list .status$channel.status update]]
    
    # Arrange for a WM_DELETE_WINDOW protocol handler to handle safe window
    # deletion.
    
    wm protocol .status$channel WM_DELETE_WINDOW \
                [list deleteStatus .status$channel $poll $pollid]
}
#-----------------------------------------------------------------------------
# addTab
#    Adds a tab to the gui and configures it appropriately.
#
# Parameters:
#   notebook     - The notebook widget name.
#   widget       - The widget to display in the tab.
#   name         - The tab name/label
#
proc addTab {notebook widget name} {
    $notebook add  $widget -text $name 
}
#-----------------------------------------------------------------------------
#
#  setupGui
#      Set up the GUI:
#     - Create a tabset
#     - Add a summary page to the tabset
#     - Add channel pages to the tabset (with autoupdate attached to the pollmanager).
#     - Add the environment page to the tabset.
#
# Parameters:
#   module       - The device that we are controlling.
#   poll         - The poll manager we'll use to manage update events.
#
#
proc setupGui {module poll} {
    
    menu .menubar
    menu .menubar.file -tearoff 0
    .menubar add cascade -label File -menu .menubar.file
    .menubar.file add command -label Save...     -command [list saveFile $module]
    .menubar.file add command -label Restore...  -command [list readFile $module]
    .menubar.file add separator
    .menubar.file add command -label Exit...     -command [list exitProgram $module]
    
    . configure -menu .menubar
    
    ttk::notebook .notebook 
    
    # Create and add the environment page.
    
    addTab .notebook [createSummary .notebook $module $poll] Summary
    
    # Now the per channel pages:
    
    set channels [$module channelCount]
    for {set chan 0} {$chan < $channels} {incr chan} {
        addTab .notebook                                                                \
               [VhsWidgets::channel .notebook.chan$chan -device $module -channel $chan] \
               Channel_$chan
        $poll add [list .notebook.chan$chan update]
        .notebook.chan$chan configure -statuscommand [list displayStatus $module $chan $poll]
    }
    # Finally the environmental summary page
    
    addTab .notebook [VhsWidgets::environment .notebook.env -device $module] Environment
    $poll add [list .notebook.env update]
    
    #  A button at the bottom of all this allows for a reset all to be done:
    
    button .resetall -text {Reset Module} -command [list resetAll $module]
    
    # Schedule the summary update to make the tab colors do what's needed:
    #
    $poll add [list colorTabs .notebook $module]
    
    pack .notebook -fill both -expand 1
    pack .resetall -anchor e
    
    
}
#-----------------------------------------------------------------------------
# Color the tabs based on the channel status; and module status.
#
# Parameters:
#   notebook   - The notebook whose tabs we color
#   module     - The module we monitor.
#
#  ttk::notebook can't do that...
#  TODO:  Set icons behind the text that are pure colors like on newscaler
#         tabs.
#
proc colorTabs {notebook module} {
    return
    set channels [$module channelCount]
 
    for {set i 0} {$i < $channels} {incr i} {
        set events [$module getChannelEvents $i]
        if {[goodStatus $events]} {
            set color [$notebook cget -background]
            
        } else {
            set color red
        }
        $notebook tab configure Channel_$i -background $color
        $notebook tab configure Channel_$i -selectbackground $color
    }
    
    #  If the module has bad events, then we color the environment tab:
    
    set status [expr [$module haveTempNoGoodEvent]         || \
                     [$module haveSupplyNotGoodEvent]      || \
                     [$module haveSafetyLoopOpenEvent]]
    if {$status} {
        set color red
    } else {
        set color [$notebook cget -background]
    }
    $notebook tab configure Environment -background $color
    $notebook tab configure Environment -selectbackground $color
}
#-----------------------------------------------------------------------------
#
#  Create the summary widget.  That displays all of the channels in a summary
#  form.
#
# Parameters:
#   parent   - The parent widget in which the summary is displayed.
#   device   - Device being controlled.
#   poll     - Poll manager used to schedule the update\
# Returns:
#   widget name.
#
proc createSummary {parent device poll} {
    set summary [vhsSummary $parent.summary -device $device -notebook $parent]
    $poll add [list $summary update]
    return $summary
}
#==========================Save/restore Commands==============================
#
proc ramp {rate} {
    global module
    $module setRampSpeed $rate
}
proc voltage {channel value} {
    global module
    $module setChannelVoltageSet $channel $value
}
proc current {channel value} {
    global module
    $module setChannelCurrentTrip $channel $value
}

#==========================Final initialization and GUI startup===============
#
#
setupModule $module
setupGui    $module $poll
$poll start;                           # Start updating the GUI.



