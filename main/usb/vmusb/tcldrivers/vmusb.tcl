#===================================================================
# class VMUSB
#===================================================================

package provide vmusb 1.0

package require Itcl
package require VMUSBDriverSupport 
package require xxusb

itcl::class VMUSB {
	inherit XXUSB
	
	private variable firmware
	
	constructor {avmusb} {
		XXUSB::constructor $avmusb
	} {}

# Interactive functions
	public method GetFirmware {}
	public method ReadRegister {reg}
	public method WriteRegister {reg value}
	public method Read16 {address}
	public method Write16 {address data}
	public method Read24 {address}
	public method Write24 {address data}
	public method Read32 {address}
	public method Write32 {address data}
	public method Read16D16 {address}
	public method Write16D16 {address data}
	public method Read24D16 {address}
	public method Write24D16 {address data}
	public method Read32D16 {address}
	public method Write32D16 {address data}
	public method ReadSBLT32 {address words repeat}
	public method ReadFBLT32 {address blocks repeat}
	public method ReadNBLT32 {nadd address mask repeat}
	public method SetBufferLength {length}
	public method SetMixedBuffer {value}
	public method SetBufferMode {value}
	public method SetAlign32 {value}
	public method SetHeader {value}
	public method SetTriggerDelay {value}
	public method SetScalerPeriod {value}
	public method SetScalerFrequency {value}
	public method SetForceDump {value}
	public method SetLEDs {value}
#	public method SetLED {LED code invert latch}
	public method SetNIMs {value}
#	public method SetNIM {NIM code invert latch}
	public method SetGateDelay {register gate delay}
	public method SetIRQVector {vector stackID IRQLevel IRQID}
	public method SetBulkBuffers {buffers}
	public method SetBulkTimeOut {timeout}
	public method ReadScaler {scaler}
# Stack programming functions
	public method sReadRegister {stack register}
	public method sReadScaler {stack scaler}
	public method sAddMarker {stack marker}
	public method sRead16 {stack address}
	public method sWrite16 {stack address data}
	public method sRead24 {stack address}
	public method sWrite24 {stack address data}
	public method sRead32 {stack address}
	public method sWrite32 {stack address data}
	public method sWait {stack twohundrednano}
	public method sReadSBLT32 {stack address words repeat}
	public method sReadFBLT32 {stack address blocks repeat}
	public method sReadNBLT32 {stack nadd address mask repeat}

  public method ExecuteLongStack {stack}
}

itcl::body VMUSB::GetFirmware {} {
  set firmware [$self readFirmwareID]
	return $firmware
}


##########################################
# THESE ARE MORE DIFFICULT
# but I could make the writeRegister and readRegister
# method public in CVMUSB
itcl::body VMUSB::ReadRegister {reg} {
	set v [::VMUSBReadRegister $self $reg]
	return $v
}

itcl::body VMUSB::WriteRegister {reg value} {
	::VMUSBWriteRegister $self $reg $value
	set check [ReadRegister $reg]
	if {![expr $check == $value]} {
		tk_messageBox -icon error -message "Error while writing register $reg to $self\n\
		Data: $value, ReadBack: $check"
		return
	}
}
##########################################

# D32 read and write functions (defaults)
itcl::body VMUSB::Read16 {address} {
	set v [$self vmeRead16 $address 0x29]
	return $v
}

itcl::body VMUSB::Write16 {address data} {
	$self vmeWrite16 $address 0x29 $data
}

###########################################
# There is no analog to these in swig cvmusb
# may have to write some new funcs
itcl::body VMUSB::Read24 {address} {
	set v [$self vmeRead32 $address 0x39]
	return $v
}

itcl::body VMUSB::Write24 {address data} {
	$self vmeWrite32 $address 0x39 $data
}

itcl::body VMUSB::Read32 {address} {
	set v [$self vmeRead32 $address 0x9] 
	return $v
}

itcl::body VMUSB::Write32 {address data} {
	$self vmeWrite32 $address 0x9 $data
}

# D16 read and write functions
itcl::body VMUSB::Read16D16 {address} {
	set v [$self vmeRead16 $address 0x29 ]
	return $v
}

itcl::body VMUSB::Write16D16 {address data} {
	$self vmeWrite16 $address 0x29 $data
}

itcl::body VMUSB::Read24D16 {address} {
	set v [$self vmeRead16 $address 0x39]
	return $v
}

itcl::body VMUSB::Write24D16 {address data} {
	$self vmeWrite16 $address 0x39 $data
}

itcl::body VMUSB::Read32D16 {address} {
	set v [$self vmeRead16 $address 0x9]
	return $v
}

itcl::body VMUSB::Write32D16 {address data} {
	$self vmeWrite16 $address 0x9 $data
}

