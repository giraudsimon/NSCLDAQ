#===================================================================
# class ACAENC671
#===================================================================


package provide caenc671 11.0

package require Utils


##
# Device driver for the CAENC671 CAMAC CFD
#
# This provides minimal functionality for controlling the C671 and 
# does not provide the support for the actual readout control 
#
itcl::class ACAENC671 {
  protected variable device ;#< a USB device from cccusb package
  private variable node     ;#< the slot occupied by the card

  ##
  # Constructor
  #
  # Store the name of the controller and the slot.
  # 
  # @param de   a CAMAC controller
  # @param no   the slot in which the device resides
  #
  constructor {de no} {
    set device $de
    set node $no
  }

  destructor {}

  ## @brief Pass a controller object into the class for use
  #
  # This is particularly useful for the case when the driver is used in an
  # initialization script where the controller is technically only known at the
  # point of the script. In this case, it is useful for the user to be able to
  # configure the device to use the current controller object.
  # 
  # @param ctlr   a cccusb::CCCUSB object  
  public method SetController {ctlr}


  ## @brief Retrieve the name of the current controller object
  #
  # @returns the name of the current controller object
  #
  public method GetController {}

  ##
  #  @brief Enable channels within bank
  #
  #  Channels are broken into two banks: 0-7 in bank 0
  #  and 8-15 in bank 1. User can provide the proper
  #  pattern to write for enabling channels. For example, if bit 0=1
  #  then the corresponding channel is enabled.
  #
  #  \param bank integer specifying bank 0 or 1
  #  \param pat  bitset specifying which channels to enable
  #
  #  \returns bit pattern of QX
  public method Enable {bank pat} 

  ##
  # @brief Set the threshold for a given channel
  #
  # The range is settable "in a range from -1 and -256mV (5mV minimum 
  # required)" in steps of 1mV. 
  #
  # \param ch the channel to target
  # \param th threshold value as an integer between 0 and 255.
  #
  # \return bit pattern of QX
  public method Threshold {ch th} 

  ##
  # @brief Set the delay value of a given channel
  #
  # The delay for a channel can be a value between 35ns and
  # 535ns, with a granularity of 1.96ns. 
  #
  # \param ch the channel to target
  # \param del the amount of delay as an integer between 0 and 255
  #
  # \return bit pattern of QX
  public method Delay {ch del} 

  ##
  #  @brief Set delayed output width for a bank
  #
  #  Set the width of the delayed output signal for a bank of channels.
  #  Minimum pulse width = 10ns and Maximum pulse width = 250ns.
  #
  #  \param bank integer specifying bank 0 or 1
  #  \param pat  integer specifying width of signal (min=0, max=255)
  #
  #  \returns bit pattern of QX
  public method Width {code wid} 

  ## @brief Set the dead time for this module
  #
  # Set the amount of time following an input signal to disable input signals.
  # This can be set from 160 ns to 2 us using an 8-bit range. 
  # (0 --> 160 ns and 255 --> 2 us)
  # 
  # @param bank integer specifying bank 0 or 1
  # @param dt  value to set delay to [0,255]
  # 
  public method DeadTime {bank dt} 

  ##
  #  @brief Set prompt output width for a bank
  #
  #  Set the width of the prompt output signal for a single channels.
  #  Minimum pulse width = 10ns and Maximum pulse width = 250ns.
  #
  #  \param bank integer specifying bank 0 or 1
  #  \param pat  integer specifying width of signal (min=0, max=255)
  #
  #  \returns bit pattern of QX
  public method PromptWidth {code wid} 


  ## @brief Set the internal majority level
  #
  # The module supports outputting a signal when the number of channels above
  # threshold exceeds a certain value. This majority threshold operates purely
  # on the channels within this module.
  #
  # @param numChannels  the majority threshold (values must be in range [1,16])
  #
  # @returns bit pattern of QX
  #
  # @throws error if argument is out of valid range
  public method InternalMajority {numChannels}

  ## @brief Set the external majority level
  #
  # The module supports outputting a signal when the number of channels above
  # threshold exceeds a certain value. This is similar to the internal majority
  # except for that fact that it operates on channels from other modules.
  # Because there are multiple modules that can contribute their above-threshold
  # channel sum, the majority threshold can be up to 43. Furthermore, the user
  # can specify whether this module will contribute to the sum or not.
  #
  # @param numChannels  the majority threshold (values must be in range [1,43])
  # @param enable       boolean to indicate whether the module contributes to
  #                     external channel sum 
  #
  # @returns bit pattern of QX
  #
  # @throws error if argument is out of valid range
  public method ExternalMajority {numChannels enable}

  ## @brief Choose the signal to be outputted on the prompt multiplex 
  #
  # The multiplexed output for the prompt signals can be configured to output
  # any one of the 16 channels. This allows the user to select between them.
  #
  # @param chn  the channel index to output (must be in range [0,15]) 
  #
  # returns bit pattern of QX
  public method SetMultiplexPrompt {chn}

  ## @brief Choose the channel to be outputted on the delay multiplex
  #
  # Enables the user to select which channel's delay output will be emitted on
  # the multiplexed delay output.
  #
  # @param chn  the channel index to output (must be in range [0,15])
  #
  # returns bit pattern of QX
  public method SetMultiplexDelay {chn}

  ## @brief Choose the channel to be outputted on the input/level multiplex
  #
  # Enables the user to select which channel's input and level will emitted on
  # the multiplexed input/level output.
  #
  # @param chn  the channel index to output (must be in range [0,15])
  #
  # @returns bit pattern of QX
  public method SetMultiplexInputLevel {chn}

  #----------------------------------------------------------------------------
  # Utilities methods

  ## @brief Convert user majority code to be understood by device
  #
  # @returns 6*(numChannels-1)
  private method computeMajorityCode {numChannels}
}


