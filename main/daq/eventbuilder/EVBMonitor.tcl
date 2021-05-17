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
# @file   EVBMonitor.tcl
# @brief  Program to provide a REST UI to the event builder.
# @author Ron Fox <fox@nscl.msu.edu>
#
if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}

package require EVBRESTClient
package require EVBRestUI
package require EVBRestControllers


##
# The UI Is based on the MVC architecture implemented in the packages required
# above.  Our main task is to:
#   *  Create/Layout the views above.
#   *  Create a client model.
#   *  Build controllers that link the client models with the views.
#   *  Schedule updates for all the controllers.
#

##
# usage
#   Output an error message and program usage and exit.
#
# @param msg - error message that will precede the usage message.
# @note all output is to stderr.
#
proc usgae {msg} {
    puts stderr $msg
    puts stderr "Usage: "
    puts stderr "    \$DAQBIN/EVBMonitor host user ?service?"
}
