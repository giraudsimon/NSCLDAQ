#===================================================================
##
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#      NSCL DAQ Development Group 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#
# @file   ACAENN568B.tcl
# @author Jeromy Tompkins and Daniel Bazin
# @note   This is basically a rewrite of the original ACAENN568B.tcl class.
#         It has been gutted to work with the NSCLDAQ USB drivers and 
#         also fully fleshed out to support all possible operations.

package provide caenn568b 11.0

package require Itcl
package require Utils
package require md5



## @brief Device driver for the CAEN N568B Spec Amp
#
# This device driver provides the mechanisms to support the configuration of
# the CAEN N568B through the ACAENV288 driver. A single instance of this driver
# should control a single N568B living as a node on the caennet controlled by
# the V288. The user has the ability to set the coarse gain, fine gain, shaping
# time, pole zero value, output polarity, and the configuration of the output
# (normal or inverted).
#
#
itcl::class ACAENN568B {
  private variable caennet  ;#< a CAENV288 object
  private variable number   ;#< node index
  private variable channels ;#< number of channels

  ## @brief Constructor
  #
  # Store necessary variables.
  #
  # @param caen   an ACAENV288 object
  # @param num    the node number of the CAENN568B to target
  #
  # @returns name of the newly constructed object
  constructor {caen num} {
    set caennet $caen
    set number $num
    set channels 16
  }

  ## @brief Destructor - noop
  destructor {}


  ## @brief Read module identifier from device
  #
  # @returns "N568 Version major.minor"
  #
  # @throws error if transmission fails with nonzero error code
  # @throws error if n568b returns nonzero response code
  #
  public method ReadModuleIdentifier {} 

  ## @brief Read parameters for all channels + offset 
  #
  # The list returned by this has the following form:
  #
  # fgain.ch0 pzero.ch0 polar.ch0 out.ch0 shape.ch0 cgain.ch0
  # fgain.ch1 pzero.ch1 polar.ch1 out.ch1 shape.ch1 cgain.ch1
  # fgain.ch0 pzero.ch2 polar.ch2 out.ch2 shape.ch2 cgain.ch2
  # ...
  # fgain.ch15 pzero.ch15 polar.ch15 out.ch15 shape.ch15 cgain.ch15
  # offset
  # 
  # All in all, there are a total of 97 params returned.
  #
  # @returns list of all params except for mux status
  #
  # @throws error if transmission fails with nonzero error code
  # @throws error if n568b returns nonzero response code
  #
  public method ReadAllParameters {}

  ## @brief Read offset from device 
  #
  # @returns offset value 
  #
  # @throws error if an nonzero error code is returned from V288 or N568B
  public method ReadOffset {}

  ## @brief Read all parameters for a channel + offset 
  #
  # @returns list of the following form
  #
  # @throws error if a nonzero error code is returned from V288 or N568B
  public method ReadChannelParameters {channel}

  ## @brief Reads status of MUX and also last accessed channel
  #
  # @returns 2 element list [list mux_status last_channel]
  #
  # @throws error if a nonzero error code is returned from V288 or N568B
  public method ReadMUXStatusAndLastChAccess {}

  ## @brief Set a parameter for a specific channel
  #
  # Possible parameter values: fgain, cgain, pzero, shape, out, polar
  # There understood parameters and their ranges are specified below:
  #  Description  Name       Range
  #  Fine gain    fgain     [0,255] 
  #  Course gain  cgain     [0,7]   (0=1, 1=2, 2=4, 3=8, ... , 7=128)
  #  Pole zero    pzero     [0,255] (0=50us ... 255=500us)
  #  Shaping      shape     [0,3]   (0=0.2us, 1=1us, 2=3us, 3=6us)
  #  Output conf. out       0 or 1  (0=direct, 1=inverted) 
  #  Polarity     polar     0 or 1  (0=positive, 1=negative)
  #
  # @param channel    channel to target in device
  # @param parameter  parameter name (see table above)
  # @param value      parameter value to write
  #
  # @returns "" 
  #
  # @throws error if channel is not in range [0,15]
  # @throws error if parameter name is not understood
  # @throws error if parameter value in not in allowed range. 
  # @throws error is a nonzero error code is returned from V288 or N568B
  public method SetParameter {channel parameter value}

  ## @brief Sets the DC offset of the signal (shifts baseline of output)
  #
  # @param value  offset value in range [0,255]
  #
  # @returns ""
  #
  # @throws error if value is not in range [0,255]
  # @throws error if a nonzero error code is returned from V288 or N568B
  public method SetOffset {value}

  ## @brief Disable the multiplexed outputs
  #
  # @returns response from the slave
  #
  # @throws error if a nonzero error code is returned from V288 or N568B
  public method DisableMUX {}

  ## @brief Enable multiplexed outputs
  #
  # @returns response from the slave 
  #
  # @throws error if a nonzero error code is returned from V288 or N568B
  public method EnableMUX {}


  ############################################################################
  # Convenience methods...
  # Interprets the data read by one of the reads and then returns the desired 
  # value.
  #
  
  ## @brief Retrieve the fine gain
  # 
  # Just a wrapper around the ReadChannelParameters
  #
  # @return value of fine gain for channel
  #
  # @throws @see ReadChannelParameters
  public method ReadFineGain {channel}
  
  ## @brief Retrieve the coarse gain
  # 
  # Just a wrapper around the ReadChannelParameters
  #
  # @return value of coarse gain for channel
  #
  # @throws @see ReadChannelParameters
  public method ReadCoarseGain {channel}

  ## @brief Retrieve the pole zero
  # 
  # Just a wrapper around the ReadChannelParameters
  #
  # @return value of pole zero for channel
  #
  # @throws @see ReadChannelParameters
  public method ReadPoleZero {channel}

  ## @brief Retrieve the shaping time 
  # 
  # Just a wrapper around the ReadChannelParameters
  #
  # @return raw shaping time setting for channel
  #
  # @throws @see ReadChannelParameters
  public method ReadShape {channel}

  ## @brief Retrieve the output configuration
  # 
  # Just a wrapper around the ReadChannelParameters
  #
  # @return output configuration for channel
  #
  # @throws @see ReadChannelParameters
  public method ReadOutputConfiguration {channel}

  ## @brief Retrieve the output polarity
  # 
  # Just a wrapper around the ReadChannelParameters
  #
  # @return output polarity for channel
  #
  # @throws @see ReadChannelParameters
  public method ReadPolarity {channel}

  ## @brief Test that we can read from the module
  #
  # This actually performs a read of the offset value
  #
  # @returns slave response for the offset
  public method TestCommunication {}


  ## @brief Parse the status register for a specific channel
  #
  # The status register encodes the data for the coarse gain,
  # shaping parameter, output config, and output polarity. This 
  # unpacks the encoded value into a list of the form:
  #
  # index   value
  # 0       output polarity
  # 1       output configuration 
  # 2       shaping parameter 
  # 3       coarse gain 
  #
  # @return list of decoded values
  #
  public method ParseStatusRegister {dataList}


  ## @brief Standard initialization routine
  #
  # This sources a file that should define the proper configuration
  # information. The user should have in this file an old-style tcl array
  # containing element names of the form %s%02d where %s is a parameter name and
  # the %02d is the channel:
  # 
  #   cgain00, cgain01, cgain02, ... , cgain15
  #   fgain00, fgain01, fgain02, ... , fgain15
  #   polar00, polar01, polar02, ... , polar15
  #   pzero00, pzero01, pzero02, ... , pzero15
  #   shape00, shape01, shape02, ... , shape15
  #   offset
  #
  # It is not necessary that the user provide every possible parameter name,
  # however, the value of any unspecified parameter will be undefined. 
  #
  # @param filename a filename containing config info
  # @param array    name containing config info
  #
  # @returns ""
  #
  # @throws error if unable to communicate with device (@see TestCommunication)
  public method Init {filename aname}
  public method InitIfChanged {filename aname hash}


  ## @brief Return the value of a nested variable name 
  #
  public method GetVariable {v} {set $v}


} ;# end of class definition


