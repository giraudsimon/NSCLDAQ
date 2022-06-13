#!/bin/bash

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
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321
#

#start tclsh\
exec tclsh $0 $@


#    This script is a frame around control panels for the vhq 2xx set of
#    VME detector bias supply modules.  The actual meat of the control panel
#    is the vhqPanel megawidget.  We do add a few other pieces to the
#    user interface:
#         A menubar that includes a menu to set ramp-speeds and current limits.
#         A text widget that logs Alarm conditions.
#         The usual menu gizmos that allow one to save/restore the module
#         state.  Note that restoring the state does not start a ramp.
#         It simply sets the target voltage.  The user must manually initiate
#         a ramp to get there... this is an equipment protection feature.
#
# Usage:
#     vhqControl.tcl ?configuration ...?
#          configuration   - are configuration files that describe the module and contain
#                            optional property variables that will be formed into a property list
#                            and installed in the module.


#  First, need to ensure that we're going to be able to find our packages.
#  These are located in the directory tree that is rooted up one level.
#  Note that ./.. is canonicalized to an absolute path.
#
set here [file dirname [info script]]
set libdir [file normalize [file join $here .. TclLibs]]
set helpdir   [file join $here .. TclLibs data vhq]


if {[lsearch $auto_path $libdir] == -1} {
    set auto_path [concat $libdir $auto_path]
}
#  Now we can require the needed packages:

package require vhqPanel
package require Iwidgets
package require snit

# Global data:

set Version 1.1
set panels       0
set activePanels 0
set Models [list                        \
               [list 202L-s 2000 100]   \
               [list 202M   2000 3000]  \
               [list 202M-h 2000 6000]  \
               [list 203M   3000 2000]  \
               [list 203M-h 3000 4000]  \
               [list 204L   4000 1000]  \
               [list 204M-h 4000 3000]  \
               [list 205L   5000 1000]  \
               [list 205L-h 5000 2000]  \
           ]

set AlarmText(manual)     {Manual mode}
set AlarmText(error)      {General error condition}
set AlarmText(Voverset)   {Voltage above max.}
set AlarmText(OverVorI)   {Voltage or current above max}
set AlarmText(ilimit)     {Current limit}
set AlarmText(BadQuality) {Poor regulation}

set configFileTypes [list            \
                    [list {Configurations} .cfg]      \
                    [list {Settings}       .settings] \
                    [list {Tcl Scripts}    .tcl]      \
                    [list {Tk Scripts}     .tk]       \
                    [list {All Files}      *]] 


#------------------------------------------------------------------------------
# getModel  maxv maxi
#        Given voltage and current maxima, determines the correct model name
#        of the device.. or issues an error indicating that we don't know of
#        a device with this model information.
# Parameters:
#   maxv    - Maximum voltage
#   maxi    - Maxium current.
#
proc getModel {maxv maxi} {
    global Models
    foreach model $Models {
        set name [lindex $model 0]
        set v    [lindex $model 1]
        set i    [lindex $model 2]

        if {($maxv == $v) && ($maxi == $i)} {
            return $name
        }
    }
    error "Unable to match $maxv $maxi with any known model."
}
# findModel model
#           Returns the list that describes a VHQ models's capabilities.
# Parameters:
#   model   - the model string.
# Errors:
#     It is an error to pass in a string that is not a know model.
#
proc findModel model {
    global Models
    foreach capabilityList $Models {
        if {$model eq [lindex $capabilityList 0]} {
            return $capabilityList
        }
    }
    error "$model is not a known vhq model."
}
#------------------------------------------------------------------------------
# getMaxV  model
#           Returns the maximum voltage capability of a model.  Note that
#           it is an error to return an unrecognized model.
# Parameters:
#   model  - The model name of the VHQ.
#
proc getMaxV model {
    set capabilities [findModel $model]
    return [lindex $capabilities 1]
}

#------------------------------------------------------------------------------
# getMaxI model
#          Returns the maximum current capability of a model.. Note that
#          it is an error to return an unrecognized model.
# Parameters:
#    model  - The model name of the VHQ.
#
proc getMaxI model {
    set capabilities [findModel $model]
    return [lindex $capabilities 2]
}

