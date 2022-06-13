
#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#       NSCL DAQ Development Group 
#       NSCL
#       Michigan State University
#       East Lansing, MI 48824-1321
# @author Jeromy Tompkins and Daniel Bazin
#
# @note The original version of this code was written by Daniel Bazin
#       and it has been modified extensively to work with the 
#       NSCLDAQ libraries. 


# Define the package
package provide trigger2367 11.0

package require Itcl
package require cccusbreadoutlist
package require CCUSBDriverSupport

## @class ATrigger2367
#
# This is the low-level driver to support the usbtrig.bit firmware loaded into
# a LeCroy Universal Logic Module 2367. The firmware is written and maintained
# by Daniel Bazin at the National Superconducting Cyclotron Laboratory and
# corresponds to the usbtrig.bit file. The firmware that this supports provides
# trigger logic for handling multiple different raw triggers and choosing what
# will constitute the formation of a live trigger. The user can sample raw
# triggers as well as delay them. A GUI for controlling the configuration of
# the firmware is provided as a client to the CCUSBReadout slow controls
# server. It is possible for the user to manipulate the device in tcl scripts
# through the readoutscript or controlscript packages or through an
# interpreter.
#
# Because this device is used for both building stacks and executing commands
# in interactive mode, it is not required that the user provide a valid
# reference to a controller at construction. One reason for this is if a
# version is created specifically for building stacks. In that scenario, there
# is no interaction with a controller ever and there may not be a live
# cccusb::CCCUSB object to use. So to get around this, all methods that will
# use a controller check to see that the user has provided a valid instance. It
# is the user's responsibility to ensure that this is up to date because the
# stored reference will persist after the cccusb::CCCUSB object has been
# destroyed.
#
#
itcl::class ATrigger2367 {
	private variable device     ;#< a reference to the cccusb::CCCUSB object
	private variable node       ;#< the slot of the ULM
	private variable self       ;#< the name of the instance

	

  ## @brief Constructor
  #
  # @param de   a cccusb::CCCUSB object. If this is not provided at 
  #             construction, then it can be defined later with the
  #             SetController method.
  # @param no   slot where ULM device resides.
  #
  # @returns name of the instance 
	constructor {de no} {
		set device $de 
		set node $no
		set self [string trimleft $this :]
	}
	
  ## @brief a no-op
	destructor {}

  # --------------- Interactive methods ---------------------------

 
  ## @brief Store a reference to a controller for later one-shot operations
  # 
  # Because it is not demanded that the constructor accepts a valid ctlr
  # object, the user must ensure that a valid controller is provided before 
  # calling the various interactive methods. 
  # 
  # @param ctlr   a cccusb::CCCUSB object
  # 
  # @return ""
  public method SetController {ctlr}

  ## @brief Retrieve the current controller
  # 
  # @return the name of the controller this communicates through
  public method GetController {}
  
  ## @brief Load firmware into the ULM
  #
  # @param firmware   a path to a valid firmware file
  #
  # @returns ""
  #
  # Exceptional returns:
  # - Error if firmware file doesn't exit
  # - Error if firmware file cannot be read
  # - Error if user has not previously set a valid ctlr
  # - Error if the signature values fail to be validated after the load 
	public method Configure {firmware}


  ## @brief Read firmware signature
  #
  # If the firmware has loaded properly, this should return the number
  # 5800. 
  #
  # @returns firmware signature
  #
  # Exceptional returns:
  # - Error if the user has not set the controller.
  public method ReadSignature1 {}

  ## @brief Read hardware model
  #
  # If the firmware is loaded properly, this should return the number 2367.
  #
  # @returns the hardware model
  #
  # Exceptional returns:
  # - Error if the user has not set the controller.
  public method ReadSignature2 {}

  ## @brief Clear the module
  # 
  # I believe that this actually clears the trigger register. 
  # Not sure what else it clears...
  #
  # @returns integer encoded with Q and X response 
  #
  # Exceptional returns:
  # - Error if the user has not set the controller.
  public method Clear {} 

  ## @brief Manipulate the GO bit
  #
  # When the Go bit is set to 1, the ULM can output triggers and a clock.
  # If it is 0, it does not accept any raw triggers.
  #
  # @param bit  the value to set the trigger bit to
  # 
  # @returns integer encoded with Q and X response 
  #
  # Exceptional returns:
  # - Error if user passes a non-boolean value 
  # - Error if user has not set the controller 
  # 
	public method Go {bit} 

  ## @brief Read status of the Go bit
  #
  # @returns integer encoded with Q and X response 
  #
  # Exceptional returns:
  # - Error if user has not set the controller 
  # 
	public method ReadGo {} 


  ## @brief Select timestamp mode
  # 
  # Sets whether the ULM will accept an external timestamp clock and external timestamp latch.
  #
  # bit 0 : accepts an external timestamp clock
  # bit 1 : accepts an external timestamp latch
  #
  # To use an external clock, the code should be 1 and to use an internal 
  # clock, the code should be 0.
  #
  # @important if the code is different than 0, the ULM will
  #            NOT output a clock signal!
  #
  # @param code  the code to write. must be in range [0,3]
  #
  # @returns integer encoded with Q and X response 
  #
  # Exceptional returns:
  # - Error is user has not set the controller.
  # - Error if argument is out of range.
  public method Select {code}

  ## @brief Read timestamp timestamp mode
  #
  # @returns integer encoded with tstamp mode and Q and X response 
  #
  # Exceptional returns:
  # - Error is user has not set the controller.
  public method ReadSelect {} 

  ## @brief Enable ... 
  #
  # @param bits value to write
  #
  # @returns integer encoded with Q and X response 
  #
  # Exceptional returns:
  # - Error is user has not set the controller 
  #
  public method Enable {bits} 


  ## @brief Check that the 2 signatures are 2367 and 5800. 
  # 
  # @return boolean
  # 
  public method IsConfigured {}


  ## @brief Set S800 GDG delay
  #
  public method SetS800GDGDelay {val} {
    Execute 1 [list sSetS800GDGDelay $val]
  } 
  public method GetS800GDGDelay {} {
    return [Execute 2 [list sGetS800GDGDelay]]
  }

  ## @brief Set S800 GDG width 
  #
  public method SetS800GDGWidth {val} {
    Execute 1 [list sSetS800GDGWidth $val]
  } 
  public method GetS800GDGWidth {} {
    return [Execute 2 [list sGetS800GDGWidth]]
  }


  ## @brief Set S800 GDG delay
  #
  public method SetSecondaryGDGDelay {val} {   
    Execute 1 [list sSetSecondaryGDGDelay $val]
  } 
  public method GetSecondaryGDGDelay {} {
    return [Execute 2 [list sGetSecondaryGDGDelay]]
  }

  ## @brief Set S800 GDG width 
  #
  public method SetSecondaryGDGWidth {val} {
    Execute 1 [list sSetSecondaryGDGWidth $val]
  } 
  public method GetSecondaryGDGWidth {} {
    return [Execute 2 [list sGetSecondaryGDGWidth]]
  }

  ## @brief Set S800 delay 
  #
  public method SetS800Delay {val} {
    Execute 1 [list sSetS800Delay $val]
  } 
  public method GetS800Delay {} {
    return [Execute 2 [list sGetS800Delay]]
  }

  ## @brief Set Coincidence gate width
  #
  public method SetCoincidenceWidth {val} {
    Execute 1 [list sSetCoincidenceWidth $val]
  } 
  public method GetCoincidenceWidth {} {
    return [Execute 2 [list sGetCoincidenceWidth]]
  }

  ## @brief Set Secondary trigger delay
  #
  public method SetSecondaryDelay {val} {
    Execute 1 [list sSetSecondaryDelay $val]
  } 
  public method GetSecondaryDelay {} {
    return [Execute 2 [list sGetSecondaryDelay]]
  }
  
  ## @brief Set bypasses
  #
  public method SetBypasses {val} { 
    Execute 1 [list sSetBypasses $val]
  } 
  public method GetBypasses {} {
    return [Execute 2 [list sGetBypasses]]
  }

  ## @brief Set S800 Downscaler
  public method SetS800DownscaleFactor {factor} {
    Execute 1 [list sSetS800DownscaleFactor $factor]
  } 
  public method GetS800DownscaleFactor {} { 
    return [Execute 2 [list sGetS800DownscaleFactor]]
  }

  ## @brief Set Secondary Downscaler
  public method SetSecondaryDownscaleFactor {factor} {
    Execute 1 [list sSetSecondaryDownscaleFactor $factor]
  } 
  public method GetSecondaryDownscaleFactor {} { 
    return [Execute 2 [list sGetSecondaryDownscaleFactor]]
  }

  ## @brief Set S800 Downscaler
  public method SetTriggerBox {bitpattern} {
    Execute 1 [list sSetTriggerBox $bitpattern]
  } 
  public method GetTriggerBox {} {
    return [Execute 2 [list sGetTriggerBox]]
  }

#  ## @brief Set Inspect 1 
#  public method SetInspect1 {wire}
#  public method GetInspect1 {}
#  ## @brief Set Inspect 2 
#  public method SetInspect2 {wire}
#  public method GetInspect2 {}
#  ## @brief Set Inspect 3 
#  public method SetInspect3 {wire}
#  public method GetInspect3 {}
#  ## @brief Set Inspect 4 
#  public method SetInspect4 {wire}
#  public method GetInspect4 {}
#

  ## @brief Set ADC Gate width
  #
  public method SetADCGateWidth {wid} {
    Execute 1 [list sSetADCGateWidth $wid]
  } 
  public method GetADCGateWidth {} { 
    return [Execute 2 [list sGetADCGateWidth]]
  }

  ## @brief Set QDC Gate width
  #
  public method SetQDCGateWidth {wid} {
    Execute 1 [list sSetQDCGateWidth $wid]
  } 
  public method GetQDCGateWidth {} {
    return [Execute 2 [list sGetQDCGateWidth]]
  }

  ## @brief Set TDC Gate width
  #
  public method SetTDCGateWidth {wid} {
    Execute 1 [list sSetTDCGateWidth $wid]
  } 
  public method GetTDCGateWidth {} {
    return [Execute 2 [list sGetTDCGateWidth]]
  }


  ## @brief Set CoincReg Gate width
  #
  public method SetCoincRegGateWidth {wid} {
    Execute 1 [list sSetCoincRegGateWidth $wid]
  } 
  public method GetCoincRegGateWidth {} {
    return [Execute 2 [list sGetCoincRegGateWidth]]
  }


  ##################################################################
  #                                                                #
  # ----------------- stack methods -------------------------------#
  #                                                                #
  ##################################################################

  ## @brief Adds a clear operation to the stack
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
  #
  # @return ""
	public method sClear {stack}

  ## @brief Adds a clear register operation to the stack
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
  #
  # @return ""
	public method sClearRegister {stack}


  ## @brief Adds a read register operation to the stack
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
  #
  # @return ""
  public method sRead {stack}


  ## @brief Adds a series of reads to the stack for the 64-bit timestamp 
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
  #
  # @return ""
	public method sStamp {stack}

  

  ## @brief Set S800 GDG delay
  #
  public method sSetS800GDGDelay {stack val}   
  public method sGetS800GDGDelay {stack }   

  ## @brief Set S800 GDG width 
  #
  public method sSetS800GDGWidth {stack val}   
  public method sGetS800GDGWidth {stack }   


  ## @brief Set S800 GDG delay
  #
  public method sSetSecondaryGDGDelay {stack val}   
  public method sGetSecondaryGDGDelay {stack }   

  ## @brief Set S800 GDG width 
  #
  public method sSetSecondaryGDGWidth {stack val}   
  public method sGetSecondaryGDGWidth {stack }   

  ## @brief Set S800 delay 
  #
  public method sSetS800Delay {stack val}   
  public method sGetS800Delay {stack }   

  ## @brief Set Coincidence gate width
  #
  public method sSetCoincidenceWidth {stack val}
  public method sGetCoincidenceWidth {stack }

  ## @brief Set Secondary trigger delay
  #
  public method sSetSecondaryDelay {stack val}
  public method sGetSecondaryDelay {stack }

  ## @brief Set bypasses
  #
  public method sSetBypasses {stack val} 
  public method sGetBypasses {stack } 

  ## @brief Set S800 Downscaler
  public method sSetS800DownscaleFactor {stack factor}
  public method sGetS800DownscaleFactor {stack}

  ## @brief Set Secondary Downscaler
  public method sSetSecondaryDownscaleFactor {stack factor}
  public method sGetSecondaryDownscaleFactor {stack}

  ## @brief Set S800 Downscaler
  public method sSetTriggerBox {stack bitpattern}
  public method sGetTriggerBox {stack}

  ## @brief Set ADC Gate width
  #
  public method sSetADCGateWidth {stack wid}
  public method sGetADCGateWidth {stack }

  ## @brief Set QDC Gate width
  #
  public method sSetQDCGateWidth {stack wid}
  public method sGetQDCGateWidth {stack }

  ## @brief Set QDC Gate width
  #
  public method sSetTDCGateWidth {stack wid}
  public method sGetTDCGateWidth {stack }

  ## @brief Set Coinc Register Gate width
  #
  public method sSetCoincRegGateWidth {stack wid}
  public method sGetCoincRegGateWidth {stack }


  # --------------- Utility method --------------------------------------------

  
	public method GetVariable {v} {set $v}

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
}

