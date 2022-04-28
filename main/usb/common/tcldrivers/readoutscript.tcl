
package provide readoutscript 1.0

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
snit::type readoutscript {

  option -controllertype -default {} -configuremethod _setControllerType
  option -initscript      -default {} -configuremethod _setScript
  option -rdolistscript   -default {} -configuremethod _setScript
  option -onendscript     -default {} -configuremethod _setScript

  
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

    if {$options(-controllertype) eq "vmusb"} {
      package require VMUSBDriverSupport
    } elseif {$options(-controllertype) eq "ccusb"} {
      package require CCUSBDriverSupport
    }
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
  # onEndRun 
  #
  # Procedures to call when acquisition mode is exited. It is only meaninful if the
  # user has defined the -onendscript option
  #
  # \param driverPtr a pointer to a USB controller
  #
  # \return OK always. 
  method onEndRun driverPtr {
    global ::Globals
    variable _deviceNamespace
    variable _deviceType

    $self _validateControllerType $options(-controllertype)

    set ::Globals::aController [${_deviceNamespace}::convert${_deviceType} $driverPtr]
    if {[string length $options(-onendscript)]>0} {
      uplevel #0 source $options(-onendscript)
    }
    return "OK"
  }

  ## 
  # addReadoutList
  #
  # Call a tcl script with access to a readout list. Typically this is used to defien
  # readout procedures in response to a trigger but can do anything that TCL allows.
  # This creates a new stack that the script will fill and then appends the result to the 
  # list passed as an argument.
  #
  # \param aList list to a USB controller (either CCUSB or VMUSB)
  method addReadoutList {aList}  {
    variable _deviceNamespace
    variable _deviceType
    global ::Globals::aTclEventList
    global ::Globals::aReadoutList

    $self _validateControllerType $options(-controllertype)

    set ::Globals::aReadoutList [${_deviceNamespace}::convert${_deviceType}ReadoutList $aList]

    # Create a local variable for use by the script
    set ::Globals::aTclEventList [list]
    if {[string length $options(-rdolistscript)]>0} {
      uplevel #0 source $options(-rdolistscript)
    }

    # append the new stuff onto the existing readout list
    set newops [${_deviceNamespace}::convertToReadoutList $::Globals::aTclEventList]
    $::Globals::aReadoutList append $newops
  }

  ##
  # _setControllerType
  #
  # This is called anytime the configure method is called for the -controllertype option.
  # By setting the -controllertype option, this checks whether or not the value is understood
  # as vmusb or ccusb. If it is one of these, then the _deviceNamespace and _deviceType are
  # set. These are then used when calling support methods.
  #
  # \param option the name of the option parameter (-controllertype)
  # \param value the value to set the option to. Supported values are "vmusb" and "ccusb"
  method _setControllerType {option value} {
    variable _deviceNamespace
    variable _deviceType
  
    $self _validateControllerType $value 

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
      error {Type of controller not specified! User must set -controllertype to either "vmusb" or "ccusb"}
    }

  }


}

