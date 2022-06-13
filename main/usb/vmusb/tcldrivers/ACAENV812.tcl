#===================================================================
# class ACAENV812
#===================================================================
itcl::class ACAENV812 {
	private variable device
	private variable base
	public variable enable
	public variable name
	public variable threshold
	public variable width
	public variable dead
	public variable majority
	
	constructor {de ba} {
		set device $de
		set base $ba
		# look for a config file
		if {[file exists "CAENV812_[string trimleft $this :].tcl"]} {
			source "CAENV812_[string trimleft $this :].tcl"
		} else {
			for {set i 0} {$i < 16} {incr i} {
				set enable($i) 1; # enable all channels
				set name($i) ""
				set threshold($i) 10; # about -10 mV
			}
			set width(0) 200; # roughly 50 ns wide
			set width(1) 200
			set dead(0) 0; # dead time 150 ns
			set dead(1) 0
			set majority 6; # Majority = 1 (nint(value*50-25)/4)
		}
		Init
	}
	
	destructor {}

# interactive functions
	public method GetVariable {v} {set $v}
	public method SaveSettings {}
	public method ControlPanel {}
	public method Control {channel}
	public method SetThreshold {channel} {$device Write16 [expr $base+$channel*2] $threshold($channel); SaveSettings}
	public method SetWidth {bank} {$device Write16 [expr $base+0x40+$bank*2] $width($bank); SaveSettings}
	public method SetDeadTime {bank} {$device Write16 [expr $base+0x44+$bank*2] $dead($bank); SaveSettings}
	public method SetMajority {} {$device Write16 [expr $base+0x48] $majority; SaveSettings}
	public method SetInhibit {value} {$device Write16 [expr $base+0x4a] $value}
	public method GetSerial {} {return [expr [$device Read16 [expr $base+0xfe]]&0xfff]}
	public method Init {}
}

itcl::body ACAENV812::Init {} {
	for {set i 0} {$i < 16} {incr i} {SetThreshold $i}
	Control 0
	SetWidth 0
	SetWidth 1
	SetDeadTime 0
	SetDeadTime 1
	SetMajority
}

itcl::body ACAENV812::Control {channel} {
	set mask 0
	for {set i 0} {$i < 16} {incr i} {
		incr mask [expr $enable($i)<<$i]
	}
	SetInhibit $mask
	SaveSettings
}

itcl::body ACAENV812::SaveSettings {} {
	set file [open "CAENV812_[string trimleft $this :].tcl" w]
	for {set i 0} {$i < 16} {incr i} {
		puts $file "set enable($i) $enable($i)"
		puts $file "set name($i) \"$name($i)\""
		puts $file "set threshold($i) $threshold($i)"
	}
	puts $file "set width(0) $width(0)"
	puts $file "set width(1) $width(1)"
	puts $file "set dead(0) $dead(0)"
	puts $file "set dead(1) $dead(1)"
	puts $file "set majority $majority"
	close $file
	return 1
}

itcl::body ACAENV812::ControlPanel {} {
	set serial [GetSerial]
	set w ".aCAENV812_[string trimleft $this :]"
	toplevel $w
	wm title $w "ACAENV812$this"
	for {set i 0} {$i < 16} {incr i} {
		checkbutton $w.enable$i -text "Ch$i" -variable [itcl::scope enable($i)] -command "$this Control $i" -bg #ffff40014001
		entry $w.name$i -textvariable [itcl::scope name($i)] -width 8 -validatecommand SaveSettings
		spinbox $w.threshold$i -textvariable [itcl::scope threshold($i)] -width 3 -increment 1 -from 1 -to 255 -repeatdelay 1000 -repeatinterval 100 -command "$this SetThreshold $i" -bg #ffff40014001
		grid $w.enable$i $w.name$i - $w.threshold$i -sticky news
	}
	label $w.wl0 -text "Width 0-7" -bg #ffff40014001
	spinbox $w.width0 -textvariable [itcl::scope width(0)] -width 3 -increment 1 -from 1 -to 255 -repeatdelay 1000 -repeatinterval 100 -command "$this SetWidth 0" -bg #ffff40014001
	label $w.wl1 -text "Width 8-15" -bg #ffff40014001
	spinbox $w.width1 -textvariable [itcl::scope width(1)] -width 3 -increment 1 -from 1 -to 255 -repeatdelay 1000 -repeatinterval 100 -command "$this SetWidth 1" -bg #ffff40014001
	grid $w.wl0 $w.width0 $w.wl1 $w.width1 -sticky news
	label $w.serial -text "Serial Number: $serial" -bg #ffff40014001
	grid $w.serial - - - -sticky news
}
