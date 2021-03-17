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
# @file mg_kvedit.tcl
# @brief  Key value editor.
# @author Ron Fox <fox@nscl.msu.edu>
#
set libdir [file normalize [file join [file dirname [info script]] .. TclLibs]]
lappend auto_path $libdir

package require kvstore
package require sqlite3
package require Tk
package require snit