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
# @file pagetest.tcl
# @brief Simple test for the page command.
# @author Ron Fox <fox@nscl.msu.edu>
#

set here [file dirname [info script]]

proc src {basename} {
    uplevel #0 source [file join $::here $basename]
}

src page.tcl
src singleModel.tcl
src ratioModel.tcl
src nameMap.tcl
src channel.tcl
src scalerconfig.tcl



# provide stub procs for the configuration:

proc getNotebook {} { return "" }
proc addPage {widget tabname} {
    pack $widget -fill both -expand 1;    # Just stack them in the toplevel for now.
}