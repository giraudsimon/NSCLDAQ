#!/bin/sh

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#

#   start wish \
exec tclsh ${0} ${@}

package require Tk

set scriptDir [file dirname [info script]]
set libDir    [file normalize [file join $scriptDir .. TclLibs]]

if {[lsearch -exact $auto_path $libDir] == -1} {
    set auto_path [concat $libDir $auto_path]
}



package require Vme
#
#   Global variables:

set BlinkRate 2                        ;# Blink rate in ms.
set BlinkIntensity 500                 ;# number of blinks.
set BlinkDone   0                      ;# Set nonzero to stop blinking.
set Crate   0                          ;# Radio button seletor
set VmeMapName  ""
set devicename /dev/btp                ;# Sbs device name.

# Return a list of the crate numbers that exist.

proc GetUnitList {} { 
   global devicename
   set units ""
   set devlist ""
 
    #   Tcl 8.4 apparently won't catch the nested failures, so first
    #  enumerate the devices that exist:

    append devlist [glob -nocomplain /dev/btp\[0-9\]] \
                   [glob -nocomplain /dev/btp1\[0-5\]]

    foreach device $devlist {
      if {[catch "set type [file type $device]"] == 0} {
	 if {$type == "characterSpecial"} {
	     scan $device "/dev/btp%d" unit
	     lappend units $unit
	 }
      }
   }
   return $units
}

# Blink the access light on the selected VME crate.


proc BlinkCrate {unit interval} {
   global BlinkDone
   global VmeMapName
   global BlinkIntensity
   
   if {$VmeMapName == ""} {
      vme create test  -device standard -crate $unit \
	 0x400000 0x1000
      set VmeMapName test
   }
   for {set i 0} {$i < $BlinkIntensity} {incr i} {
      test set -l 0 0    ;# Safer given CAEN adc address sws.
   }

   if {!$BlinkDone} {
       after $interval "BlinkCrate $unit $interval"
   } else {
       vme delete test
       set VmeMapName ""
   }
}


# Respond to the locate button.. when it's start:

proc StartLocator {widget crates} {
    global BlinkRate
    global Crate
    global BlinkDone
    global VmeMapName
    
    
    set VmeMapName ""
    set BlinkDone   0
    if {[catch {BlinkCrate $Crate $BlinkRate} msg] == 0} {
	
	$widget config -text Stop \
		-command "StopLocator $widget  $crates"
	
	foreach button [pack slaves $crates] {
	    $button config -state disabled
	}
    } else {                   ;# Crate open failed.
	tk_dialog .error "No such" "The Selected crate could not be located: $msg" \
		warning 0 Dismiss
    }
}


# Repond to the locate button ... when it's stop:

proc StopLocator {widget crates} {
   global BlinkDone
   $widget config -text Locate \
      -command "StartLocator $widget $crates"
   set BlinkDone 1

   foreach button [pack slaves $crates] {
      $button config -state normal
   }

}

#  Setup the gui relative to top. 

proc SetupGui {top} {
   global Crate

   # Create the widgets.  The crate selects live in 
   # A one frame, and the action button in another.
   #
   
   frame $top.crateselect -relief groove -borderwidth 3
   frame $top.actions -relief groove -borderwidth 3

   set cratelist [GetUnitList]
   foreach crate $cratelist { 
      radiobutton $top.crateselect.c$crate -variable Crate \
	 -value $crate -text $crate
   }

   button $top.actions.locate -text Locate \
      -command "StartLocator $top.actions.locate $top.crateselect"
      
   button $top.actions.exit -text Exit -command exit

   # layout the widgets:
   
   pack $top.crateselect $top.actions -side top -fill x
   
   foreach crate $cratelist {
      pack $top.crateselect.c$crate  -side left
   }
   pack $top.actions.locate $top.actions.exit  -anchor c -side left
}


SetupGui {}




