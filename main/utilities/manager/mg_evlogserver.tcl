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
# _isPost
#    Require this is a post.  POST operations also must have the user.
# @param sock  - socket on which we're talking with the client.
# @return bool - true if successful.
#
proc _isPost {sock} {
    if {[GetRequestType $sock] ne  "POST"} {
        ErrorReturn $sock "This operation require  a POST method"
        return 0
    }
    set info [GetPostedData $sock]
    if {![dict exists $info user]} {
        ErrorReturn $sock "logger POST operations require a 'user' parameter"
        return 0
    }
    return 1
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
    if {![_isPost $sock]} {
        return -1
    }
    
    set info [GetPostedData $sock]
    if {![dict exists $info logger]} {
        ErrorReturn $sock "logger 'enable' operations require a 'logger' parameter"
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
        ::eventlog::enable db $id
        
        
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
        ::eventlog::disable db $id
        
        Httpd_ReturnData $sock application/json [json::write object   \
            status [json::write string OK]                            \
            message [json::write string ""]                           \
        ]
    }
}
##
# _makeInfoObject
#   Given a definition of a logger from e.g. evnlog::listLoggers
#   Returns a JSON encoding of that dict.
#
# @param l   - the logger we're encoding.
# @return JSON encoded object representing l
#
proc _makeInfoObject {l} {
    json::write object                                     \
        id        [dict get $l id]                         \
        daqroot [json::write string [dict get $l daqroot]] \
        ring    [json::write string [dict get $l ring]]    \
        host    [json::write string [dict get $l host]]    \
        partial [dict get $l partial]                      \
        destination [json::write string [dict get $l destination]] \
        critical [dict get $l critical]                    \
        enabled  [dict get $l enabled]                     \
        container [json::write string [dict get $l container]] 
        
    
}
##
# _listLoggers
#    List the loggers that have been defined in the database.
#    This insulates the caller from needing to know, or have access to the
#    database file the logger is running from. Any body returning method can
#    be used for service.  The result, in addition to the normal
#    status and message bits and pieces as a loggers attribute that is a
#    JSON array of objects that contain the logger definitions.  Each
#    logger defintion is a JSON object with attributes named after the dict
#    keys in the logger definition with associated values.
#
# @param sock - socket in communication with the client.
#
proc _listLoggers {sock} {
    set info [eventlog::listLoggers db]
    set result [list]
    foreach item $info {
        lappend result [_makeInfoObject $item]
    }
    Httpd_ReturnData $sock application/json [json::write object   \
        status [json::write string OK]                            \
        message [json::write string ""]                           \
        loggers [json::write array {*}$result]
    ]
}
##
#   _setRecording
#      Set the state of recording on or off.
#       - Must be a POST.
#       - Must have user parameter
#       - Must have 'state' parameter that is an integer 0 or 1.
#       It's perfectly legal to set the state to what it already is.
#
# @param sock   - socket communicating with the client.
#
proc _setRecording {sock} {
    if {![_isPost $sock]} {
         return
    }
    set info [GetPostedData $sock]
    if {![dict exists $info state]} {
         ErrorReturn $sock "/record operation must have a 'state' parameter"
         return
    }
    set state [dict get $info state]
    if {![string is integer -strict $state]} {
         ErrorReturn $sock "/record 'state' parameter must be an integer"
         return
    }
    if {$state == 1} {
        ::eventlog::enableRecording db 
    } elseif {$state == 0} {
        ::eventlog::disableRecording db
    } else {
         ErrorReturn $sock "/record 'state' parameter must be 0|1 not $state"
         return
    }
    Httpd_ReturnData $sock application/json [json::write object         \
        status [json::write string OK] message [json::write string ""]  \
    ]
}
##
# _isRecording
#    Returns to the client JSON with the usual status and message
#    attributes along with state - a boolean that represents the
#    recording state.
#
# @param sock   - Socket used to communicate with the client.
#
proc _isRecording {sock} {
    set state [eventlog::isRecording db]
    Httpd_ReturnData $sock application/json [json::write object      \
        status [json::write string OK] message [json::write string ""] \
        state $state                                                  \
    ]
}
##
# _startAllLoggers
#    Requests that the appropriate set of loggers start.
#    Note that this may do nothing (e.g. if recording is disabled).
#
# @param sock - socket connected to the client.
#
proc _startAllLoggers {sock} {
    #  Must be a POST method:
    
    if {![_isPost $sock]} {
         return;
    }
    
    #  Request all the loggers start:
    
    set status [catch {::eventlog::start db} msg]
    if {$status} {
        ErrorReturn $sock "Failed to start event loggers: $msg : $::errorInfo"
        
    } else {
        Httpd_ReturnData $sock application/json [json::write object        \
            status [json::write string OK] message [json::write string ""] \
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
#    *   isrecording - get state of recording flag.
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
        _listLoggers $sock
    } elseif {$suffix eq "/record"} {
        _setRecording $sock
    } elseif {$suffix eq "/isrecording"} {
        _isRecording $sock
    } elseif {$suffix eq "/start"} {
        _startAllLoggers $sock
    } elseif {$suffix eq "/stop"} {
        ErrorReturn $sock "$suffix not implemented"
    } elseif {$suffix eq "/status"} {
        ErrorReturn $sock "$suffix not implemented"
    } else {
        ErrorReturn $sock "$suffix not implemented"
    }
}
#------------------------------------------------------------------------------
#  Stop containers that might be associated with event loggers:

set opened 0
if {[info commands db] eq ""} {
    sqlite3 db $::env(DAQ_EXPCONFIG)
    set opened 1
}

set loggers [eventlog::listLoggers db]

# Sort the containers by host:

array set containersByHost [list]
foreach logger $loggers {
   if {[dict get $logger container] ne ""} {
       lappend containersByHost([dict get $logger host])   
   }
}

# Before killing the containers in the host,
# Uniquify the lists so we don't try to kill the same container twice.

foreach host [array names containersByHost] {
    array set containers [list]
    foreach c $containersByHost($host) {
        set containers($c) ""
    }
    set containerList [array names containers]
    foreach c $containerList {
       container::deactivate $host $c
    }
}

if {$opened} {
    db close
}
