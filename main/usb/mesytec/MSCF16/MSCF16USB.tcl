#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2015.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#    Jeromy Tompkins
#	   NSCL
#	   Michigan State University
#	   East Lansing, MI 48824-1321

package provide mscf16usb 1.0

package require snit
package require Utils


## @brief The USB interface for the MSCF-16 
#
# Provides some basic control over the MSCF-16 through a standard interface.
# The convention is adopted that the first channel is indexed at 0. This is
# consistent with other drivers.
#
# This opens a special file for communicating with the device. The channel is 
# owned by the instance of this class and is closed when the instance is
# destroyed. It depends on external code to ensure that no thrashing of the
# device occurs because on Linux, it is possible for multiple instances of this
# to be attached to the same file.
#
# Because the performance of communication over the uSB protocol is so poor, the
# driver utilizes a lazy update mechanism. The state of the module will only
# ever be read if a "set" operation has been carried out since the last read.
# Otherwise, the value is returned from the shadow state maintained by the
# instance. For this reason, best performance is achieved by performing all
# set operations first and then all get operations afterwards. The worst
# performance will be found by interleaving set and get operations all of the
# time. 
#
# Parsing of the state of the device is handle by an instance of the
# MSCF16DSParser snit::type.
snit::type MSCF16USB {

  variable m_serialFile ;# the communication channel to the device 
  variable m_needsUpdate ;# has device state changed since last update?
  variable m_moduleState ;# the dict storing the state

  component m_parser

  ## @brief Construct the driver 
  #
  # Opens the file
  #
  # @param serialFile   path to the special file
  #
  # @throws error if fails to open file
  constructor {serialFile} {
    install m_parser using MSCF16DSParser %AUTO%

    set m_serialFile [open $serialFile "r+"]

    chan configure $m_serialFile -blocking 0

    # we don't know anything about the state of the module
    # so we are certainly in need of an update
    set m_needsUpdate 1
    set m_moduleState [dict create]
  }

  ## @brief Close the file and destroy the parser
  #
  destructor {
    catch {$m_parser destroy}
    catch {close $m_serialFile}
  }

  ## brief Set the gain for a group
  #
  # To set the common setting for the device, use ch 4.
  # This translates the channel index provided to be consistent with what is
  # expected by the device.
  # To access the common value, use index 4.
  #
  # @param ch   group index (must be in range [0,4])
  # @param val  value to write (must be in range [0,15])
  #
  # @throws error if ch is out of range
  # @throws error if val is out of range
  method SetGain {ch val} {
    if {![Utils::isInRange 0 4 $ch]} {
      set msg "Invalid channel provided. Must be in range \[0,4\]."
      return -code error -errorinfo MSCF16USB::SetGain $msg
    }
    if {![Utils::isInRange 0 15 $val]} {
      set msg "Invalid value provided. Must be in range \[0,15\]."
      return -code error -errorinfo MSCF16USB::SetGain $msg
    }

    return [$self _Transaction [list SG [expr $ch+1] $val]]
  }

  ## @brief Retrieve the current gain value
  #
  # Updates the shadow state if needed.
  # To access the common value, use index 4.
  #
  # @param  ch  group index (must be in range [0,4])
  #
  # @return integer value requested
  #
  # @throws error if ch is out of range
  method GetGain {ch} {
    if {$m_needsUpdate} {
      $self Update
    }
    if {![Utils::isInRange 0 4 $ch]} {
      set msg "Invalid channel provided. Must be in range \[0,4\]."
      return -code error -errorinfo MSCF16USB::GetGain $msg
    }

    return [lindex [dict get $m_moduleState Gains] $ch]
  }


  ## @brief Set the shaping time for a channel
  #
  # This translates the group index provided up by one to be consistent with
  # what is accepted by the device.
  #
  # To access the common value, use index 4.
  #
  # @param ch   group index ( must be in range [0,4])
  # @param val  value to write (must be in range [0,3])
  #
  # @return ""
  #
  # @throws error if ch is out of range
  # @throws error if val is out of range
  method SetShapingTime {ch val} {
    if {![Utils::isInRange 0 4 $ch]} {
      set msg "Invalid channel provided. Must be in range \[0,4\]."
      return -code error -errorinfo MSCF16USB::SetShapingTime $msg
    }
    if {![Utils::isInRange 0 3 $val]} {
      set msg "Invalid value provided. Must be in range \[0,3\]."
      return -code error -errorinfo MSCF16USB::SetShapingTime $msg
    }
    return [$self _Transaction [list SS [expr $ch+1] $val]]
  }

  ## @brief Retrieve the value of the shaping time for a group
  #
  # To access the common value, use index 4.
  #
  # @param ch   group index ( must be in range [0,4])
  # 
  # @returns integer value requested
  #
  # @throws error if ch is out of range
  method GetShapingTime {ch} {
    if {$m_needsUpdate} {
      $self Update
    }
    if {![Utils::isInRange 0 4 $ch]} {
      set msg "Invalid channel provided. Must be in range \[0,4\]."
      return -code error -errorinfo MSCF16USB::GetShapingTime $msg
    }
    return [lindex [dict get $m_moduleState ShapingTime] $ch]
  }

  ## @brief Set the pole zero value for a channel
  # 
  # To set the common value, use index 16.
  #
  # @param ch   channel index ( must be in range [0,16])
  # @param val  value to write (must be in range [0,3])
  #
  # @return ""
  #
  # @throws error if ch is out of range
  # @throws error if val is out of range
  method SetPoleZero {ch val} {
    if {![Utils::isInRange 0 16 $ch]} {
      set msg "Invalid channel provided. Must be in range \[0,16\]."
      return -code error -errorinfo MSCF16USB::SetPoleZero $msg
    }
    if {![Utils::isInRange 0 255 $val]} {
      set msg "Invalid value provided. Must be in range \[0,255\]."
      return -code error -errorinfo MSCF16USB::SetPoleZero $msg
    }

    return [$self _Transaction [list SP [expr $ch+1] $val]]
  }

  ## @brief Retrieve the pole zero value for a channel
  # 
  # To access the common value, use index 16.
  #
  # @param ch   channel index ( must be in range [0,16])
  #
  # @return the integer value requested
  #
  # @throws error if ch is out of range
  method GetPoleZero {ch} {
    if {$m_needsUpdate} {
      $self Update
    }
    if {![Utils::isInRange 0 16 $ch]} {
      set msg "Invalid channel provided. Must be in range \[0,16\]."
      return -code error -errorinfo MSCF16USB::GetPoleZero $msg
    }

    return [lindex [dict get $m_moduleState PoleZero] $ch]
  }

  ## @brief Set the threshold value for a channel
  # 
  # To set the common value, use index 16.
  #
  # @param ch   channel index ( must be in range [0,16])
  # @param val  value to write (must be in range [0,255])
  #
  # @return response from device
  #
  # @throws error if ch is out of range
  # @throws error if val is out of range
  method SetThreshold {ch val} {
    if {![Utils::isInRange 0 16 $ch]} {
      set msg "Invalid channel provided. Must be in range \[0,16\]."
      return -code error -errorinfo MSCF16USB::SetThreshold $msg
    }
    if {![Utils::isInRange 0 255 $val]} {
      set msg "Invalid value provided. Must be in range \[0,255\]."
      return -code error -errorinfo MSCF16USB::SetThreshold $msg
    }

    return [$self _Transaction [list ST [expr $ch+1] $val]]
  }

  ## @brief Retrieve the threshold value for a channel
  # 
  # To access the common value, use index 16.
  #
  # @param ch   channel index ( must be in range [0,16])
  #
  # @return the integer value requested
  #
  # @throws error if ch is out of range
  method GetThreshold {ch} {
    if {$m_needsUpdate} {
      $self Update
    }
    if {![Utils::isInRange 0 16 $ch]} {
      set msg "Invalid channel provided. Must be in range \[0,16\]."
      return -code error -errorinfo MSCF16USB::GetThreshold $msg
    }

    return [lindex [dict get $m_moduleState Thresholds] $ch]
  }

  ## @brief Set the channel to be monitored
  #
  # It is important to note that this translates the channel index up by 1 to
  # write to the device to match the range accepted by the device.
  #
  # @param channel  the channel index (must be in range [0,15])
  #
  # @return response from device
  #
  # @throws error if channel is out of range
  method SetMonitor {channel} {
    if {![Utils::isInRange 0 15 $channel]} {
      set msg "Invalid channel provided. Must be in range \[0,15\]."
      return -code error -errorinfo MSCF16USB::SetMonitor $msg
    }
    return [$self _Transaction [list MC [expr $channel+1]]]
  }


  ## @brief Retrieve the index of the channel being monitored
  # 
  # The value returned from the device is decreased by 1 to match the
  # conventional range accepted by the driver. This ensures that all
  # communication with the device uses the channel range [0,15].
  #
  # @return channel index 
  method GetMonitor {} {
    if {$m_needsUpdate} {
      $self Update
    }
    set val [dict get $m_moduleState Monitor]
    return [expr $val-1]
  }

  ## @brief Sets the configuration mode of the device
  #
  # It appears that you can set individual channel values when the device is in
  # common mode. However, it is unwise to trust this behavior because other
  # Mesytec devices do not allow such interaction. So... be sure that whatever
  # you are trying to configure is appropriate to the config mode the device is
  # in at the time.
  # 
  # This maps "common" to 0 and "individual" to 1.
  #
  # @param mode   the config mode (either common or individual)
  # 
  # @returns response from the device
  #
  # @throws error if mode is neither "common" or "individual"
  method SetMode {mode} {
    set code 0
    switch $mode {
      common     {set code 0} 
      individual {set code 1} 
      default {
        set msg "Invalid mode provided. Must be either common or individual."
        return -code error $msg
      }
    }
    return [$self _Transaction [list SI $code]]
  }

  ## @brief Retrieve the config mode of the device
  #
  # Triggers an update of shadow state if required.
  #
  # @returns config mode (either "common" or "individual")
  #
  # @throws error if state of device is not understood by this driver
  method GetMode {} {
    if {$m_needsUpdate} {
      $self Update
    }

    set rawMode [dict get $m_moduleState Mode]
    
    set mode unknown
    switch $rawMode {
      single {set mode individual}
      common {set mode common} 
      default {
        set msg "Configuration mode of device ($rawMode) is not supported by "
        append msg "driver."
        return -code error MSCF16USB::GetMode $msg
      }
    } 
    return $mode
  }

  ## @brief Enable/Disable remote control operation
  #
  # @param on   boolean value indicating whether remote operation is turned on
  #
  # @returns response from device
  #
  # @throws error if on is not a boolean value
  method EnableRC {on} {
    if {![string is boolean $on]} {
      set msg "Invalid argument provided. Must be a boolean value."
      return -code error -errorinfo MSCF16USB::EnableRC $msg
    }

    if {[string is true $on]} {
      return [$self _Transaction "ON"]
    } else {
      return [$self _Transaction "OFF"]
    }
  }

  ## @brief Reads status or remote control operation
  #
  # Trigger update of shadow state if required.
  #
  # @return status of remote control (either 0 or 1)
  method RCEnabled {} {
    if {$m_needsUpdate} {
      $self Update
    }

    return [string is true [dict get $m_moduleState Remote]]
  }

  ## @brief Send the DS command and updates shadow state
  #
  # This also updates the flag indicating that the next Get operation requires
  # another update. 
  #
  method Update {} {
    set response [$self _Transaction "DS"]

    set m_moduleState [$m_parser parseDSResponse $response]
    set m_needsUpdate 0
  }

  #---------------------------------------------------------------------------#
  # Utility methods
  #

  ## @brief _Write to the device a command
  #
  # This is the low-level method for sending a command to the device without
  # reading back. It keeps track of whether the command string will end up
  # changing the device state or not. 
  #
  # @important It is the caller's responsibility to ensure that the command is
  #            valid.
  #
  # @param script   the command string
  method _Write {script} {
  # check that the command name (the first element of the script list) is
  # going to modify the state of the module
  
    if {[lindex $script 0] ni [list DS]} { 
    # we will need to update our state the next time the user queries a value
      set m_needsUpdate 1
    }

    puts $m_serialFile $script
    chan flush $m_serialFile
  }

  ## @brief Read response from the device
  #
  # Low-level command to read data from the device. The command will poll the
  # device until it receives the entire response. It consider a response
  # complete once the "mcfd-16>" string is found. Each successive attempt to
  # read from the device occurs after a 25 ms pause.
  #
  # @return response from device
  method _Read {} {
    set totalResponse "" ;# string we will keep building and ultimately return

    # read and append result to totalResponse
    set response [chan read $m_serialFile]
    append totalResponse $response 

    # keep trying to read until mscf> or mscf-RC> is found in the response
    # Depending on the state of remote control, the prompt returns a different
    # string.
    set patt {mscf(-RC){0,1}>$}
    set iter 0
    while {![regexp $patt $totalResponse]} {
      after 25 
      set response [chan read $m_serialFile]
      append totalResponse $response
      incr iter
    }

    # done
    return $totalResponse
  }

  ## @brief Complete a symmetric transaction (_Write then _Read)
  #
  # @param script basic command to execute
  # 
  # @returns response from device
  method _Transaction {script} {
    $self _Write $script

    return [$self _Read]
  }

  ## @brief Assigns the shadow state
  #
  # @warning There is no check to see whether the dict passed in is 
  #          good. It will potentially fail on the next Get operation,
  #          but might not. 
  #
  # @param state  a dict with the proper structure for the shadow state
  method _setModuleState {state} {
    set m_moduleState $state
  }

} ;# end of MSCF16USB snit::type



