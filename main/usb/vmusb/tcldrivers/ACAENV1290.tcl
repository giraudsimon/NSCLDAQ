#===================================================================
# class ACAENV1290
#===================================================================
itcl::class ACAENV1290 {
	private variable device
	private variable base
	private variable matchWindowWidth
	private variable windowOffset
	private variable extraSearchWindowWidth
	private variable rejectMargin
	private variable triggerTimeSubtraction
	
	constructor {de ba} {
		set device $de
		set base $ba
	}
	
	destructor {}

# interactive functions
	public method GetVariable {v} {set $v}
	public method SetControlRegister {value} {$device Write16 [expr $base+0x1000] $value}
	public method ReadStatusRegister {} {return [$device Read16 [expr $base+0x1002]]}
	public method Reset {}
	public method Clear {}
	public method WriteOpcode {opcode}
	public method ReadOpcode {}
	public method SetTriggerMatchingMode {}
	public method SetContinuousStorageMode {}
	public method ReadTriggerConfiguration {name}
	public method SetMatchWindowWidth {width}; # width in ns
	public method SetWindowOffset {offset}; # offset in ns >0 or <0
	public method SetExtraSearchWindowWidth {width}; # width in ns
	public method SetRejectMargin {margin}; # margin in ns
	public method EnableTriggerSubtraction {}
	public method DisableTriggerSubtraction {}
	public method SetEdgeDetection {config}
	public method SetResolution {code}
	public method SetMaximumHits {hits}
	public method EnableChannel {channel}
	public method DisableChannel {channel}
	public method EnableAllChannels {}
	public method DisableAllChannels {}
	public method EnableTDCMarkers {}
	public method DisableTDCMarkers {}
	public method EnableTDCErrors {}
	public method DisableTDCErrors {}
	public method InitReA3 {}
# stack functions
	public method sRead {stack}
}

#interactive functions implementation
itcl::body ACAENV1290::Reset {} {
	$device Write16 [expr $base+0x1014] 0
	after 1000
}

itcl::body ACAENV1290::Clear {} {
	$device Write16 [expr $base+0x1016] 0
	after 1000
}

itcl::body ACAENV1290::WriteOpcode {opcode} {
	# Check if micro controller is ready
	set check [$device Read16 [expr $base+0x1030]]
	if {[expr ($check&1) == 0]} {
		tk_messageBox -icon error -message "Could not write to micro controller in module V1290 $this"
		return
	}
	$device Write16 [expr $base+0x102e] $opcode
}

itcl::body ACAENV1290::ReadOpcode {} {
	# Check if micro controller is ready
	set check [$device Read16 [expr $base+0x1030]]
	if {[expr ($check&2) == 0]} {
		tk_messageBox -icon error -message "Could not read from micro controller in module V1290 $this"
		return
	}
	set opcode [$device Read16 [expr $base+0x102e]]
	return $opcode
}

itcl::body ACAENV1290::SetTriggerMatchingMode {} {
	WriteOpcode 0
	after 1000
	WriteOpcode 0x200
	set check [ReadOpcode]
	if {[expr ($check&1) == 0]} {
		tk_messageBox -icon error -message "Failed to set V1290 module $this in trigger matching mode"
	}
}

itcl::body ACAENV1290::SetContinuousStorageMode {} {
	WriteOpcode 0x100
	after 1000
	WriteOpcode 0x200
	set check [ReadOpcode]
	if {[expr ($check&1) == 1]} {
		tk_messageBox -icon error -message "Failed to set V1290 module $this in continuous storage mode"
	}
}

itcl::body ACAENV1290::ReadTriggerConfiguration {name} {
	WriteOpcode 0x1600
	set matchWindowWidth [ReadOpcode]
	set windowOffset [ReadOpcode]
	set extraSearchWindowWidth [ReadOpcode]
	set rejectMargin [ReadOpcode]
	set triggerTimeSubtraction [ReadOpcode]
	return [set $name]
}

itcl::body ACAENV1290::SetMatchWindowWidth {width} {
	set value [expr int($width/25)&0xfff]; # convert in units of 25 ns
	WriteOpcode 0x1000
	WriteOpcode $value
	after 1000
	set check [ReadTriggerConfiguration matchWindowWidth]
	if {![expr $check == $value]} {
		tk_messageBox -icon error -message "Failed to set Match Window Width in V1290 module $this"
	}
}

itcl::body ACAENV1290::SetWindowOffset {offset} {
	set value [expr int($offset/25)]
	if {$value > 40} {
		set value 40
	} elseif {$value < 0} {
		set value [expr $value+65536]
	}
	set value [expr $value&0xfff]
	WriteOpcode 0x1100
	WriteOpcode $value
	after 1000
	set check [ReadTriggerConfiguration windowOffset]
	if {![expr $check == $value]} {
		tk_messageBox -icon error -message "Failed to set Window Offset in V1290 module $this"
	}
}