# --- END OF INTERFACE ---

#######################################################
#######################################################

# IMPLEMENTATIONS


#
#
#
itcl::body ATrigger2367::SetController {ctlr} {
  set device $ctlr 
}

#
#
#
itcl::body ATrigger2367::GetController {} {
  return $device
}

#
#
#
itcl::body ATrigger2367::Clear {} {
  if {$device ne ""} {
    return [$device simpleControl $node 0 9]
  } else {
    set msg "ATrigget2367::Clear user must set the controller first with "
    append msg "SetController" 
    return -code error $msg
  }
}

#
#
#
itcl::body ATrigger2367::Go {bit} {
  if {![string is boolean $bit]} {
    return -code error "ATrigger2367::Go passed a non-boolean value"
  }

  set value [string is true $bit]

  if {$device ne ""} {
    return [$device simpleWrite24 $node 11 16 $value]
  } else {
    set msg "ATrigget2367::Go user must set the controller first with "
    append msg "SetController" 
    return -code error $msg
  }
}

#
#
#
itcl::body ATrigger2367::ReadGo {} {
  if {$device ne ""} {
    return [$device simpleRead24 $node 11 0]
  } else {
    set msg "ATrigget2367::ReadGo user must set the controller first with "
    append msg "SetController" 
    return -code error $msg
  }
}

