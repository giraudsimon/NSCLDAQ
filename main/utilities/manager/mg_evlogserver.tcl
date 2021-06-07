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
# @file   mg_evlogserver.tcl
# @brief  Piece of the manager server that controls event loggers.
# @author Ron Fox <fox@nscl.msu.edu>
#

## Note that the global variable dbFile contains the database file.
lappend auto_path $::env(DAQTCLLIBS)
package require restutils
package require json::write
package require sqlite3
package require containers
package require sequence;   #We may force shutdown.
package require programs


Url_PrefixInstall /Loggers [list loggerHandler]




#-----------------------------------------------------------------------------
#  Handler for  /Loggers domain.  We have the following
#  subdomains:
#    *   enable   - enable a logger by destination.
#    *   disable  - disable a logger by destination.
#    *   list     - List the known loggers.
#    *   start    - Start all loggers.
#    *   stop     - Stop all loggers.
#    *   status   - Show the status of all loggers.
#
#
# @param sock   - Socket information used to send data back to the caller.
# @param suffix - Suffix (subdomain information) that will lead with a '/'.
#
#  See the specific executors for information about what is returned to the
#  user and requirements (e.g. parameters, POST vs. GET method
#  HOWEVER, note that all replies have the minimal 
proc loggerHandler {sock suffix} {
    if {$suffix eq "/enable"} {
        ErrorReturn $sock "$suffix not implemented"
    } elseif {$suffix eq "/disable"} {
        ErrorReturn $sock "$suffix not implemented"
    } elseif {$suffix eq "/list"} {
        ErrorReturn $sock "$suffix not implemented"
    } elseif {$suffix eq "/start"} {
        ErrorReturn $sock "$suffix not implemented"
    } elseif {$suffix eq "/stop"} {
        ErrorReturn $sock "$suffix not implemented"
    } elseif {$suffix eq "/status"} {
        ErrorReturn $sock "$suffix not implemented"
    } else {
        ErrorReturn $sock "$suffix not implemented"
    }
}
    
