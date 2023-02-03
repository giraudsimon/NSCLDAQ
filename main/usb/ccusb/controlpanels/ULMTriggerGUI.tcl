#!/usr/bin/env tclsh

set here [file dirname [file normalize [info script]]]
lappend auto_path $here
lappend auto_path [file join $here .. TclLibs]
lappend auto_path [file join $here .. lib]


package require InstallRoot
package require cmdline
package require Itcl 
package require ULMTriggerGUI

# Tk package parses argv when it is required... It will cause a crash 
# when it encounters unknown commands, so we have to hide the real arguments
# from it.
set argv_tmp $argv
set argv [list]
package require Tk
set argv $argv_tmp

set options {
  {module.arg "ccusb"         "name of module registered to slow-controls server"}
  {slot.arg   -1         "slot ULM occupies \[MANDATORY\]" }
  {host.arg "localhost"  "host running VMUSBReadout slow-controls server" }
  {port.arg  27000       "port the slow-controls server is listening on" }
  {ring.arg "" "name of ring VMUSBReadout is filling <your username>" }
  {configfile.arg ""  "name of file to write GUI state to" }
  {nostartprompts.arg 0  "whether to prompt user at startup regarding ULM communication" }
}

set usage " --module value ?option value? :"

if {("-help" in $argv_tmp) || ("-?" in $argv_tmp)} {
  puts [cmdline::usage $options $usage]
  exit
}

set res [catch {
  array set ::params [::cmdline::getoptions argv $options]
  foreach key [array names ::params] {
    set ::params(-$key) $::params($key)
    array unset ::params $key
  }

} msg]
if {$res == 1} {
  puts $msg
  exit
}

if {[lindex [array get params -module] 1] eq ""} {
  puts [::cmdline::usage $options $usage]
  puts "User must pass a valid value for the --module option"
  exit
}

if {[lindex [array get params -slot] 1] == -1} {
  puts [::cmdline::usage $options $usage]
  puts "User must pass a valid value for the --slot option"
  exit
}

set module [lindex [array get params -module] 1]
set slot [lindex [array get params -slot] 1]
set host [lindex [array get params -host] 1]
set port [lindex [array get params -port] 1]
set ring [lindex [array get params -ring] 1]
set path [lindex [array get params -configfile] 1]
set noPrompt [lindex [array get params -nostartprompts] 1]
puts "no prompt = $noPrompt"


ATrigger2367 mygui 0 1 $host $port $module $slot $ring
mygui SetupGUI . $path
if {$path ne {}} {

  if {! $noPrompt} {
    set msg "Do you want to write the config file settings to the ULM at startup?"
    append msg "\n\nWARNING: Choose 'NO' if a run is in progress."
    set preference [tk_messageBox -icon question \
      -message $msg -type yesno]

    if {$preference eq "yes"} {
      mygui LockGUI 0
      mygui ReadConfigFile
    }
  } else {
    mygui LockGUI 0
    mygui ReadConfigFile
  }
} else {

  if {! $noPrompt} {
    set msg "Do you want the GUI to display the current ULM state at start up?"
    append msg "\n\nWARNING: Choose 'NO' if a run is in progress!"
    set preference [tk_messageBox -icon question \
      -message $msg -type yesno]
    if {$preference eq "yes"} {
      mygui ReadStatus
    }
  } else {
    mygui ReadStatus
  }
}

# Configure some window manager details 
wm protocol . WM_DELETE_WINDOW { mygui OnExit ; destroy . } 
wm resizable . false false