#
#
#
itcl::body ATrigger2367::Select {code} {
  if {$code<0 || $code>3} {
    return -code error "ATrigger2367::Select passed a value of range."
  }
  if {$device ne ""} {
    return [$device simpleWrite24 $node 12 16 $code]
  } else {
    set msg "ATrigget2367::Select user must set the controller first with "
    append msg "SetController" 
    return -code error $msg
  }
}

#
#
#
itcl::body ATrigger2367::ReadSelect {} {
  if {$device ne ""} {
    return [$device simpleRead24 $node 12 0]
  } else {
    set msg "ATrigget2367::ReadSelect user must set the controller "
    append msg "first with SetController" 
    return -code error $msg 
    
  }
}
#
#
#
itcl::body ATrigger2367::Enable {bits} {
  if {$device ne ""} {
    return [$device simpleWrite24 $node 13 16 $bits]
  } else {
    set msg "ATrigget2367::Select user must set the controller first with "
    append msg "SetController" 
    return -code error $msg
  }
}


#
#
#
itcl::body ATrigger2367::ReadSignature1 {} {
  if {$device ne ""} {
    return [$device simpleRead24 $node 14 0]
  } else {
    set msg "ATrigget2367::ReadSignature1 user must set the controller first "
    append msg "with SetController" 
    return -code error $msg
  }
}

