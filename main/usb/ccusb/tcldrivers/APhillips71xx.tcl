#===================================================================
# class APhillips71xx
#===================================================================
#

package provide phillips71xx 11.0

package require Itcl
package require cccusbreadoutlist
package require CCUSBDriverSupport
package require Utils 


## @brief Low-level driver from Phillips 71xx series digitizers
#
#
itcl::class APhillips71xx {
	private variable device   ;#< reference to USB controller
	private variable node     ;#< slot number (i.e. N)
	
  ## @brief Constructor
  #
  # @param de   a cccusb::CCCUSB object
  # @param no   the slot in which the module resides
  #
	constructor {de no} {
		set device $de
		set node $no
	}
	
	destructor {}
	

  public method SetController {ctlr} {
    set device $ctlr
  }

  public method GetController {} {
    return $device
  }
  # ---------------------------------------------------------------------------
  # Single-shot commands

  ## @brief Clear hit register, LAM, and data memory
  #
	public method Clear {} { Execute 1 sClear }

  ## @brief Enables LAM
  #
  # LAM is enabled on the S1 strobe.
  #
	public method EnableLAM {} { Execute 1 sEnableLAM }

  ## @brief Test LAM
  #
  # @returns QX code, (Q=1 if LAM enabled and present)
  #
	public method TestLAM {} { Execute 1 sTestLAM } 
  ## @brief Disables LAM
  #
  # LAM is disabled on the S2 strobe.
  #
	public method DisableLAM {} { Execute 1 sDisableLAM }

  ## @brief Write value to the control register
  #
  # Control register bits are as follows:
  #   bit 0    :   PED enable
  #   bit 1    :   Lower threshold enable
  #   bit 2    :   Upper threshold enable
  #   bits 7-3 :   Not meaningful and should be left as 0
  #   bits 15-8:   Conversion delay (read-only)
  #
  #   @param reg  value to write to register (should only use bits 0-2).
  #
	public method SetControlRegister {reg} { 
    Execute 1 [list sSetControlRegister $reg]
  }

  ## @brief Reset targeted bits in the control register bits to a value
  #
  # For any nonzero bit in the integer provided, the corresponding bit in the
  # control register is set to 0.
  #
  # @see SetControlRegister for the meaning of bits in the control register
  #
  # @param reg  value to write to register (should only use bits 0-2).
	public method ResetControlRegister {reg} { 
    Execute 1 [list sResetControlRegister $reg]
  }

  ## @brief Read the control register value
  #
  # This not only contains the information for the ped, lower thresh, and upper
  # thresh, but also will contain info on the conversion dealy in bits 8-15.
  #
  #  @returns register value with QX encoded in bits 24 and 24.
  #
	public method ReadControlRegister {} {
    return [Execute 2 [list sReadControlRegister]]
  }

  ## @brief Read the hit register data
  #
  # Set bits correspond to channels whose pedestal corrected values fall within
  # their upper and lower thresholds
  #
  # @returns the value of the hit register
  #
	public method ReadHitRegister {} {return [Execute 2 [list sReadHitRegister]]}

  ## @brief Read the data associated with a channel
  #
  # Reads the available data for a channel. 
  #
  # @param channel  the channel whose data is being requested
  #
  # @returns the value read back and the QX bits encoded in bits 24 and 25 
  #
	public method ReadChannel {channel} { return [Execute 2 [list sReadChannel]]}

  ## @brief Waits for LAM and then reads the data associated with a channel 
  #
  # Reads the available data for a channel once a LAM is made available from
  # the module.
  #
  # @param channel  the channel whose data is being requested
  #
  # @returns the value read back and the QX bits encoded in bits 24 and 25 
  #
	public method ReadChannelLAM {channel} { 
    return [Execute 2 [list sReadChannelLAM]]
  }

  ## @brief Immediately reads only the channels with "interesting" data 
  #
  # The channels with pedestal-corrected values above the lower and below the
  # upper threshold are read out and returned. These are the channels that
  # satisfy the characteristics defined by the user as worth being read out. 
  #
  # The data words returned have the channel encoded into bits 12-15 and the
  # 12-bit value in bits 0-11.
  #
  # @returns a list of the channel data
  #
	public method ReadSparse {}



  ## @brief Sets the module's programming mode
  #
  # This is useful for writing or reading the pedestal, lower thresh, and upper
  # thresh values. To write any of those values, you actually can bypass this
  # function and just call either SetPedestals, SetLowerThresholds, or
  # SetUpperThresholds b/c they implicitly call this function. However, if you
  # want to read back the values, you would use this method to choose what type
  # of data to access and then read the data using a subsequent call to
  # GetLastProgrammedData. The mapping for the mode are as follows:
  #
  # mode: 
  #   0 - pedestal
  #   1 - lower thresholds
  #   2 - upper thresholds
  #   4 - test mode
  #
  # @param mode   value indicating which type of data to access
  #
  public method SetProgrammingMode {mode} {
    return [Execute 1 [list sSetProgrammingMode $mode]]
  }

  ## @brief Write the pedestals to memory
  #
  # This implicitly calls the SetProgrammingMode with an argument of 0 and then 
  # writes the entire list of pedestal values. Provided a list of n pedestal
  # values, the method will write pedestals to the first n channels of the
  # device. If more than 16 pedestal values are provided, the excess values will
  # be ignored. The pedestals values must be provided in the range [-4095,4095].
  #
  #  @param peds  a list of pedestal values
  #
  #  @throws error if any one pedestal value is out of range
  public method SetPedestals {peds} {
    Execute 1 [list sSetPedestals $peds]
  }

  ## @brief Write the lower thresholds to memory
  #
  # This implicitly calls the SetProgrammingMode with an argument of 1 and then 
  # writes the entire list of threshold values. Provided a list of n threshold
  # values, the method will write to the first n channels of the
  # device. If more than 16 threshold values are provided, the excess values will
  # be ignored. The threshold values must be provided in the range [0,4095].
  #
  #  @param peds  a list of threshold values
  #
  #  @throws error if any one threshold value is out of range
  public method SetLowerThresholds {lth} {
    Execute 1 [list sSetLowerThresholds $lth]
  }

  ## @brief Write the upper thresholds to memory
  #
  # This implicitly calls the SetProgrammingMode with an argument of 2 and then 
  # writes the entire list of threshold values. Provided a list of n threshold
  # values, the method will write to the first n channels of the
  # device. If more than 16 threshold values are provided, the excess values will
  # be ignored. The threshold values must be provided in the range [0,4095].
  #
  #  @param peds  a list of threshold values
  #
  #  @throws error if any one threshold value is out of range
  public method SetUpperThresholds {uth} {
    Execute 1 [list sSetUpperThresholds $uth]
  }

  ## @brief Read 16 channels of data corresponding to last set programming mode
  #
  # this is to be used in conjunction with the SetProgrammingMode method. The
  # data returned from this method corresponds to the mode set previously by
  # SetProgrammingMode. For example, if the user call SetProgrammingMode with an
  # argument of 1 (i.e. lower thresholds), the 16 channels of lower threshold
  # values will be returned from this.
  #
  # @returns  list of 16 values corresponding to last set programming mode
  #
  public method GetLastProgrammedData {} {
    return [Execute 2 sGetLastProgrammedData]
  }

  #----------------------------------------------------------------------------#
  # Complex commands composed of other simple commands 
  #
  #
  #
  
  ## @brief Write the list of pedestals to the module
  #
  # This writes a list of pedestals to the module and ensures that they are set
  # properly by reading back the values written. Pedestal values are interpreted
  # as signed values.
  #
  # @param peds   a list of 16 pedestal values (must be 16 elements long)
  #
  # @throws error under following conditions
  #   - User a list with fewer than 16 pedestal values
  #   - Read-back of value failed to agree with the value written
  #
  public method WritePedestals {peds}

  ## @brief Write the list of lower threshold values to the module
  #
  # This writes a list of lower thresholds to the module and ensures that they
  # are set properly by reading back the values written. Lower threshold values
  # are interpreted as signed values. The first 16 threshold values in the list
  # are used and any remaining are ignored.
  #
  # @param llt  a list of at least 16 threshold values 
  #
  # @throws error under following conditions - User a list with fewer than 16
  # threshold values - Read-back of value failed to agree with the value written
  #
	public method WriteLowerThresholds {llt}

  ## @brief Write the list of upper threshold values to the module
  #
  # This writes a list of upper thresholds to the module and ensures that they
  # are set properly by reading back the values written. Upper threshold values
  # are interpreted as signed values. The first 16 threshold values in the list
  # are used and any remaining are ignored.
  #
  # @param ult  a list of at least 16 threshold values 
  #
  # @throws error under following conditions - User a list with fewer than 16
  # threshold values - Read-back of value failed to agree with the value written
  #
	public method WriteUpperThresholds {ult}


  #----------------------------------------------------------------------------#
  # Stack building methods
  #

  ## @brief Add command to stack to clear hit mask, LAM, and data memory
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object
	public method sClear {stack}

  ## @brief Add command to stack to enable the LAM
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object
  public method sEnableLAM {stack}

  ## @brief Add command to stack to enable the LAM
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object
  public method sTestLAM {stack}

  ## @brief Add command to stack to disable the LAM
  # @param 
  public method sDisableLAM {stack}

  ## @brief Add command to stack to write value to the control register
  # 
  #  For the meaning of bits in the control register @see SetControlRegister
  #
  #  @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
  public method sSetControlRegister {stack reg}

  ## @brief Add command to stack to rewrite in the control register
  # 
  #  For the meaning of bits in the control register @see SetControlRegister
  #
  #  @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
  public method sResetControlRegister {stack reg}

  ## @brief Add command to stack to read the control register
  # 
  #  @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
  public method sReadControlRegister {stack}

  ## @brief Add command to stack to read the hit register
  #
  #  @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
  public method sReadHitRegister {stack}

  ## @brief Add command to stack to read channel data immediately 
  #
  #  @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
	public method sReadChannel {stack channel}

  ## @brief Add command to stack to read channel data after LAM is present 
  #
  #  @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
	public method sReadChannelLAM {stack channel}


  ## @brief Add commands to stack to write pedestal values 
  #
  # @see SetPedestals for more information
  #
  #  @param stack a cccusbreadoutlist::CCCUSBReadoutList object 
  #  @param peds  list of pedestal values   
	public method sSetPedestals {stack peds}

  ## @brief Add commands to stack to write lower threshold values 
  #
  # @see SetLowerThresholds for more information
  #
  #  @param stack a cccusbreadoutlist::CCCUSBReadoutList object 
  #  @param peds  list of threshold values   
	public method sSetLowerThresholds {stack lth}

  ## @brief Add commands to stack to write upper threshold values 
  #
  # @see SetUpperThresholds for more information
  #
  #  @param stack a cccusbreadoutlist::CCCUSBReadoutList object 
  #  @param peds  list of threshold values   
	public method sSetUpperThresholds {stack uth}


  ## @brief Add commands to stack to read data for a programming mode 
  #
  # @see GetLastProgrammedData for more details.
  #
  # @param stack a cccusbreadoutlist::CCCUSBReadoutList object
	public method sGetLastProgrammedData {stack}

  ## @brief Add commands to stack to read data for a programming mode 
  #
  # @see SetProgrammingMode for more details.
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object
  # @param mode   index to select programming mode 
  #               (0-ped, 1-lowthresh, 2-upperthresh, 4-test)
  public method sSetProgrammingMode {stack mode}

  ## @brief Add commands to stack to read all data with meaningful values
  #
  # This uses the value returned from the hit register to determine which
  # channels have data between the lower and upper thresholds following
  # pedestal substraction. These channels are then read out from the module.
  #
  #  @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
	public method sReadSparse {stack}

  ## @brief Add commands to stack to read all data with significant values
  #
  # This is the very same thing as the sReadSparse method except that it waits
  # for the LAM to be present.
  #
  #  @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
	public method sReadSparseLAM {stack}




  # --------------------------------------------------------------------------#
  #
  # Utility methods



  ## @brief Process stack consisting of commands created by a method 
  #
  # Given a list whose first element is a proc name and subsequent elements are
  # arguments (i.e. procname arg0 arg1 arg2 ...) , this creates a stack
  #
  #  procname stack arg0 arg1 arg2 ...
  #  
  #  This is useful for converting the various stack building scripts into
  #  single-shot operations. For example the user can read the control register
  #  by calling
  #
  #  $obj Execute [list sReadControlRegister]
  #
  #  note that this is exactly how the single-shot operations are implemented.
  #
  #  @param script  list of form {procname arg0 arg1 arg2 ...}
  #
  #  @returns resulting data as a tcl list of 32-bit words  
  #
  private method Execute {grouping script}


  ## @brief Compare data read from the device with an expected set of values
  #
  # This handles the fact that data read from the device have an XQ value
  # encoded in bits 24 and 25. This method traverses both lists and compares the
  # lower 24 bits of each element for equality. 
  #
  # @param  expected  list of values to compare against
  # @param  readback  list of values to compare with possible XQ encoding
  #
  # @returns boolean
  # @retval 0 - at least one element differs
  # @retval 1 - lower 24-bits of each element in both list are identical
  public method ReadDataIsAsExpected {expected readback}


} ;# End of the APhillips71xx



