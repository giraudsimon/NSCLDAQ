#===================================================================
# class AMesytecMADC32
#===================================================================
itcl::class AMesytecMADC32 {
	private variable device
	private variable base
	
	constructor {de ba} {
		set device $de
		set base $ba
	}
	
	destructor {}

# interactive functions
	public method GetID {} {return [$device Read16 [expr $base+0x6004]]}
	public method GetFirmware {} {return [$device Read16 [expr $base+0x600e]]}
	public method SetThreshold {channel value}
	public method SetThresholds {value}
	public method SetIRQLevel {level} {$device Write16 [expr $base+0x6010] $level}
	public method SetDataLength {code} {$device Write16 [expr $base+0x6032] $code}
	public method SetIRQVector {vector} {$device Write16 [expr $base+0x6012] $vector}
	public method ResetFIFO {} {$device Write16 [expr $base+0x6034] 0}
	public method SetMultiEvent {mode} {$device Write16 [expr $base+0x6036] $mode}
	public method SetMaxTransfer {words} {$device Write16 [expr $base+0x601a] $words}
	public method Init {}
# stack functions
	public method sRead {stack}
	public method sResetFIFO {stack}
}

# interactive functions implementation
itcl::body AMesytecMADC32::SetThreshold {channel value} {
	$device Write32D16 [expr $base+0x4000+$channel*2] $value
	set check [$device Read32D16 [expr $base+0x4000+$channel*2]]
	if {$check < 0} {set check [expr $check+65536]}
	if {![expr $check == $value]} {
		tk_messageBox -icon error -message "Error while writing threshold to $this\nSet value=$value; Read back=$check"
		return
	}
}

itcl::body AMesytecMADC32::SetThresholds {value} {
	for {set i 0} {$i < 32} {incr i} {
		SetThreshold $i $value
	}
}

itcl::body AMesytecMADC32::Init {} {
	SetIRQVector 0
	SetIRQLevel 0
	SetMultiEvent 3
	SetMaxTransfer 1
	SetThresholds 0
	SetDataLength 2
	ResetFIFO
}

# stack functions implementation
itcl::body AMesytecMADC32::sRead {stack} {
	$device sReadSBLT32 $stack $base 35 0; # Read up to 35 x 32 bit words of data
#	$device sReadNBLT32 $stack [expr $base+0x6030] $base 0xff 0 
	sResetFIFO $stack
}

itcl::body AMesytecMADC32::sResetFIFO {stack} {
	$device sWrite16 $stack [expr $base+0x6034] 0
}