#
#
#
itcl::body ATrigger2367::ReadSignature2 {} {
  if {$device ne ""} {
    return [$device simpleRead24 $node 15 0]
  } else {
    set msg "ATrigget2367::ReadSignature2 user must set the controller first "
    append msg "with SetController" 
    return -code error $msg
  }
}

#
#
#
itcl::body ATrigger2367::Configure {firmware} {
# Read firmware bit file
  if {![file exists $firmware]} {
    set msg "ATrigger2367::Configure firmware file $firmware for $self doesn't "
    append msg "exist!"
    return -code error -message $msg
  }

  # at this point, the file exists so let's open it
	if {[catch {set file [open $firmware r]} message]} {
    # oops we couldn't open it. 
    set msg "ATrigger2367::Configure Cannot open firmware file $firmware for "
    append msg "$self"
		return -code error $msg
	}

  # check to make sure that we have a controller to talk to
  if {$device eq ""} {
    set msg "ATrigger2367::Configure user must set the controller first with "
    append msg "SetController"
    return -code error $msg
  }

	fconfigure $file -translation binary
	set config [read $file]
	close $file
	set size [file size $firmware]

  # Read the entire file into a single array of bytes
	binary scan $config c$size bytes

  # Find first 0xff in firmware file
	set index 0
	while {[lindex $bytes $index] != -1} {incr index}

  # Init FPGA programming sequence in Xilinx chip
	$device simpleControl $node 0 30  ;# enter general programming mode
	$device simpleControl $node 0 28  ;# select CAMAC configuration mode
	$device simpleControl $node 0 25  ;# program Xilinx chip
	after 10                          ;# wait 10 ms before proceeding

  # Loop on firmware configuration until end
	for {set i $index} {$i < $size} {incr i} {
		$device simpleWrite24 $node 0 16 [lindex $bytes $i]
	}
  # Exit programming mode by clearing NA(0)F(9)
  Clear

  # Check configuration
  if {![IsConfigured]} {
    set msg "ATrigger2367::Configure Failed to configure module $self! "
    append msg "Firmware signatures were not verified."
		return -code error $msg 
	}
}

