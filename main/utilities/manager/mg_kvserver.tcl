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
# @file  mg_kvserver.tcl
# @brief Serve out key value store.
# @author Ron Fox <fox@nscl.msu.edu>
#
lappend auto_path $::env(DAQTCLLIBS)
package require restutils
package require sqlite3
package require json::write
package require kvstore


Url_PrefixInstall /KVStore [list kvHandler]

#-------------------------------------------------------------------------------
# Local utilities.


##
# _varsToObjectArray
#    Given the dict from kvstore::listAll - returns an array of name value objects.
#
# @param info - dict  of variables.
# @return string - JSON encoded array of name value objects.
#
proc _varsToObjectArray {info} {
    set resultList [list]
    dict for {name value} $info {
        lappend resultList [json::write object                        \
            name [json::write string $name]                           \
            value [json::write string $value]                         \
        ]
    }
    return [json::write array {*}$resultList]
}

#------------------------------------------------------------------------------
# Domain handler

##
# kvHandler - handle domain requests for /KVStore
#            *  value (GET) returns the value of a specific variable.
#            *  listnames (GET) returns the name of all variables.
#            *  list (GET) returns the name/values of all variables.
#            *  set (POST) Sets a new value for name (from user).
#
# @param sock   - socket object on which the request was created.
# @param suffix - Remainder of the URI.
#
proc kvHandler {sock suffix} {
    if {$suffix eq "/value"} {
        set queryData [GetPostedData $sock]
        
        if {[dict exists $queryData name]} {
            set name [dict get $queryData name]
        
            if {[kvstore::_exists db $name]} {
                set value [::kvstore::get db $name]
        
                Httpd_ReturnData $sock application/json  [json::write object                    \
                    status [json::write string OK]                           \
                    message [json::write string ""]                          \
                    name   [json::write string $name]                        \
                    value  [json::write string $value]                       \
                ]
            } else {
        
                ErrorReturn $sock "No such variable $name"
            }
        } else {
            ErrorReturn $sock "/value requires a 'name' query parameter"
        }
    } elseif {$suffix eq "/listnames"} {
        
        set names [MakeStringList [kvstore::listKeys db]]
        
        Httpd_ReturnData $sock application/json [json::write object                          \
            status [json::write string OK]                                \
            message [json::write string ""]                               \
            names   [json::write array {*}$names]                         \
        ]
    } elseif {$suffix eq "/list"} {
        
        set info [::kvstore::listAll db]
        
        set result [_varsToObjectArray $info]
        Httpd_ReturnData $sock application/json [json::write object                         \
            status [json::write string OK]                               \
            message [json::write string ""]                              \
            variables $result                                            \
        ]
    } elseif {$suffix eq "/set"} {
        if {[GetRequestType $sock] eq "POST"} {
            #  Need user, name value post data:
            set postData [GetPostedData $sock]
            if {(![dict exists $postData user]) ||
                (![dict exists $postData name]) ||
                (![dict exists $postData value])} {
                    ErrorReturn $sock "/set requires 'user', 'name' and 'value' POST data"
                }
            set name [dict get $postData name]
            set value [dict get $postData value]
        
            if {![kvstore::_exists db $name]} {
                ErrorReturn $sock "/set - no such variable $name"
            } else {
                kvstore::modify db $name $value
                Httpd_ReturnData $sock application/json  [json::write object                  \
                    status [json::write string OK]                          \
                    message [json::write string ""]                         \
                    name    [json::write string $name]                      \
                    value  [json::write string [kvstore::get db $name]]     \
                ]
            }
    
        } else {
            ErrorReturn $sock "/set requires a POST method"
        }
    } else {
        ErrorReturn $sock "$suffix subcommand not implemented"
        
    }
}
    