#
#
#
#
#
itcl::body ACAENC671::SetController {ctlr} {
  set device $ctlr
}

#
#
#
#
#
itcl::body ACAENC671::GetController {} {
  return $device
}


#
#
#
itcl::body ACAENC671::Enable {bank pat} {
  if {![Utils::isInRange 0 1 $bank]} {
    return -code error \
      "ACAENC671::Enable bank $bank is invalid. Must be either 0 or 1."
  }

  return [$device simpleWrite24 $node $bank 18 $pat]
}

#
#
#
itcl::body ACAENC671::Threshold {ch th} {
  if {![Utils::isInRange 0 15 $ch]} {
    return -code error \
      "ACAENC671::Threshold ch $ch is invalid. Must be in range \[0,15\]."
  }

  if {![Utils::isInRange 0 255 $th]} {
    set msg "ACAENC671::Threshold $th is an invalid threshold setting. "
    append msg {Must be in range [0,255].}
    return -code error $msg
  } 

  return [$device simpleWrite24 $node $ch 16 $th]
}

#
#
#
#
itcl::body ACAENC671::Delay {ch del} {
  if {![Utils::isInRange 0 15 $ch]} {
    return -code error \
      "ACAENC671::Delay ch $ch is invalid. Must be in range \[0,15\]."
  }

  if {![Utils::isInRange 0 255 $del]} {
    set msg "ACAENC671::Delay $del is an invalid delay setting. "
    append msg {Must be in range [0,255].}
    return -code error $msg
  } 
  return [$device simpleWrite24 $node $ch 17 $del]
}

#
#
#
#
#
itcl::body ACAENC671::Width {bank wid} {
  if {![Utils::isInRange 0 1 $bank]} {
    return -code error \
      "ACAENC671::Width bank $bank is invalid. Must be either 0 or 1."
  }

  if {![Utils::isInRange 0 255 $wid]} {
    set msg "ACAENC671::Width $wid is an invalid width setting. "
    append msg {Must be in range [0,255].}
    return -code error $msg
  } 

  set A [expr {$bank + 0}]
  return [$device simpleWrite24 $node $A 20 $wid]
}