#------------------------------------------------------------------------------
# onDestroy widget toplevel
#         Called when a widget in a toplevel tree is destroyed.
#         If the widget matches the panel name, the panel is
#         destroyed formally to ensure that it's destructor is invoked.
#         If the widget is the toplevel, the activePanels global is decremented
#         and, when it hits 0
# Parameters:
#   widget   - The widget being destroyed.
#   toplevel - The toplevel widget.
#
proc onDestroy {widget toplevel} {
    global activePanels

    if {$widget eq "$toplevel.panel"} {
        catch {destroy $toplevel.panel}
    }
    if {$widget eq $toplevel} {
        incr activePanels -1
        if {$activePanels <= 0} {
            exit 0
        }
    }
}
#------------------------------------------------------------------------------
# logAlarms logwidget panelwidget what which value
#      Called to log alarm conditions.
#      Conditions set are logged in red.
#      Conditions cleared are logged in green.
#      Log messages produced are of the form:
#        timestamp Channel which Condition what has been value ? set : cleared
# Parameters:
#   logwidget    - path to the text widget in which we are logging.
#   panelwidget  - path to the panel widget that produced the log.
#   what         - The alarm condition (see the vhqPanel for more info on this.
#   which        - Channel that logged the condition.
#   value        - 1 if condition was set, 0 if cleared.
#
proc logAlarm {logwidget panelwidget what which value} {
    global AlarmText
    set timestamp  [clock format [clock seconds]]
    set reason     $AlarmText($what)
    if {$value} {
        set tag red
        set condition "Set"
    } else {
        set tag green
        set condition "Cleared"
    }
    # Construct the text:

    set text [format "%s : Channel %s %s has been %s\n" \
                        $timestamp $which $reason $condition]
    $logwidget insert end $text $tag
    $logwidget see    end

}
#------------------------------------------------------------------------------
# getConfigFile
#      Retrieve the name of a configuration file.
#      This is prompted for graphically.
# Returns:
#    Configuration file or "" If none provided
#
proc getConfigFile {} {
    return [tk_getOpenFile -defaultextension .cfg      \
                           -filetypes $::configFileTypes \
            ]
}
#------------------------------------------------------------------------------
# configurePanel  panel config
#        Configures a vhq control panel by
#        sourcing in a configuration file, turning it into
#        a property list and
#        loading the property list into the panel.
# Parameters:
#    panel   - The vhq megawidget.
#    config  - The config file name.
#
proc configurePanel {panel config} {

    source $config
    set properties [list]

    # SetPoint(a/b)  -> targeta, targetb
    # ILimit(a/b)   -> ilimita, ilimitb
    # RampSpeed(a/b) -> speeda, speedb

    foreach channel [list a b] {
        if {[array names SetPoint $channel] != ""} {
            lappend properties [list target$channel $SetPoint($channel)]
        }
        if {[array names ILimit $channel] != "" } {
            lappend properties [list ilimit$channel $ILimit($channel)]
        }
        if {[array names RampSpeed $channel] != ""} {
            lappend properties [list speed$channel $RampSpeed($channel)]
        }
    }

    # maxv, maxi ->: -model.

    if {([info var maxv] != "") && ([info var maxi] != "")} {
        lappend properties [list model [getModel $maxv $maxi]]
    }
    $panel setProperties $properties


}
#------------------------------------------------------------------------------
# helpAbout
#      Called in response to the Help->About menu entry.  A little messageBox
#      is displayed with program version and copyright information.
#
proc helpAbout {} {
    global Version
    append about "vhqControl Version $Version\n"
    append about "Author: Ron Fox\n"
    append about "(c) 2005 Michigan State University\n"
    append about "    All rights reserved\n"
    append about "Licensed for distribution under the\n"
    append about "GNU Public License.  See:\n"
    append about "  http://www.gnu.org/licenses/gpl.txt\n"
    append about "for details."
    tk_messageBox -icon info -title About \
        -message $about
}
#------------------------------------------------------------------------------
# helpTopics
#      Displays a hyperhelp widget (if one is not already active)
#      The help widget is direcdted to the overview topic.
#
# Implicit inputs:
#    helpdir    - Global containing help directory path.
#    .help      - Help widget name.
#
proc helpTopics {} {
    global helpdir
    if {![winfo exists .help]} {
        ::iwidgets::hyperhelp .help -helpdir $helpdir \
                                    -topics [list overview configfile gui menubar]
    }
    .help showtopic overview

    # All the stuff below is intended to pop the help window up
    # to the top of the stack in a very system dependent and, unfortunately
    # unreliable way.
    #

    wm withdraw .help
    wm deiconify .help
    focus .help
}
#------------------------------------------------------------------------------
# fileNew
#         Prompts for a new configuration file and opens a new panel
#         on the vhq in that file.
#
proc fileNew {} {
    set config [getConfigFile]
    if {$config ne ""} {
        processConfigFile $config
    }
}
#------------------------------------------------------------------------------
# fileRead panel
#      Prompts for and reads a configuration file, loading its settings
#      into the current panel.  No seeks are done on any voltage targets
#      however.
# Parameters:
#    panel   - The panel widget into which the configuration is read.
#
proc fileRead panel {
    set config [getConfigFile]
    if {$config ne ""} {
        configurePanel $panel $config
    }
}


