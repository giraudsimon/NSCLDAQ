#===================================================================
##
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#      NSCL DAQ Development Group 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#
# @file   manCAENN568BTests.tcl
# @author Jeromy Tompkins & Daniel Bazin
# @note   A heavily modified version of the original ACAENV288 caennet driver
#         originally written by Daniel Bazin.

package provide caenv288 11.0

package require Itcl
package require cvmusb

## @brief VM-USB driver for the CAEN V288 H.S. Caennet Controller
#
itcl::class ACAENV288 {

	protected variable device ;#< a cvmusb::CVMUSB object
	private variable base     ;#< base address

	private variable status   ;#< status register
	private variable transmit ;#< transmit register
	private variable reset    ;#< reset register

  ## @brief Constructor
  #
  # @param de   a cvmusb::CVMUSB object
  # @param ba   base address
  #
  # @returns  name of the device
	constructor {de ba} {
		set device $de
		set base $ba

    # set up the various register offsets for later convenience
		set status   [expr $base+0x2]
		set transmit [expr $base+0x4]
		set reset    [expr $base+0x6]
    
    # because the device fails to accept a write the first time after
    # a reset, we will ensure that it does not fail the first time around. It
    # is possible that the user is creating this after the first power cycle
    # and this it will immediately fail if we don't call GetStatus
    if {$de ne ""} {
      # it is possible not to have been provided a vmusb
      GetStatus
    }
  }
	
  ## @brief Destructor - noop
  #
  #
	destructor {}
	
  public method SetController {ctlr} { set device $ctlr}
  public method GetController {} { return $device}

  ## @brief Send command with one parameter value
  #
  # Formulates the proper write sequence for the CAENNet protocol and then
  # sends it to the slaves.
  #
  # @param slave  node index of the targeted slave
  # @param code   operation code to perform
  # @param value  value to write
  #
  # @returns error code produced by the operation 
  # @retval 0       - success
  # @retval 0xfffd  - no data to transmit
  # @retval 0xfffe  - controller identifier incorrect
  # @retval 0xffff  - addressed slave does not exist
  #
  # @throws timeout occurs when waiting for error code
	public method Send {slave code value}


  ## @brief Send a command that requires no data 
  #
  # This is the same thing as Send but without parameter values to write 
  #
  # @returns error code produced by the operation
  # @retval 0       - success
  # @retval 0xfffd  - no data to transmit
  # @retval 0xfffe  - controller identifier incorrect
  # @retval 0xffff  - addressed slave does not exist
  #
  # @throws timeout occurs when waiting for error code
	public method SendCode {slave code}

  ## @brief Send a command with an arbitrary number of parameters 
  #
  # This is actually the most general form of the send commands. It forms the
  # body of all the other commands combined.
  # 
  # @param  slave  node index of targeted slave
  # @param  code   command code to send
  # @param  values parameter values to accompany command
  # @param  nval   number of values to write
  #
  # @returns error code produced by the operation
  # @retval 0       - success
  # @retval 0xfffd  - no data to transmit
  # @retval 0xfffe  - controller identifier incorrect
  # @retval 0xffff  - addressed slave does not exist
  #
  # @throws error under the following circumstances
  #  - timeout occurs when waiting for error code
  #  - 
	public method SendAll {slave code values nval}
  

  ## @brief Retrieve a variable name
  #
	public method GetVariable {v} {set $v}

  ## @brief Reset the module entirely
  #
  # - buffers are cleared
  # - pending interrupts are cleared
  # - data transfers are aborted
  # - no commands are accepted
  #
  # This method inserts a 5 ms wait following the device communication to
  # protect the device's 3ms unresponsiveness following a reset operation.
  #
	public method Reset {}


  ## @brief Read the status register
  #
  # @returns value of least significant bit in status register
  # 0 - valid operation
  # 1 - no valid operation 
	public method GetStatus {} 


  ## @brief Send commands out to devices
  #
  # Writes a 1 to the transmit buffer register.
  #
  # @throws error if status register is non-zero following transmission
	public method TransmitData {}

  ## @brief Write data to the transmit buffer for future transmission 
  #
  # @param word   value to write to the transmit buffer
  #
  # @throws error if status register is non-zero following transmission
	public method WriteTransmitBuffer {word}

  ## @brief High-level command that handles receipt of data
  #
  # Polls the device for the response and gives up if after 1000 iterations,
  # there is still no valid response, then an error is returned.  The number of
  # words read is the first element of the returned data and it is exclusive.
  #
  # @returns list of words read from the module of the form {nwords data...}
  #
  # @throws error if timeout occurs before valid data
	public method Receive {}

  ## @brief Poll the device for an error response following a transmission
  #
  # @returns error code read from the device
  #
  # @throws error if timeout occurs while waiting for response.
	public method ReceiveError {}
}


