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

package require cmdline

##
# @file ddasReadout.tcl
# @brief Driver for DDASReadout/Sorter.
# @author Ron Fox <fox@nscl.msu.edu>
#


# Program usage string:

set usage "ddasReadout option...\nDriver for the split DDASReadout/ddasSort programs\nOptions:"
set options {
    {port.arg -1 "Enable DDASReadout TclServer functionality on specified port"}
    {readoutring.arg "" "Ring into which DDASReadout puts data"}
    {sourceid.arg 0     "Source ID with which to tag the data"}
    {init-script.arg "" "DDASReadout initialization script run in main interpreter"}
    {log.arg ""         "DDASReadout log file"}
    {debug.arg 0      "DDASReadout debug level - controls log file contents"}
    {readouthost.arg ""   "Host in which to run the DDASReadout program"}
    {sortring.arg    ""   "Ringbuffer into which the sorter puts its data" }
    {sorthost.arg   ""   "Host in which the sorter runs"}
}

proc usage {} {
    ::cmdline::usage $::options $::usage
}

