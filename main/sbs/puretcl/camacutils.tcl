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
# @file camacutils.tcl
# @brief Utilities for the pure tcl CAMAC drivers.
# @note this file comes about as a result of daqdev/NSCLDAQ#700
    

# @author Ron Fox <fox@nscl.msu.edu>
#
package provide camacutils 1.0
namespace eval camacutils {
    ##
    # camacutils::isValid
    #   If the b,c,n are not valid, throws an error.
    # @param b - branch number.
    # @param c - crate within the branch
    # @param n - slot within the crate.
    #
    proc isValid {b c n} {
           if {($b < 0) || ($b > 7)} {
               error "Branch $b is out of range"
           }
           #--ddc change to match number of vc32's
           if {($c < 7)} {
               error "Crate $c is out of range"
           }
           if {($n < 0) || ($n > 31)} {
               error "Slot $n is out of range"
           }
       }
    ##
    # camacutils::isValidf
    #    Throws an error if the function code is not valid.
    # @param f - function code to check.
    proc isValidf {f} {
        if {($f < 0) || ($f > 31)} {
            error "Function code $f is out of range"
        }
    }
    ##
    # camacutils::isValida
    #    Throws an error if a module subaddress is out of
    #    range.
    # @param a - subaddress.
    #
    proc isValida {a} {
        if { ($a < 0) || ($a > 15)} {
            error "Subaddress $a is out of range"
        }
    }
    ##
    # camacutils::isRead
    #   Determines if a function is a read operation.
    #
    # @param f - the function.
    # @return boolean - true if f is a CAMAC read operation.
    #
    proc isRead  {f} {
        return [expr ($f < 8)]
    }
    ##
    # camacutils::isWrite
    #    Determines if a function code is a valid write operation.
    #
    # @param f - the function code to check.
    # @return boolean - true if f is a write.
    #
    proc isWrite {f} { 
        return [expr ($f > 15) && ($f < 24)]
    }
    ##
    # camacutils::isCtl
    #   Determins if a function code is a control operation.
    #   (non-data-transfer).
    #
    # @param f - the function code.
    # @return boolean - true if the function is neither a write nor a read.
    #
    proc isCtl   {f} {
        return [expr !([isWrite $f] || [isRead $f])]
    }
       
}