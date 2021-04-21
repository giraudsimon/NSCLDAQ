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
# @file ReadoutRESTClient.tcl
# @brief Provide a packaged REST client for Readout REST services.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names ::env DAQTCLLIBS] eq "DAQTCLLIBS"} {
    lappend auto_path $::env(DAQTCLLIBS)
}
package require snit
package require clientutils
package require http


namespace eval ReadoutRESTClient {
    
    #Domains the REST server servers up:
    
    variable control /control
    variable status  /status
    variable parameters /parameters
}

##
# @class ReadoutRESTClient
#     Instances of these provide the client interface to a Readout running
#     the REST server plugin
#
# OPTIONS
#   -host - Host the server is running in - default localhost
#   -user - The user the server is running under - default running user.
#   -service - the service (defaults to ReadoutREST)
# METHODS:
#   begin  - Begin a run.
#   end    - End a run.
#   init   - Initialize system.
#   shutdown - stop the readout.
#   getState - Return the state of the run.
#   getTitle - Return the title.
#   getRunNumber - Get the run number.
#   getStatistics - Get the readout statistics.
#   setTitle  - Set the run title.
#   setRunNumber -  Set the run number.
#
snit::type ReadoutRESTClient {
    option -host -default localhost
    option -user -default $::tcl_platform(user)
    option -service -default ReadoutREST
    
    ##
    #  The constructor just processes the arguments as options.
    #  each method figures out the URI makes the request and processes
    #  the results.  No persistent state is held in the client.
    #
    # @param args - option value ...
    #
    constructor {args} {
        $self configurelist $args
    }
    #--------------------------------------------------------------------
    # Private utilities.
    
    ##
    # _createURL
    #   Generates a URL for a service.
    #
    # @param service    - The service rquested e.g. /control.
    # @param suffix     - optional suffix e.g. /status may require /statistics.
    # @return string    - the URL appropriate to the request.
    #
    method _createURL {service {suffix {}}} {
        set port [::clientutils::getServerPort \
            $options(-host) $options(-user) $options(-service)
        ]
        return http://$options(-host):${port}${service}${suffix}
    }
    ##
    # _transition
    #   Attempt a transition.
    #  @param transitonName
    #  @result string newState on success.
    #
    method _transition {transitionName} {
        set url [$self _createURL /control]
        set params [::http::formatQuery operation $transitionName]
        set token [::http::geturl $url -query $params]
        
        set resultDict [::clientutils::checkResult $token]
        return [dict get $resultDict newstate]
    }
    #--------------------------------------------------------------------
    #  Public methods.
    #
    ##
    # begin
    #   Attempt to begin a run
    # @return new state
    #
    method begin {} {
        return [$self _transition BEGIN]
    }
    ##
    # end
    #   Attempt to end an active run.
    #
    # @return new state.
    #
    method end {} {
        return [$self _transition END]
    }
    ##
    # init
    #   Request an init operation
    #
    # @return new state.
    #
    method init {} {
        return [$self _transition INIT]
    
    }
    ##
    # shutdown
    #   Ask for a shutdown.
    #
    method shutdown {} {
        return [$self _transition SHUTDOWN]
    }
        
    
    
    
}