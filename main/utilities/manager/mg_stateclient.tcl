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
# @file  mg_stateclient.tcl
# @brief API For state client.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide stateclient 1.0

lappend auto_path $::env(DAQTCLLIBS)
package require portAllocator
package require http
package require snit
package require json

namespace eval stateclient {
    variable SERVICE DAQManager
    variable client $::tcl_platform(user)
    variable base   "State";          # URI base.
    variable state  "status";         # subdomain with status.
    variable next   "allowed";        # subdomain with allowed next states.
    variable transition "transition"; # Transition request subdomain
}
##
# _getServerPort
#    Figures out the server port.
#
# @param host  - Host on which the server is believed to be running.
# @param user  - username of the person that started the server.
# @return integer - port number or error if there's no server on that host.

proc _getServerPort {host user} {
    set manager [portAllocator create %AUTO% -hostname $host]
    set port [$manager findServer $stateclient::SERVICE $user]
    $manager destroy
    
    # findServer returns "" if there's no matching server.
    
    if {$port eq ""} {
        error "There is no manager server run by $user on $host"
    }
    return $port;
}

##
# @class StateClient
#
#    Provides a client to the manager that hides the nitty gritty of
#    making requests and decoding the results of those requests.
# OPTIONS
#     -host    - Host running the manager.
#     -user    - User running the manager.
# METHODS:
#   currentState  - Returns the current state.
#   nextStates    - Returns the legal next states.
#   transition    - Perform a state transition.
#
# @note Each requests resolves the port the manager is running on.
#       this makes objects resilient with respect to manager stops/restarts.
#
#
snit::type StateClient {
    option -host
    option -user
    
    constructor args {
        $self configurelist $args
    }
    #------------------------------------------------------------------
    #  private methods.
    
    ##
    #  _checkResult
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
    method _checkResult {token} {
        http::wait $token
        
        #  Check for protocol level errors:
            
        
        
        set status [http::ncode $token]
        if {$status != 200}  {
            set message [http::error $token]
            error "Unexpected httpd status: $status : $message"
        }
        
        # Get and decode the JSON:
        
        set json [http::data $token]
        http::cleanup $token
        set jsondict [json::json2dict $json]
        
        if {[dict get $jsondict status] ne "OK"} {
            error "Manager refused state request: [dict get $jsondict message]"
        }
        # All good:

        return $jsondict    
    }
    
    
    
    
    #-------------------------------------------------------------------
    # Public methods.
    #
    
    ##
    # currentState
    #   Figures out the current state of the system:
    #
    # @return string -state name.
    #
    method currentState {} {
        
        set port [_getServerPort $options(-host) $options(-user)]
        
        # Construct the full URL:
        
        set baseuri http://$options(-host):$port
        set uri "$baseuri/$stateclient::base/$stateclient::state"

        
        # Perform the request synchronously:
        
        set token [http::geturl $uri]
        set jsondict [$self _checkResult $token]
        
        return [dict get $jsondict state]
        
    }
    ##
    # nextStates
    #    Figures out the allowed next states:
    #
    # @return list - elements of the list are allowed next states.
    #
    method nextStates {} {
        set port [_getServerPort $options(-host) $options(-user)]
        
        # Construct the URI:
        set baseuri http://$options(-host):$port
        set uri "$baseuri/$stateclient::base/$stateclient::next"
        
        set token [http::geturl $uri]
        set jsondict [$self _checkResult $token]
        
        return [dict get $jsondict states]
    }
    ##
    # transition
    #   Request a state transition.
    # @param nextState - requested next state.
    # @return actual next state (it's possible failures will result
    #               in a shutdown).
    #
    method transition {nextState} {
        set port [_getServerPort $options(-host) $options(-user)]
        
        set baseuri http://$options(-host):$port
        set uri "$baseuri/$stateclient::base/$stateclient::transition"
        
        # We need to pass query data in a post so:
        
        set postData [http::formatQuery  \
            user $stateclient::client state $nextState
        ]
        set token [http::geturl $uri -query $postData]; #-query forces POST
        
        set jsondict [$self _checkResult $token]
        return [dict get $jsondict state]
        
    }
    
}