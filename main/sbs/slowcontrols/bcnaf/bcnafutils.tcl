#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file   bcnafutils.tcl
# @brief  provide common utilities for the wienerbcnaf and cesbcnaf scripts.
# @note   This file is a dirfect result of daqdev/NSCLDAQ#700.


# @author Ron Fox <fox@nscl.msu.edu>
#
package provide bcnafutils 1.0

namespace eval bcnafutils {}

##
# bcnafutils::checkUsage
#    Check that the invoking program had b,c,n,a,f parameters
#    and, if the f was a write a d parameter.
#    If not a usage error is output to stderr and
#    if so, these are unpacked to global b,c,n,a,f,d
#
# @param util - name of the utility to put in the usage mssage
#
# @note implicit parameter: ::argv
#
proc bcnafutils::checkUsage {util} {
    set nparams [llength $::argv]

    if {$nparams < 5} {
      usage
      exit -1
    }
    
    set ::b [lindex $::argv 0]
    set ::c [lindex $::argv 1]
    set ::n [lindex $::argv 2]
    set ::a [lindex $::argv 3]
    set ::f [lindex $::argv 4]
    set ::d ""
    if {($f > 15) && ($f < 24)} {
        if {$nparams != 6} {
            puts stderr "usage:\n\t  $util b c n a f \[d\]"
            exit -1
        }
        set ::d [lindex $::argv 5]
    }
    
}
