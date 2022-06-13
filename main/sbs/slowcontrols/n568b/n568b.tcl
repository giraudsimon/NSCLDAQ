#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321
#

#
#  Package to support the n568 shaper module from CAEN on CAENNET.
#  The assumption is that the CAENNET package has already been
#  sourced in.
#


package provide n568b 2.0
package require caennet
namespace eval n568b {

}


    # Return the device id string given a controller/node

#---------------------------------------------------------------
#  n568b::Id controller node
#          Returns the identification string of the module.
# Parameters:
#   controller   - A controller handle gotten from the caennet
#                  package
#   node         - The node number of the module on that net.
# Returns:
#   The identification string from the device.
proc n568b::Id {controller node} {

    after 25;    # in case we need the full 100ms.
    set reply [n568::_command $controller $node 0 {getting id string}]
    

    for {set i 1} {$i < [llength $reply] } {incr i} {
        append id [format %c [lindex $reply $i]]

    }

    return $id

}
# Format the parameters for a channel either gotten via GetAllParameters
# or Get Channel Parameters see GetAllParameters for a description of the
# output list.
#
proc n568b::FormatParams {stat} {
    lappend result [lindex $stat 0]      ;# Fine gain.
    lappend result [lindex $stat 1]      ;# Pole zero.
    set     status [lindex $stat 2]      ;# Needs decoding.


    # polarity

    if {($status & 0x40) != 0} {
	lappend result "-"
    } else {
	lappend result "+"
    }
    # Output configuration.

    if {[expr $status & 0x20] != 0} {
	lappend result inverted
    } else {
	lappend result normal
    }


    # Shaping time.

    set shape [expr ($status >> 3) & 3]
    switch $shape {
	0 { lappend result 0.2 }
	1 { lappend result 1.0 }
	2 { lappend result 3.0 }
	3 { lappend result 6.0 }
    }

    #  Coarse gain:

    set cgain [expr ($status & 7)]
    lappend result [expr pow(2, $cgain)]


    return $result

}
#------------------------------------------------------------------
# n568b::GetAllParameters controller node
# 		Return all device parameters .
# 		These are returned as a list of channel information.
# Parameters:
#   controller   - A controller handle gotten from the caennet
#                  package.
#   node         - The node number of the module on that net.
# Returns:
#	 For each channel, a list containing the following members is supplied:
# 	Fine gain, Pole zero
# 	polarity (+/-), Configuration (normal/inverted) shapetime (us)
# 	Coarse Gain ... Offset Value
#
proc n568b::GetAllParameters {controller node} {
    set reply [n568b::_command $controller $node 1 {getting all parameters}]
    
    #  There are 16 statuses just like a 'normal' individual status.
    #
    set base 1
    set result ""
    for {set channel 0} {$channel < 16} {incr channel} {
	set chstat [list [lindex $reply $base]   \
			 [lindex $reply [expr $base + 1]]   \
			 [lindex $reply [expr $base + 2]] ]
	lappend result [FormatParams $chstat]
	incr base 3
    }
    lappend result [lindex $reply $base]
    return $result
}
#-------------------------------------------------------------
# n568b::GetOffset controller node
#
#      Return the offset value.
# Parameters:
#   controller   - A controller handle gotten from the caennet
#                  package.
#   node         - The node number of the module on that net.
# Returns:
#    The value of the common offset.
#
proc n568b::GetOffset {controller node} {
    set reply [n568b::_command $controller $node 2 {Getting offset}]
    
    return [lindex $reply 1]
}
#----------------------------------------------------------------
# n568b::GetChannelParameters controller node channel
#
# 	Return the parameters for a single channel.   This is given in the
# 	same format as channel information for GetAllParameters
# Parameters:
#   controller   - A controller handle gotten from the caennet
#                  package.
#   node         - The node number of the module on that net.
#   channel      - The number of the channel for which we want
#                  to get information.
# Returns:
#    See GetAllParameters.. we'll return a single element of the grand list
#    that function returns.
#
proc n568b::GetChannelParameters {controller node channel} {
    set function [expr ($channel << 8) | 3]
    set reply [n568b::_command $controller $node $function {Getting channel parameters}]

    
    set chstat [list [lindex $reply 1] \
		     [lindex $reply 2] \
		     [lindex $reply 3]]
    return [FormatParams $chstat]
}
#---------------------------------------------------------------------
# n568b::GetMuxStatus controller node
#
#     Return the multiplexor status.
# Parameters:
#   controller  - The controller handle for the caennet controller.
#   node        - The node id of the module.
#
# Returns:
#    A 2 element list of the form
#      on|off channel
#      on|off indicates if the multiplexor is enabled.
#      channel is the currently programmed mux channel.
#
proc n568b::GetMuxStatus {controller node} {
    set reply [n568b::_command $controller $node 4 n568b::getMuxStatus]
    
    set reply [lindex $reply  1]
    if {($reply & 0x80) != 0} {
	lappend result on
    } else {
	lappend result off
    }
    lappend result [expr $reply  & 7]
    return $result
}
#----------------------------------------------------------------
# n568b::SetFineGain controller node channel value
#
#     Set channel fine gain.
#
# Parameters:
#   controller     - The controller handle gotten from the
#                    caennet package
#   node           - The node id of the shaper module.
#   channel        - The channel to set.
#   value          - The new fine gain value.
#
#
proc n568b::SetFineGain {controller node channel value} {
    if {($channel < 0) || ($channel > 15) } {
	error "n568b - invalid channel number $channel" \
		n568b::SetFineGain
    }
    set command [expr ($channel << 8) | 0x10]
    if {$value > 0xff} {
	error "n568b - invalid fine gain $value" \
		n568b::SetFineGain
    }
    
    after 75
    set reply [caennet::send $controller $node $command $value]
    n568b::_checkerror $reply n568b::SetFineGain
    
}
#------------------------------------------------------------------
# n568b::SetCoarseGain controller node channel value
#
#      Set channel n coarse gain.
# Parameters:
#   controller   - The controller handle gotten from the caennet package.
#   node         - The node of the shaper on that controller's caennet.
#   channel      - The channel whose coarse gain will be set.
#   value        - The new value for the coarse gain.
#                  Note this is a value 0-7 not the actual coarse
#                  gain!!!!
#
proc n568b::SetCoarseGain {controller node channel value} {

    if {($channel < 0) || ($channel > 15) } {
	error "n568b - invalid channel $channel" \
		n568b::SetCoarseGain
    }
    set command [expr ($channel << 8) | 0x11]
    if {$value > 7} {
	error "n568b - invalid coarse gain value $value" \
		n568b::SetCoarseGain
    }
    after 75
    set reply [caennet::send $controller $node $command $value]
    n568b::_checkerror $reply n568b::SetCoarseGain
}
#-----------------------------------------------------------------
# n568b::SetPole0 controller node channel value
#
#     Set channel pole zero adjust.
# Parameters:
#   controller  - Controller handle gotten from the caennet package.
#   node        - The node id of the shaper on the controller's caennet.
#   channel     - The channel for which we are settinga  pole 0 adjust.
#   value       - The new value of the pole 0 adjust.
#
proc n568b::SetPole0 {controller node channel value} {
    if {($channel < 0) || ($channel > 15) } {
	error "n568b - invalid channel number $channel" \
		n568b::SetPole0
    }
    set command [expr ($channel << 8) | 0x12]
    if {($value < 0) || ($value > 0xff) } {
	error "n568b - invalid pole zero value $value" \
		n568b::SetPole0
    }
    after 75
    set reply [caennet::send $controller $node $command $value]
    n568b::_checkError $reply  n568b::SetPole0

}
#---------------------------------------------------------------
# n568b::SetShapingTime controller node channel value
#
#     Set channel shaping time (this is a value 0-3 not the actual
#     shaping time.
# Parameters:
#   controller   - Controller handle to a caennet gotten from the caennet package
#   node         - Node id of the shaper on the controller's caennet.
#   channel      - Channel whose shaping time will be set.
#   value        - New value for the shaping time.
#
proc n568b::SetShapingTime {controller node channel value} {

    if {($channel < 0) || ($channel > 15) } {
	error "n568b - invalid channel number $channel" \
		n568b::SetShapingTime
    }
    set command [expr ($channel << 8) | 0x13]
    if {($value < 0) || ($value > 0x3) } {
	error "n568b - invalid shaping time selector $value" \
		n568b::SetShapingTime
    }
    after 75
    set reply [caennet::send $controller $node $command $value]
    n568b::_checkError $reply  n568b::SetShapingTime

}
#-----------------------------------------------------------
# n568b::SetPolarity controller node channel value
#
#    Set the output polarity.
# Parameters:
#    controller  - Caennet controller handle gotten from
#                  the caennet package.
#    node        - The node on the caennet of the shaper.
#    channel     - The channel whose polarity will be set.
#    value       - The value 0 means positive and 1 means negative.
#
proc n568b::SetPolarity {controller node channel value} {
    if {($channel < 0) || ($channel > 15) } {
	error "n568b - invalid channel number $channel" \
		n568b::SetPolarity
    }
    set command [expr ($channel << 8) | 0x14]
    if {($value != 0 ) && ($value != 1) } {
	error "n568b - invalid polarity selector  $value" \
		n568b::SetPolarity
    }
    after 75               ;# Seems to need a delay  between consecutives
    set reply [caennet::send $controller $node $command $value]
    n568b::_checkerror $reply  n56b::SetPolarity
    
}
#-----------------------------------------------------------------------
# n568b::SetOutputConfiguration controller node channel value
#
#     Set the output configuration values for a channel
#
# Parameters:
#    controller  - The handle of the caennet controller gotten with the
#                  caennet package.
#    node        - The node of the shaper module
#    channel     - The channel whose output config will be set.
#    value       -  0 - direct, 1 -inverted.
#
proc n568b::SetOutputConfiguration {controller node channel value} {
    if {($channel < 0) || ($channel > 15) } {
	error "n568b - invalid channel number $channel" \
		n568b::SetOutputConfiguration
    }
    set command [expr ($channel << 8) | 0x15]
    if {($value != 0 ) && ($value != 1) } {
	error "n568b - invalid configuration  selector  $value" \
		n568b::SetOutputConfiguration
    }
    after 75		;#  Seems to need a delay  for consecutives
    set reply [caennet::send $controller $node $command $value]
    n568b::_checkerror $reply n568b::SetOutputConfiguration
    
}
#-------------------------------------------------------------------
# n568b::SetOffset controller node value
#
#       Set the common offset value (shared amongst all channels).
# Parameters:
#    controller  -  The controller handle gotten from caenet
#    node        - The node of the shaper on the caennet.
#    value       - The new value to set for the common offset.
#
proc n568b::SetOffset {controller node value} {
    set command 0x16
    if {($value < 0) || ($value > 0xff) } {
	error "n568b - invalid offset value $value" \
		n568b::SetOffset
    }
    after 75
    set reply [caennet::send $controller $node $command $value]
    n568b::_checkerror $reply n568b::SetOffset
    
}
#------------------------------------------------------------------
# n568b::EnableMuxOut
#
#     Enable the multiplexed monitor output.
# Parameters:
#   controller - Controller handle gotten from caennet.
#   node       - The node of the shaper on the controller.
#
proc n568b::EnableMuxOut {controller node} {
    set command 0x21
    after 75
    set reply [caennet::send $controller $node $command]
    n568b::_checkerror $reply  n568b::EnableMuxOut
} 
#------------------------------------------------------------------
# n568b::DisableMuxOut controller node
#
#       Disable the multiplexed monitor output.
# Parameters:
#    controller -  The caennet controller handle.
#    node       -  The id of the shaper on the caennet.
#
proc n568b::DisableMuxOut {controller node} {
    set command 0x20
    n568b::_command $controller $node $command {Disabling mux}
}
##
# _checkerror
#   Checks for a caennet error and throws if there is one:
#
# @param reply - reply from caennet::send.
# @param doing - string describing what's being attempted.
#
proc n568b::_checkerror {reply doing} {
    set error [lindex $reply 0]
    if {$error != 0} {
        error "CAENnet reply message error $error " \
		$doing $error
    }
}
##
# _command
#  Perform a command with a 75ms delay preceding it that the
#  n568 seems to need.  The command has no data of its own.
#
# @param controller - controller handle
# @param node       - caennet node number.
# @param cmd        - the command to send.
# @param doing      - operation  in english
# @return list      - full reply list.
# @note an error is thrown if the reply indicates a failure.
proc n568b::_command {controller node command doing} {
    after 75
    set reply [caennet::send $controller $node $command]
    n568b::_checkerror $reply $doing
    return $reply
}




