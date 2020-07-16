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
#  Caennet device driver for Tcl:
#
#
#   Top level procedures added:
#   caennet::create baseadd
#   caennet::delete controller
#   caennet::reset  controller
#   caennet::send   controller slaveaddr code ?{data}? 
#

#  Set up the auto_path so that it includes the canonicalized dir
#  above this package's location if necessary.
#  this should ensure that we'll find the Vme
#   package.

set here    [file dirname [info script]]
set libDir  [file normalize [file join $here ..]]

if {[lsearch $auto_path $libDir] == -1} {
    set auto_path [concat $libDir $auto_path]
}

   
package provide caennet 1.5
package require Vme

namespace eval caennet {

    variable caennetcontrollers "" ;# list of known controllers.
    variable timeout   1000    ;# Loop passes for response timeout.

    
    proc isController { controller } {
	variable caennetcontrollers
	return [lsearch -exact $caennetcontrollers $controller]
    }

    proc RemoveController { idx } {
	variable caennetcontrollers
	set caennetcontrollers \
		[lreplace $caennetcontrollers $idx $idx]
    }

    proc AddController { controller } {
	variable caennetcontrollers
	lappend caennetcontrollers $controller
    }
    proc waitfordata {controller} {
	variable timeout
	for {set i 0} {$i < $timeout} {incr i} {
	    set status [$controller get -w 0]
	    if {!([$controller get -w 2] & 1)} {
		return $status
	    }
	}
	error "Timedout waiting for response from $controller"
    }
    proc write {controller pkt} {
	foreach word $pkt {
	    $controller set -w 0 $word
	    set stat [$controller get -w 2]
	    if {$stat  & 1} {
		error "Problem writing $word from $pkt to $controller"
	    }
	}
	$controller set -w 4 1
	if {[$controller get -w 2] & 1} {
	    error "Problem starting transmission of $pkt to $controller"
	}
    }
    proc read {controller} {
	set reply [waitfordata $controller]
	if {$reply != 0} {	;#  If problem with the reply:
	    reset $controller   ;#  Reset the controller and
	    return $reply       ;#  just return the reason.
	}
	while {1} {
	    set word [$controller get -w 0]
	    set status [$controller get -w 2]
	    if {$status & 1} break  ;# End of message.
	    lappend reply $word
	}
	return $reply
    }

    namespace export create delete reset send

}
# caennet::createHandle base crate
#      Creates the handle name for a controller.
# Parameters:
#    base   - base address of the controller.
#    crate  - VME crate in which the controller lives.
# Returns:
#    The controller name string for that controller.
#
proc caennet::createHandle {base crate} {
    append memname caennet_ $base _ $crate
    return $memname
}

# caennet::gethandle base crate=0
#      If a caennet controller as described exists, return its
#      handle, otherwise, return an empty string
#
# Parameters:
#    base    - The base address of the vme/CAENNET controller.
#    crate   - The VME crate in which the controller lives or 0 if
#              there is none.
# Returns:
#     the handle or "" if the controller instance has not been made
#     yet.
#
proc caennet::gethandle {base {crate 0}} {
    set name [createHandle $base $crate]
    if {[::caennet::isController $name] == -1} {
	return [list]
    }
    return $name
}

#
#  caennet::create baseadd
#    Creats a caennet controller at baseadd returning the handle to the
#    controller.  Typical use:
#
#      set caennet1 [caennet::create 0x200000]
#      caennet::reset $caennet1
#
proc caennet::create {baseadd {crate 0}}  {
    set memname [caennet::gethandle $baseadd $crate]
    if {$memname == [list]} {
	set memname [caennet::createHandle $baseadd $crate]
	vme create $memname -crate $crate -device standard $baseadd 10
	::caennet::AddController $memname
    }
    return $memname
}
#
#  caennet::delete controller
#     Deletes the named caennet controller
# 
proc caennet::delete { controller } {
    
    set ctlindex [::caennet::isController $controller]
    if {$ctlindex == -1} {
	error "$controller is not a caennet controlelr"
    }
    vme delete $controller
    
    ::caennet::RemoveController $ctlindex
}
#
#   caennet::reset controller 
#
#  Reset the caennet controller (done by writing to offset 6 and waiting
#  for 5ms (3 is what the book says but ...).
#
#   Assumptions:  the package has already loaded the Vme package.
#

proc caennet::reset { controller } {
    if {[::caennet::isController $controller] == -1} {
	error "$controller is not a caennet controller"
    }

    $controller set -w 6 0xffff
    after 3
#     Stupid controller rejects the first write after this so get that
#     over with now.

    $controller set -w 0 1
    $controller get -w 2 

}

proc caennet::send {controller slave function {data {}}} {
    if {[::caennet::isController $controller] == -1} {
	error "$controller is not a caennet controller"
    }
    if {($slave < 0) || ($slave > 99)} {
	error "$slave must be in \[0,99\]"
    }
#    generate the packet as a list:

    set packet [concat 1 $slave $function]
    if {[llength $data] > 0} {
	set packet [concat $packet $data]
    }
    caennet::write $controller $packet
    set result [caennet::read $controller]
    return $result
}