#------------------------------------------------------------------------------
# fileWrite panel
#      Prompts for the name of a configuration file.  Get the properties from
#      the panel, convert them into properties and save them to file where
#      they can either be used as a control panel configuration file or
#      as a settings file.
# Parameters:
#   panel - The widget that is the vhq panel.
#
proc fileWrite panel {
    set config [tk_getSaveFile -defaultextension .cfg                        \
                               -title {Chose a file to write}                \
                               -filetypes $::configFileTypes                 \
               ]
    if {$config ne ""} {
        # If we can't open the config file... can't save:

        if {[catch {open $config w} fd]} {
            puts "Unable to open file: $config : $fd"
            return
        }


        # Get the properties we want to save.

        set properties [$panel getProperties]
        set crate      [$panel cget -crate]
        set base       [$panel cget -base]
        set name       [$panel cget -name]
        set model      [$panel cget -model]

        puts $fd "# vhq configuration file saved [clock format [clock seconds]]\n"
        puts $fd "set name $name"
        puts $fd "set crate $crate"
        puts $fd "set base $base"
        puts $fd "set maxv [getMaxV $model]"
        puts $fd "set maxi [getMaxI $model]"
        puts $fd "#"

        # Now the property list:
        #

        foreach property $properties {
            set name  [lindex $property 0]
            set value [lindex $property 1]
            set chan  [string index $name end]
            switch -glob $name {
                ilimit[ab] {
                    puts $fd "set ILimit($chan) $value"
                }
                speed[ab] {
                    puts $fd "set RampSpeed($chan) $value"
                }
                target[ab] {
                    puts $fd "set SetPoint($chan) $value"
                }
                model {
                    #  Ignore.
                }
                default {
                    error "Unrecognized property keyword: $name"
                }
            }
        }
        close $fd
    }
}

#-------------------------------------------------------------------------------
# fileClose toplevel
#      Processes the File->Close menu entry.   The user is prompted to be sure
#      that s/he knows that closing the panel will toss any unsaved settings.
#      If they confirm, the panel is closed.  Note that closing the last panel
#      results in program exit due to the action of the onDestroy member.
# Parameters:
#    toplevel   - The toplevel widget to destroy on confirmation.
#
proc fileClose toplevel {
    set answer [tk_messageBox -title {Close panel?} -type okcancel -icon question \
                              -default cancel \
                -message {If you close this panel, any unsaved settings in the panel will be lost.\
Click Ok to close or Cancel to leave the panel open}]
    if {$answer eq "ok"} {
        destroy $toplevel
    }

}