#
# This only returns the first 12 bits rather than the entire 16 bits of 
# available data and the QX response.
#
itcl::body APhillips71xx::ReadChannel {channel} {
	set data [Execute 2 [list sReadChannel $channel]]
	return [expr {$data & 0xfff}]
}

#
# Reads and returns entire amount of data that is retrieved.
#
itcl::body APhillips71xx::ReadChannelLAM {channel} {
  set data [Execute 2 [list sReadChannelLAM $channel]]
	return $data
}

#itcl::body APhillips71xx::ReadSparse {} {
#  return [Execute 2 [list sReadSparse]]
#}



#-----------------------------------------------------------------------------#
#
# Composite methods


#
# Write the pedestals and ensure they get set
# 
itcl::body APhillips71xx::WritePedestals {peds} {

	if {[llength $peds] < 16} {
    set msg "APhillips71xx::WritePedestals Insufficient number of pedestals "
    append msg "provided."
		return -code error $msg
	}


  # Write the pedestals
  SetPedestals $peds
  # Read them back
  set data [GetLastProgrammedData]

  # check that the read back data is the same as what was written
  if {![ReadDataIsAsExpected $peds $data]} {
    set msg "APhillips71xx::WritePedestals Failed to set the pedestal values. "
    append msg "Read back different values than were written."
    return -code error $msg
  }

}

