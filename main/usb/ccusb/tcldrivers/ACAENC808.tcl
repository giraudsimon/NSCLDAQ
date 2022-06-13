#
#
#
#
#===================================================================
# class ACAENC808
# 
# v0.1 S.J.Williams Dec 27th 2012
#      Supports CAEN C808 CAMAC CFD
# v0.2 Jeromy Tompkins 9/29/2014
#      Driver implemented on the cccusb::CCCUSB package
#===================================================================

package provide caenc808 11.0

## @brief Low-level driver for the CAEN C808 CFD
#
# This package provides the basic functionality of the 16-ch CAMAC CFD. It
# maintains two banks of inputs that can be treated independently in terms of
# configuration parameters. 
#
# The device driver is intended for use with the CCUSBReadout program and will
# therefore issue commands to a cccusb::CCCUSB object that is either passed in
# at construction or through the SetController method. The preferred mechanism
# is for the SetController method to be used in the context of the CCUSBReadout
# program becuase anytime this will be called will be in the context of an
# Initialize, addReadoutList, or onEnd hook. These all technically shouldh have
# the same controller object but it is possible that the controller changes
# between runs and it is important that the controller created during the
# Initialize operation is the one that is used. For example, it is preferred
# that the user write:
#
# @verbatim
# package require caenc808;
#
# # pass in no controller at construction
# ACAENC808 cfd {} 12
#
# cfd SetController $Globals::aController
# cfd EnableAllChannels
#
# @endverbatim
itcl::class ACAENC808 {
  private variable device ;#< a CAMAC controller like in cccusb
  private variable node   ;#< the slot in which the module resides

  ## @brief Constructor 
  # 
  # Store the names of the cc-usb device and the slot
  # 
  # @param de   a cccusb::CCCUSB object
  # @param no   the slot in which the device resides
  constructor {de no} {
  set device $de
  set node $no
  }

  ## @brief Destructor... no-op
  destructor {}


  # @brief Pass a controller into the device externally 
  public method SetController {ctlr}

  # @brief Retrieve the current controller 
  public method GetController {}

  # interactive functions
  public method GetVariable {v} {set $v}
  public method EnableOnlyChannels {chans}
  public method EnableAllChannels {}
  public method DisableAllChannels {}
  public method SetThreshold {chan thresh}
  public method SetThresholds {thresh}
  public method SetWidth {group width}
  public method SetDeadTime {group deadt}
  public method SetMajority {maj}
  public method Init {}
}


## @brief Pass a controller object into the class for use
#
# This is particularly useful for the case when the driver is used in an
# initialization script where the controller is technically only known at the
# point of the script. In this case, it is useful for the user to be able to
# configure the device to use the current controller object.
# 
# @param ctlr   a cccusb::CCCUSB object  
itcl::body ACAENC808::SetController {ctlr} {
  set device $ctlr
}

## @brief Retrieve the name of the current controller object
#
# @returns the name of the current controller object
#
itcl::body ACAENC808::GetController {} {
  return $device
}

##
# @brief Enable a list of channels 
#
# A high level function for specifying the channels 
# to set. The function encodes the list of channels
# into a single 16-bit integer containing the same 
# information. It then writes that integer to the
# device.
#
# \param chans a list of channel numbers to enable
#
# \return the QX response generated when the value was written
itcl::body ACAENC808::EnableOnlyChannels {chans} {
# Enable channels from a list

  if {[llength $chans] > 16} {
    tk_messageBox -icon error \
      -message "Too many channels (>16) in $this enable list"
    return
  }

  set enablelist 0

  # Compute the bitset that has a bit set for each of the
  # specified channels in the argument 
  for {set i 0} {$i < [llength $chans]} {incr i} {
    set enablelist [expr $enablelist|[expr (1<<[lindex $chans $i])]]
  }

  # write the value
  return [$device simpleWrite24 $node 0 17 $enablelist]
}