# 32 bit VME short block mode read -- needs firmware 79000900 or later
# This block mode is limited to 64 transfers of 32 bit words
itcl::body VMUSB::ReadSBLT32 {address words repeat} {
  # this starts the immediately at stack word 1 (item 4 in sect. 4.4)
  # repeat is a bit that sets fifo or normal blt mode
#	lappend command [expr 0x10b + ($repeat<<10)]; # A32, NW=1, NA=repeat
#	lappend command [expr $words<<8]; # BLT=words
#	lappend command [expr ($address&0xffff)]
#	lappend command [expr int($address>>16)]
#	set command [$self FinishStack $command]
#	set result [XXUSBExecuteLongStack $self $command]

  set result {}
  if {$repeat} {
    set result [$self vmeFifoRead $address 0x0b $words]
  } else {
    set result [$self vmeBlockRead $address 0x0b $words]
  }

	return $result
}

#####################################################
#####################################################
# VMUSBReadout does not support MBLT as far as I know
# Would need to implement this

# 32 bit VME full block mode read -- needs firmware 79000900 or later
# This block mode can read up to 2^23 BLT transfers, specified in the parameter blocks
itcl::body VMUSB::ReadFBLT32 {address blocks repeat} {
	lappend command [expr 0x90b + ($repeat<<10)]; # A32, MB=1, NW=1, NA=repeat
	lappend command 0xff00; # BLT=0xff
	lappend command [expr $blocks&0xffff]
	lappend command [expr int($blocks>>16)]
	lappend command [expr ($address&0xffff)]
	lappend command [expr int($address>>16)]
	set result [ExecuteLongStack $command]
	return $result
}
######################################################

# 32 bit VME Number Data  block mode read -- needs firmware 79000900 or later
# the number of words to be read are specified at address nadd
itcl::body VMUSB::ReadNBLT32 {nadd address mask repeat} {
#	lappend command 0x109
#	lappend command 4; # ND=1
##	lappend command 0xfffc; # Number Extract Mask low
##	lappend command 0xff; # Number Extract Mask high (24 contiguous bits max)
#	lappend command [expr $mask&0xffff]; # Number Extract Mask low
#	lappend command [expr $mask>>16]; # Number Extract Mask high (24 contiguous bits max)
#	lappend command [expr ($nadd&0xffff)]
#	lappend command [expr int($nadd>>16)]
#	lappend command [expr 0x10b + ($repeat<<10)]; # A32, NW=1, NA=repeat
#	lappend command 0; # number of words specified at address nadd
#	lappend command [expr ($address&0xffff)]
#	lappend command [expr int($address>>16)]
#	set command [$self FinishStack $command]
#	set result [XXUSBExecuteLongStack $self $command]

  set pkg cvmusbreadoutlist
  set result {}

  ${pkg}::CVMUSBReadoutList stack 

  if {$repeat} {
    stack addBlockCountRead32 $nadd $mask 0x09 
    stack addMaskedCountFifoRead32 $address 0x0b 
  } else {
    # allocate 4 Mbytes. This number is for the number of transfers
    # expected which is in units of 32-bits or 8 bytes.
    set maxcount [expr (4<<20)/8]
    stack addBlockCountRead32 $nadd $mask 0x09 
    stack addMaskedCountBlockRead32 $address 0x0b 
  }

	return $result
}

itcl::body VMUSB::SetBufferLength {length} {
	set bits [expr int(log($length)/log(2)+.5)]
	if {$bits > 14} {set bits 14}
	if {$bits < 6} {set bits 5}
	set code [expr 14-$bits]
	set mode [$self readGlobalMode]
	puts [format "Original global mode : %x" $mode]
	puts [format "Setting new bits     : %x" $code]
	set mode [expr ($mode&0xfffffff0)+$code]
	puts [format "New global mode      : %x" $mode]
	$self writeGlobalMode $mode
}

itcl::body VMUSB::SetBufferMode {value} {
	puts "SetBufferMode $value"
	set mode [$self readGlobalMode]
	puts [format "\tOriginal global mode : %x" $mode]
	set mode [expr ($mode&0xffffffef)+($value<<4)]
	puts [format "\tNew global mode      : %x" $mode]
	$self writeGlobalMode $mode
}

itcl::body VMUSB::SetMixedBuffer {value} {
	puts "SetMixedBuffer $value"
	set mode [$self readGlobalMode]
	puts [format "\tOriginal global mode : %x" $mode]
	set mode [expr ($mode&0xffffffdf)+($value<<5)]
	puts [format "\tNew global mode      : %x" $mode]
	$self writeGlobalMode $mode
}

itcl::body VMUSB::SetForceDump {value} {
	puts "SetForceDump $value"
	set mode [$self readGlobalMode]
	puts [format "\tOriginal global mode : %x" $mode]
	set mode [expr ($mode&0xffffffbf)+($value<<6)]
	puts [format "\tNew global mode      : %x" $mode]
	$self writeGlobalMode $mode
}