#
#
#
#
#
itcl::body ACAENC671::DeadTime {bank dt} {
  if {![Utils::isInRange 0 1 $bank]} {
    return -code error \
      "ACAENC671::DeadTime bank $bank is invalid. Must be either 0 or 1."
  }

  if {![Utils::isInRange 0 255 $dt]} {
    set msg "ACAENC671::DeadTime $dt is an invalid dead time setting. "
    append msg {Must be in range [0,255].}
    return -code error $msg
  } 

  # if we made it here then we can execute the command!
  set A [expr {$bank+4}]

  return [$device simpleWrite24 $node $A 20 $dt]
}

#
#
#
#
#
itcl::body ACAENC671::PromptWidth {bank wid} {
  if {![Utils::isInRange 0 1 $bank]} {
    return -code error \
      "ACAENC671::PromptWidth bank $bank is invalid. Must be either 0 or 1."
  }

  if {![Utils::isInRange 0 255 $wid]} {
    set msg "ACAENC671::PromptWidth $wid is an invalid output width setting. "
    append msg {Must be in range [0,255].}
    return -code error $msg

  } 

  set A [expr {$bank+6}]
  return [$device simpleWrite24 $node $A 20 $wid]
}


#
#
#
#
#
itcl::body ACAENC671::InternalMajority {numChans} {
  if {![Utils::isInRange 1 16 $numChans]} {
    set msg "ACAENC671::InternalMajority $numChans is an invalid internal "
    append msg {majority setting. Must be in range [1,16].}
    return -code error $msg
  }
  set code [computeMajorityCode $numChans]
  return [$device simpleWrite24 $node 3 20 $code]
}

#
#
#
#
#
itcl::body ACAENC671::ExternalMajority {numChans enable} {
  if {![Utils::isInRange 1 43 $numChans]} {
    set msg "ACAENC671::ExternalMajority $numChans is an invalid external "
    append msg {majority setting. Must be in range [1,43].}
    return -code error $msg
  }
  
  set code [computeMajorityCode $numChans]
  set en [string is true $enable]
  set enable [expr {$en<<8}]

  set code [expr {$enable|$code}]
  return [$device simpleWrite24 $node 2 20 $code]
}


#
#
#
#
#
itcl::body ACAENC671::SetMultiplexPrompt {chn} {
  if {![Utils::isInRange 0 15 $chn]} {
    set msg "ACAENC671::SetMultiplexPrompt $chn is an invalid channel. "
    append msg {Must be in range [0,15].}
    return -code error $msg
  }
  
  set code [expr {1<<8}]
  return [$device simpleWrite24 $node $chn 19 $code]

}

#
#
#
#
#
itcl::body ACAENC671::SetMultiplexDelay {chn} {
  if {![Utils::isInRange 0 15 $chn]} {
    set msg "ACAENC671::SetMultiplexDelay $chn is an invalid channel. "
    append msg {Must be in range [0,15].}
    return -code error $msg
  }
  
  set code [expr {1<<8}]
  return [$device simpleWrite24 $node $chn 21 $code]

}


#
#
#
#
#
itcl::body ACAENC671::SetMultiplexInputLevel {chn} {
  if {![Utils::isInRange 0 15 $chn]} {
    set msg "ACAENC671::SetMultiplexInputLevel $chn is an invalid channel. "
    append msg {Must be in range [0,15].}
    return -code error $msg
  }
  
  set code [expr {1<<8}]
  return [$device simpleWrite24 $node $chn 22 $code]

}



# ------------------------------------------------------------------------------
# Utility methods

itcl::body ACAENC671::computeMajorityCode {numChannels} {
  return [expr {6*($numChannels-1)}]
}
