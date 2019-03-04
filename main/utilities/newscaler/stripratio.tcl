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
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file stripratio.tcl
# @brief Implement stripRatio snit::type
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide stripratio 1.0
package require snit

##
# @class stripratio
#    Provides data for a strip chart data set that is a ratio of channel rates.
#    See stripParam for the in stripparam.tcl for the methods.
# OPTIONS
#   *  -numerator - numerator channel object.
#   *  -denominator - demominator scaler object.
#
snit::type stripRatio {
    
    option -numerator
    option -denominator
    
    variable lastTime -1
    
    ##
    # name
    #   Produce a name for the strip chart.
    #
    method name {} {
	set num  [$options(-numerator) cget -name]
	set den  [$options(-denominator) cget -name]
        return "${num}_over_$den"
    }
    ##
    # rate
    #   Return the ratio of the numerator rate to the denominator rate.
    #
    # @return double
    #
    method rate {} {
        set num [$options(-numerator) rate]
        set den [$options(-denominator) rate]
        if {$den > 0.0} {
        
            return [expr {$num/$den}]
        } else {
            return 0
        }
    }
    ##
    # time
    #    Returns the average time of the constituent times.  This is the
    #    best we can do (I think) for now.
    #
    # @return double
    #
    method time {} {
        set num [$options(-numerator) elapsed]
        set den [$options(-denominator) elapsed]
        
        set lastTime [$self _time]
        
        return $lastTime
    }
    ##
    # hasUpdated
    #
    # @return boolean - 1 if the time has changed since last retrieved
    #                   0 if not.
    #
    method hasUpdated {} {
        return [expr {$lastTime != [$self _time]}]

    }
    ##
    # clear
    #   ensure next hasUpdated returns true.
    #
    method clear {} {
        set lastTime -1
    }
    #-----------------------------------------------------------------------
    # Private methods:
    
    ##
    #   _time
    # Compute the current elapsed time as the average of the numerator
    # demonimantor elapsed times.
    #
    method _time {} {
        set num [$options(-numerator) elapsed]
        set den [$options(-denominator) elapsed]
        return [expr {double($num+$den)/2.0}]
    }
}