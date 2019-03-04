#===================================================================
# class CCUSB
#===================================================================

package provide ccusb 1.0
package require Itcl
package require xxusb

itcl::class CCUSB {
	inherit XXUSB
	
	private variable firmware
	
	constructor {device} {
		XXUSB::constructor $device
	} {}

	public method GetFirmware {}
#	public method ReadRegister {reg}
#	public method WriteRegister {reg data}
#	public method SetRegister {reg data}
	public method SetBufferLength {length}
	public method SetMixedBuffer {value}
	public method SetForceDump {value}
	public method SetLatchEvent {value}
	public method SetHeader {value}
#	public method SetRepeatDelay {value}
	public method SetTriggerDelay {value}
	public method SetLAMTimeout {value}
	public method SetScalerPeriod {value}
	public method SetScalerFrequency {value}
	public method SetLED {LED code invert latch}
	public method SetNIM {NIM code invert latch}
	public method SetGateDelay {register intput output gate delay invert latch}
	public method ReadScaler {scaler}
	public method WriteScaler {scaler data}
	public method SetLAMMask {mask}
	public method SetBulkBuffers {buffers}
	public method SetBulkTimeOut {timeout}
	public method sReadScaler {stack scaler}
	public method sAddMarker {stack marker}


  public method ComputeBufferLengthBits {length}
  public method SetBits {originalVal settableBits valueToWrite}
}

##
# CCUSB::GetFirmware
#
# Read the firmware ID register (reg file addr 0x0) and return the value to the 
# caller.
#
# \return the valude of the firmware id register
itcl::body CCUSB::GetFirmware {} {
  set firmware [$self readFirmware]
  
	return $firmware
}

##
# CCUSB::SetBufferLength
#
# Set the bits of the global mode register that deal with the buffer length to 
# the appropriate value. 
#  
# \param length the buffer size. Valid values: 4096, 2048, 1024, 512, 256, 128, 64, single
#
# \throws error when user provided an invalid value
itcl::body CCUSB::SetBufferLength {length} {
  set code [ComputeBufferLengthBits $length]
	set mode [$self readGlobalMode]

	set mode [SetBits $mode 0x07 $code]

#	WriteRegister 1 $mode
  $self	writeGlobalMode $mode
}

##
# CCUSB::ComputeBufferLengthBits
#
# Set the bits of the global mode register that deal with the buffer length to 
# the appropriate value. 
#  
# \param length the buffer size. Valid values: 4096, 2048, 1024, 512, 256, 128, 64, single
#
# \throws error when user provided an invalid value
itcl::body CCUSB::ComputeBufferLengthBits {length} {
  set code 0
  switch $length {
    4096 {set code 0}
    2048 {set code 1}
    1024 {set code 2}
    512  {set code 3}
    256  {set code 4}
    128  {set code 5}
    64   {set code 6}
    single {set code 7}
    default {error "CCUSB::ComputeBufferLengthBits argument invalid. User specified $length"}
  }
  return [expr $code&0x07]
}


## CCUSB::SetBits
#
# Given a mask that identify bits to overwrite, combine the original bit set and 
# the new bit set as though it were a bitwise-OR of the two with the protected 
# bits left alone
#
# \param originalVal     the original bit set
# \param replacementBitMask a bit set for which any bit set to 1 are overwritten
# \param newValue        a bit set that is intended to replace those bits unprotected 
#                        by the settableBitMask
#
# \return the new bit set
itcl::body CCUSB::SetBits {originalVal replacementBitMask newValue} {
  set protectedBitMask [expr ~$replacementBitMask]
  set maskedValToWrite [expr ($replacementBitMask)&$newValue]
  set protectedBits    [expr $originalVal&$protectedBitMask]

  # combine the protected bits and the new bits
  return [expr $protectedBits | $maskedValToWrite]
}


##
# CCUSB::SetMixedBuffer 
#
# Sets the bit for specifying mix buffer mode in the global mode regster
#
# \param value a boolean value
#
# \throws if the argument is not boolean
itcl::body CCUSB::SetMixedBuffer {value} {
  if {![string is boolean $value]} {
    error "CCUSB::SetMixedBuffer invalid argument. $value is not a boolean value."
  }
	set mode [$self readGlobalMode]
	set code [expr $value<<5]
	set mode [SetBits $mode 0x20 $code] 
	$self writeGlobalMode $mode
}

##
# CCUSB::SetForceDump 
#
# Sets the bit for specifying force buffer mode in the global mode regster
#
# \param value a boolean value
#
# \throws if the argument is not boolean
itcl::body CCUSB::SetForceDump {value} {
  if {![string is boolean $value]} {
    error "CCUSB::SetForceDump invalid argument. $value is not a boolean value."
  }
	set mode [$self readGlobalMode]
	set code [expr $value<<6]

	set mode [SetBits $mode 0x40 $code]

	$self writeGlobalMode $mode
}

##
# CCUSB::SetLatchEvent 
#
# Sets the bit for specifying latch event mode in the global mode regster
#
# \param value a boolean value
#
# \throws if the argument is not boolean
itcl::body CCUSB::SetLatchEvent {value} {
  if {![string is boolean $value]} {
    error "CCUSB::SetLatchEvent invalid argument. $value is not a boolean value."
  }
	set mode [$self readGlobalMode]
	set code [expr $value<<4]
	set mode [SetBits $mode 0x10 $code]

	$self writeGlobalMode $mode
}