#-----------------------------------------------------------------------------#
# Implementations

#
#
#
#
itcl::body ACAENV288::Reset {} {
  $device vmeWrite16 $reset 0x39 1; 
  # wait for 5 ms to ensure that no further commands are executed before the
  # module is ready
  after 5

  # the module rejects the first write after a reset for some reason unless I
  # perform a getstatus
  GetStatus
}

#
#
#
#
itcl::body ACAENV288::GetStatus {} {
  set retval [$device vmeRead16 $status 0x39]
  set retval [expr $retval&0x1]
  return $retval
}

#
#
#
#
itcl::body ACAENV288::TransmitData {} {
  $device vmeWrite16 $transmit 0x39 1 
	if {[GetStatus]!=0} {
    set msg "ACAENV288::TransmitData error starting data packet transmission"
		return -code error $msg 
	}
}


#
#
#
#
itcl::body ACAENV288::WriteTransmitBuffer {word} {
	$device vmeWrite16 $base 0x39 $word
  if {[GetStatus]!=0} {
    set msg "ACAENV288::WriteTransmitBuffer Error writing data into transmit "
    append msg "data buffer ($word)"
    return -code error $msg
  }
}

#
#
#
#
itcl::body ACAENV288::Receive {} {

  set buffer [list]
  set nwords 0

  # poll for 1000 iterations
	set timeout 1000
  set iter 0
  for {set iter 0} {$iter < $timeout} {incr iter} { 
    set rdb [$device vmeRead16 $base 0x39]
    if {[GetStatus]==0} {
      break;
    }
    after 1
  }

  # throw error on timeout
	if {$iter == $timeout} {
    set msg "ACAENV288::Receive Timeout while receiving data"
		return -code error $msg
	}

  # if we didnt timeout, then store the most recently read word
  lappend buffer $rdb
  incr nwords 

  # Read the slave's response
  while {1} {
    set rdb [$device vmeRead16 $base 0x39]
    if {[GetStatus]==0} {
#      puts "data : $rdb"
      lappend buffer $rdb
      incr nwords
    } else {
      break
    }
	}

  # prepend the list with the number of words to follow
  return $buffer
}

#
#
#
#
itcl::body ACAENV288::ReceiveError {} {

  # Poll the device until data is received or a timeout occurs
	set timeout 500
  set rdb 0
	while {$timeout>0 && [GetStatus]!=0} {
#		set rdb [$device Read24D16 $base]
    set rdb [$device vmeRead16 $base 0x39]
		incr timeout -1
    after 1
	}
	if {$timeout == 0} {
    set msg "ACAENV288::ReceiveError Timeout while receiving data"
    append msg [format "Error code = 0x%x" $rdb]
    return -code error $msg
	} else {
    return $rdb
  }

}

#
#
#
#
itcl::body ACAENV288::Send {slave code value} {
	if {$slave < 0 || $slave > 99} {
#		tk_messageBox -message "CAEN V288: slave address code out of range" -icon error
    return -code error "ACAENV288::Send Slave address code out of range"
	}
  GetStatus
	WriteTransmitBuffer 1
	WriteTransmitBuffer $slave
	WriteTransmitBuffer $code
	WriteTransmitBuffer $value
	TransmitData
  set error [ReceiveError]
	return $error
}

#
#
#
#
#
itcl::body ACAENV288::SendCode {slave code} {
	if {$slave < 0 || $slave > 99} {
    set msg "ACAENV288::SendCode slave address code out of range"
	  return -code error $msg	
	}
  GetStatus
	WriteTransmitBuffer 1
	WriteTransmitBuffer $slave
	WriteTransmitBuffer $code
	TransmitData
  set error [ReceiveError]
	return $error
}

#
#
#
#
#
itcl::body ACAENV288::SendAll {slave code values nval} {
	if {$slave < 0 || $slave > 99} {
		tk_messageBox -message "CAEN V288: slave address code out of range" -icon error
    set msg "ACAENV288::SendAll Slave address code out of range"
    return -code error $msg
	}
	if {$nval < 0 || $nval > 253} {
    set msg "ACAENV288::SendAll Number of set values out of range"
    return -code error $msg
	}
  GetStatus
	WriteTransmitBuffer 1
	WriteTransmitBuffer $slave
	WriteTransmitBuffer $code
	for {set i 0} {$i < $nval} {incr i} {
		WriteTransmitBuffer [lindex $values $i]
	}
	TransmitData
  set error [ReceiveError]
	return $error 
}
