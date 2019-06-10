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
# @file stripparam.tcl
# @brief Object that has strip parameter data.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide stripparam 1.0
package require snit
package require channel


##
# stripParam
#   Represents the data for a stripChart parameter.
#
snit::type stripParam {
    option -channel
    
    variable lastTime -1
    
    ## name
    #   Produce a name for the strip chart.
    #
    method name {} {
        return [$options(-channel) cget -name]
    }
    
    ##
    # rate
    #   Return the rate of the underlying scaler:
    #
    method rate {} {
        return [$options(-channel) rate]
    }
    ##
    # time
    #   Return the elapsed time over which the underyling scaler has accumulated:
    #
    method time {} {
        set lastTime [$options(-channel) elapsed]
        return $lastTime
    }

    ##
    # hasUpdated
    #   @return bool - 1 if the channel time has changed since the last time call, 0
    #                  if not.
    #
    method hasUpdated {} {
        return [expr {$lastTime != [$options(-channel) elapsed]}]
    }
    ##
    # clear
    #   Clear the last udpated time so the next hasUpdated will be true.
    #
    method clear {} {
        set lastTime -1
    }
}