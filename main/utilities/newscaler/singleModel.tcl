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
# @file singleModel.tcl
# @brief Encapsulates a single scaler in the model interface for a display line.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide singleModel 1.0

package require snit

##
# @class singleModel
#
#  Encapsulates the data for a single scaler channel for a line that is configured
#  as a 'display_single" line.
#
# OPTIONS:
#   -channel - the command name of the channel object that is encapsulated.
#
# METHODS (provide the defined model interface).
#
# * getNumeratorName   - Get the scaler name.
# * getDenominatorName - returns ""
# * getNumeratorRate   - Return the scaler rate.
# * getDeominatorRate  - Returns ""
# * getNumeratorTotal  - Returns the total number of counts.
# * getDenominatorTotal - Returns ""
# * getRateRatio       - Returns ""
# * getTotalRatio      - Returns ""
# * alarmState         - returns the alarm state for the line.
#
snit::type singleModel {
    option -channel
    option -name
   
    ##
    # getNumeratorName
    # @return string - value of -name
    #
    method getNumeratorName {} {
        return $options(-name)
    }
    ##
    # getDenominatorName
    #
    # @return string (empty).
    #
    method getDenominatorName {}  {
        return ""
    }
    ##
    # getNumeratorRate
    #
    # @return float - rate of counts in scaler.
    #
    method getNumeratorRate {} {
        set channel $options(-channel)
        return [format %6.2f [$channel rate]]
    }
    ##
    # getDenominatorRate
    #
    #  @return string (empty)
    #
    method getDenominatorRate {} {
        return ""
    }
    ##
    # getNumeratorTotal
    #
    # @return longint - Total scaler counts.
    #
    method getNumeratorTotal {} {
        set channel $options(-channel)
        
        return [format %6d [$channel total]]
    }
    ##
    # getDenominatorTotal
    #
    # @return string ""
    #
    method getDenominatorTotal {} {
        return ""
    }
    ##
    # getRateRatio
    #
    # @return string (empty)
    #
    method getRateRatio {} {
        return ""
    }
    ##
    # getTotalRatio
    #
    # @return string ""
    #
    
    method getTotalRatio {} {
        return ""
    }
    ##
    # alarmState
    #
    # @return string - in {ok, high, low} indicating if there is an alarm
    #                  in the underlying channel and, if so, which one.
    #
    method alarmState {} {
        set channel $options(-channel)
        return [$channel alarming]
    }
}