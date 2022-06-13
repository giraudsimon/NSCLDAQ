#!/bin/bash
#
#  Simple script to load a shaper with the set of values in a settings file.
#  Note that in SEE, only the first 8 channels of the shaper are used.
#
#  Usage:
#     loadshaper.tcl   config  settings
#
#      config is a file that contains the configuration of the shaper.
#      module.  It is expected to set:
#      controller               - Name of the CAENnet controller.
#      nodeid                   - CAENnet node of the shaper.
#
#      Note: The controller name is of the form caennet_baseaddress.
#      where baseaddress is the base address in VME space of the caennet
#      controller.
#
#      settings is a file that contains the values to program into the shaper.
#      It is assumed to set:
#
#       offset                   - Module common offset
#       outconfig(0:7)           - Selects the output configuration.
#       outpolarity(0:7)         - Selects the channel input polarity.
#       shapetime(0:7)           - Selects the channel shaping times.
#       coarsegain(0:7)          - Selects the channel coarse gains.
#       finegain(0:7)            - Selects the channel fine gains.
#       pole0(0:7)               - per channel pole0 adjust.
#      Regardless of the request, the multiplexor is disabled.
#      The module is programmed as suggested onpage 12 of the N586B manual.
#
# Start tclsh \
exec tclsh $0 $@



#
#   Locate out package directory and require the shaper support package from
#   it
#

set here [file dirname [info script]]
set libDir [file normalize [file join $here .. TclLibs]]

if {[lsearch -exact $auto_path $libDir] == -1} {
    set auto_path [concat $libDir $auto_path]
}


package require caennet
package require n568b  2.0

#
#   list the program usage.
#
proc Usage {} {
    puts "Usage: "
    puts "   loadshaper.tcl config ?settings?"
    puts "Where:"
    puts "   config    - is the shaper module configuration file."
    puts "   settings  - is the shaper module settings file."

}


#
#   Source in the configuration files:
#

if {([llength $argv] < 1) || ([llength $argv] > 2)} {
    Usage
    exit
}

set config   [lindex $argv 0]

if {[file exists $config] && [file readable $config]} {
    source $config
} else {
    puts "The configuration file $config does not exist or is not readable"
    Usage
    exit
}

if {[llength $argv ]== 2} {
    set settings [lindex $argv 1]
    if {[file exists $settings] && [file readable $settings]} {
        source $settings
    } else {
        puts  "The settings file $settings does not exist or is not readable"
        Usage
        exit
    }
}

#
#  Parse the controller to produce the caennet  base address (crate 0)
#  create the controller and reset the caennet.
#



set module [caennet::create $base $crate]
caennet::reset $module


#
#  Set the output configuration on all channels any undefined will
#  be left alone.
#
for {set i 0} {$i < 16} {incr i} {
    if {[array names outconfig $i] != ""} {
        n568b::SetOutputConfiguration $module $nodeid $i $outconfig($i)
    }
}


#
#   Set the output polarity on all channels any undefined are left alone
#

for {set i 0} {$i < 16} {incr i} {
    if {[array names outputpolarity $i] != ""} {
        n568b::SetPolarity $module $nodeid $i $outpolarity($i)
    }
}

#
#  Set the shaping time on all channels:
#
for {set i 0} {$i < 16} {incr i} {
    if {[array  names shapetime $i] != ""} {
        n568b::SetShapingTime $module $nodeid $i $shapetime($i)
    }
}
#
#  Set the coarse gain on all channels:
#

for {set i 0} {$i < 16} {incr i} {
    if  {[array names coarsegain $i] != ""} {
        n568b::SetCoarseGain $module $nodeid $i $coarsegain($i)
    }
}

#
#  Set the fine gain on all channels:
#

for {set i 0} {$i < 16} {incr i} {
    if {[array names finegain $i] != ""} {
        n568b::SetFineGain $module $nodeid $i $finegain($i)
    }
}

#
#   Set the pole 0 adjust on all channels:
#
for {set i 0} {$i < 16} {incr i} {
    if {[array names pole0 $i] != ""} {
        n568b::SetPole0 $module $nodeid $i $pole0($i)
    }
}

#
#   Set the common offset
#
if {[info vars offset] != ""} {
    n568b::SetOffset $module $nodeid $offset
}


#
#  Disable the mux:
#

n568b::DisableMuxOut $module $nodeid

