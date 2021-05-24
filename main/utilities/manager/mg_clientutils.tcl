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
# @file mg_clientutils.tcl
# @brief  Utilities used by manager clients.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide clientutils 1.0
package require portAllocator
package require http
package require json

namespace eval clientutils {
    variable SERVICE DAQManager
    variable client $::tcl_platform(user)
}

##
# clientutils::getServerPort
#    Figures out the server port.
#
# @param host  - Host on which the server is believed to be running.
# @param user  - username of the person that started the server.
# @param service - optional service name - defaults to the DAQManager
#                service if omitted.
# @return integer - port number or error if there's no server on that host.

proc ::clientutils::getServerPort {host user {service {}}} {
    if {$service eq ""} {
        set service $::clientutils::SERVICE
    }
    set manager [portAllocator create %AUTO% -hostname $host]
    set port [$manager findServer $service $user]
    $manager destroy
    
    # findServer returns "" if there's no matching server.
    
    if {$port eq ""} {
        error "There is no server for the '$service' service run by $user on $host "
    }
    return $port;
}

##
# clientutils::checkResult
#
#   Given an http token
#   - wait on completion.
#   - Ensure the numeric status code is 200 (OK).
#   - parse the data as JSON.
#   - Ensure the status field of the JSON is 'OK'
#
# @param token  - http token for the request.
# @return dict - on success, the JSON dict.
# 
# @note we don't perform the request as we don't know a-priori the
#       request method or any query (POST) data:
# @note we clean up the token.
#
proc clientutils::checkResult {token} {
    http::wait $token
    
    #  Check for protocol level errors:
        
    
    
    set status [http::ncode $token]
    if {$status != 200}  {
        set message "[http::error $token] [http::data $token]"
        error "Unexpected httpd status: $status : $message"
    }
    
    # Get and decode the JSON:

    set parseOk [catch {
        set json [http::data $token]
        http::cleanup $token
        set jsondict [json::json2dict $json]
    } msg] 
    if {$parseOk} {
        error "JSON Parse of: \n $json \nFailed:  $msg $::errorInfo"
	}	 
    if {[dict get $jsondict status] ne "OK"} {
        error "Manager refused state request: [dict get $jsondict message]"
    }
    # All good:

    return $jsondict    
}