#------------------------------------------------------------------------------
# fileQuit
#      Exit the program.. We ensure the user understands that all saved
#      settings will be lost before actually exiting...and if they approve
#      just exit.
#
proc fileQuit {} {
    set answer [tk_messageBox -icon question -type okcancel -default cancel \
        -message {If you exit, all unsaved settings will be lost and all panels closed.
click "Yes" to exit and "Cancel" to continue running}]
    if {$answer eq "ok"} {
        exit 0
    }
}
#------------------------------------------------------------------------------
# configIlimits panel
#        Pops up a dialog that prompts for new current limits.  The initial
#        current limits loaded into the dialog are gotten from the control
#        panel.  The model is also used to set the maximum current limits
#        allowed for each channel.
# Parameters:
#    panel  - The vhq control panel for which we are prompting for limits.
#
proc configIlimits panel {
    set oldLimits [$panel cget -ilimit]
    set model     [$panel cget -model]
    ilimitPrompt .ilimits -chana [lindex $oldLimits 0] -chanb [lindex $oldLimits 1] \
                          -maxilimit [getMaxI $model]

    #  If the user hits the X they may have destroyed the widget so require it to exist
    #  first:
    if {[winfo exists .ilimits]} {
        set alimit [.ilimits cget -chana]
        set blimit [.ilimits cget -chanb]
        $panel configure -ilimit [list $alimit $blimit]
        destroy .ilimits
    }
}

#------------------------------------------------------------------------------
# configSpeeds  panel
#      Configure the ramp speed for both channels of a vhq control panel.
#      The ramp speed control how quickly each channel ramps towards target
#      voltages.
#         To do this we pop up a modal dialog rampSpeedPrompt that
#      prompts for the ramp speeds for both channels (pre-loaded with current
#      speeds).
# Parameters:
#    panel - The panel for which the ramp speeds will be set.
#
proc configSpeeds panel {
    set ramps [$panel cget -rampspeed]
    set model [$panel cget -model]
    rampSpeedPrompt .ramps -chana [lindex $ramps 0] -chanb [lindex $ramps 1]

    # If the user hit the X button on the dialog decoration, the widget
    # won't exist so:
    #
    if {[winfo exists .ramps]} {
        set aspeed [.ramps cget -chana]
        set bspeed [.ramps cget -chanb]
        $panel configure -rampspeed [list $aspeed $bspeed]
        destroy .ramps
    }
}


