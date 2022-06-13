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


package require cmdline

# Add the DAQROOT/TclLibs to the auto_path
set here [file dirname [file normalize [info script]]]
lappend auto_path $here
lappend auto_path [file join $here ../TclLibs]

# the next two lines are a workaround to prevent the Tk argument parsing from seeing
# switches like -help.
set argv2 $argv
set argv {}

package require mdgg16guiapp


# Handle the options
set options {
  {-module.arg     ""          "name of module registered to slow-controls server"}
  {-host.arg       "localhost" "host running VMUSBReadout slow-controls server" }
  {-port.arg       27000       "port the slow-controls server is listening on" }
  {-configfile.arg   ""        "configuration file"}
}
set usage " -module value ?option value? :"

if {("-help" in $argv2) || ("-?" in $argv2)} {
  puts [cmdline::usage $options $usage]
  exit
}

if {"--module" ni $argv2} {
  puts "The -module option is MANDATORY and was not provided."
  puts [cmdline::usage $options $usage]
  exit
}

set res [catch {
  array set ::params [::cmdline::getoptions argv2 $::options]
} msg]
if {$res == 1} {
  puts $msg
  exit
}

# Set up the style
ttk::style configure "Title.TLabel" -foreground "midnight blue" \
                                    -font "helvetica 28 bold"
ttk::style configure "Header.TLabel" -background "cornflower blue"
ttk::style configure "Header.TFrame" -background "cornflower blue"
#ttk::style configure "Even.TCheckbutton" -background ""
ttk::style configure "Odd.TCheckbutton" -background "snow3"
ttk::style configure "Odd.TFrame" -background "snow3"
ttk::style configure "OutOfSync.TLabel" -background "yellow" \
                                        -foreground "red" \
                                        -font "helvetica 16 bold"



MDGG16GuiApp app {*}[array get ::params]

wm resizable . false false