#-----------------------------------------------------------------------------#



## @brief A parser for the DS respone of the MSCF-16
#
# The actual format of the DS response depends heavily on the status of the
# software(firmware?) version of the device. As a result, the logic is
# externalized into this. Basically this just takes the full DS response as a
# string and then parses it into a proper dict.
#
# External callers should only rely on the parseDSResponse method to exist. All
# of the other methods are just utility methods and are subject to change if the
# format of the DS response changes.
snit::type MSCF16DSParser {

  
  variable mscf ;#!< the dict to fill

  ##  @brief Constructor is a no-op
  #
  constructor {args} {
  }

  ## @brief The main entry point
  # 
  # Basically, this splits the entire string into lines and then operates on
  # them one by one. It ignores the lines it is uninterested in.
  #
  # @param response  the complete DS response from the device
  #
  # @returns the parse dictionary
  method parseDSResponse {response} {
    set response [split $response "\n"]

    # this is just assumed to be in the right order b/c I don't have an
    # actual module to tell me what it should be.
    set tmp [$self parseGains [lindex $response 15]]
    dict set mscf Gains $tmp

    set tmp [$self parseThresholds [lindex $response 16]]
    dict set mscf Thresholds $tmp

    set tmp [$self parsePoleZero [lindex $response 17]]
    dict set mscf PoleZero $tmp

    set tmp [$self parseShapingTime [lindex $response 18]]
    dict set mscf ShapingTime $tmp

    set tmp [$self parseMonitor [lindex $response 20]]
    dict set mscf Monitor $tmp
    
    set tmp [$self parseConfigMode [lindex $response 24]]
    dict set mscf Mode $tmp

    set tmp [$self parseRemote [lindex $response 26]]
    dict set mscf Remote $tmp

    return $mscf
  }

  ## @brief parse the gain line
  #
  method parseGains {line} {

    set lineVals [list]
    set patt {^gains: (\d+ ){4}c:(\d+)$}
    if {[regexp $patt $line]} {
      set line [string range $line 7 end]
      set lineVals [split $line " "]
      set comVal [string range [lindex $lineVals 4] 2 end]
      lset lineVals 4 $comVal
    }
    return $lineVals
  }

  ## @brief parse the threshold line
  #
  method parseThresholds {line} {
    set lineVals [list]

    set patt {^threshs: (\d+ ){16}c:(\d+)$}
    if {[regexp $patt $line]} {
      set line [string range $line 9 end]
      set lineVals [split $line " "]
      set comVal [string range [lindex $lineVals 16] 2 end]
      lset lineVals 16 $comVal

    }
    return $lineVals
  }

  ## @brief parse the pole zero line
  #
  method parsePoleZero {line} {

    set lineVals [list]

    set patt {^pz: (\d+ ){16}c:(\d+)$}
    if {[regexp $patt $line]} {
      set line [string range $line 4 end]
      set lineVals [split $line " "]
      set comVal [string range [lindex $lineVals 16] 2 end]
      lset lineVals 16 $comVal

    }
    return $lineVals
  }

  ## @brief parse the shaping time line
  #
  method parseShapingTime {line} {
    set lineVals [list]
    set patt {^shts: (\d+ ){4}c:(\d+)$}
    if {[regexp $patt $line]} {
      set line [string range $line 6 end]
      set lineVals [split $line " "]
      set comVal [string range [lindex $lineVals 4] 2 end]
      lset lineVals 4 $comVal
    }
    return $lineVals

  }

  ## @brief parse the line containing the multiplicity
  #
  method parseMultiplicity {line} {
    set lineVals [list]
    set patt {^mult: (\d+) (\d+)$}
    if {[regexp $patt $line v0 v1]} {
      set lineVals [list $v0 $v1]
    }
    return $lineVals
  }

  ## @brief parse the line containing info of monitor
  #
  method parseMonitor {line} {
    set nSet [scan $line "monitor: %d" monitor]

    if {$nSet != 1} {
      return -code error "Failed to parse monitor"
    }
    return $monitor
  }

  ## @brief parse the line containing info of configuration mode
  #  
  method parseConfigMode {line} {
     set nSet [scan $line "%s mode" mode]

     if {$nSet != 1} {
       return -code error "Failed to parse config mode"
     }

     return $mode
  }

  ## @brief parse the line containing info state of remote control
  #  
  method parseRemote {line} {
     set nSet [scan $line "rc %s" remote ]
     if {$nSet != 1} {
       return -code error "Failed to parse rc mode"
     }
     return $remote
  }
    
} ;# end of MSCF16DSParser snit::type

