#===================================================================
# class ALeCroy4300B
#===================================================================
#
#
package provide lecroy4300b 11.0

package require Itcl
package require Utils
package require CCUSBDriverSupport



itcl::class ALeCroy4300B {
	private variable device ;#< a reference to a cccusb::CCCUSB object
	private variable node   ;#< slot in which the module resides
	private variable self   ;#< name of this instance
	
  ## @brief Constructor 
  #
  # Store references of controller, slot, and instance name.
  #
  # @param de   name of a cccusb::CCCUSB object
  # @param no   slot number
  #
  # @returns name of the new instance
	constructor {de no} {
		set device $de
		set node $no
		set self [string trimleft $this :]
	}
	
  ## @brief Destructor - a no-op
	destructor {}
	
  ## @brief Set the controller
  #
  # @param ctlr   a cccusb::CCCUSB object
  public method SetController {ctlr}

  ## @brief Retrieve the name of the current controller 
  #
  # @returns a cccusb::CCCUSB object
  public method GetController {}



  ## @brief Clear the module to allow new gates 
  #
  # @returns xq response
	public method Clear {}

  ## @brief Write a pedestal for a specific channel to the device
  #
  # Pedestals must be values in the range [0,255].
  #
  # @param  ch  the target channel
  # @param  ped value to write 
  #
	public method SetPedestal {ch ped}

  ## @brief Write a list of pedestals to the device
  #
  # This writes a list of pedestal values to the device. The first element of
  # the list will be passed for channel 0, the second for channel 1, and so on.
  # The user must provide a value for all 16 channels.
  #
  # @param peds a list of 16 pedestal values
  #
  # @throws error if the user provides fewer than 16 pedestal values
	public method SetPedestals {peds}

  public method GetPedestal {ch}
  public method GetPedestals {}

  ## @brief Write an array of threshold values
  #
  # Provided the name of an array containing the pedestal values, write the
  # pedestals to the device. This is ultimately a front-end to the SetPedestals
  # method that converts an array to a list and then passes the created list to
  # the SetPedestals method. The array provided must contain indices with the
  # following form ped%.2d (i.e. ped.00, ped.01, ...).
  #
  # @param  arrname name of array 
  #
  # @throws error if any of the following are true
  #   - array name provided does not exist
  #   - array does not contain 16 elements named ped%.2d
  #
	public method WritePedestals {{arrname SCINT_ENERGY}}

  ## @brief Write a value to the control register 
  #
  # @param  control value to write 
  #
  # @returns the xq response
	public method SetControlRegister {control}

  ## @brief Read the control register 
  #
  # @returns the control rogister with xq encoded in bits 24 and 25
	public method GetControlRegister {}



  public method ReadLAM {} 
  public method ReadAndClearLAM {} 


  #---------------------------------------------------------------------------#
  # Compound methods
  
  ## @brief Initialize the module 
  #
  # The following modes are set:
  # CAMAC LAM enabled; 
  # CAMAC sequential read; 
  # CAMAC Compression enable
  # CAMAC Pedestal subtraction enabled
  # 
	public method Init {} {
    set CPS [expr 1<<11] ;#< ped subtraction
    set CCE [expr 1<<12] ;#< compression enable 
    set CSR [expr 1<<13] ;#< sequential readout
    set CLE [expr 1<<14] ;#< LAM enable

    set mode [expr {$CLE | $CSR | $CCE | $CPS}]
    SetControlRegister $mode
  }

  #---------------------------------------------------------------------------#
  # Stack methods
  
  ## @brief Add command to stack to clear the module for accepting new gates
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList
	public method sClear {stack}

  ## @brief Add command to stack to read a value from the stack 
  #
  # This is only meaningful if the module is configured for data compression.
  # Simply adds a read24 of NA(0)F(2)
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList
	public method sReadHeader {stack}

  ## @brief Add command to stack to read a specific channel from the device
  #
  # Simply adds a read24 of NA(channel)F(2)
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList
	public method sReadChannel {stack channel}

  ## @brief Add command to stack to perform a qscan of maximum length 17
  #
  # @todo CHECK THAT THIS CAN ACTUALLY READ ALL 16 CHANNELS + header
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList
	public method sReadSparse {stack}

  ## @brief Adds commands to the stack to write 16 pedestal values 
  #
  # Pedestals provided are assigned to channels with the corresponding index.
  # For example, element 0 is written to channel 0, element 1 is written to
  # channel 1, ...
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList
  # @param peds   a list of 16 pedestal values
  #
  # @throws error if 
  #  - there are fewer than 16 pedestal values provided
  #  - any of the pedestal values provided are outside of range [0,255]
  public method sSetPedestals {stack peds}
  public method sGetPedestals {stack}

  ## @brief Adds commands to the stack to write a single pedestal value 
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList
  # @param peds   a list of 16 pedestal values
  public method sSetPedestal {stack ch ped}
  public method sGetPedestal {stack ch}


  ## @brief Add command to stack to write a value to the control register
  #
  # @param stack  a cccusbreadoutList::CCCUSBReadoutList
  # @param regval the value to write to the register
  #
  public method sSetControlRegister {stack regval}

  ## @brief Add command to stack to read the control register
  #
  # @param stack  a cccusbreadoutList::CCCUSBReadoutList
  #
  public method sGetControlRegister {stack}


  public method sReadLAM {stack}
  public method sReadAndClearLAM {stack}



  ## @brief Process stack consisting of commands created by a method 
  #
  # Given a list whose first element is a proc name and subsequent elements are
  # arguments (i.e. procname arg0 arg1 arg2 ...) , this creates a stack
  #
  #  procname stack arg0 arg1 arg2 ...
  #  
  #  This is useful for converting the various stack building scripts into
  #  single-shot operations. For example the user can set the QDC gate width to
  #  200 by calling 
  #
  #  Execute [list sSetQDCGateWidth 200]
  #
  #  note that this is exactly how the single-shot operations are implemented.
  #
  #  Ultimately, this calls cccusb::CCCUSB::executeList, which returns a swig-
  #  wrapped vector<uint16_t>. This is converted into a standard tcl list but 
  #  the user needs to provide some info to help the conversion. A write command
  #  will cause there to only be 16-bits of data returned such that a list of
  #  32-bit integers cannot be formed by combining an even number of uint16_ts.
  #  However, any read24 command will return 32-bit numbers so that every other
  #  uint16_t should be combined into a single uint32_t integer. For this, the
  #  user must provide the grouping parameter. This specifies how many uint16_t
  #  should be combined to form an element in the resulting Tcl list. This
  #  should typically be 1 (writes) or 2 (reads).
  #
  #  @param grouping  how to group uint16_ts to form resulting tcl list 
  #  @param script    list of form {procname arg0 arg1 arg2 ...}
  #
  #  @returns resulting data from the stack execution as a tcl list of 32-bit words
  #
  public method Execute {grouping script} 


} ;# End of itcl::class


