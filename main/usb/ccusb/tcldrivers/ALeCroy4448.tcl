#===============================================================================
# class ALeCroy4448
#===============================================================================

package provide lecroy4448 11.0 

package require Itcl
package require cccusb
package require cccusbreadoutlist
package require CCUSBDriverSupport


##
# A low-level driver for controlling a LeCroy 4448 coincidence register. The
# driver is designed to be used with the the cccusb and cccusbreadoutlist
# packages. 
#
itcl::class ALeCroy4448 {
  private variable device ;#< reference to a cccusb::CCCUSB 
  private variable node   ;#< the slot in the crate
  private variable self   ;#< name of the instance

  ##
  # Constructor
  #
  # Creates an instance of this class. 
  #
  # \param de a swig-wrapped cccusb::CCCUSB device
  # \param no the slot number in which the device resides 
  #
  # \return the name of the object
  constructor {de no} {
    set device $de
    if {[CCUSBDriverSupport::isValidSlot $no]} {
      set node $no
    } else {
      return -code error "ALeCroy4448::constructor passed invalid slot index."
    }
    set self [string trimleft $this :]
  }

  ## 
  # Destructor
  #
  # This does nothing.
  destructor {}


  ## @brief Pass a controller object into the class for use
  #
  # This is particularly useful for the case when the driver is used in an
  # initialization script where the controller is technically only known at the
  # point of the script. In this case, it is useful for the user to be able to
  # configure the device to use the current controller object.
  # 
  # @param ctlr   a cccusb::CCCUSB object  
  public method SetController {ctlr} {
    set device $ctlr
  }

  ## @brief Retrieve the name of the current controller object
  #
  # @returns the name of the current controller object
  #
  public method GetController {} {
    return $device
  }

  ##
  # GetVariable
  #
  # The purpose of this is not clear at all but I will leave it.
  #
  # \param v the name of a value to dereference
  #
  # \return the value of the variable named in the argument
  public method GetVariable {v} {set $v}

  #############################################################################
  #############################################################################
  # Single-shot operations

  ##
  # Clear
  #
  # Interactively clear the module
  #
  # \return the QX code returned from the control operation 
  public method Clear {} 


  ## @brief Read a single register
  #
  # @param regName  name of register to read (@see MapRegisterName)
  #
  public method ReadRegister regName

  ## @brief Perform a read and clear operation for a specific register
  #
  # @param regName  name of register (@see MapRegisterName)
  #
  public method ReadAndClearRegister regName

  ## @brief Clear a specific register 
  #
  # @param regName  name of register (@see MapRegisterName)
  #
  public method ClearRegister regName

  #############################################################################
  #############################################################################
  # Stack building methods

  ##
  # sClear
  #
  # Add a clear to the stack passed into it as an argument 
  #
  # \param stack a cccusbreadoutlist::CCCUSBReadoutList object
  public method sClear {stack}

  ## 
  # sRead
  #
  # Add procedures to read a register to the stack specified.
  #
  # \param stack    a cccusbreadoutlist::CCCUSBReadoutList object
  # \param register a register name (@see MapRegisterName for valid values)
  public method sRead {stack register}

  ## 
  # sReadAndClear
  #
  # Add procedures to read and clear register to the stack specified.
  #
  # \param stack    a cccusbreadoutlist::CCCUSBReadoutList object
  # \param register a register name (@see MapRegisterName for valid values)
  public method sReadAndClear {stack register}

  ## 
  # sClearRegister
  #
  # Add procedures to clear a specific register to the stack specified.
  #
  # \param stack    a cccusbreadoutlist::CCCUSBReadoutList object
  # \param register a register name (@see MapRegisterName for valid values)
  public method sClearRegister {stack register}



  #############################################################################
  #############################################################################
  # Utility Methods

  ## @brief Transform a register name into 0, 1, or 2
  #
  # For convenience sake, it is possible to refer to the register values as
  # either 0, 1, 2, A, B, or C. This simply takes one of those names and maps
  # it appropriately to the values 0, 1, or 2. Here is the mapping:
  #
  # 0 -> 0
  # 1 -> 1
  # 2 -> 2
  # A -> 0
  # B -> 1
  # C -> 2 
  #
  # @param regName name of a register
  #
  # @returns register index corresponding to mapping specified
  #
  # @throws error if user provided invalid register name.
  public method MapRegisterName {regName} {
    if {[string equal $regName A] || $regName==0} {
      set reg 0
    } elseif {[string equal $regName B] || $regName==1} {
      set reg 1
    } elseif {[string equal $regName C] || $regName==2} {
      set reg 2
    } else {
      set msg "ALeCroy4448::MapRegisterName $regName is an invalid register "
      append msg "name. Must be either A, B, C, 0, 1, or 2."
      return -code error $msg 
    }
    return $reg
  }


  public method Execute {grouping script} 
} ;# Done with class definition

#------------------------------------------------------------------------------
# Single shot commands 

itcl::body ALeCroy4448::Clear {} {
  if {[catch {set val [Execute 1 [list sClear]]} msg]} {
    # map the error context to this method 
    return -code error [lreplace $msg 0 0 ALeCroy4448::Clear]
  }
  return $val
}

# Read register F(0)A
#
itcl::body ALeCroy4448::ReadRegister regName {
  if {[catch {set val [Execute 2 [list sRead $regName]]} msg]} {
    # map the error context to this method 
    return -code error [lreplace $msg 0 0 ALeCroy4448::ReadRegister]
  }
  return $val
}


# Read and clear register F(2)A
# 
itcl::body ALeCroy4448::ReadAndClearRegister regName {
  if {[catch {set val [Execute 2 [list sReadAndClear $regName]]} msg]} {
    # map the error context to this method 
    return -code error [lreplace $msg 0 0 ALeCroy4448::ReadAndClearRegister]
  }
  return $val
}


# Clear register F(9)A
# 
itcl::body ALeCroy4448::ClearRegister regName {
  if {[catch {set val [Execute 1 [list sClearRegister $regName]]} msg]} {
    # map the error context to this method 
    return -code error [lreplace $msg 0 0 ALeCroy4448::ClearRegister]
  }
  return $val
}



###############################################################################
# Stack building commands

# sClear 
#
# Adds a clear, A(0)F(11), of the module to a cccusbreadoutlist
# type of stack object.
itcl::body ALeCroy4448::sClear {stack} {
  set A 0
  set F 11
  $stack addControl $node $A $F 
}


itcl::body ALeCroy4448::sRead {stack register} {
  set reg [MapRegisterName $register]
  set A $reg
  set F 0
  $stack addRead24 $node $A $F 
}

itcl::body ALeCroy4448::sReadAndClear {stack register} {
  if {[catch {set reg [MapRegisterName $register]} msg]} {
    return -code error [lreplace $msg 0 0 ALeCroy4448::sReadAndClear]
  }
  $stack addRead24 $node $reg 2 
}

itcl::body ALeCroy4448::sClearRegister {stack register} {
  if {[catch {set reg [MapRegisterName $register]} msg]} {
    return -code error [lreplace $msg 0 0 ALeCroy4448::sClearRegister]
  }
  $stack addControl $node $reg 9 
}




# A utility to facility single-shot operation evaluation
#
# Given a list whose first element is a proc name and subsequent elements are
# arguments (i.e. procname arg0 arg1 arg2 ...) , this creates a stack
#
#  procname stack arg0 arg1 arg2 ...
#  
#
itcl::body ALeCroy4448::Execute {grouping script} {
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