#
#
# Write the lower thresholds and ensure they get set
# 
itcl::body APhillips71xx::WriteLowerThresholds {llt} {
  # check that there are enough of thresholds 
  if {[llength $llt] < 16} {
    set msg "APhillips71xx::WriteLowerThresholds Insufficient number of "
    append msg "thresholds in provided. There must be at least 16."
    return -code error $msg
  }

  SetLowerThresholds $llt
  set data [GetLastProgrammedData]

  if {![ReadDataIsAsExpected $llt $data]} {
    set msg "APhillips71xx::WriteLowerThresholds Failed to set the lower "
    append msg "threshold values. Read back different values than were "
    append msg "written."
    return -code error $msg
  }
}

#
# Write the upper thresholds and ensure they get set
# 
itcl::body APhillips71xx::WriteUpperThresholds {ult} {
  if {[llength $ult] < 16} {
    set msg "Unsufficient number of thresholds in WriteUpperThresholds "
    append msg "of $this"
    return -code error $msg
  }

  SetUpperThresholds $ult
  set data [GetLastProgrammedData]

  if {![ReadDataIsAsExpected $ult $data]} {
    set msg "APhillips71xx::WriteUpperThresholds Failed to set the upper "
    append msg "threshold values. Read back different values than were "
    append msg "written."
    return -code error $msg
  }

}


