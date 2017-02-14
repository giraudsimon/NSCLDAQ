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
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file AXLM72GenericProxy.tcl
# @brief Client side proxy methds for AXLM72Generic slow controls server class.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide AXLM72GenericProxy 1.0
package require Itcl



##
# @class AXLM72GenericProxy
#
#    This class provides proxy methods for client side access to a server's
#    AXLM72Generic class.   The AXLM72Generic class lives in the VMUSBReadout
#    Tcl server.  The client creates an instance of AXLM72Generic (or inherits
#    from it) and invokes its methods as  if they were its own.  The
#    proxy methods translate to the appropriate Get/Set interactions with the
#    server to get the requested actions performed in the server.
#
#
itcl::class AXLM72GenericProxy {
    variable socket
    variable name
    variable host  localhost
    variable port  27000
    
    constructor {devName {svrhost localhost} {svrport 27000}} {
        set name $devName
        set host $svrhost
        set port $svrport
        set socket [socket $host $port]
        fconfigure $socket -buffering line
    }
    ##
    # Methods we proxy:
    
    public method Write {offset value}
    public method AccessBus mask
    public method ReleaseBus
    public method BootFPGA
    public method SetFPGABoot value
    public method Configure filename
    public method Read Offset {}
    
    private method _transaction message
    private method _buildMessage {t what {value {}}}
    

    
}
#-------------------------------------------------------------------------------
# Implementng private methods.
#

##
# _buildMessage
#   Builds a message from it pieces
#
# @param t - Transaction type (e.g. Set or Get).
# @param what - Offset or code that says what to set/get.
# @param value - Value associated with the operations (may be empty if there
#                is no value such as in a Get).
# @return string - the message to send.
#
itcl::body AXLM72GenericProxy::_buildMessage {t what {value {}}} {
    set message [list $t $name $what]
    if {$value ne ""} {
        lappend message $value
    }
    return $message
}
##
# _transaction
#   Perform a transaction given a message.
#
# @param message -the message to send.
# @return string - the reply string.
#
itcl::body AXLM72GenericProxy::_transaction message {
    puts "Transaction sending $message"
    if {[catch {puts $socket $message} msg]} {
        puts "Error: $msg"
        error "Communication failure (send) '$msg'"
    }
    if {[catch {gets $socket} msg]} {
        puts "Receive error: $msg"
        error "Communication failure (receive) '$msg'"
    }
    puts "Got back $msg"
    if {[string range $msg 0 4] eq "ERROR"} {
        error "Error response from remote: $msg"
    }
    return $msg
}
##
# Write
#   Perform a write operation:
#
#  @param offset - Offset from XLM base at which to write.
#  @param value  - Value to write.
#
itcl::body  AXLM72GenericProxy::Write {offset value} {
    set message [_buildMessage Set $offset $value]
    return [_transaction $message]
}
##
# AccessBus
#   Access the database.  This is needed before reading from pretty much
#   anything.  Release is the inverse method.
#
# @param mask - the mask of bits that say whih busses are allocated.
#
itcl::body AXLM72GenericProxy::AccessBus mask {
    set message [_buildMessage Set Access $mask]
    return [_transaction $message]
}
##
# ReleaseBus
#    Release bus accessed by AccessBus - all internal busses are released
#    regardless of the state/order of acquisition.
#
itcl::body AXLM72GenericProxy::ReleaseBus {} {
    set message [_buildMessage Set Release 0]
    return [_transaction $message]
}
##
# BootFPGA
#    Put the FPGA in boot mode
#
itcl::body AXLM72GenericProxy::BootFPGA {} {
    set message [_buildMessage Set Boot 0]
    return [_transaction $message]
}

##
# SetFPGABoot
#   Set the boot source for the FPGA.
#
# @param which - Boot source as follos:
#
#
# - sector 0 flash (code = 0x0) 
# - sector 1 flash (code = 0x1)
# - sector 2 flash (code = 0x2) 
# - sector 3 flash (code = 0x3) 
# - sram A (code = 0x10000)
#
itcl::body AXLM72GenericProxy::SetFPGABoot which {
    set message [_buildMessage Set BootSource $which]
    return [_transaction $mesage]
}
##
# Configure
#    Loads firmware file into SRAMA from which it can be booted.
#
# @param file - name of the firmare file.
#
itcl::body AXLM72GenericProxy::Configure filename {
    set message [_buildMessage Set BootSource  $filename]
    return [_transaction $message]
}
##
# Read
#    Read a longwords from the device.
#
# @param offset - offset from module base address to read.
#
itcl::body AXLM72GenericProxy::Read {offset} {
    set message [_buildMessage Get $offset]
    return [_transaction $message]
}
