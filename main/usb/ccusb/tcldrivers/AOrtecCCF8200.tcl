#===================================================================
# class AOrtecCCF8200
#===================================================================
#
package provide ortecccf8200 11.0

package require Itcl
package require Utils
package require CCUSBDriverSupport


## @brief Low-level driver for the Ortec Camac CF8200 CFD
#
# This class provides the ability to do basic write, read, and control
# operations on a target device. It provides access to the entire function set
# supported by the module. 
#
itcl::class AOrtecCCF8200 {

	protected variable device ;#< a cccusb::CCCUSB object
	private variable node     ;#< slot in which module resides

  ## @brief Constructor
  #
  # @param de   a cccusb::CCCUSB object
  # @param no   the slot number
  #
  # @returns name of created object
	constructor {de no} {
		set device $de
		set node $no
	}
	
  ## @brief Destructor is a no-op
	destructor {}
	
  #---------------------------------------------------------------------------#
  # Composite commands
  #
 
  
  ## @brief Write all 8 threshold values
  #
  # Writes a list of threshold values to the corresponding channels of the
  # device. The first element of the list goes to channel 0 and the last element
  # to channel 7. It is required that the user provide all 8 threshold values.
  # If only a single threshold value needs setting, use the WriteThreshold
  # method.
  #
  # @param thres  list of 8 parameter values
  # 
  # @throws error if user provided a different number of elements than 8
  # @throws error if any of the parameter values are not in range [0,255]
  # @throws error if a valid written to device cannot be validated by a readback
  public method WriteThresholds {thres}

  ## @brief Write width for the A and B outputs
  #
  # This is just a convenience method for writing the width of A and B outputs
  # in one method. This just delegates to the WriteAWidth and WriteBWidth
  # methods
  #
  # @param widths list of 2 width values
  #
  # @throws error if parameter is not a list of 2 values
  # @throws error if either width value is not in range [0,255]
	public method WriteWidths {widths}


  #---------------------------------------------------------------------------#
  # Basic single-shot commands 
  #
  # - For the most part, these methods just use their stack building equivalent
  #   to the Execute method along with their arguments.
  # - On an error, all will map the context of the error reflect their own
  #   method name.
  #


  ## @brief Write theshold value for a channel
  #
  # @param ch     the target channel (must be in range [0,7])
  # @param thresh threshold value (must be in range [0,255])
  #
  # @returns QX response of write
  #
  # @throws error if ch is not in range [0,7]
  # @throws error if thresh is not in range [0,255]
  public method WriteThreshold {ch thresh} 

  ## @brief Write width for A outputs 
  #
  # @param width width value (must be in range [0,255])
  #
  # @returns QX response of write
  #
  # @throws error if width is not in range [0,255]
  public method WriteAWidth {width} 

  ## @brief Write width for B outputs 
  #
  # @param width width value (must be in range [0,255])
  #
  # @returns QX response of write
  #
  # @throws error if width is not in range [0,255]
  public method WriteBWidth {width} 


  ## @brief Write inhibit mask 
  #
  # This is the mechanism to disable channels. Each bit in the mask corresponds
  # to a channel in the device. If a bit is set, the corresponding channel is
  # inhibited. There are 8 channels so this must be an 8 bit value.
  #
  # @param mask channel bit mask (must be in range [0,255])
  #
  # @returns QX response of write
  #
  # @throws error if mask is not in range [0,255]
  public method WriteInhibitMask {mask} 

  ## @brief Read threshold value for a specific channel
  #
  # @param  ch  the target channel (must be in range [0,7])
  #
  # @returns value of threshold read and Q and X encoded in bits 24 and 25,
  #          respectively
  #
  # @throws error if ch is not in range [0,7]
  public method ReadThreshold {ch}

  ## @brief Read width of A outputs 
  #
  # @returns width value read and Q and X encoded in bits 24 and 25,
  #          respectively
  public method ReadAWidth {}

  ## @brief Read width of B outputs 
  #
  # @returns width value read and Q and X encoded in bits 24 and 25,
  #          respectively
  public method ReadBWidth {}

  ## @brief Read inhibit mask
  #
  # @returns inhibit mask and Q and X encoded in bits 24 and 25,
  #          respectively
  public method ReadInhibitMask {}


  ## @brief Clear the module - F(9)
  # 
  # @returns XQ response 
  public method Clear {}

  ## @brief Generate test signals - F(25) 
  # 
  # @returns XQ response 
  public method GenerateTest {}

  #---------------------------------------------------------------------------#
  # Stack building methods
  #
  #  - These are the real meat and potatoes of all the functionality. All
  #    single-shot methods rely on these.
  #

  ## @brief Adds threshold write to a stack
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object
  # @param ch     target channel (must be in range [0,7])
  # @param thresh threshold value (must be in range [0,255])
  #
  # @throws error if ch is not in range [0,7]
  # @throws error if thresh is not in range [0,255]
  public method sWriteThreshold {stack ch thresh} 

  ## @brief Adds command to set A output width to stack
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object
  # @param width  the width (must be in range [0,255])
  #
  # @throws error if width is not in range [0,255]
  public method sWriteAWidth {stack width} 

  ## @brief Adds command to set B output width to stack
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object
  # @param width  the width (must be in range [0,255])
  #
  # @throws error if width is not in range [0,255]
  public method sWriteBWidth {stack width} 

  ## @brief Adds command to write inhibit mask to stack
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object
  # @param width  the width (must be in range [0,255])
  #
  # @throws error if width is not in range [0,255]
  public method sWriteInhibitMask {stack mask} 

  ## @brief Adds a command to read a specific threshold value to a stack
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object
  # @param ch     the target channel (must be in range [0,7])
  #
  # @throws error if ch is not in range [0,7]
  public method sReadThreshold {stack ch} 

  ## @brief Adds a command to read the A output width to a stack
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object
  public method sReadAWidth {stack} 

  ## @brief Adds a command to read the B output width to a stack
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object
  public method sReadBWidth {stack} 

  ## @brief Adds a command to read the inhibit mask to a stack
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object
  public method sReadInhibitMask {stack} 


  ## @brief Adds a command to clear the device to a stack - F(9)
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object
  public method sClear {stack} 

  ## @brief Adds a command to generate test signals to a stack - F(25)A(0)
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object
  public method sGenerateTest {stack} 


  #---------------------------------------------------------------------------#
  # Utility methods

  ## @brief Access a private variables value
	public method GetVariable {v} {set $v}

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
  public method Execute {grouping script}


} ;# End of class