#-----------------------------------------------------------------------------#
#
# Stack building methods
#
# These are so simple that they don't really need any explaining. We just
# add 1 or 2 commands to the stack. Done.
#


#
itcl::body APhillips71xx::sClear {stack} {
  $stack addControl $node 3 11 
}


itcl::body APhillips71xx::sEnableLAM {stack} {
  $stack addControl $node 0 26 
}

itcl::body APhillips71xx::sTestLAM {stack} {
  $stack addControl $node 0 8 
}

itcl::body APhillips71xx::sDisableLAM {stack} {
  $stack addControl $node 0 24
}


itcl::body APhillips71xx::sSetControlRegister {stack reg} {
  $stack addWrite24 $node 0 19 $reg
}


itcl::body APhillips71xx::sResetControlRegister {stack reg} {
  $stack addWrite24 $node 0 23 $reg
}


itcl::body APhillips71xx::sReadControlRegister {stack} {
  $stack addRead24 $node 0 6 
}


itcl::body APhillips71xx::sReadHitRegister {stack} {
  $stack addRead24 $node 1 6 
}


itcl::body APhillips71xx::sReadChannel {stack channel} {
	$stack addRead24 $node $channel 0
}

itcl::body APhillips71xx::sReadChannelLAM {stack channel} {
	$stack addRead24 $node $channel 0 1
}

itcl::body APhillips71xx::sReadSparse {stack} {
  $stack addAddressPatternRead16 $node 1 6 
  sReadChannel $stack 0 
}

itcl::body APhillips71xx::sReadSparseLAM {stack} {
  set lamWait 1
  $stack addAddressPatternRead16 $node 1 6 $lamWait
  sReadChannel $stack 0
}

itcl::body APhillips71xx::sSetProgrammingMode {stack mode} {
  $stack addWrite24 $node $mode 17 0
}