#
#
#
itcl::body ATrigger2367::IsConfigured {} {
  set type    [expr [ReadSignature2] & 0xffffff]
  set version [expr [ReadSignature1] & 0xffffff]
  
  return [expr {($type==2367) && ($version==5800)}]
}


#
#
#
itcl::body ATrigger2367::sClear {stack} {
  $stack addControl $node 0 9
}

#
#
#
itcl::body ATrigger2367::sClearRegister {stack} {
  $stack addControl $node 0 10
}

#
#
#
itcl::body ATrigger2367::sRead {stack} {
  $stack addRead24 $node 0 3
}

#
#
#
itcl::body ATrigger2367::sStamp {stack} {
  $stack addRead24 $node 1 3   ;# bits 0-15
  $stack addRead24 $node 2 3   ;# bits 16-31
  $stack addRead24 $node 3 3   ;# bits 32-47
  $stack addRead24 $node 4 3   ;# bits 48-63
}


#
#
#
itcl::body ATrigger2367::sSetS800GDGDelay {stack val} {
  $stack addWrite24 $node 0 16 $val
}

#
#
#
itcl::body ATrigger2367::sGetS800GDGDelay {stack} {
  $stack addRead24 $node 0 0   
}

#
#
#
itcl::body ATrigger2367::sSetS800GDGWidth {stack val} {
  $stack addWrite24 $node 1 16 $val
}

#
#
#
itcl::body ATrigger2367::sGetS800GDGWidth {stack} {
  $stack addRead24 $node 1 0   
}

#
#
#
itcl::body ATrigger2367::sSetSecondaryGDGDelay {stack val} {
  $stack addWrite24 $node 2 16 $val
}