#------------------------------------------------------------------------------
# createPanel config
#      Read a configuration file (known to exist at this time)
#      and create a panel based on the information in it.
# Parameters:
#    config   - A configuration file that must contain definitions of at least:
#              name  - The device name.
#              crate - The VME crate with the device.
#              base  - The base address of the device.
#
proc createPanel config {
    global panels
    global activePanels
    source $config

    if {([info exists $name]  != "") &&
        ([info exists $crate] != "")  &&
        ([info exists $base]  != "")} {



        toplevel .vhq$panels

        if {[catch {vhqPanel .vhq$panels.panel -crate $crate -name $name -base $base} panel]} {
            tk_messageBox -icon error -title "Panel Create Failed" \
                -message "Could not create control panel from config file $config: $panel"
            destroy .vhq$panels
            return
        }

        pack $panel -side top -fill x -expand 1
        set status [frame .vhq$panels.status -relief groove -borderwidth 4]

        # Tack a status/alarm log on the bottom of the control widget.

        text      $status.log    -height 10  -yscrollcommand [list $status.scroll set]
        scrollbar $status.scroll -orient vertical -command [list $status.log yview]
        pack $status.log    -side left -fill x -expand 1
        pack $status.scroll -side right -fill y -expand 1
        pack $status -side top -fill x -expand 1

        # Make tags for colorizing log entries:

        $status.log tag add red   0.0 0.0
        $status.log tag add green 0.0 0.0

        $status.log tag configure red -foreground red
        $status.log tag configure green -foreground green

        $panel configure -alarmscript [list logAlarm $status.log]

        # Tack a menu on the top of the toplevel widget.

        set menu [menu .vhq$panels.menu]
        .vhq$panels configure -menu $menu

        menu $menu.file        -tearoff 0
        $menu.file add command -label New...    -command fileNew
        $menu.file add separator
        $menu.file add command -label Read...   -command [list fileRead $panel]
        $menu.file add command -label Write...  -command [list fileWrite $panel]
        $menu.file add separator
        $menu.file add command  -label Close    -command [list fileClose .vhq$panels]
        $menu.file add separator
        $menu.file add command  -label Quit     -command fileQuit


        menu $menu.configure   -tearoff 0
        $menu.configure add command -label {Set Current Limits...} \
                                    -command [list configIlimits $panel]
        $menu.configure add command -label {Set Ramp Speeds...}    \
                                    -command [list configSpeeds $panel]
        menu $menu.help        -tearoff 0
        $menu.help add command -label About...  -command helpAbout
        $menu.help add command -label Topics... -command helpTopics

        $menu add cascade -label File       -menu $menu.file
        $menu add cascade -label Configure  -menu $menu.configure
        $menu add cascade -label Help       -menu $menu.help

        bind .vhq$panels <Destroy> [list onDestroy %W .vhq$panels]

        incr panels
        incr activePanels
        return $panel
    } else {
        error "Configuration file $config does not contain enough information to make the panel"
    }


}
#------------------------------------------------------------------------------
# processConfigFile config
#      Processes a configuration file.  The configuration file is sourced in
#      and the appropriate control panel created.   Any settings are turned into
#      a propertylist which is then loaded into the control panel.
# Parameters:
#   config  - The configuration file to process.
#
proc processConfigFile config {
    if {![file exists $config] || ![file readable $config]} {
        tk_messageBox -title {Bad Config} -icon error \
            -message "$config either does not exist or is not readable.  Check existence and permissions"
        return
    } else {
        set panel [createPanel    $config]
        if {$panel ne ""} {
            configurePanel $panel $config
        }
    }

}

# Process command line arguments..  If there are none, prompt for a configuration
# file.


if {([info var argv] == "") || ([llength $argv] == 0)} {
    set configfiles [getConfigFile]
} else {
    set configfiles $argv
}

if {[llength $configfiles] == 0} {
    tk_messageBox -icon info -title {No configs} \
        -message {You must specify at least one Hv configuration file for this program to work.}
    exit 0
} else {
    foreach config $configfiles {
        processConfigFile $config
    }
}
wm withdraw .


#===================================== Dialogs ================================

