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
# @file   mg_pgmstatusclient.tcl
# @brief API for program status client.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide programstatusclient 1.0

if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}
package require http
package require snit
package require clientutils


namespace eval programstatusclient {
    variable base "/Programs"
    variable status "/status"
}

##
# @class ProgramClient
#
#   Provides a client to the manager server domain that hides the
#   nitty gritty details of making requests and decoding the results of
#   those requests.
#
#  OPTIONS:
#     -host   - host on which the server is running
#     -user   - user that started the server.
# METHODS
#   status   - returns status of program and containers.
snit::type ProgramClient {
    option -host
    option -user
    
    constructor {args} {
        $self configurelist $args
    }
    #--------------------------------------------------------------------
    # Public methods.
    
    ##
    # status
    #   @return dict  The dict has the following keys:
    #   *  containers  - contains a list of dicts describing the
    #                    containers.
    #   *  programs    - contains a list of dicts descsribing the
    #                    programs.
    #
    #  Each container dict has the following keys (per the design docs with
    #  the JSON translated to  a dict):
    #     *   name   - Name of the container definition.
    #     *   image  - Container image file path.
    #     *   bindings - list of bindings specifications.#
    #                   each binding is given in a format suitable for
    #                   use in the --bind option of singularity subcommands.
    #     *   activations - list of hostss in which the container has
    #                  been activated.
    #
    #  Each program dict hast he following keys (per the design docs with
    #  JSON translated to a dict):
    #     *   name   - name of the program.
    #     *   path   - path to the program image file.
    #     *   type   - program type (textual).
    #     *   host   - host in which the program runs.
    #     *   container - if nonempty string, the container in which the
    #                  program should be run.
    #     *   active - boolean that is true if the program is currently active.
    #
    method status {} {
        # Construct the full URL:
        set port [::clientutils::getServerPort $options(-host) $options(-user)]
        set baseurl http://$options(-host):$port
        set uri "$baseurl/$::programstatusclient::base/$::programstatusclient::status"
        
        # Perform the request synchronously:
        
        set token [http::geturl $uri]
        set jsondict [::clientutils::checkResult $token]

        return $jsondict        
    }

    
}