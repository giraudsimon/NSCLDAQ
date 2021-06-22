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
# @file mg_loggerRESTClient.tcl
# @brief Provide a REST client to the event logger REST interface.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide loggerrestclient 1.0

if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}
package require snit
package require clientutils
package require http


namespace eval LoggerClient {
    variable base /Loggers
    variable enable  /enable
    variable disable /disable
    variable list    /list
    variable record  /record
    variable isRecording /isrecording
    variable startLoggers /start
}


##
# @class LoggerRestClient
#    Provides a REST client for the eventlogger REST interface
#    of the manager.
#
# OPTIONS:
#    -host   - host the manager runs in
#    -user   - user the manager runs in.
#
# METHODS:
#    enablelogger  - Enable a logger by destination.
#    disableLogger  - Disable a logger by dest.
#    listLoggers     - List the loggers.
#    record          - Set recording state.
#    isRecording     - Get recording state.
#    start           - Start all loggers that should start.
#
snit::type LoggerRestClient {
    option -host
    option -user
    
    constructor args {
        $self configurelist $args
    }
    
    #------------------------------------------------------------------------
    # Private UTILS:
    #
    
    ##
    # _makeUrl
    #    Create the full url
    #
    #  @param subdomain - the subdomain selector.
    #  @return string   - full URI
    #
    method _makeUrl {subdomain} {
        set port [clientutils::getServerPort $options(-host) $options(-user)]
        set result http://$options(-host):$port/$::LoggerClient::base/$subdomain
        return $result
    }
    ##
    # _get
    #   Perform a get operation.
    #
    # @param url  - the URL of the get.
    #
    method _get {url} {
        set token [http::geturl $url]
        return [clientutils::checkResult $token]
    }
    ##
    # _post
    #   Do a POST.  Note each put contains a user. field.
    #
    # @param url - url of the operation.
    # @param args - name/value pairs for POST params.
    #
    method _post {url args} {
        set q [http::formatQuery user $::tcl_platform(user) {*}$args]
        set token [http::geturl $url -query $q]
        return [clientutils::checkResult $token]
    }
    
    #-------------------------------------------------------------------
    # Public methods.
    
    ##
    # enableLogger
    #    Enable a logger given its destination.
    #
    # @param dest - logger destination.
    #
    method enableLogger {dest} {
        set url [$self _makeUrl $::LoggerClient::enable]
        return [$self _post $url logger $dest]
        
    }
    ##
    # disableLogger
    #   Disable a logger given its destination.
    # @param dest - logger destination
    #
    method disableLogger {dest} {
        set url [$self _makeUrl $::LoggerClient::disable]
        return [$self _post $url logger $dest]
    }
    ##
    # listLoggers
    #
    method listLoggers {} {
        set url [$self _makeUrl $::LoggerClient::list]
        return [$self _get $url]
    }
    ##
    # record
    #    set recording state
    #
    # @param state - boolean on/off ; new state value.
    #
    method record {state} {
        set uri [$self _makeUrl $::LoggerClient::record]
        return [$self _post $uri state $state]
    }
    ##
    # isRecording
    #   @return the recording sttae.
    #
    method isRecording {} {
        set uri [$self _makeUrl $::LoggerClient::isRecording]
        set json [$self _get $uri]
        return [dict get $json state]
    }
    ##
    # start
    #    Starts all the loggers:
    # @return - the json or an error if this fails.
    method start {} {
        set uri [$self _makeUrl $::LoggerClient::startLoggers]
        return [$self _post $uri]
    }
    
}
    
