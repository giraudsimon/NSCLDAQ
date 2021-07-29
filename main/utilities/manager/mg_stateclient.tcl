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

if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}
package require clientutils
package require snit
package require http

namespace eval stateclient {
    
    variable base   "State";          # URI base.
    variable state  "status";         # subdomain with status.
    variable next   "allowed";        # subdomain with allowed next states.
    variable transition "transition"; # Transition request subdomain
    variable forceExit "shutdown";    # Kill off the menager.
}
##
# _getServerPort
#    Figures out the server port.
#
# @param host  - Host on which the server is believed to be running.
# @param user  - username of the person that started the server.
# @return integer - port number or error if there's no server on that host.

proc _getServerPort {host user} {
    return [clientutils::getServerPort $host $user]
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
#   kill          - Shutdown the system and kill the server.
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
        clientutils::checkResult $token
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
            user $clientutils::client state $nextState
        ]
        set token [http::geturl $uri -query $postData]; #-query forces POST
        
        set jsondict [$self _checkResult $token]
        return [dict get $jsondict state]
        
    }
    ##
    # kill
    #    If necessary SHUTDOWN and then schedule the manager to exit.
    #
    method kill {} {
        set port [_getServerPort $options(-host) $options(-user)]
        set baseuri http://$options(-host):$port
        set uri $baseuri/$stateclient::base/$stateclient::forceExit
        
        set postData [http::formatQuery       \
            user $clientutils::client         \
        ]
        set token [http::geturl $uri -query $postData]
        
        $self _checkResult $token
    }
    
}