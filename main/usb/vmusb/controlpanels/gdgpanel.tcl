#!/usr/bin/tclsh
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

set here [file dirname [info script]]
set tcllibs [file join $here .. TclLibs]
set auto_path [concat $tcllibs $auto_path]

package require Tk
package require usbcontrolclient
package require gdgcontrol
package require gdgwidget
package require gdgconfigfile



set panelIndex 0
set controllers [list]
set connection ""

#  Configuration items..
#  The array elements can be overridden by 
#  env variables with the same name as the array index.
#  e.g. export CONTROLHOST=llnldaq.llnl.gov
#  sets the host on which the daq is running to llnldaq.llnl.gov
#


set config(CONTROLHOST)   localhost
set config(CONTROLPORT)   27000
set config(CONFIGDIR)     [file join ~ config]
set config(CONFIGFILE)    controlconfig.tcl


proc Reconnect {args} {
    global connection
    global controllers
    global config
    set host $config(CONTROLHOST)
    set port $config(CONTROLPORT)

    while {[catch {controlClient %AUTO% -server $host -port $port} connection]} {
	tk_messageBox -icon info \
	    -message "The control panel requires readout be running, please start it: $connection"
    }
    foreach object $controllers {
	$object configure -connection $connection
	$object UpdateValues
    }
}

#  addPanel  - creates a new panel for a module.
#
proc addPanel {notebook module connection} {
    global panelIndex
    global controllers

    set name [lindex $module 1]
    set type [lindex $module 0]
    if {$type ne "jtecgdg"} return;                    # only gdg's.

    set w [gdgwidget $notebook.g$panelIndex -title $name]
    set c [gdgcontrol %AUTO% -widget $w -connection $connection \
	                     -name $name -onlost Reconnect]
    lappend controllers $c

    $c UpdateValues

    $notebook add $w -text $name
       
    incr panelIndex
}
#-----------------------------------------------------------------
# saveConfiguration  filename
#     Save the current settings to file 'filename'
#
proc saveConfiguration filename {
    global controllers

    set f [open $filename w]
    foreach controller $controllers {
	set name   [$controller cget -name]
	set values [$controller getAll]
	puts $f [list $name $values]
    }

    close $f
}
#-----------------------------------------------------------------
#  restoreConfigurationFile filename
#     Restore the configuration file 'filename'.
#     If the file is missing controllers, nothing happens to those
#     controllers.  If the file has extra controllers, they are ignored.
#

proc restoreConfigurationFile filename {
    global controllers

    # Read the file into configuration(controller-name).

    set f [open $filename r]
    while {![eof $f]} {
	set entry [gets $f]
	set name   [lindex $entry 0]
	set values [lindex $entry 1]
	if {$name ne ""} {
	    set configuration($name) $values
	}
    }
    close $f
    
    # Restore the controller values.

    foreach controller $controllers {
	set name [$controller cget -name]
	if {[array names configuration $name] ne ""} {
	    $controller setAll $configuration($name)
	}
    }
}
#-----------------------------------------------------------------
# onRestore - prompts for a filename then restores that file.
#
proc onRestore {} {
    set file  [tk_getOpenFile -defaultextension {.conf}           \
		  -filetypes                                     \
 		     {{{Configuration Files} {.conf}     }
		      {{All files}               *       }}         ]
    if {$file ne ""} {
	restoreConfigurationFile $file
    }
}

#-----------------------------------------------------------------
#
# onSave  - Prompts for a filename then saves the file.
#
proc onSave {} {
    set file [tk_getSaveFile -defaultextension {.conf}           \
		  -filetypes                                     \
 		     {{{Configuration Files} {.conf}     }
		      {{All files}               *       }}         ]
    if {$file ne ""} {
	saveConfiguration $file
    }
}

#-----------------------------------------------------------------
# 
#  onExit - Prompts to be sure that's what the user wants and, if so
#  exits.
#
#
proc onExit {} {
    set answer [tk_messageBox -icon question -title "Exit?" \
		    -type yesno                             \
		    -message {Are you sure you want to exit?}]
    if {$answer eq "yes"} {
	saveConfiguration "failsafe.conf"
	bind . <Destroy> [list]
	exit
    }
}

#----------------------------------------------------------------
#
# Called if widgets are gettting deleted. If the main toplevel
# is getting deleted, the user clicked the x button and
# we need to save the failsafe file
#
proc emergencyExit widget {
    if {$widget eq "."} {
	saveConfiguration "failsafe.conf"
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
    set ::config(CONTROLHOST) [$widget cget -host]
    set ::config(CONTROLPORT) [$widget cget -port]
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
#----------------------------------------------------------------
#  Entry point.
#
#  

#  Update the configuration based on env vars:

foreach item [array names config] {
    if {[array names env $item] ne ""} {
	set config($item) $env($item)
    }
}

##
# Prompt for the connection parameters.
#
wm withdraw .
toplevel .prompter
slowControlsPrompter .prompter.p -host $config(CONTROLHOST) -port $config(CONTROLPORT) \
    -type VMUSBReadout -okcmd [list setConnectionInfo %W] \
    -cancelcmd [list cancelPrompt %W]
set prompted 0
pack .prompter.p
bind .prompter <Destroy> [list cancelPrompt .prompter.p]
vwait prompted

wm deiconify .


#  Get connected.
#
Reconnect

ttk::notebook .panel

set modules [processConfig ~/config/controlconfig.tcl]
foreach module $modules {
    addPanel .panel $module $connection
}


pack .panel -fill both -expand 1
menu .menubar
menu .menubar.file -tearoff 0
.menubar.file add command -label "Save..."    -command onSave
.menubar.file add command -label "Restore..." -command onRestore
.menubar.file add separator
.menubar.file add command -label "Exit"       -command onExit

.menubar add cascade -label "File" -menu .menubar.file


. config -menu .menubar
bind . <Destroy> [list emergencyExit %W]