#--------------------------------------------------------------------
# Single shot method implementation
#

#
#
#
itcl::body ALeCroy4300B::SetController {ctlr} {
  set device $ctlr
}

#
#
#
itcl::body ALeCroy4300B::GetController {} {
  return $device
}

#
#
#
itcl::body ALeCroy4300B::SetPedestals {peds} {
  set res [catch {Execute 1 [list sSetPedestals $peds]} msg]
  if {$res} {
    return -code error [lreplace $msg 0 0 ALeCroy4300B::SetPedestals]
  }
  return $msg
}

#
#
#
itcl::body ALeCroy4300B::GetPedestals {} {
  return [Execute 2 [list sGetPedestals]]
}

#
#
#
itcl::body ALeCroy4300B::SetPedestal {ch peds} {
  set res [catch {Execute 1 [list sSetPedestal $ch $peds]} msg]
  if {$res} {
    set msg [lreplace $msg 0 0 ALeCroy4300B::SetPedestal]
    # we have to join this to avoid the [0,255] becomming "quoted" as a single
    # independent string 
    return -code error [join $msg " "]
  }
  return $msg
}

#
#
#
itcl::body ALeCroy4300B::GetPedestal {ch} {
  set res [catch {Execute 2 [list sGetPedestal $ch]} msg]
  if {$res} {
    set msg [lreplace $msg 0 0 ALeCroy4300B::GetPedestal]
    # we have to join this to avoid the [0,255] becomming "quoted" as a single
    # independent string 
    return -code error [join $msg " "]
  }
  return $msg
}

