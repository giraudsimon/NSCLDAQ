#!/bin/sh
# Start Wish. \
exec tclsh ${0} ${@}
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

#  Set up the auto_path to include our install directories.
#  We assume we're one down from the top.

set here [file dirname [info script]]
set libdir [file normalize [file join $here .. TclLibs]]

#   
#  Prepend to auto_path only if it's not already 
#  there.
#
if {[lsearch -exact $auto_path $libdir] == -1} {
    set auto_path [concat $libdir $auto_path]
}

package require camac
package require bcnafutils

# usage: bcnaf.tcl b c n a f [d]

proc usage {} {
    puts stderr " usage:\n\t  wienerbcnaf.tcl b c n a f \[d\]"
}


bcnafutils::checkUsage wienerbcnaf.tcl

set module [wienercamac::cdreg $b $c $n]
set output [wienercamac::cfsa $module  $f $a $d]
set data [lindex $output 0]
set q    [lindex $output 2]
set x    [lindex $output 1]

set dataout [format "Data = %d(10), %x(16), %o(8)" $data $data $data]
puts "$dataout X=$x Q=$q"

