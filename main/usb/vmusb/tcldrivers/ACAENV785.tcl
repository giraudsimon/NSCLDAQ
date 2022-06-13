#===================================================================
# class ACAENV785
# 
# v0.1 S.J.Williams Dec 5th 2012
#      Supports CAEN V785(N) 32(16) Channel VME ADC 
#===================================================================
itcl::class ACAENV785 {
    private variable device
    private variable base
    private variable version

    constructor {de ba} {
	set device $de
	set base $ba
    }

    destructor {}

# interactive functions
    public method GetVariable {v} {set $v}
    public method GetVersion {}
    public method EnableHiResThresholds {}
    public method SetThreshold {chan threshold}
    public method ReadThreshold {chan}
    public method SetThresholds {threshold}
    public method DisableChannel {channel}
    public method DisableChannels {start end}
    public method DataReset {}
    public method SoftwareReset {}
    public method SetReadoutMode {value}
    public method SetBitSet2 {bit}
    public method ClearBitSet2 {bit}
    public method ReadBitSet2 {}
    public method SetBitSet1 {bit}
    public method ClearBitSet1 {bit}
    public method ReadBitSet1 {}
    public method Init {}

# stack functions
    public method sRead {stack}
}

# interactive functions implementation
itcl::body ACAENV785::GetVersion {} {
    # Get the version type of the 785 board. 0x1N refers to ECL inputs,
    # 0xEN refers to NIM inputs (where N is any hex number). This version
    # type affects the register map for the thresholds. Since we don't 
    # care about the subversion, discard it.

    set ver [$device Read32D16 [expr $base+0x8032]]
    set version [expr $ver >> 4]
}

itcl::body ACAENV785::SetBitSet2 {bit} {
    # Set a bit of the BitSet2 register. A write of 0 to any other
    # bit with this call will not unset those bits.
    
    $device Write32D16 [expr $base+0x1032] [expr 1<<$bit]
}

itcl::body ACAENV785::ReadBitSet2 {} {
    # Read the current contents of the BitSet2 register
    
    set readback [$device Read32D16 [expr $base+0x1032]]
    return $readback
}

itcl::body ACAENV785::ClearBitSet2 {bit} {
    # Unset a bit of the BitSet2 register (requires a write of 1
    # to the bit of interest)

    $device Write32D16 [expr $base+0x1034] [expr 1<<$bit]
}
itcl::body ACAENV785::SetBitSet1 {bit} {
    # Set a bit of the BitSet1 register. A write of 0 to any other
    # bit with this call will not unset those bits.
    
    $device Write32D16 [expr $base+0x1006] [expr 1<<$bit]
}

itcl::body ACAENV785::ReadBitSet1 {} {
    # Read the current contents of the BitSet1 register
    
    set readback [$device Read32D16 [expr $base+0x1006]]
    return $readback
}

itcl::body ACAENV785::ClearBitSet1 {bit} {
    # Unset a bit of the BitSet1 register (requires a write of 1
    # to the bit of interest)

    $device Write32D16 [expr $base+0x1008] [expr 1<<$bit]
}
    

itcl::body ACAENV785::EnableHiResThresholds {} {
    # Enable (threshold << 1) comparison
    SetBitSet2 8
}
    
itcl::body ACAENV785::SetThreshold {chan threshold} {    
    # Sets lower level discriminator on ADC channel chan (referred to 
    # as zero suppression in manual). Action depends on firmware in card. 
    # From release 5.1 onward there are two resolution options, where the 
    # adc value is compared to either (threshold << 4) or (threshold << 1).
    # This option is dependant on if EnableHiResThresholds has been called.
    # The default is (threshold << 4).

    # check that we know if we have a V785 or a V785N
    if {$version == 0x1} {
	set regsize 2
    } elseif {$version == 0xE} {
	set regsize 4
    } else {
	tk_messageBox -icon error -message "Warning, the version type of V785 module $this is currently $version.\nCheck that ACAENV785::GetVersion has been called in the init script"
	return
    }
    
    $device Write32D16 [expr $base+0x1080+($chan*$regsize)] $threshold
}

itcl::body ACAENV785::ReadThreshold {chan} {
    # Read back lld
    # check that we know if we have a V785 or a V785N
    if {$version == 0x1} {
	set regsize 2
    } elseif {$version == 0xE} {
	set regsize 4
    } else {
	tk_messageBox -icon error -message "Warning, the version type of V785 module $this is currently $version.\nCheck that ACAENV785::GetVersion has been called in the init script"
	return
    }
    
    set thresh [$device Read32D16 [expr $base+0x1080+($chan*$regsize)]]
    return $thresh
}

itcl::body ACAENV785::SetThresholds {threshold} {
    # set all thresholds to a common value

    # check that we know if we have a V785 or a V785N
    if {$version == 0x1} {
	set nchan 32
    } elseif {$version == 0xE} {
	set nchan 16
    } else {
	tk_messageBox -icon error -message "Warning, the version type of V785 module $this is currently $version.\nCheck that ACAENV785::GetVersion has been called in the init script"
	return
    }
    set nchan 16
    for {set i 0} {$i < $nchan} {incr i} {
	SetThreshold $i $threshold
    }
}

itcl::body ACAENV785::DisableChannel {channel} {
    # Disable a channel by setting a kill bit.

    # check that we know if we have a V785 or a V785N
    if {$version == 0x1} {
	set regsize 2
    } elseif {$version == 0xE} {
	set regsize 4
    } else {
	tk_messageBox -icon error -message "Warning, the version type of V785 module $this is currently $version.\nCheck that ACAENV785::GetVersion has been called in the init script"
	return
    }

    $device Write32D16 [expr $base+0x1080+($channel*$regsize)] 0x100
}

itcl::body ACAENV785::DisableChannels {start end} {
    # Disable a range of channels.
    for {set i $start} {$i < [expr $start+$end]} {incr i} {
	DisableChannel $i
    }
}

itcl::body ACAENV785::DataReset {} {
    # Perform a data reset

    SetBitSet2 2
    ClearBitSet2 2
}

itcl::body ACAENV785::SoftwareReset {} {
    # Perform a software reset

    SetBitSet1 7
    ClearBitSet1 7
}


itcl::body ACAENV785::sRead {stack} {
    # Read data from the multiple event buffer (MEB). Needs to have
    # SetReadoutMode 0x0024 to enable single event, BERR generation
    # at end of event

    $device sReadSBLT32 $stack $base 34 0; # Read up to 34 x 32 bit words of data
}

itcl::body ACAENV785::SetReadoutMode {value} {
    # Set the single/full MEB readout and BERR generation modes
    set mode [expr $value&0xffff]
    $device Write32D16 [expr $base+0x1010] $value
}

itcl::body ACAENV785::Init {} {
    # Initialize card
    DataReset
    GetVersion
    SetReadoutMode 0x0024; # Enable BLOCK MODE AND BERR for single event block mode readout
}