itcl::body ACAENV1290::SetExtraSearchWindowWidth {width} {
	set value [expr int($width/25)&0xfff]; # convert in units of 25 ns
	WriteOpcode 0x1200
	WriteOpcode $value
	after 1000
	set check [ReadTriggerConfiguration extraSearchWindowWidth]
	if {![expr $check == $value]} {
		tk_messageBox -icon error -message "Failed to set Extra Search Window Width in V1290 module $this"
	}
}

itcl::body ACAENV1290::SetRejectMargin {margin} {
	set value [expr int($margin/25)&0xfff]; # convert in units of 25 ns
	WriteOpcode 0x1300
	WriteOpcode $value
	after 1000
	set check [ReadTriggerConfiguration rejectMargin]
	if {![expr $check == $value]} {
		tk_messageBox -icon error -message "Failed to set Reject Margin in V1290 module $this"
	}
}

itcl::body ACAENV1290::EnableTriggerSubtraction {} {
	WriteOpcode 0x1400
	after 1000
	set check [ReadTriggerConfiguration triggerTimeSubtraction]
	if {![expr ($check&1) == 1]} {
		tk_messageBox -icon error -message "Failed to enable time subtraction in V1290 module $this"
	}
	
}

itcl::body ACAENV1290::DisableTriggerSubtraction {} {
	WriteOpcode 0x1500
	after 1000
	set check [ReadTriggerConfiguration triggerTimeSubtraction]
	if {![expr ($check&1) == 0]} {
		tk_messageBox -icon error -message "Failed to disable time subtraction in V1290 module $this"
	}	
}

itcl::body ACAENV1290::SetEdgeDetection {config} {
	# 00: pair mode; 01: trailing only; 10: leading only; 11: trailing & leading
	set config [expr $config&3]
	WriteOpcode 0x2200
	WriteOpcode $config
	after 1000
	WriteOpcode 0x2300
	set check [ReadOpcode]
	if {![expr ($check&3) == $config]} {
		tk_messageBox -icon error -message "Failed to set edge detection in V1290 module $this"
	}	
}
	
itcl::body ACAENV1290::SetResolution {code} {
	# only valid in trailing & leading edge detection modes
	# 00: 800 ps; 01: 200 ps; 10: 100 ps; 11: 25 ps (default)
	set code [expr $code&3]
	WriteOpcode 0x2400
	WriteOpcode $code
	after 1000
	WriteOpcode 0x2600
	set check [ReadOpcode]
	if {![expr ($check&3) == $code]} {
		tk_messageBox -icon error -message "Failed to set resolution in V1290 module $this"
	}	
}

itcl::body ACAENV1290::SetMaximumHits {hits} {
	# max hits is 2^(hits-1) up to hits=8; 9: no limit; >10: meaningless
	set code [expr $hits&0xf]
	WriteOpcode 0x3300
	WriteOpcode $code
	after 1000
	WriteOpcode 0x3400
	set check [ReadOpcode]
	if {![expr $check == $code]} {
		tk_messageBox -icon error -message "Failed to set maximum hits in V1290 module $this: code=$code; check=$check"
	}	
}

itcl::body ACAENV1290::EnableChannel {channel} {
	set channel [expr $channel&0x1f]
	set code [expr 0x4000+$channel]
	WriteOpcode $code
	after 1000
}

itcl::body ACAENV1290::DisableChannel {channel} {
	set channel [expr $channel&0x1f]
	set code [expr 0x4100+$channel]
	WriteOpcode $code
	after 1000
}

itcl::body ACAENV1290::EnableAllChannels {} {
	WriteOpcode 0x4200
	after 1000
}

itcl::body ACAENV1290::DisableAllChannels {} {
	WriteOpcode 0x4300
	after 1000
}

itcl::body ACAENV1290::EnableTDCMarkers {} {
	WriteOpcode 0x3000
	after 1000
	WriteOpcode 0x3200
	set check [ReadOpcode]
	if {![expr ($check&1) == 1]} {
		tk_messageBox -icon error -message "Failed to enable TDC markers in V1290 module $this"
	}	
}

itcl::body ACAENV1290::DisableTDCMarkers {} {
	WriteOpcode 0x3100
	after 1000
	WriteOpcode 0x3200
	set check [ReadOpcode]
	if {![expr ($check&1) == 0]} {
		tk_messageBox -icon error -message "Failed to disable TDC markers in V1290 module $this"
	}	
}

itcl::body ACAENV1290::EnableTDCErrors {} {
	WriteOpcode 0x3500
	after 1000
}

itcl::body ACAENV1290::DisableTDCErrors {} {
	WriteOpcode 0x3600
	after 1000
}

itcl::body ACAENV1290::InitReA3 {} {
	Reset
	SetTriggerMatchingMode
	SetMatchWindowWidth 1500
	SetWindowOffset -1000
	SetMaximumHits 1
	DisableTDCMarkers
	DisableTDCErrors
	EnableTriggerSubtraction
	SetControlRegister 0x21; # enable BERR, INL
}

# stack functions implementation
itcl::body ACAENV1290::sRead {stack} {
	$device sReadFBLT32 $stack $base 16 0; # Read up to 1024 x 32 bit words (64 per block)
}
