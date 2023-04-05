#!/usr/bin/env tclsh

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#    Jeromy Tompkins
#	   NSCL
#	   Michigan State University
#	   East Lansing, MI 48824-1321

# find out where we are... this should be in $DAQROOT/bin
set here [file dirname [file normalize [info script]]]
# add current directory 
lappend auto_path $here
# add $DAQROOT/TclLibs to the auto_path
lappend auto_path [file join $here .. TclLibs]

# Tk package parses argv when it is required... It will cause a crash 
# when it encounters unknown commands, so we have to hide the real arguments
# from it.
set argv_tmp $argv
set argv [list]
package require Tk

package require mcfd16guiapp


package require cmdline ;# for the command line option parsing

# Handle the options
set options {
  {protocol.arg   ""          "type of communication protocol (either \"usb\", \"mxdcrcbus\", or \"test\")"}
  {serialfile.arg ""          "name of serial file (e.g. /dev/ttyUSB0) \[MANDATORY for USB\]"}
  {module.arg     ""          "name of module registered to slow-controls server \[MANDATORY for MxDC RCbus\]"}
  {host.arg       "localhost" "host running VMUSBReadout slow-controls server" }
  {port.arg       27000       "port the slow-controls server is listening on" }
  {devno.arg      0           "address of targeted device on rcbus"}
  {configfile.arg ""          "file to load GUI state on startup"}
}
set usage " --protocol value ?option value? :"

if {("-help" in $argv_tmp) || ("-?" in $argv_tmp)} {
  puts [cmdline::usage $options $usage]
  exit
}


# Enforce the required options for the usb and mxdcrcbus protocols
proc assertProtocolDependencies {arrname} {
  global params
#  puts $arrname
#  upvar 1 $arrname params

  set fatal 0
  if {$params(-protocol) eq ""} {
    puts "User must pass either \"usb\" or \"mxdcrcbus\" for the --protocol option"
    set fatal 1
  }
  if {$params(-protocol) eq "usb"} {
    if {$params(-serialfile) eq ""} {
      puts "The --serialfile argument is mandatory for usb protocol but none was provided."
      set fatal 1
    }
  } elseif {$params(-protocol) eq "mxdcrcbus"} {
    if {$params(-module) eq ""} {
      puts "The --module argument is mandatory for mxdcrcbus protocol but none was provided."
      set fatal 1
    }
  } elseif {$params(-protocol) ne "test"} {
    set msg "User provided an argument to the --protocol argument that is not "
    append msg "supported. Only \"usb\" and \"mxdcrcbus\" are allowed."
    puts $msg
    set fatal 1
  }

  if {$fatal == 1} {
    flush stdout
    exit
  }
}
################## GLOBAL STUFF #############################################
#

proc ConfigureStyle {} {
  ttk::style configure "Title.TLabel" -foreground "firebrick" \
                                      -font "helvetica 28 bold"

  ttk::style configure Header.TLabel -background {orange red} 
  ttk::style configure Header.TFrame -background {orange red}

  ttk::style configure Grouped.TEntry -background snow3
  ttk::style configure Grouped.TRadiobutton -background snow3
  ttk::style configure Grouped.TSpinbox -background snow3
  ttk::style configure Grouped.TFrame -background snow3
  ttk::style configure Grouped.TLabel -background snow3

  ttk::style configure Even.TEntry -background snow3
  ttk::style configure Even.TRadiobutton -background snow3
  ttk::style configure Even.TSpinbox -background snow3
  ttk::style configure Even.TFrame -background snow3
  ttk::style configure Even.TLabel -background snow3

  ttk::style configure Odd.TEntry -background snow3
  ttk::style configure Odd.TRadiobutton -background snow3
  ttk::style configure Odd.TSpinbox -background snow3
  ttk::style configure Odd.TFrame -background snow3
  ttk::style configure Odd.TLabel -background snow3

  ttk::style configure Commit.TButton -background orange

  ttk::style configure Pulser.TLabel -background snow3
  ttk::style configure Pulser.TFrame -background snow3
  ttk::style configure Pulser.TRadiobutton -background snow3
}


########### START THE ACTUAL EXECUTIONAL PORTION OF THE SCRIPT ###############


# try to parse... do so in a catch block because -help will cause a thrown 
# exception
if {[llength $argv_tmp]==0} {
  puts [::cmdline::usage $::options $::usage]
  exit
}

set res [catch {
  array set ::params [::cmdline::getoptions argv_tmp $::options]
	foreach key [array names ::params] {
		set ::params(-$key) $::params($key)
		array unset ::params $key
	}
} msg]
if {$res == 1} {
  puts $msg
  exit
}

assertProtocolDependencies ::params ;# exits on for bad cmdline options

ConfigureStyle ;# make elements pretty with colorful styles


## the actual code to start the application

ttk::frame .app
MCFD16GuiApp app -widgetname .app {*}[array get ::params]

grid .app -sticky nsew
grid rowconfigure . 0 -weight 1
grid columnconfigure . 0 -weight 1
wm resizable . false false
