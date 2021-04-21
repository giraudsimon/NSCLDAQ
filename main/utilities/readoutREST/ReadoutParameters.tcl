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
# @file ReadoutParameters.tcl
# @brief Allow run number and title to be set.
# @author Ron Fox <fox@nscl.msu.edu>
#
if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}
package require restutils

Url_PrefixInstall /setparam SetParameter

##
# SetParameter
#    Sets a readout parameter.  Currently the parameters are
#    - title - the run title.
#    - run   - The run number.
#
# @param socket - socket on which the request/reply is made.
# @param suffix - The suffixe of the URL (must be empty).
#
#  @note The state must be "idle"
#  @note The method must be "POST"
#  @note The post parameters:  "name" and "value" are required and
#        carry the parameter name and value.
#
proc SetParameter {socket suffix} {
    if {[GetRequestType $socket] eq "POST"}  {
        if {[runstate] ne "idle"} {
            ErrorReturn $socket "Run parameters can only be set in the idle state"
        } else {
            set params [GetPostedData $socket]
            if {(![dict exists $params name]) || (![dict exists $params value])} {
                ErrorReturn $socket "/settparam requires both 'name' and 'value' parameters"
            } else {
                set name [dict get $params name]
                set value [dict get $params value]
                if {$name eq "title"} {
                    set ::title $value;      # All types allowed.
                    Httpd_ReturnData $socket application/json [json::write object \
                        status [json::write string OK]                          \
                        message [json::write string ""]                         \
                    ]
                } elseif {$name eq "run"} {
                    # Must be an integer:
                    
                    if {[string is integer -strict $value]} {
                        set ::run $value
                        Httpd_ReturnData $socket application/json [json::write object  \
                            status [json::write string OK]                       \
                            message [json::write string ""]                      \
                        ]
                    } else {
                        ErrorReturn $socket "Run number '$value' must be an integer"
                    }
                } else {
                    ErrorReturn $socket "/setparam does not have a $name parameter"
                }
            }
        }
    } else {
        ErrorReturn $socket "The /setparam subdomain can only be accessed via POST"
    }
}
    


