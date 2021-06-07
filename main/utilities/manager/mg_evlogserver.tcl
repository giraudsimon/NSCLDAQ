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
package require eventloggers


Url_PrefixInstall /Loggers [list loggerHandler]

##
# Note that event loggers don't have names.  However, each logger has a
# unique destination.  We therefore identify a logger by its destination path.
#  E.g. an event logger that logs data to /user/fox/stagearea will be
#  identified by /user/fox/stagearea when referring to it.
#

#------------------------------------------------------------------------------
#  Utilties:

##
# _findLogger
#   Given a logger destination returns the id of the logger or -1 if there
#   is no match.
#
# @param db - database command.
# @param dest - logger destination to find.
#
proc _findLogger {db dest} {
    set loggers [::eventlog::listLoggers $db]
    set result -1
    
    foreach logger $loggers {
        if {$dest eq [dict get $logger destination]} {
            set result [dict get $logger id]
            break
        }
    }
    
    return $result
}
##
# _requireLoggerPost
#   Require a logger and post operation.
#
# @param sock - socket on which the user is requesting.
# @return id of the specified logger
# @retval -1 if there's a problem.  In that case the error message has
#            already been sent to the client.
#
proc _requireLoggerPost  {sock} {
    if {[GetRequestType $sock] ne  "POST"} {
        ErrorReturn $sock "logger 'enable' operations require  a POST method"
        return -1
    }
    set info [GetPostedData $sock]
    if {![dict exists $info logger]} {
        ErrorReturn $sock "logger 'enable' operations require a 'logger' parameter"
        return -1
    }
    if {![dict exists $info user]} {
        ErrorReturn $sock "logger 'enable operations require a 'user' parameter"
        return -1
    }
    # Get the id of the logger to enable:
    
    set dest [dict get $info logger]
    set id [_findLogger db $dest]
    if {$id < 0} {
        ErrorReturn $sock "There is no logger with a dest. '$dest'"
    }
    return $id
    
}

##
# _enableLogger
#   Turns on a specific logger.  Note that turning on a logger that's
#   already enabled is a no-op.
#
#   - method must be POST.
#   - Parameters must have:  logger - destination of the logger.
#   - Parameters must have:  user   - Who is requesting the action.
# @param sock - socket information
proc _enableLogger {sock} {
    set id [_requireLoggerPost $sock]
    if {$id > 0} {
        ::eventloggers::enableLogger db $id
        
        
        Httpd_ReturnData $sock application/json [json::write object   \
            status [json::write string OK]                            \
            message [json::write string ""]                           \
        ]
    }
 }
##
# _disableLogger
#    Disables a logger.
#
# @param sock - the socket on which data will be returned to the caller.
#
# @note the request:
#    - Must use the POST method.
#    - Must have a logger post field that contains the destination of the logger.
#    - Must have a user field that contains the name of the user initiaiting
#      the operation.
#
proc _disableLogger {sock} {
    set id [_requireLoggerPost $sock]
    if {$id > 0} {
        ::eventloggers::disableLogger db $id
        
        Httpd_ReturnData $sock application/json [json::write object   \
            status [json::write string OK]                            \
            message [json::write string ""]                           \
        ]
    }
}

#-----------------------------------------------------------------------------
#  Handler for  /Loggers domain.  We have the following

##
# loggerHandler
#  subdomains:
#    *   enable   - enable a logger by destination.
#    *   disable  - disable a logger by destination.
#    *   list     - List the known loggers.
#    *   record   - Set state of recording flag.
#    *   recordingstate - get state of recording flag.
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
        _enableLogger $sock
    } elseif {$suffix eq "/disable"} {
        _disableLogger $sock
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
    
