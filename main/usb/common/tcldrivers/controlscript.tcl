
package provide controlscript 1.0

package require snit
package require Globals

## \brief a driver to execute tcl scripts
#
# This is a pseudo device driver because it does not actually provide
# control over a single device. Instead, it allows the user to execute
# a tcl script as if it was a slow controlled device. In principle this
# is extremely generic and can be used for anything the user wants to put 
# in a tcl script, however, its original intent was to allow users to 
# control slow-controlled devices via procedures defined in a tcl script.
#
snit::type controlscript {

  option -controllertype -default {} -configuremethod _setControllerType
  option -initscript     -default {} -configuremethod _setScript
  option -updatescript   -default {} -configuremethod _setScript
  option -monitorscript  -default {} -configuremethod _setScript
  option -monitorproc    -default {} -configuremethod _setProc

  
  ##
  # Constructor
  #
  # Sets the various parameters options that are passed as 
  # as arguments
  #
  # \param args the list option value pairs to set
  # 
  constructor args {
    $self configurelist $args
  }

  ## 
  # Initialize
  #
  # Handle initialization routines that are needed by the
  # device. This will execute the script defined by the -initscript option.
  # This script is also executed if the init command is called without any
  # alternate control configuration file. 
  #
  # \param driverPtr a pointer to a USB controller
  #
  # \return OK always. 
  method Initialize driverPtr { 
    global ::Globals
    variable _deviceNamespace
    variable _deviceType
  
    $self _validateControllerType $options(-controllertype)

    set ::Globals::aController [${_deviceNamespace}::convert${_deviceType} $driverPtr]
    if {[string length $options(-initscript)]>0} {
      uplevel #0 source $options(-initscript)
    }
    return "OK"
  }

  ## 
  # Update
  #
  # When an update command is called, the script located by the -updatescript
  # is executed in the global namespace.
  #
  # \param driverPtr a pointer to a USB controller
  #
  # \return OK always. 
  method Update driverPtr {
    global ::Globals::aController
    variable _deviceNamespace
    variable _deviceType

    $self _validateControllerType $options(-controllertype)

    set ::Globals::aController [${_deviceNamespace}::convert${_deviceType} $driverPtr]
    if {[string length $options(-updatescript)]>0} {
      uplevel #0 source $options(-updatescript)
    }
    return "OK"
  }

  ##
  # Set
  # 
  # This method is not used in this driver
  # 
  # \return error always
  method Set {ctlr parameter value}  {
    error "Set is not supported" 
  }


  ##
  # Get
  # 
  # This method is not used in this driver
  # 
  # \return error always
  method Get {ctlr parameter value}  {
    error "Get is not supported" 
  }


  ## 
  # addMonitorList
  #
  # Add to the monitor list that is executed periodically by the tclserver. This creates
  # a new stack that the script will fill and then appends the result to the 
  # list passed as an argument.
  #
  # \param aList list to a USB controller (either CCUSB or VMUSB)
  method addMonitorList {aList}  {
    variable _deviceNamespace
    variable _deviceType
    global ::Globals::aTclEventList
    global ::Globals::aReadoutList

    $self _validateControllerType $options(-controllertype)

    set ::Globals::aReadoutList [${_deviceNamespace}::convert${_deviceType}ReadoutList $aList]

    # Create a local variable for use by the script
    set ::Globals::aTclEventList [list]
    if {[string length $options(-monitorscript)]>0} {
      uplevel #0 source $options(-monitorscript)
    }

    # append the new stuff onto the existing readout list
    set newops [${_deviceNamespace}::convertToReadoutList $::Globals::aTclEventList]
    $::Globals::aReadoutList append $newops
  }

  ## 
  # processMonitorList
  #
  # Handle the data returned from monitor list
  #
  # \param data a tcl list of data bytes remaining to be processed 
  # \return number of bytes processed
  method processMonitorList {data}  {

    $self _validateControllerType $options(-controllertype)

    set nbytes 0
    if {[string length $options(-monitorproc)]>0} {
      set nbytes [uplevel #0 $options(-monitorproc) $data]
    }
    
    return $nbytes 
  }
  ##
  # _setControllerType
  #
  # This is called anytime the configure method is called for the -controllertype 
  # option. By setting the -controllertype option, this checks whether or not the 
  # value is understood as vmusb or ccusb. If it is one of these, then the 
  # _deviceNamespace and _deviceType are set. These are then used when calling 
  # support methods.
  #
  # \param option the name of the option parameter (-controllertype)
  # \param value the value to set the option to. Supported values are "vmusb" and "ccusb"
  method _setControllerType {option value} {
    variable _deviceNamespace
    variable _deviceType

    $self _validateControllerType $value

    # if we are here, there are only two values possible for value 
    # deal with them
    if { $value eq "vmusb" } {
      set _deviceNamespace "::VMUSBDriverSupport"
      set _deviceType "VmUSB"
    } elseif { $value eq "ccusb" } {
      set _deviceNamespace "::CCUSBDriverSupport"
      set _deviceType "CCUSB"
    } 
  
    set options($option) $value

  }

  ##
  # _setScript
  #
  # This is called anytime the configure method is called for a script option and
  # it checks whether or not a file exists at the path provided. 
  #
  # \param option the name of the option parameter (-controllertype)
  # \param value the value to set the option to. Supported values are "vmusb" and "ccusb"
  method _setScript {option value} {
    if {[file exists $value]} {
      set options($option) $value
    } else {
      error "File path $value specified for $option, but no file exists there."
    }
  }

  ##
  # _setProc
  #
  # This is called anytime the configure method is called for the -monitorproc option. It
  # it checks whether or not a proc exists at the path provided. 
  #
  # \param option the name of the option parameter (-controllertype)
  # \param value the value to set the option to. Supported values are "vmusb" and "ccusb"
  method _setProc {option value} {
    set procs [info proc $value]
    if {[llength $procs]!=0} {
      set options($option) $value
    } else {
      error "Proc ($value) specified for $option doesn't exist"
    }
  }

  ##
  # _validateControllerType
  #
  # This checks whether or not the the value passed to it is either "vmusb" or "ccusb". An exceptional 
  # return occurs if the value is neither.
  #
  # \param value a string 
  #
  # \return "" if value is vmusb or ccusb. Otherwise an error msg is returned.
  method _validateControllerType {value} {
    if { $value ne "vmusb" && $value ne "ccusb"} {
      set msg {Type of controller not specified! User must set }
      append msg {-controllertype to either "vmusb" or "ccusb"}
      return -code error $msg
    }

    if {$value eq "vmusb"} {
      package require VMUSBDriverSupport
    } elseif {$value eq "ccusb"} {
      package require CCUSBDriverSupport
    }

  }


} 