#-----------------------------------------------------------------------------#
# Implementation
#

#
# Module id code = 0x0
#
itcl::body ACAENN568B::ReadModuleIdentifier {} {

  # wait to prevent thrashing of the module by sending too many commands too 
  # fast
  after 75

  set status [$caennet SendCode $number 0]

  # check for bad transmit code
  if {$status} {
    set msg "$this: error reading module id. Module number: $number. "
    append msg [format "Error code: 0x%x." $status]
    return -code error -errorinfo ACAENN568B::ReadModuleIdentifier $msg
  } else {
    set data [$caennet Receive]

    # check for bad response code from n568b
    set status [lindex $data 0]
    if {$status !=0 } {
      set msg "$this: bad response code from n568b at node=$number. "
      append msg [format "Error code=0x%x." $status]
      return -code error -errorinfo ACAENN568B::ReadModuleIdentifier $msg
    }

    # good data... convert the binary data to a normal character string.
    set trimmedData [lreplace $data 0 0] ;# remove the error code 
    return [binary format c* $trimmedData]
  }
}

#
# read all params code = 0x1
#
itcl::body ACAENN568B::ReadAllParameters {} {
  after 75

  set status [$caennet SendCode $number 1]
  if {$status} {
    set msg "$this: error reading channel parameters. Module number: $number. "
    append msg "Error code: $status"
    return -code error -errorinfo ACAENN568B::ReadAllParameters $msg
  } else {

    # read slave's response
    set data [$caennet Receive]

    # check for a bad response code
    set status [lindex $data 0]
    if {$status!=0} {
      set msg "$this: bad response for n568b at node $number. "
      append msg [format "Error code=0x%x." $status]
      return -code error -errorinfo ACAENN568B::ReadAllParameters $msg
    }

    set data [lreplace $data 0 0] ;# strip the error code for simple processing

    # the response was good...parse the status reg for each of the channels 
    set result [list]
    for {set ch 0} {$ch < 16} {incr ch} {

      set fgain     [lindex $data [expr $ch*3+0]]
      set pzero     [lindex $data [expr $ch*3+1]]
      set statusReg [lindex $data [expr $ch*3+2]] 

      # split encoded values into a list (1 element per decoded value)
      set parsedStatReg [ParseStatusRegister $statusReg]

      # append the current channel list with the previous list
      set result [concat $result [list $fgain $pzero] $parsedStatReg]
    }

    lappend result [lindex $data end]
    return $result
  }
}