itcl::body APhillips71xx::sSetPedestals {stack peds} {

  # 
  # check to see if the pedestal values are sensible
  if {![Utils::listElementsInRange -4095 4095 $peds]} {
    set msg "APhillips71xx::sSetPedestals at least one pedestal value is out "
    append msg {of range. Must be in range [-4095,4095].}
    return -code error $msg
  }

  # in addWrite32, there is an implicit conversion to a uint32_t in
  # C++. A negative value in a 64-bit architecture is represented by 
  # a bunch of bytes when interpreted as a unsigned int is larged than
  # UINT32_MAX. This causes a swig exception. We thus have to be careful
  # to truncate properly to 32 bits for that conversion. We also
  # know that the module works with 16 bits twos-complement representations
  # so that we need to truncate to 16 bits. This at least what worked in
  # the S800DAQ implementation.
  set pedsSafeForUint32 [list]
  foreach ped $peds {
    lappend pedsSafeForUint32 [expr 0xffff&$ped] 
  }

  # the values were sensible so 
  sSetProgrammingMode $stack 0
  set nPeds [llength $pedsSafeForUint32]
  if {$nPeds>16} {set nPeds 16}

  for {set ch 0} {$ch<$nPeds} {incr ch} {
    set ped [lindex $pedsSafeForUint32 $ch]
    $stack addWrite24 $node $ch 20 $ped
  }
}

itcl::body APhillips71xx::sSetLowerThresholds {stack lth} {

  # check to see if the pedestal values are sensible
  if {![Utils::listElementsInRange 0 4095 $lth]} {
    set msg "APhillips71xx::sSetLowerThresholds at least one threshold value "
    append msg {is out of range. Must be in range [0,4095].}
    return -code error $msg
  }

  # set into lower threshold programming mode
  sSetProgrammingMode $stack 1
  set nThresh [llength $lth]
  if {$nThresh>16} {set nThresh 16}

  for {set ch 0} {$ch<$nThresh} {incr ch} {
    set thresh [lindex $lth $ch]
    $stack addWrite24 $node $ch 20 $thresh
  }
}


itcl::body APhillips71xx::sSetUpperThresholds {stack lth} {

  # check to see if the pedestal values are sensible
  if {![Utils::listElementsInRange 0 4095 $lth]} {
    set msg "APhillips71xx::sSetUpperThresholds at least one threshold value "
    append msg {is out of range. Must be in range [0,4095].}
    return -code error $msg
  }

  # set into lower threshold programming mode
  sSetProgrammingMode $stack 2
  set nThresh [llength $lth]
  if {$nThresh>16} {set nThresh 16}

  for {set ch 0} {$ch<$nThresh} {incr ch} {
    set thresh [lindex $lth $ch]
    $stack addWrite24 $node $ch 20 $thresh
  }
}


itcl::body APhillips71xx::sGetLastProgrammedData {stack} {
  for {set ch 0} {$ch < 16} {incr ch} {
    $stack addRead24 $node $ch 1
  }
}

# A utility to facility single-shot operation evaluation
#
# Given a list whose first element is a proc name and subsequent elements are
# arguments (i.e. procname arg0 arg1 arg2 ...) , this creates a stack
#
#  procname stack arg0 arg1 arg2 ...
#  
#
itcl::body APhillips71xx::Execute {grouping script} {

#ensure there is a device to execute the readout list
  if {$device ne ""} {

    # create a new readout list
    set rdoList [cccusbreadoutlist::CCCUSBReadoutList %AUTO%]

    # extract the proc we want to use to manipulate the stack
    set cmd [lindex $script 0]

    # if there are arguments provided, use them. otherwise, dont.
    if {[llength $script]>1} { 
      $cmd $rdoList {*}[lreplace $script 0 0] 
    } else {
      $cmd $rdoList 
    }

    # At this point the list will contain some commands added by the cmd
    # command

    # execute the list
    set data [$device executeList $rdoList [expr 4<<20]] 

    # the returned data is actually a std::vector<uin16_t> wrapped by swig. 
    # Convert this into a list of 32-bit integers and return it as a tcl list
    return [::CCUSBDriverSupport::shortsListToTclList data $grouping]
    
  } else { 
    set msg "APhillips71xx::Execute user must set the controller first with "
    append msg "SetController"
    return -code error $msg
  } 
}



itcl::body APhillips71xx::ReadDataIsAsExpected {expected readback} {

  # considered the same until proven different 
  set result 1 

  set nExpected [llength $expected]
  set nReadBack [llength $readback]

  # if they are different lengths, then readback is not what we expected.
  if {$nExpected != $nReadBack} {
    return 0
  }

  for {set ch 0} {$ch < $nExpected} {incr ch} {

    set readValue    [lindex $readback $ch]
    set writtenValue [lindex $expected $ch]

    # handle the fact that module supports signed 13-bit numbers
    set writtenValue [expr $writtenValue&0x1fff]
    set readValue    [expr $readValue&0x1fff]

    if {$writtenValue != $readValue} {
      set result 0
      break
    }
  }

  return $result
}