##
# CCUSB::SetHeader 
#
# Sets the bit for specifying optional header in the global mode regster
#
# \param value a boolean value
#
# \throws if the argument is not boolean
itcl::body CCUSB::SetHeader {value} {
  if {![string is boolean $value]} {
    error "CCUSB::SetHeader invalid argument. $value is not a boolean value."
  }
	set mode [$self readGlobalMode]
	set code [expr $value<<8]
	set mode [SetBits $mode 0x0100 $code]
#	WriteRegister 1 $mode
  $self writeGlobalMode $mode
}

#itcl::body CCUSB::SetRepeatDelay {value} {
# value is in units of 100 ns (max delay 700 ns)
#	set mode [ReadRegister 1]
#	set mode [expr ($mode&0x1fff)+($value<<13)]
#	WriteRegister 1 $mode
#}

itcl::body CCUSB::SetTriggerDelay {value} {
  if {![string is integer $value] || ($value<0) || ($value>=256)} {
    error "CCUSB::SetTriggerDelay invalid argument. $value is not an integer in the range \[0,256)."
  }
	set delays [$self readDelays]
	set delays [expr ($delays&0xff00)+$value]
#	WriteRegister 2 $delays
  $self writeDelays $delays
}

itcl::body CCUSB::SetLAMTimeout {value} {
	set delays [$self readDelays]
	set delays [expr ($delays&0xff)+($value<<8)]
#	WriteRegister 2 $delays
  $self writeDelays $delays
}

itcl::body CCUSB::SetScalerPeriod {value} {
	set scaler [$self readScalerControl]
	set scaler [expr ($scaler&0xff0000)+$value]
#	WriteRegister 3 $scaler
	$self writeScalerControl $scaler
}

itcl::body CCUSB::SetScalerFrequency {value} {
	set scaler [$self readScalerControl]
	set scaler [expr ($scaler&0xffff)+($value<<16)]
	$self writeScalerControl $scaler
}

itcl::body CCUSB::SetLED {LED code invert latch} {
	::CCUSBSetLED $self $LED $code $invert $latch
}

itcl::body CCUSB::SetNIM {NIM code invert latch} {
	::CCUSBSetNIM $self $NIM $code $invert $latch
}

itcl::body CCUSB::SetGateDelay {register input output gate delay invert latch} {
	if {[string equal $register A]} {set r 0}
	if {[string equal $register B]} {set r 1}
	if {![info exist r]} {
		tk_messageBox -icon error -message "Error while setting gate & delay of $self\n\
		unknown register: $register ; must be either A or B"
		return
	}
	::CCUSBSetDGG $self $r $input $output $gate $delay $invert $latch
}

itcl::body CCUSB::SetLAMMask {mask} {
#	WriteRegister 9 $mask
	$self writeLAMMask $mask
}

itcl::body CCUSB::ReadScaler {scaler} {
  set data 0
	if {[string equal $scaler A]} {
    set data [$self readScalerA]    
  } elseif {[string equal $scaler B]} {
    set data [$self readScalerA]    
	} else {
		tk_messageBox -icon error -message "Error while reading scaler of $self\n\
		unknown scaler: $scaler ; must be either A or B"
		return
	}
	return $data
}

#itcl::body CCUSB::WriteScaler {scaler data} {
#	if {[string equal $scaler A]} {set r 11}
#	if {[string equal $scaler B]} {set r 12}
#	if {![info exist r]} {
#		tk_messageBox -icon error -message "Error while reading scaler of $self\n\
#		unknown scaler: $scaler ; must be either A or B"
#		return
#	}
#	WriteRegister $r $data
#}

itcl::body CCUSB::SetBulkBuffers {buffers} {
	set bulk [$self readUSBBulkTransferSetup]
	set bulk [expr ($bulk&0xf00)+$buffers]
#	WriteRegister 14 $bulk
	$self writeUSBBulkTransferSetup $bulk
}

itcl::body CCUSB::SetBulkTimeOut {timeout} {
	set bulk [$self readUSBBulkTransferSetup]
	set bulk [expr ($bulk&0xff)+($timeout<<8)]
	$self writeUSBBulkTransferSetup $bulk
}

itcl::body CCUSB::sReadScaler {stack scaler} {
	if {[string equal $scaler A]} {set r 11}
	if {[string equal $scaler B]} {set r 12}
	if {![info exist r]} {
		tk_messageBox -icon error -message "Error while reading scaler of $self\n\
		unknown scaler: $scaler ; must be either A or B"
		return
	}
	set N 25
	set A $r
	set F 0
	set command [expr ($N<<9)+($A<<5)+$F+0x4000]
	AddToStack $stack $command
}

itcl::body CCUSB::sAddMarker {stack marker} {
	set N 0
	set A 0
	set F 16
	set command [expr ($N<<9)+($A<<5)+$F]
	AddToStack $stack $command
	AddToStack $stack $marker
}

