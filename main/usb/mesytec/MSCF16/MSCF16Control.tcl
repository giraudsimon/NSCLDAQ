#!/usr/bin/env tclsh

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2015.
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

# this should be at $DAQROOT/bin
set here [file dirname [file normalize [info script]]]
# add the current directory to the auto_path
lappend auto_path $here
# add $DAQROOT/TclLibs to the auto_path
lappend auto_path [file join $here .. TclLibs]

# Tk package parses argv when it is required... It will cause a crash 
# when it encounters unknown commands, so we have to hide the real arguments
# from it.
set argv_tmp $argv
set argv [list]
package require Tk

package require mscf16guiapp
package require cmdline ;# for the command line option parsing

# Handle the options
set options {
  {-serialfile.arg ""          "name of serial file (e.g. /dev/ttyUSB0) \[MANDATORY for USB\]"}
  {-configfile.arg "MSCF16state.tcl"     "file name to load/save gui state"}
}
set usage " --serialfile value ?option value? :"

# if the user just wants help, do nothing more than provide them what they want
if {("-help" in $argv_tmp) || ("-?" in $argv_tmp)} {
  puts [cmdline::usage $options $usage]
  exit
}


#----------------------------------------------------------------------
# Here begins the code to process arguments and launch the program.
#
set res [catch {
  array set ::params [::cmdline::getoptions argv_tmp $::options]
} msg]
if {$res == 1} {
  puts $msg
  exit
}

# assert that -serialfile is provided
if {$params(-serialfile) eq ""} {
  puts "User must specify a -serialfile option"
  puts ""
  puts [cmdline::usage $options $usage]
  flush stdout
  exit
}

# I really prefer to deal with dicts rathers than arrays...
set optionsDict [array get ::params]
set ChannelNamePath [dict get $optionsDict -configfile]

# create the app (constructs the driver and gui)
MSCF16GuiApp app -widgetname .form {*}$optionsDict

# grid it.
grid .form -sticky nsew
grid rowconfigure . 0 -weight 1
grid columnconfigure . 0 -weight 1

# set up some window rules
wm title . "MSCF-16 Controls - [file tail $ChannelNamePath]"
wm protocol . WM_DELETE_WINDOW {
  ::app destroy
  destroy .
}
wm resizable . false false

# establish the style of the widgets
ttk::style configure Header.TFrame -background goldenrod3
ttk::style configure Header.TLabel -background goldenrod3 \
                                   -font {helvetica 14 bold}
#ttk::style configure Group.TSpinbox -background snow3 
ttk::style map Group.TFrame -background {disabled snow3 !disabled snow3 focus snow3}
ttk::style map Group.TLabel -background {disabled snow3 !disabled snow3 focus snow3}
ttk::style map Group.TSpinbox -background {disabled snow3 !disabled snow3 focus snow3}
ttk::style map Group.TCheckbutton -background {disabled snow3 !disabled snow3 focus snow3}
ttk::style map Group.TRadiobutton -background {disabled snow3 !disabled snow3 focus snow3}
ttk::style map Group.TEntry -background {disabled snow3 !disabled snow3 focus snow3}