itcl::body VMUSB::SetAlign32 {value} {
	puts "SetAlign32 $value"
	set mode [$self readGlobalMode]
	puts [format "\tOriginal global mode : %x" $mode]
	set mode [expr ($mode&0xffffff7f)+($value<<7)]
	puts [format "\tNew global mode      : %x" $mode]
	$self writeGlobalMode $mode
}

itcl::body VMUSB::SetHeader {value} {
	puts "SetAlign32 $value"
	set mode [$self readGlobalMode]
	puts [format "\tOriginal global mode : %x" $mode]
	set mode [expr ($mode&0xfffffeff)+($value<<8)]
	puts [format "\tNew global mode      : %x" $mode]
	$self writeGlobalMode $mode
}

itcl::body VMUSB::SetTriggerDelay {value} {
	puts "SetTriggerDelay $value"
	set mode [$self readDAQSettings]
	puts [format "\tOriginal daq settings : %x" $mode]
	set mode [expr ($mode&0xffffff00)+$value]
	puts [format "\tNew daq settings      : %x" $mode]
	$self writeDAQSettings $mode
}

itcl::body VMUSB::SetScalerPeriod {value} {
	puts "SetScalerPeriod $value"
	set mode [$self readDAQSettings]
	puts [format "\tOriginal daq settings : %x" $mode]
	set mode [expr ($mode&0xffff00ff)+($value<<8)]
	puts [format "\tNew daq settings      : %x" $mode]
	$self writeDAQSettings $mode
}

itcl::body VMUSB::SetScalerFrequency {value} {
	puts "SetScalerFrequency $value"
	set mode [$self readDAQSettings]
	puts [format "\tOriginal daq settings : %x" $mode]
	set mode [expr ($mode&0x0000ffff)+($value<<16)]
	puts [format "\tNew daq settings      : %x" $mode]
	$self writeDAQSettings $mode
}

#itcl::body VMUSB::SetLED {LED code invert latch} {
#	::VMUSBSetLED $self $LED $code $invert $latch
#}

#itcl::body VMUSB::SetNIM {NIM code invert latch} {
#	::VMUSBSetNIM $self $NIM $code $invert $latch
#}

itcl::body VMUSB::SetLEDs {value} {
	puts [format "SetLEDs %x" $value]
	set mode [expr $value&0xffffffff]
	puts [format "Value to write %x" $mode]
	flush stdout
	$self writeLEDSource $mode
}

itcl::body VMUSB::SetNIMs {value} {
	puts [format "SetNIMs %x" $value]
	set mode [expr $value&0xffffffff]
	puts [format "Value to write %x" $mode]
	flush stdout
	$self writeDeviceSource $mode
#	WriteRegister 16 $mode
}

itcl::body VMUSB::SetGateDelay {register gate delay} {
	set mode [expr ($gate<<16)+$delay]
	if {[string equal $register A]} {
	  $self writeDGG_A $mode
  } else if {[string equal $register B]} {
	  $self writeDGG_B $mode
  } else {
		tk_messageBox -icon error -message "Error while setting gate & delay of $self\n\
		unknown register: $register ; must be either A or B"
		return
	}
}
	
itcl::body VMUSB::ReadScaler {scaler} {
  set data 0 
	if {[string equal $scaler A]} {
    set data [$self ReadScalerA]
  } else if {[string equal $scaler B]} {
    set data [$self ReadScalerB]
  } else {
		tk_messageBox -icon error -message "Error while reading scaler of $self\n\
		unknown scaler: $scaler ; must be either A or B"
		return
	}
	return $data
}

itcl::body VMUSB::SetIRQVector {vector stackID IRQLevel IRQID} {
#	vector goes from 1 to 8
	if {$vector < 1 || $vector > 8} {
		tk_messageBox -icon error -message "Error while setting IRQ vector of $self\n\
		vector must be between 1 and 8"
		return
	}
  # register is either 1, 2, 3, or 4 b/c each register stores 2 vectors
	set reg [expr int(($vector-1)/2)+1]
	set code [$self readVector $reg]
	set code1 [expr ($IRQID&0xff) + ($IRQLevel&0x7)<<8 + ($stackID&0x3)<<12]
  # handle whether if vector id is even or odd
	if {($vector/2)*2 == $vector} { 
    # even registers go in the upper 16 bits
		set code [expr ($code&0xffff) + ($code1<<16)]
	} else {
    # odd registers go in the lower 16 bits
		set code [expr $code&0xffff0000 + $code1]
	}
	$self writeVector $reg $code
# 	for now we limit the IRQID to 8 bits; implement 16 bits with expansion registers later
}