##
# Enable all of the channels
#
# A high level function to enable all channels.
#
# \returns the QX response generated when the value is written
itcl::body ACAENC808::EnableAllChannels {} {
# Enable all channels in module

  return [$device simpleWrite24 $node 0 17 0xffff]
}

##
# Disable all channels
#
# A high-level function to disable all channels.
#
# \returns the QX response generated when the value was written
itcl::body ACAENC808::DisableAllChannels {} {
# Disable all channels in module

  return [$device simpleWrite24 $node 0 17 0]
}

##
# Set a threshold
#
# Set the threshold of a certain channel to a specific value. Values
# are in the range [1,255] and map directly to the threshold height in
# mV. For example, 
#      thr=1   corresponds to -1 mV.
#      thr=50  corresponds to -50 mV.
#
# \param chan the channel whose threshold is to be set
# \param thresh the value to set the threshold to (must be in range [1,255]
#
# \returns the QX response generated when the value was written
itcl::body ACAENC808::SetThreshold {chan thresh} {
# Set arming threshold for cfd channel

  return [$device simpleWrite24 $node $chan 16 $thresh]
}


##
# Set thresholds of all channels to a common value
#
# \param thresh the threshold value
#
itcl::body ACAENC808::SetThresholds {thresh} {
# Set a common threshold

  for {set i 0} {$i < 16} {incr i} {
    SetThreshold $i $thresh
  }
}


## 
# Set the width for a channel group
#
# the channels for the device are broken into two groups: 
# group 0 = chs 0-7
# group 1 = chs 8-15 
# 
# The width is between 15ns and 250 ns and set by an 8 bit integer.
#
# \param group channel group whose widths are to be set
# \param width value to set the width to 
#
# \returns the QX response generating when executing the write
#
itcl::body ACAENC808::SetWidth {group width} {
# Set output width for channel group 0 (chans 0-7) or group 1 (chans 8-16)

  if {$width > 255} {
    tk_messageBox -icon error \
      -message "Width > 255 not allowed for group $group of $this"
    return
  }

  if {$group > 1} {
    tk_messageBox -icon error \
      -message "There are only two width groups for $this, 0 and 1"
    return
  }

  return [$device simpleWrite24 $node $group 18 $width]
}

##
# Set the dead time for a channel group
#
# This sets the amount of time that the module ignores signals 
# after a threshold has been crossed.
#
# the channels for the device are broken into two groups: 
# group 0 = chs 0-7
# group 1 = chs 8-15 
# 
# The width is between 150ns and 2us and set by an 8 bit integer with nonlinear
# interpolation between the two.
# 
# \param group channel group whose dead times are to be set
# \param deadt the value to set the dead time to 
#
# \returns the QX response generating when executing the write
#
itcl::body ACAENC808::SetDeadTime {group deadt} {
# Set dead time for channel group 0 (chans 0-7) or group 1 (chans 8-16)

  if {$deadt > 255} {
    tk_messageBox -icon error \
      -message "Dead time > 255 not allowed for group $group of $this"
    return
  }

  if {$group > 1} {
    tk_messageBox -icon error \
      -message "There are only two width groups for $this, 0 and 1"
    return
  }

  return [$device simpleWrite24 $node $group 19 $deadt]
}


##
# Set majority threshold
#
# This serves as a sort of multiplicity threshold and can be in the range
# from 0 to 20. Values greater than 16 have meaning only when the user 
# has daisy-chained modules together such that they share a majority value.
#
# \param maj the value to set the majority to
# 
# \returns the QX response generating when executing the write
#
itcl::body ACAENC808::SetMajority {maj} {
# Set majority level
# need to convert level using formula majthr = nint((majlev*50)-25)/4)

  set majthrf [expr (((maj*50)-25)/4)]
  set majthr round($majthrf)

  return [$device simpleWrite24 $node 0 20 $majthr]
}

##
# Initialize
#
# This does not do anything
#
itcl::body ACAENC808::Init {} {
# Initialization
}
