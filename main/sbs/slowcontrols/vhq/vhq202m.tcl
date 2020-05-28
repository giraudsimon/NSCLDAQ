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


# (C) Copyright Michigan State University 1938, All rights reserved 
#
#  VHQ 202M vme high voltage controller module driver package.
#   Top level procedures added:
#
#    vhq::create  base ?crate?                      - Create a new module.
#    vhq::id      module                          - Get serial no of module
#    vhq::stat1   module                          - Return status reg 1.
#    vhq::stat2   module                          - Return status reg 2.
#    vhq::rampspeed module a|b ?value?            - Set/get channel ramp spd
#    vhq::setv     module a|b   ?value?            - Set/Get target voltage
#                                                   if value present ramps.
#    vhq::limit   module voltage|current a|b ?value? - Set/get v/i limit for
#                                                   selected channel.
#    vhq::actual  a|b                             - Returns actual chan {v i}
#
# 

set here    [file dirname [info script]]
set libDir  [file normalize [file join $here ..]]

if {[lsearch $auto_path $libDir] == -1} {
    set auto_path [concat $libDir $auto_path]
}

  
package provide vhq 1.0
package require Vme

namespace eval vhq {


    variable controllers "" ;# list of known controllers.

    # Manipulate controller membership list:

    
    proc isController { controller } {
        variable controllers
        return [lsearch -exact $controllers $controller]
    }

    proc RemoveController { idx } {
	variable controllers
	set controllers \
		[lreplace $controllers $idx $idx]
    }

    proc AddController { controller } {
	variable controllers
	set controllers [
	lappend controllers $controller]
    }

    # Turn status register 1 bits for a channel into {bitname 0|1} form.

    # Returns a list of bit names/bit values
    #
    proc decodeBits {bits bitnames} {
        set result [list]
        foreach bit $bitnames {
            lappend result "$bit [expr $bits & 1]"
            set bits [expr $bits >> 1]
        }
        return $result
    }
    proc decode1 {bits} {
        
        set bitnames {vz manual plus off kill rampup stable error}
        return [decodeBits $bits $bitnames]
        
    }

    proc decode2 {bits} {
        set bitnames \
            {ilimit OpComplete FpChanged Voverset Inhibited OverVorI BadQuality}
        return [decodeBits $bits $bitnames]
    }

    namespace export create delete id stat1 stat2
    namespace export rampspeed setv limit actual

}
#
#   Create a handle for a new vhq 202m controller.
#   base is the base address of the device in short I/O space
#    /dev/vme16d16  note that all registers are byte wide.
#
proc vhq::create {base {crate 0}} {
    set name vhq202m_
    append name $crate _ $base

    if {[vhq::isController $name] != -1} {
        error "There's already a controller mapped to that address"
    }

    vme create $name -device /dev/vme16d16 -crate $crate $base 0x50
    vhq::AddController $name

    return $name
}

#
# vhq::delete name
#   Delete an existing vhq202m controller handle and all resources
#   associated with it.
#
proc vhq::delete {name} {
    vhq::_checkController $name
    
    vme delete $name
    RemoveController $idx
} 
#
#  vhq::id name
#     Returns the serial number of a vhq controller module.
#
proc vhq::id {name} {
    vhq::_checkController $name
    return [$name get -w 0x3c]
}
#
#  vhq::stat1 name
#    Returns the bits of status register 1. The bits are encoded
#    as a 2 element tcl list.  The first element contains bits for chan a
#    the second for chan b.  Each element itself is a list of 2 element
#    sublist.  The sublists are of the form: {bitname 0|1}
#
proc vhq::stat1 {name} {
    vhq::_checkController $name

    set reg [$name get -w 0]
    set abits [expr $reg & 0xff]
    set bbits [expr ($reg >> 8) & 0xff]

    set alist [vhq::decode1 $abits]
    set blist [vhq::decode1 $bbits]

    return [list $alist $blist]
}
#
#  vhq::stat2 name
#    returns decoded bits of status register 2.  The bits are encoded
#    as a three element list.  The first element is the tot bit.
#    the second element is the bits associated with channel a
#    the third element is the bits associated with channel b.
#    each bit is encoded as a two element list: {bitname 0|1}
#
proc vhq::stat2 {name} {
    vhq::_checkController $name

    set reg [$name get -w 0x30]
    if {$reg & 1} {
	set tot {tot 1}
    } else {
	set tot {tot 0}
    }

    set rega [expr ($reg >> 1) & 0x7f]
    set regb [expr ($reg >> 9) & 0x7f]

    set alist [vhq::decode2 $rega]
    set blist [vhq::decode2 $regb]

    return [list $tot $alist $blist]
 
}
#
#  vhq::rampspeed name chan ?value?
#
#   Get and optionally sets the current rampspeed.
#    chan     - a|b
#    ?value?  - if provided is the new ramp speed in [2,255]
#
proc vhq::rampspeed {name chan args} {
    vhq::_checkController $name

#   Figure out which channel is specified:

    if {$chan == "a"} {
	set offset 0x0c      ;# channel a rampspeed
    } elseif {$chan == "b"} {
	set offset 0x10
    } else {
	error "Channel selector was $chan must be 'a' or 'b'"
    }

#       If the new value is supplied, then first set a new ramp speed.

    if {[llength $args] == 1} {
	if { ($args < 2) || ($args > 255) } {
	    error "Ramp speed must be in the range \[2,255\]"
	}
	$name set -w $offset $args
	after 50        ;# controller takes that long to react.
    } elseif {[llength $args] > 1} {
	error "Too  many parameters"
    }
 

#  then return the current ramp speed:

    return [$name get -w $offset]
}