#
#
#
itcl::body ALeCroy4300B::Clear {} {
  return [Execute 1 [list sClear]]
}



#
#
#
itcl::body ALeCroy4300B::WritePedestals {{arrname SCINT_ENERGY}} {
	global $arrname 

  # check that the array exists
	if {![info exists $arrname]} {
    set msg "ALeCroy4300B::WritePedestals Cannot find array $arrname for $self"
		return -code error $msg
	}

  # ensure that the array has at least 16 elements
  set names [array names $arrname]
  if {[llength $names]<16} {
    set msg "ALeCroy4300B::WritePedestals Array contains fewer than 16 "
    append msg "elements."
    return -code error $msg
  }

  # convert array into an equivalent list
	for {set i 0} {$i < 16} {incr i} {
    set elpair [array get $arrname [format "ped%.2d" $i]]
    lappend peds [lindex $elpair 1] 
  }

  # set the pedestals
	return [SetPedestals $peds]
}

#
#
#
itcl::body ALeCroy4300B::SetControlRegister {regval} {
  return [Execute 1 [list sSetControlRegister $regval]]
}


#
#
#
itcl::body ALeCroy4300B::GetControlRegister {} {
  return [Execute 2 [list sGetControlRegister]]
}


itcl::body ALeCroy4300B::ReadLAM {} {
  return [Execute 1 [list sReadLAM]]
}

itcl::body ALeCroy4300B::ReadAndClearLAM {} {
  return [Execute 1 [list sReadAndClearLAM]]
}

#-----------------------------------------------------------------------------#
# Stack building method implementations
#

#
#
#
itcl::body ALeCroy4300B::sClear {stack} {
  $stack addControl $node 0 9	
}

#
#
#
itcl::body ALeCroy4300B::sReadHeader {stack} {
  $stack addRead24 $node 0 2
}

#
#
#
itcl::body ALeCroy4300B::sReadChannel {stack channel} {
  $stack addRead24 $node $channel 2
}

#
#
#
itcl::body ALeCroy4300B::sReadSparse {stack} {
  $stack addQStop $node 0 2 17 1
}

#
#
#
itcl::body ALeCroy4300B::sSetPedestals {stack peds} {
	if {[llength $peds] < 16} {
    set msg "ALeCroy4300B::sSetPedestals Fewer than 16 pedestal values "
    append msg  "provided."
		return -code error $msg
	}

	for {set i 0} {$i < 16} {incr i} {
    sSetPedestal $stack $i [lindex $peds $i]
	}
}

#
#
#
itcl::body ALeCroy4300B::sGetPedestals {stack} {
	for {set i 0} {$i < 16} {incr i} {
    sGetPedestal $stack $i
	}
}
#
#
#
itcl::body ALeCroy4300B::sSetPedestal {stack ch ped} {
  if {![Utils::isInRange 0 255 $ped]} {
    set msg "ALeCroy4300B::sSetPedestal Pedestal value provided is out of "
    append msg {range. Must be in range [0,255].}
    return -code error $msg
  }

  $stack addWrite24 $node $ch 17 $ped 
}

#
#
#
itcl::body ALeCroy4300B::sGetPedestal {stack ch} {
  if {![Utils::isInRange 0 15 $ch]} {
    set msg "ALeCroy4300B::sGetPedestal Channel index provided is out of range. " 
    append msg {Must be in range [0,15].}
    return -code error $msg
  }

  # read the pedestal and don't wait for a LAM
  $stack addRead24 $node $ch 1 0 
}

#
#
#
itcl::body ALeCroy4300B::sSetControlRegister {stack regval} {
  $stack addWrite24 $node 0 16 $regval
}

#
#
#
itcl::body ALeCroy4300B::sGetControlRegister {stack} {
  # read NA(0)F(0) and don't wait for LAM
  $stack addRead24 $node 0 0 0 
}


#
#
#
itcl::body ALeCroy4300B::sReadLAM {stack} {
  $stack addControl $node 0 8
}

#
#
#
itcl::body ALeCroy4300B::sReadAndClearLAM {stack} {
  $stack addControl $node 0 10
}



# A utility to facility single-shot operation evaluation
#
itcl::body ALeCroy4300B::Execute {grouping script} {
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
    set msg "ATrigget2367::Execute user must set the controller first with "
    append msg "SetController"
    return -code error $msg
  } 
}
