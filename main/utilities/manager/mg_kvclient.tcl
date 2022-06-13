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
# @file   mg_kvclient.tcl
# @brief  Client for key value server.
# @author Ron Fox <fox@nscl.msu.edu>
#


package provide kvclient 1.0

if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}
package require snit
package require clientutils
package require http

namespace eval KvClient {
    variable base "/KVStore"
    variable get  "/value"
    variable listNames "/listnames"
    variable list "/list"
    variable set  "/set"
}

##
# @class KvClient
#    Provides a client to the key value server which allows access to the
#    key value store via the manager server.
#
# OPTIONS
#    -host   - host in which the server is running.
#    -user   - User that's running the server.
#
# METHODS:
#   getValue    - Get the value of a variable.
#   setValue    - Set the value of a variable.
#   listNames   - List the known variable names.
#   list        - List the names and values.
#
snit::type KvClient {
    option -host -default ""
    option -user -default ""
    
    constructor {args} {
        $self configurelist $args
    }
    #---------------------------------------------------------------------------
    # Private utils
    
    ##
    # baseUrl
    #   compute the base url
    # @param port - port number on which the server is listening.
    # @return string http://hostname:portnumber/base.
    #
    method baseUrl {port} {
        return http://$options(-host):$port/$::KvClient::base
    }
    
    #---------------------------------------------------------------------------
    # public methods.
    #
    
    ##
    # getValue
    #   Return the value of a variable.
    # @param name - name of the variable.
    #
    method getValue {name} {
        set port [clientutils::getServerPort $options(-host) $options(-user)]
        set url [$self baseUrl $port]$KvClient::get?[http::formatQuery name $name]
        
        set token [http::geturl $url]
        set result [clientutils::checkResult $token]
        return [lindex [dict get $result value] 0]
    }
    ##
    # setValue
    #    Set variable to a new value.
    #
    # @param name - variable name.
    # @param value - New value of the variable.
    # @note the variable must already exist.
    #
    method setValue {name value} {
        set user $::tcl_platform(user)
        set port [clientutils::getServerPort $options(-host) $options(-user)]
        set baseurl [$self baseUrl $port]
        
        set url $baseurl$KvClient::set
        set postData [http::formatQuery user $user name $name value $value]
        
        set token [http::geturl $url -query $postData]
        set result [clientutils::checkResult $token]
    }
    ##
    # listNames
    #   Lists the names of the variables that have been defined.
    #
    # @return list of text - variable names.
    #
    method listNames {} {
        set port [clientutils::getServerPort $options(-host) $options(-user)]
        set base [$self baseUrl $port]
        set url $base$::KvClient::listNames
        
        set token [http::geturl $url]
        set info [clientutils::checkResult $token]
        return [dict get $info names]
    }
    ##
    # list
    #  @return dict of name/values.
    #
    method list {} {
        set port [clientutils::getServerPort $options(-host) $options(-user)]
        set base [$self  baseUrl $port]
        set url $base$::KvClient::list
        
        set token [::http::geturl $url]
        set info [clientutils::checkResult $token]
        
        set result [dict create]
        foreach item [dict get $info variables] {
            dict set result [dict get $item name] [dict get $item value]    
        }
        return $result
    }
}
    
