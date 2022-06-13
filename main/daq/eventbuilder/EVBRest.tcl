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
# @file   EVBRest.tcl
# @brief Start Event builder with REST interface
# @author Ron Fox <fox@nscl.msu.edu>
#

lappend auto_path $env(DAQTCLLIBS)
package require EventBuilder
package require EVBStatistics
package require EVBREST

EVB::Start fox
EVBStatistics::Start

close stdin

vwait forever;              # Event loop.