#-----------------------------------------------------------------------------
# ilimitPrompt
#     Snit megawidget to prompt for current limits.  This consists
#     of a pair of pre-loaded spinboxes (one for each channel)
#     and the usual Ok, Cancel buttons at the bottom.
#
# Options:
#     -chana    - The current limit for channel a
#     -chanb    - The current limit for channel b
#     -maxilimit- Maximum current limits allowed.
#
snit::widget ilimitPrompt {
    hulltype toplevel;               # It's a dialog

    option -chana      50
    option -chanb      50
    option -maxilimit  100

    #   Initial values of the current limits... reloaded on cancel.

    variable initiala  50
    variable initialb  50

    constructor args {
        set work   [frame $win.work    -relief groove -borderwidth 4]
        set action [frame $win.action]

        # the work area:

        label   $work.labela -text {Channel A Limit}
        spinbox $work.valuea -width 5  \
                             -from 0 -to $options(-maxilimit) -increment 1
        $work.valuea set $initiala
        label   $work.unitsa -text {uA}

        label   $work.labelb  -text {Channel B Limit}
        spinbox $work.valueb -width 5  \
                             -from 0 -to $options(-maxilimit) -increment 1
        $work.valueb set $initialb
        label   $work.unitsb -text {uA}

        grid    $work.labela $work.valuea $work.unitsa
        grid    $work.labelb $work.valueb $work.unitsb

        # the action area.

        button $action.ok     -text Ok       -command [mymethod onOk]
        button $action.cancel -text Cancel   -command [mymethod onCancel]
        pack $action.ok $action.cancel -side left

        pack $work $action -side top -fill x -expand 1

        #  Configuration of the widgets:

        $self configurelist $args

        # Make this modal:

        frame $win.hidden
        focus $win
        grab  $win
        tkwait window $win.hidden
    }
    # name cget -chana
    #       Return the value of the a spinbox.
    #
    oncget -chana {
        return [$win.work.valuea get]
    }
    # name cget -chanb
    #       return the value of the b spinbox.
    #
    oncget -chanb {
        return [$win.work.valueb get]
    }
    #
    # name configure -chana value
    #       Set the value of the channel a spinbox.
    #
    onconfigure -chana value {
        $win.work.valuea set $value
        set options(-chana)  $value
        set initiala         $value
    }
    # name configure -chanb value
    #       Set the value of channel b spinbox.
    #
    onconfigure -chanb value {
        $win.work.valueb set $value
        set options(-chanb)  $value
        set initialb         $value
    }
    # name configure -maxilimit value
    #     Set the maximum current limit for the spinboxes.
    #
    onconfigure -maxilimit value {
        $win.work.valuea configure -to $value
        $win.work.valueb configure -to $value
    }
    #---
    #   onOk
    #      Ok was clicked.. just destroy the hidden window to
    #      relase us.
    #
    method onOk {} {
        destroy $win.hidden
    }
    #---
    #  onCancel
    #      Cancel was clicked... configure the original
    #      values back in the spinboxes and chain on to onOk.
    #
    method onCancel {} {
        $self configure -chana $initiala
        $self configure -chanb $initialb
        $self onOk
    }
}
#------------------------------------------------------------------------------
# rampSpeedPrompt
#     This is a dialog megawidget to prompt for ramp speeds for both channels.
# options
#    -chana    - initial value of channel a speed.
#    -chanb    - initial value of channel b speed.
#
snit::widget rampSpeedPrompt {
    hulltype toplevel
    option -chana     100
    option -chanb     100

    variable initiala 100
    variable initialb 100

    constructor args {
        set work   [frame $win.work -relief groove -borderwidth 4]
        set action [frame $win.action]

        #  Layout the work area with the spinboxes and all.

        label    $work.alabel    -text {Channel A ramp Speed}
        spinbox  $work.avalue    -from 0 -to 255 -increment 1 -width 5
        $work.avalue set $initiala
        label    $work.aunits    -text {V/sec}

        label    $work.blabel    -text {Channel B ramp Speed}
        spinbox  $work.bvalue    -from 0 -to 255 -increment 1 -width 5
        $work.bvalue set $initialb
        label    $work.bunits    -text {V/Sec}

        grid $work.alabel $work.avalue $work.aunits
        grid $work.blabel $work.bvalue $work.bunits

        # Layout the action area:

        button $action.ok      -text Ok     -command [mymethod onOk]
        button $action.cancel  -text Cancel -command [mymethod onCancel]
        pack $action.ok $action.cancel -side left

        pack $work $action -side top  -fill x -expand 1

        # Configure any changes..

        $self configurelist $args

        #  Make this modal:

        frame $win.hidden
        focus $win
        grab  $win
        tkwait window $win.hidden

    }
    # name cget -chana
    #    Get the value from channel a.
    #
    oncget -chana {
        return [$win.work.avalue get]
    }
    # name cget -chanb
    #    Get the value from channel b.
    #
    oncget -chanb {
        return [$win.work.bvalue get]
    }
    # name configure -chana value
    #       Configure the current value of the channel a spinbox.
    #
    onconfigure -chana value {
        $win.work.avalue set $value
        set options(-chana)    $value
        set initiala           $value
    }
    # name configure -chanb value
    #       Configure current value of channel b spinbox.
    #
    onconfigure -chanb value {
        $win.work.bvalue set $value
        set options(-chana)    $value
        set initialb           $value
    }

    #----
    #  onOk
    #         Ok click... just kill $win.hidden.
    method onOk {} {
        destroy $win.hidden
    }

    #----
    #  onCancel
    #       Cancel click... restore the initial a/b and
    #       then end modality.
    #
    method onCancel {} {
        $win.work.avalue set $initiala
        $win.work.bvalue set $initialb

        $self onOk
    }
}