#
# read offset code = 0x2
#
itcl::body ACAENN568B::ReadOffset {} {

  # ensure that we don't send commands to the n568b at too rapid a pace.
  # this ensures at least 75 ms between commands.
  after 75
  set status [$caennet SendCode $number 0x2]
  if {$status} {
    set msg "$this: error reading global parameters. Module number: $number. "
    append msg "Error code: $status"
    return -code error -errorinfo ACAENN568B::ReadOffset $msg
  }

  # get the response from the slave
  set data [$caennet Receive]

  # check for bad error code from slave
  set status [lindex $data 0]
  if {$status!=0} {
    set msg "$this: bad response from n568b at node $number. "
    append msg [format "Error code=0x%x." $status]
    return -code error -errorinfo ACAENN568B::ReadOffset $msg
  }

  # strip the error code before sending out..
  return [lreplace $data 0 0]
}

#
# read all chan parameters + offset 
#   code = 0xn03
# where n is the channel number
#
itcl::body ACAENN568B::ReadChannelParameters {channel} {
  if {![Utils::isInRange 0 15 $channel]} {
    set msg "$this: Channel $channel is out of range. "
    append msg {Must be in range [0,15].} 
    return -code error -errorinfo ACAENN568B::ReadChannelParameters $msg
  }

  # wait for at least 75 ms prior to sending command to make sure we don't
  # thrash the n568b.. this is important
  after 75

  # the manual is wrong. To read parameters for a single channel, the code is:
  # 0xn03, where n is the slot number. This differs from the manual's value 0x0n3.
  set code [expr ($channel<<8)| 0x3]
  set status [$caennet SendCode $number $code]
  if {$status} {
    set msg "$this: error reading channel parameters. Module number: $number. "
    append msg "Error code: $status"
    return -code error -errorinfo ACAENN568B::ReadChannelParameters $msg
  } 

  # wait for response from n568b
  set data [$caennet Receive]

  # check for bad response code
  set status [lindex $data 0]
  if {$status!=0} {
    set msg "$this: Bad response from N568B. Module number: $number."
    append msg [format "Error code=0x%x." $status]
    return -code error -errorinfo ACAENN568B::ReadChannelParameters $msg
  }

  # unpack the encoded data retrieved from the device
  set parsedStatus [ParseStatusRegister [lindex $data 3]]
  set result [concat [lrange $data 1 2] $parsedStatus]
}