itcl::body VMUSB::SetBulkBuffers {buffers} {
	set bulk [$self readBulkXferSetup]
	set bulk [expr ($bulk&0xffffff00)+$buffers]
	$self writeBulkXferSetup $bulk
}

itcl::body VMUSB::SetBulkTimeOut {timeout} {
	set bulk [$self readBulkXferSetup]
	set bulk [expr ($bulk&0xffff00ff)+($timeout<<8)]
	$self writeBulkXferSetup $bulk
}

# Stack programming functions

itcl::body VMUSB::sReadRegister {stack register} {
	lappend command [expr 0x1109]; # SLF=1, NW=1, A32
	lappend command [expr $register&0x3f]
	AddToStack $stack $command
}

itcl::body VMUSB::sReadScaler {stack scaler} {
	if {[string equal $scaler A]} {set r 28}
	if {[string equal $scaler B]} {set r 32}
	if {![info exist r]} {
		tk_messageBox -icon error -message "Error while coding scaler readout of $self\n\
		unknown scaler: $scaler ; must be either A or B"
		return
	}
	sReadRegister $stack $r
}

itcl::body VMUSB::sAddMarker {stack marker} {
	lappend command [expr 0x2000]; # MRK=1
	lappend command [expr $marker&0xffff]
	AddToStack $stack $command
}

itcl::body VMUSB::sRead16 {stack address} {
	lappend command [expr 0x29 + 0x100]; # A16 access + NW=1
  lappend command [expr $address]
	AddToStack $stack $command
}

itcl::body VMUSB::sWrite16 {stack address data} {
	lappend command [expr 0x29]; # A16 access + NW=0
  lappend command [expr $address]
	lappend command [expr $data&0xffff]
	AddToStack $stack $command
}

itcl::body VMUSB::sRead24 {stack address} {
	lappend command [expr 0x39 + 0x100]; # A24 access + NW=1
	lappend command [expr $address]; 
	AddToStack $stack $command
}

itcl::body VMUSB::sWrite24 {stack address data} {
	lappend command [expr 0x39]; # A24 access + NW=0
	lappend command [expr $address&0xffff]; 
	lappend command [expr $data&0xffff]
	AddToStack $stack $command
}

itcl::body VMUSB::sRead32 {stack address} {
	lappend command [expr 0x9 + 0x100]; # A32 access + NW=1
	lappend command [expr $address]
	AddToStack $stack $command
}

itcl::body VMUSB::sWrite32 {stack address data} {
	lappend command [expr 0x9]; # A32 access + NW=0
	lappend command [expr $address]
	lappend command [expr $data]
	AddToStack $stack $command
}

itcl::body VMUSB::sWait {stack twohundrednano} {
	if {$twohundrednano > 255 } {set twohundrednano 255}
	lappend command [expr 0x8000 + $twohundrednano]; # DLY=1
	AddToStack $stack $command
}

# 32 bit VME short block mode read -- needs firmware 79000900 or later
# This block mode is limited to 64 transfers of 32 bit words
itcl::body VMUSB::sReadSBLT32 {stack address words repeat} {
  # A32, MB=1, NW=1, NA=repeat, N BLT words
	lappend command [expr 0x10b | ($repeat<<10) | ($words<<24)]; 
  lappend command [expr $address]
	AddToStack $stack $command
}

# 32 bit VME full block mode read -- needs firmware 79000900 or later
# This block mode can read up to 2^23 BLT transfers, specified in the parameter blocks
itcl::body VMUSB::sReadFBLT32 {stack address blocks repeat} {
 # A32, MB=1, NW=1, NA=repeat, BLT=0xff000000
	lappend command [expr 0x90b + ($repeat<<10) | (0xff000000)];
	lappend command [expr $blocks]
	lappend command [expr $address]
	AddToStack $stack $command
}

# 32 bit VME Number Data  block mode read -- needs firmware 79000900 or later
# the number of words to be read are specified at address nadd
itcl::body VMUSB::sReadNBLT32 {stack nadd address mask repeat} {
  # A32, AM=0x09, ND=1
	lappend command [expr 0x109 | (1<<18)]
	lappend command [expr $mask]; # Number Extract Mask low
	lappend command [expr $nadd]
	lappend command [expr 0x10b + ($repeat<<10)]; # A32, NW=1, NA=repeat
	lappend command [expr $address]
	AddToStack $stack $command
}

itcl::body VMUSB::ExecuteLongStack {stack} {
  # The stack is structured as the xxusb_long_stack_execute
  # function from libxxusb.so demands. That is :
  # Size
  # list of operations
  global ::VMUSBDriverSupport::convertToReadoutList 
  
#  set trimlist [lrange $stack 2 [llength $stack]]
#  set rdolist [::list_to_rdolist $trimlist]
  set rdolist [::VMUSBDriverSupport::convertToReadoutList $stack]

  $self executeList $rdolist [expr 4<<20] 
}