#
#
#
itcl::body ATrigger2367::sGetSecondaryGDGDelay {stack} {
  $stack addRead24 $node 2 0   
}

#
#
#
itcl::body ATrigger2367::sSetSecondaryGDGWidth {stack val} {
  $stack addWrite24 $node 3 16 $val
}

#
#
#
itcl::body ATrigger2367::sGetSecondaryGDGWidth {stack} {
  $stack addRead24 $node 3 0   
}

#
#
#
itcl::body ATrigger2367::sSetS800Delay {stack val} {
  $stack addWrite24 $node 4 16 $val
}

#
#
#
itcl::body ATrigger2367::sGetS800Delay {stack} {
  $stack addRead24 $node 4 0   
}

#
#
#
itcl::body ATrigger2367::sSetCoincidenceWidth {stack val} {
  $stack addWrite24 $node 5 16 $val
}

#
#
#
itcl::body ATrigger2367::sGetCoincidenceWidth {stack} {
  $stack addRead24 $node 5 0   
}

#
#
#
itcl::body ATrigger2367::sSetSecondaryDelay {stack val} {
  $stack addWrite24 $node 6 16 $val
}

#
#
#
itcl::body ATrigger2367::sGetSecondaryDelay {stack} {
  $stack addRead24 $node 6 0   
}

#
#
#
itcl::body ATrigger2367::sSetBypasses {stack val} {
  $stack addWrite24 $node 7 16 $val
}

#
#
#
itcl::body ATrigger2367::sGetBypasses {stack} {
  $stack addRead24 $node 7 0   
}

#
#
#
itcl::body ATrigger2367::sSetS800DownscaleFactor {stack factor} {
  $stack addWrite24 $node 8 16 $factor
}

#
#
#
itcl::body ATrigger2367::sGetS800DownscaleFactor {stack} {
  $stack addRead24 $node 8 0   
}

#
#
#
itcl::body ATrigger2367::sSetSecondaryDownscaleFactor {stack factor} {
  $stack addWrite24 $node 9 16 $factor
}

#
#
#
itcl::body ATrigger2367::sGetSecondaryDownscaleFactor {stack} {
  $stack addRead24 $node 9 0   
}

#
#
#
itcl::body ATrigger2367::sSetTriggerBox {stack bitpattern} {
  $stack addWrite24 $node 10 16 $bitpattern
}

#
#
#
itcl::body ATrigger2367::sGetTriggerBox {stack} {
  $stack addRead24 $node 10 0   
}

#
#
#
itcl::body ATrigger2367::sSetADCGateWidth {stack wid} {
  $stack addWrite24 $node 0 18 $wid
}

#
#
#
itcl::body ATrigger2367::sGetADCGateWidth {stack} {
  $stack addRead24 $node 0 2  
}

#
#
#
itcl::body ATrigger2367::sSetQDCGateWidth {stack wid} {
  $stack addWrite24 $node 1 18 $wid
}

#
#
#
itcl::body ATrigger2367::sGetQDCGateWidth {stack} {
  $stack addRead24 $node 1 2  
}

#
#
#
itcl::body ATrigger2367::sSetTDCGateWidth {stack wid} {
  $stack addWrite24 $node 2 18 $wid
}

#
#
#
itcl::body ATrigger2367::sGetTDCGateWidth {stack} {
  $stack addRead24 $node 2 2  
}

#
#
#
itcl::body ATrigger2367::sSetCoincRegGateWidth {stack wid} {
  $stack addWrite24 $node 3 18 $wid
}

#
#
#
itcl::body ATrigger2367::sGetCoincRegGateWidth {stack} {
  $stack addRead24 $node 3 2  
}



# A utility to facility single-shot operation evaluation
#
# Given a list whose first element is a proc name and subsequent elements are
# arguments (i.e. procname arg0 arg1 arg2 ...) , this creates a stack
#
#  procname stack arg0 arg1 arg2 ...
#  
#
itcl::body ATrigger2367::Execute {grouping script} {
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