#
#  vhq::setv name chan ?value?
#    Reads and optionally sets the value of the voltage target for
#    a channel. If the voltage target is set then a ramp is 
#    begun towards the target.
#
proc vhq::setv {name chan args} {
    vhq::_checkController $name

#   Figure out which channel is specified:

    if {$chan == "a"} {
	set voffset 0x04      ;# channel a voltage
	set soffset 0x34
    } elseif {$chan == "b"} {
	set voffset 0x08
	set soffset 0x38
    } else {
	error "Channel selector was $chan must be 'a' or 'b'"
    }

#       If the new value is supplied, then set the target and start a
#       ramp.
#       note that the voltage target will not be modified if out of range
#       (hardware).
#

    if {[llength $args] == 1} {
	$name set -w $voffset $args
	after 50        ;# controller takes that long to react.
    } elseif {[llength $args] > 1} {
	error "Too  many parameters"
    }
 
    set vtarget [$name get -w $voffset]
    if {[llength $args] == 1} {
	$name get -w $soffset
    }



    return $vtarget
}
#
#  vhq::limit name v|c a|b ?value?
#
#    Optionally sets a new value for current limits, gets the value
#    of the current or voltage limit.
#    The current limit is returned as a 2 element list.  The first element
#    is the hardware limit, the second the soft limit.
#    the voltage limit is returned as a single value reflecting the 
#    hardware voltage limit.  Units are 10% of full scale.
#
proc vhq::limit {name type chan args} {
    vhq::_checkController $name
#
#    First determine the channels, as these determine the offsets
#    to all registers:
#
    if {$chan == "a"} {
	set hardlimits 0x24
	set softilim   0x44
    } elseif {$chan == "b" } {
	set hardlimits 0x28
	set softilim   0x48
    }

#   If voltage, then only return the hard voltage limit:

    if {$type == "v"} {
	if {[llength $args] != 0} {
	    error "Cannot set voltage limits"
	}
	set rawlim [$name get -w $hardlimits]
	return [expr ($rawlim >> 4) & 0xf]
   

#   If current, optionally set the current limit and wait that damned 50ms
#   until the register reflects this:
#
    } elseif {($type =="c") || ($type =="i")} {
	if {[llength $args] == 1} {
	    $name set -w $softilim $args
	    after 50
	} elseif {[llength $args] > 1} {
	    error "Too  many parameters"
	}
	set rawlim [$name get -w $hardlimits]
	set rawlim [expr ($rawlim & 0xf)]
	set softlim [$name get -w $softilim]
	return [list $softlim $rawlim]


    } else {
	error "Type of limit must be v - voltage or c or i for current"
    }
    error "vhq driver bug... control should not have landed here."
    

}
#
#  vhq::actual name a|b
#    Returns the actual voltage from the selected chan.
#
proc vhq::actual {name chan} {
    vhq::_checkController $name

#   Figure out which channel is specified:

    if {$chan == "a"} {
	set voffset 0x14      ;# channel a voltage
	set ioffset 0x1c
    } elseif {$chan == "b"} {
	set voffset 0x18
	set ioffset 0x20
    } else {
	error "Channel selector was $chan must be 'a' or 'b'"
    }

    set values [$name get -w $voffset]
    return [lappend values [$name get -w $ioffset]]
}

##
# vhq::_checkController
#   If the named controller does not exist, otuput an error
#   message
# @param name  - name of the controller
#
proc vhq::_checkController {name} {
    set idx [vhq::isController $name]
    if {$idx == -1} {
        error "No such controller $name exists"
    }
}