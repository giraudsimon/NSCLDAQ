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
# @file emptyModel.tcl
# @brief Display model for an empty line.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide emptyModel 1.0
package require snit

##
# @class emptyModel
#
# Provides a scaler display page line model that has nothing in it.
# All methods provice emtpy strings except alarmState which provides "ok"
snit::type emptyModel {
    method getNumeratorName {} {return ""}
    method getDenominatorName {} {return ""}
    method getNumeratorRate {} {return ""}
    method getDenominatorRate {} {return ""}
    method getNumeratorTotal {} {return ""}
    method getDenominatorTotal {} {return ""}
    method getRateRatio {} {return ""}
    method getTotalRatio {} {return ""}
    method alarmState {} {return "ok"}
}