#
#
# mux status + last channel access code = 0x004 
#
#
itcl::body ACAENN568B::ReadMUXStatusAndLastChAccess {} {
  after 75
  set status [$caennet SendCode $number 0x004]
  if {$status} {
    set msg "$this: error reading MUX status and last channel access. Module "
    append msg "number: $number. Error code: $status"
    return -code error -errorinfo ACAENN568B::ReadMUXStatusAndLastChAccess $msg 
  } 
  set data [$caennet Receive]

  set status [lindex $data 0]
  if {$status!=0} {
    set msg "$this: bad response from n568b at node $number. "
    append msg [format "Error code=0x%x." $status]
    return -code error -errorinfo ACAENN568B::ReadMUXStatusAndLastChAccess $msg
  }

  # remove the error code
  set datum [lindex $data 1] 

  # parse value
  set result [list]
  lappend result [expr $datum>>7]  ;# mux status
  lappend result [expr $datum&0xf] ;# last channel accessed

  return $result
}

# 
# In this method the parameter to be set is coded in index
#
# Command codes for writes are of the form: 0xn1m
# where n is the channel number and m is the parameter index.
#
# Parameter indices are as follows:
# 0   fine gain
# 1   coarse gain
# 2   pole zero
# 3   shape
# 4   output polarity
# 5   output configuration
#
itcl::body ACAENN568B::SetParameter {channel parameter value} {

  # allowed parameters and their value upper bounds
  set plist {fgain cgain pzero shape polar out}
  set llist {255 7 255 3 1 1}

  # find the parameter in the list
  set index [lsearch $plist $parameter]
  if {$index == -1} {
    set msg "$this: invalid parameter ($parameter)"
    return -code error -errorinfo ACAENN568B::SetParameter $msg
  }

  # if here, the parameter name was valid...is the value provided in range?
  if {![Utils::isInRange 0 [lindex $llist $index] $value]} {
    set msg "$this: $parameter value out of range. "
    append msg "Must be in range \[0,[lindex $llist $index]\]."
    return -code error -errorinfo ACAENN568B::SetParameter $msg
  }

  # is the channel valid? 
  if {![Utils::isInRange 0 15 $channel]} {
    set msg "$this: channel number out of bounds. "
    append msg {Must be in range [0,15].}
    return -code error -errorinfo ACAENN568B::SetParameter $msg
  }


  # For some reason the n568b cannot handle back-to-back operations
  # and needs a healthy amount of time to be ready for the next operation
  after 75

  # all arguments checked out...send the command for the channel
  set code [expr ($channel<<8) | 0x10 | $index]
  set status [$caennet Send $number $code $value]

  # handle the response
  if {$status} {
    set msg "$this: error setting $parameter. Module number: $number. "
    append msg [format "Error code=0x%x." $status]
    return -code error -errorinfo ACAENN568B::SetParameter $msg

  } else {

    # receive error code from slave
    set data [$caennet Receive] 

    # scream if it is a nonzero code 
    set status [lindex $data 0]
    if {$status!=0} {
      set msg "$this: bad response from device at node $number. "
      append msg [format "Error code=0x%x." $status]
      return -code error -errorinfo ACAENN568B::SetParameters $msg
    }

  }

  # we don't really return anything on successful transmission
  return ""
}

#
# set offset code = 0x16
#
itcl::body ACAENN568B::SetOffset {value} {

  if {![Utils::isInRange 0 255 $value]} {
    set msg "Offset value out of bounds. Must be in range \[0,255\]."
    return -code error -errorinfo ACAENN568B::SetOffset $msg
  }

  after 75
  set status [$caennet Send $number 0x16 $value]
  if {$status} {
    set msg "$this: error enabling MUX. Module number: $number. "
    append msg [format "Error code:0x%x." $status]
    return -code error -errorinfo ACAENN568B::SetOffset $msg
  } 
  set data [$caennet Receive]
  set status [lindex $data 0]
  if {$status!=0} {
    set msg "$this: bad response from device at node $number. "
    append msg [format "Error code=0x%x." $status]
    return -code error -errorinfo ACAENN568B::SetOffset $msg
  }

  # strip off the error code before sending out
  return [lreplace $data 0 0]
}