###############################################################################
# Single-shot implements

#
#
#
itcl::body AOrtecCCF8200::WriteThreshold {ch thresh} { 

  if {[catch {Execute 1 [list sWriteThreshold $ch $thresh]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::WriteThreshold]
    return -code error [join $err " "]
  }

  return $msg
}

#
#
#
itcl::body AOrtecCCF8200::WriteAWidth {width} { 

  if {[catch {Execute 1 [list sWriteAWidth $width]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::WriteAWidth]
    return -code error [join $err " "]
  }

  return $msg
}

#
#
#
itcl::body AOrtecCCF8200::WriteBWidth {width} { 

  if {[catch {Execute 1 [list sWriteBWidth $width]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::WriteBWidth]
    return -code error [join $err " "]
  }

  return $msg
}

#
#
#
itcl::body AOrtecCCF8200::WriteInhibitMask {mask} { 

  if {[catch {Execute 1 [list sWriteInhibitMask $mask]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::WriteInhibitMask]
    return -code error [join $err " "]
  }

  return $msg
}

#
#
#
itcl::body AOrtecCCF8200::ReadThreshold {ch} { 

  if {[catch {Execute 2 [list sReadThreshold $ch]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::ReadThreshold]
    return -code error [join $err " "]
  }

  return $msg
}

#
#
#
itcl::body AOrtecCCF8200::ReadAWidth {} { 

  if {[catch {Execute 2 [list sReadAWidth]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::ReadAWidth]
    return -code error [join $err " "]
  }

  return $msg
}

#
#
#
itcl::body AOrtecCCF8200::ReadBWidth {} { 

  if {[catch {Execute 2 [list sReadBWidth]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::ReadBWidth]
    return -code error [join $err " "]
  }

  return $msg
}


#
#
#
itcl::body AOrtecCCF8200::ReadInhibitMask {} { 

  if {[catch {Execute 2 [list sReadInhibitMask]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::ReadInhibitMask]
    return -code error [join $err " "]
  }

  return $msg
}

#
#
#
itcl::body AOrtecCCF8200::Clear {} { 

  if {[catch {Execute 1 [list sClear]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::Clear]
    return -code error [join $err " "]
  }

  return $msg
}

#
#
#
itcl::body AOrtecCCF8200::GenerateTest {} { 

  if {[catch {Execute 1 [list sGenerateTest]} msg]} {
    set err [lreplace $msg 0 0 AOrtecCCF8200::GenerateTest]
    return -code error [join $err " "]
  }

  return $msg
}

#-----------------------------------------------------------------------------#
# Stack building implementation
#
#

#
#
#
itcl::body AOrtecCCF8200::sWriteThreshold {stack ch thresh} {
  if {![Utils::isInRange 0 7 $ch]} {
    set msg {AOrtecCCF8200::sWriteThreshold Channel argument out of bounds. }
    append msg {Must be in range [0,7].}
    return -code error $msg
  }

  if {![Utils::isInRange 0 255 $thresh]} {
    set msg {AOrtecCCF8200::sWriteThreshold Threshold argument out of bounds. }
    append msg {Must be in range [0,255].}
    return -code error $msg
  }

  $stack addWrite24 $node $ch 16 $thresh
}


#
#
#
itcl::body AOrtecCCF8200::sWriteAWidth {stack width} {
  if {![Utils::isInRange 0 255 $width]} {
    set msg {AOrtecCCF8200::sWriteAWidth Width argument out of bounds. }
    append msg {Must be in range [0,255].}
    return -code error $msg
  }

  $stack addWrite24 $node 0 17 $width
}


#
#
#
itcl::body AOrtecCCF8200::sWriteBWidth {stack width} {
  if {![Utils::isInRange 0 255 $width]} {
    set msg {AOrtecCCF8200::sWriteBWidth Width argument out of bounds. }
    append msg {Must be in range [0,255].}
    return -code error $msg
  }

  $stack addWrite24 $node 1 17 $width
}


#
#
#
itcl::body AOrtecCCF8200::sWriteInhibitMask {stack mask} {
  if {![Utils::isInRange 0 0xff $mask]} {
    set msg {AOrtecCCF8200::sWriteInhibitMask Mask sets bits other than bits 0-7. }
    return -code error $msg
  }

  $stack addWrite24 $node 2 17 $mask
}


#
#
#
itcl::body AOrtecCCF8200::sReadThreshold {stack ch} {
  if {![Utils::isInRange 0 7 $ch]} {
    set msg {AOrtecCCF8200::sReadThreshold Channel argument out of bounds. }
    append msg {Must be in range [0,7].}
    return -code error $msg
  }

  $stack addRead24 $node $ch 0 0 ;# no lam wait
}



#
#
#
itcl::body AOrtecCCF8200::sReadAWidth {stack} {
  $stack addRead24 $node 0 1 0 ;# no lam wait
}


#
#
#
itcl::body AOrtecCCF8200::sReadBWidth {stack} {
  $stack addRead24 $node 1 1 0 ;# no lam wait
}


#
#
#
itcl::body AOrtecCCF8200::sReadInhibitMask {stack} {
  $stack addRead24 $node 2 1 0 ;# no lam wait
}


#
#
#
itcl::body AOrtecCCF8200::sClear {stack} {
  $stack addControl $node 0 9 
}

#
#
#
itcl::body AOrtecCCF8200::sGenerateTest {stack} {
  $stack addControl $node 0 25 
}



#-----------------------------------------------------------------------------#
# Complex commands

#
#
#
itcl::body AOrtecCCF8200::WriteThresholds {thres} {
  # enforce the list length
  if {[llength $thres] != 8} {
    set msg "AOrtecCCF8200::WriteThresholds Fewer than 8 threshold values "
    append msg "provided."
    return -code error $msg
  }


  # write all values to the device and check
	for {set i 0} {$i < [llength $thres]} {incr i} {

    set thr_i [lindex $thres $i] 

    # write
		WriteThreshold $i [lindex $thr_i $i]

    # read - the value returned contains QX in bits 24-25
		set rdback [ReadThreshold $i]
    set decodedRdBack [::CCUSBDriverSupport::lower24Bits $rdback]

    # validate 
    if {$decodedRdBack!=$thr_i} {
      set msg "AOrtecCCF8200::WriteThresholds Failed to read back value "
      append msg "written for channel $i. Wrote $thr_i, Read $decodedRdBack"
			return -code error $msg
		}

	}
}


#
#
#
itcl::body AOrtecCCF8200::WriteWidths {widths} {
  # enforce list length
  if {[llength $widths]!=2} {
    set msg "AOrtecCCF8200::WriteWidths List provided contains more or less "
    append msg "than 2 values." 
    return -code error $msg
  }

  # Write the widths without checking
	WriteAWidth [lindex $widths 0]
	WriteBWidth [lindex $widths 1]
}

# A utility to facility single-shot operation evaluation
#
# Given a list whose first element is a proc name and subsequent elements are
# arguments (i.e. procname arg0 arg1 arg2 ...) , this creates a stack
#
#  procname stack arg0 arg1 arg2 ...
#  
#
itcl::body AOrtecCCF8200::Execute {grouping script} {


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

}