#
# disable mux code = 0x20
#
itcl::body ACAENN568B::DisableMUX {} {
  after 75
  set status [$caennet SendCode $number 0x20]
  if {$status} {
    set msg "$this: error disabling MUX. Module number: $number. "
    append msg "Error code: $status"
    return -code error -errorinfo ACAENN568B::DisableMUX $msg
  } 
  return $status 
}

#
#
# enable mux code = 0x21
#
#
itcl::body ACAENN568B::EnableMUX {} {
  after 75
  set status [$caennet SendCode $number 0x21]
  if {$status} {
    set msg "$this: error enabling MUX. Module number: $number. "
    append msg "Error code: $status"
    return -code error -errorinfo ACAENN568B::EnableMUX $msg
  }
  return $status 
}

#
# Value extracted from return value of ReadChannelParameters
#
itcl::body ACAENN568B::ReadFineGain {channel} {
  set data [ReadChannelParameters $channel]
  return [lindex $data 0]
}

#
# Value extracted from return value of ReadChannelParameters
#
itcl::body ACAENN568B::ReadCoarseGain {channel} {
  set data [ReadChannelParameters $channel]
  return [lindex $data 5]
}

#
# Value extracted from return value of ReadChannelParameters
#
itcl::body ACAENN568B::ReadPoleZero {channel} {
  set data [ReadChannelParameters $channel]
  return [lindex $data 1]
}

#
# Value extracted from return value of ReadChannelParameters
#
itcl::body ACAENN568B::ReadShape {channel} {
  set data [ReadChannelParameters $channel]
  return [lindex $data 4]
}

#
# Value extracted from return value of ReadChannelParameters
#
itcl::body ACAENN568B::ReadOutputConfiguration {channel} {
  set data [ReadChannelParameters $channel]
  return [lindex $data 3] 
}


#
# Value extracted from return value of ReadChannelParameters
#
itcl::body ACAENN568B::ReadPolarity {channel} {
  set data [ReadChannelParameters $channel]
  return [lindex $data 2] 
}


#
#
#
itcl::body ACAENN568B::TestCommunication {} {
  set status [ReadOffset]
  if {$status != 0} {
    return $status
  } else {
    $caennet Receive
    return 0
  }
}

itcl::body ACAENN568B::ParseStatusRegister {register} {
  
  set result [list]
  lappend result [expr ($register&0x40)>>6]; # output polarity
  lappend result [expr ($register&0x20)>>5]; # output configuration
  lappend result [expr ($register&0x18)>>3]; # shape
  lappend result [expr ($register&0x7)];     # coarse gain

  return $result
}

#
#
#
itcl::body ACAENN568B::Init {filename aname} {
  source $filename
  foreach name [array names $aname] {
    set value [lindex [array get $aname $name] 1]
    if {![string is alpha $name]} {
      scan $name {%[a-z]%d} parameter channel
      SetParameter $channel $parameter $value
    }
    if {[string match $name offset]} {
      SetOffset $value
    }
  }
  DisableMUX
}
##
# InitIfChanged
#
# Init with hash
#   Initializes only if the input file has changed.
#
# @param filename - configuration file name.
# @param aname    - Name of the array containing the config data.
# @param hashfile - Name of file with hash of filename.
#
itcl::body ACAENN568B::InitIfChanged {filename aname hashfile} {
  set mustLoad 0;              # Assume we're lucky.
  set newhash [::md5::md5 -hex -file $filename];  # almost always need this
  
  # Figure out if we need to reload/recompute:
  
    if {[file exists $hashfile]} {
	set hashfd [open $hashfile r]
	set oldhash [gets $hashfd]
	close $hashfd
	
	if {$newhash != $oldhash} {
	    set mustLoad 1
	} else {
	    set mustLoad 0
	}
    } else {
	set mustLoad 1
    }
    #  At this point mustLoad is true if we must reload/recompute:
    
    if {$mustLoad} {
	Init $filename $aname
	
	# Rewrite the hash file with the updated has.
		
	set hashfd [open $hashfile w]
	puts $hashfd $newhash
	close $hashfd
    }
}
