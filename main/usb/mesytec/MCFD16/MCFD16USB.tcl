#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
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


package provide mcfd16usb 1.0

package require snit
package require Utils


## @brief USB Communications Package for the Mesystec MCFD16
#
# This package provides a higher-level API for configuring the MCFD-16 device.
# In general, individual control of the device parameters is write only. That is
# not to say that one cannot read the state of the module. However, the module
# will only return the values of the module via a full dump of all parameters.
# This is extremely slow and requires a good deal of parsing to extract the
# parameters. For this reason, the device will only read from the module when it
# needs to. the mechanism to accomplish is by keeping a parsed copy of the data
# returned from the module at its last parameter dump. As long as no writes to
# the module are attempted, the user can read the state without any new
# communication with the module, because the cached state is valid. However,
# once a write operation occurs, there is the possibility for the internal state
# of the module to be different than the state cached by this driver. An
# internal flag will be set such that the next "Get" operation called will read
# the parameter values from the module, parse, and cache them. Given this
# functionality, optimal performance will be achieved when all writes are
# performed together before any subsequent reads. 
snit::type MCFD16USB {

  variable m_serialFile ;# the communication channel to the device 
  variable m_needsUpdate ;# has device state changed since last update?
  variable m_moduleState ;# the dict storing the state


  ## @brief Constructor
  #
  constructor {serialFile} {
    set m_serialFile [open $serialFile "r+"]

    chan configure $m_serialFile -blocking 0

    # we don't know anything about the state of the module
    # so we are certainly in need of an update
    set m_needsUpdate 1
    set m_moduleState [dict create]

  }

  ## @brief Destructor 
  #
  # Closes the file channel used for communication with the device
  destructor {
    catch {close $m_serialFile}
  }

  ## @brief Set threshold for a specific channel
  #
  # @param ch     target channel (must be in range [0,16])
  # @param thresh threshold value (must be in range [0,255])
  #
  # @returns response message from the device
  #
  # @throws error if ch out of range
  # @throws error if thresh out of range
  method SetThreshold {ch thresh} {
  # check for valid channel
    if {![Utils::isInRange 0 16 $ch]} {
      set msg {Invalid channel argument. Must be in range [0,16].}
      return -code error -errorinfo MCFD16USB::SetThreshold $msg
    }

    # check for valid threshold 
    if {![Utils::isInRange 0 255 $thresh]} {
      set msg {Invalid threshold argument. Must be in range [0,255].}
      return -code error -errorinfo MCFD16USB::SetThreshold $msg
    }

    # all is well with the arguments
    return [$self _Transaction "ST $ch $thresh"]
  }

  ## @brief Retrieve threshold value for specific channel
  #
  # @param ch   the channel (must be in range [0,16])
  #
  # @returns  value associated with the channel
  #
  # @throws error if the channel is out of range
  method GetThreshold {ch} {
  # check for valid channel
    if {![Utils::isInRange 0 16 $ch]} {
      set msg {Invalid channel argument. Must be in range [0,16].}
      return -code error -errorinfo MCFD16USB::GetThreshold $msg
    }

    if {$m_needsUpdate} {
      $self Update
    }

    return [lindex [dict get $m_moduleState {Threshold}] $ch]
  }

  ## @brief Configure polarity for a specific channel pair
  #
  # @param chanPair the channel pair (must be in range [0,8]).
  # @param val      polarity value (either "pos" or "neg")
  #
  # @throws error if chanPair is out of range
  # @throws error if val is neither "neg" or "pos"
  method SetPolarity {chanPair val} {
  # check for channel pair argument
    $self _ThrowOnBadChannelPair $chanPair

    # this is the operational code and the check for the value
    switch $val {
      pos {$self _Transaction "SP $chanPair 0"} 
      neg {$self _Transaction "SP $chanPair 1"} 
      default  {
        set msg {Invalid value provided. Must be "pos" or "neg".}
        return -code error -errorinfo MCFD16USB::SetPolarity $msg
      } 
    }
  }

  ## @brief Retrieve polarity setting for a given channel pair
  # 
  # The mechanics behind retrieving the data is the same as described in the
  # description of the class.
  #
  # @param chanPair the channel pair
  #
  # @returns the polarity value
  # @retval neg - negative polarity
  # @retval pos - positive polarity
  method GetPolarity {chanPair} {
    return [$self _GetChannelPairParam Polarity $chanPair]
  }


  ## @brief Set the gain value for a channel pair
  #
  # @param chanPair the channel pair (must be in range [0,8]).
  # @param val      gain value (must be either 0, 1, or 2)
  #
  # @throws error if chanPair is out of range
  # @throws error if val is not 0, 1, or 2
  method SetGain {chanPair val} {
    if {$val ni [list 0 1 2]} {
      set msg "Invalid gain value. Must be either 0, 1, or 2."
      return -code error -errorinfo MCFD16USB::SetGain $msg
    }

    $self _ThrowOnBadChannelPair $chanPair

    # the USB takes an input of 1, 3, or 10 so there will be a mapping
    # of the user's argument to what can be written.
    # 0 -> 1 , 1 -> 3, 2 -> 10
    set val [lindex {1 3 10} $val]

    # all is well with the arguments
    return [$self _Transaction "SG $chanPair $val"]
  }

  ## @brief Retrieve gain setting for a given channel pair
  # 
  # The mechanics behind retrieving the data is the same as described in the
  # description of the class.
  #
  # @param chanPair the channel pair
  #
  # @returns gain value 
  #
  # @throws error if chanPair is out of range
  method GetGain {chanPair} {

    set val [$self _GetChannelPairParam Gain $chanPair]

    # for consistency with RCbus, the value needs to be mapped back to 
    # the range [0,2].
    if {[catch {dict get {1 0 3 1 10 2} $val} mappedVal]} {
      set msg "MCFD16USB::GetGain Value read from the device ($val) is not "
      append msg "understood by the driver."
      return -code error $msg
    }

    # mapping succeeded, so return the new mapping
    return $mappedVal
  }

  ## @brief Enable optional bandwidth filter for fast rise time signal
  # 
  # The MCFD16 provides an optional extra bandwidth filter to handle signals
  # with rise times below 10 ns. This will enable or disable the use of this.
  #
  # @param on   boolean to determine whether to enable or disable the limiter
  #
  # @returns string message from the module
  #
  # @throws error if on is not a boolean value
  method EnableBandwidthLimit {on} {
    if {![string is boolean $on]} {
      set msg "Invalid argument provided. Must be a boolean type."
      return -code error -errorinfo MCFD16USB::EnableBandwidthLimit $msg
    }

    # all is well with the arguments
    return [$self _Transaction "BWL [string is true $on]"]
  }

  ## @brief Inquire whether the bandwidth limiter is set or not
  #
  # @returns boolean
  # @retval 0 - disabled
  # @retval 1 - enabled
  method GetBandwidthLimit {} {
    return [$self _GetBooleanParam {Bandwidth limit} {Off 0 On 1}]
  }

  ## @brief Set into CFD or LED mode 
  #       
  # This feature is only effective if the device is in remote control mode.
  # Otherwise, it is not effective.
  #
  # @param "led" for leading edge, "cfd" for constrant fraction
  #
  # @throws error if parameter is neither led or cfd.
  method SetDiscriminatorMode {mode} {
    switch $mode {
      led {$self _Transaction "CFD 0"}
      cfd {$self _Transaction "CFD 1"}
      default {
        set msg {Invalid argument provided. Must be either "led" or "cfd".}
        return -code error -errorinfo MCFD16USB::SetDiscriminatorMode $msg
      }
    }
  }

  ## @brief Inquire whether in LED or CFD mode
  #
  # @returns "led" for leading edge, "cfd" for constant fraction
  #
  method GetDiscriminatorMode {} {
    set convTable {{Constant fraction} cfd {Leading Edge} led}
    return [$self _GetBooleanParam Discrimination $convTable]
  }

  ## @brief Establish the width of output for a channel pair
  # 
  # The value provided to this method is translated to an actual time width
  # according to the table in the manual. The minimum value (16) corresponds to 6 ns
  # and the max value (222) corresponds to 664 ns. The is a mapping of all
  # values in between. 
  #
  # @param chanPair   the channel pair index to target (must be in range [0,8])
  # @param value      the width (must be in range [16,222])
  #
  # @returns message response from the module
  #
  # @throws error if chanPair is out of range
  # @throws error if value is out of range
  method SetWidth {chanPair value} {
    $self _ThrowOnBadChannelPair $chanPair

    if {![Utils::isInRange 16 222 $value]} {
      set msg {Invalid width argument provided. Must be in range [16,222].}
      return -code error -errorinfo MCFD16USB::SetWidth $msg
    }

    return [$self _Transaction "SW $chanPair $value"]
  }

  ## @brief Retrieve the actual signal width in ns
  #
  # The module returns the width after it has been converted into a time. So
  # this actually provides the user the width of the signal in units of ns. It
  # is important to recognize then that the return value of this method will
  # almost certainly be different than the value used by a prior SetWidth call.
  #
  # @param chanPair   the channel pair (must be in range [0,8])
  #
  # @returns width in nanoseconds
  #
  # @throws error if chanPair is out of range
  method GetWidth {chanPair} {
    return [$self _GetChannelPairParam WidthRaw $chanPair]
  }


  ## @brief Set the deadtime for a channel value pair
  #
  # This is just like the width parameter in that the device converts the value
  # provided into an actual time. The mapping is listed explicitly in the manual
  # but it varies:
  #  min value = 27 --> 20 ns
  #  max value = 222 --> 64 ns
  #
  # @param chanPair the channel pair targeted (must be in range [0,8])
  # @param value the value to set the deadtime to (must be in range [27,222])
  #
  # @returns response from the device
  #
  # @throws error if chanPair is out of range
  # @throws error if value is out of range
  method SetDeadtime {chanPair value} {
    $self _ThrowOnBadChannelPair $chanPair

    if {![Utils::isInRange 27 222 $value]} {
      set msg {Invalid deadtime argument provided. Must be in range [27,222].}
      return -code error -errorinfo MCFD16USB::SetDeadtime $msg
    }

    return [$self _Transaction "SD $chanPair $value"]
  }


  ## @brief Retrieves the dead time for a channel pair
  #
  # This returns the mapped dead time value.  It will NOT return the same value
  # that was written.
  #
  # @param chanPair the targeted channel pair
  #
  # @returns the dead time value in nanoseconds
  method GetDeadtime {chanPair} {
    return [$self _GetChannelPairParam DeadtimeRaw $chanPair]
  }

  ## @brief Set the delay line for a channel pair
  # 
  # @param chanPair   the channel pair targeted (must be in range [0,8])
  # @param value      delay setting (must be in range [0,4])
  # 
  # @returns response from  module
  #
  # @throws error if chanPair is out of range
  # @throws error if value is out of range
  method SetDelay {chanPair value} {

    $self _ThrowOnBadChannelPair $chanPair

    if {![Utils::isInRange 0 4 $value]} {
      set msg {Invalid delay argument provided. Must be in range [0,4].}
      return -code error -errorinfo MCFD16USB::SetDelay $msg
    }
   
    # the arg input has a range between 0 and 4 for consistency with rcbus.
    # However, usb protocol natively takes a range between 1 and 5. We 
    # will map the users value by adding 1 to it.
    incr value 1

    return [$self _Transaction "SY $chanPair $value"]
  }

  ## @brief Retrieve the delay setting for a channel pair
  #
  # @param chanPair   targeted channel pair (must be in range [0,8])
  #
  # @returns value of the delay
  #
  # @throws error if chanPair is out of range
  method GetDelay {chanPair} {
    set val [$self _GetChannelPairParam Delay $chanPair]

    # to account for the mapping on Set, we will map to the range [0,4] from
    # the native range of [1,5] returned from the device
    incr val -1

    return $val
  }

  ## @brief Set the constant fraction for a channel pair
  #
  # @param chanPair   targeted channel pair (must be in range [0,8])
  # @param value      must be either 20 or 40
  #
  # @returns response from device
  # 
  # @throws error if chanPair is out of range
  # @throws error if value is neither 20 or 40
  method SetFraction {chanPair value} {

    $self _ThrowOnBadChannelPair $chanPair

    if {$value ni [list 20 40]} {
      set msg "Invalid fraction argument provided. Must be either 20 or 40."
      return -code error -errorinfo MCFD16USB::SetFraction $msg
    }

    return [$self _Transaction "SF $chanPair $value"]
  }

  ## @brief Retrieve the fraction setting for a channel pair
  #
  # @param chanPair   targeted channel pair (must be in range [0,8])
  #
  # @returns fraction setting
  #
  # @throws error if chanPair is out of range
  method GetFraction {chanPair} {
    return [$self _GetChannelPairParam Fraction $chanPair]
  }

  ## @brief Enable/Disable channels
  #
  # The channel mask is used to disable channels. Each bit corresponds to a
  # pair of channels. If the bit is set to 1, the corresponding channels are
  # disabled. For example, bit 0 corresponds to chns 0 and 1 and bit 7 to either
  # chns 14 and 15. 
  #
  # @param  mask  bit mask  (must be in range [0,255])
  #
  # @returns response from device
  #
  # @throws error if mask is out of range
  method SetChannelMask {mask} {

    if {![Utils::isInRange 0 255 $mask]} {
      set msg {Invalid mask argument provided. Must be in range [0,255].}
      return -code error -errorinfo MCFD16USB::SetChannelMask $msg
    }

    return [$self _Transaction "SK $mask"]
  }

  ## @brief Retrieve the channel mask for a bank of channels
  #
  # @returns integer value of channel mask
  method GetChannelMask {} {

    if {$m_needsUpdate} {
      $self Update
    }

    return [dict get $m_moduleState {Mask register}]
  }

  ## @brief Enable either of the test pulsers
  #
  # There are two test pulsers that are chosen from.
  # Index   1 --> 2.5 MHz
  #         2 --> 1.22 kHz
  #
  # @params index   pulser index (must be either 1 or 2)
  #
  # @returns response from the device
  #
  # @throws error if index is neither 1 or 2
  method EnablePulser {index} {
    if {$index ni [list 1 2]} {
      set msg {Invalid pulser index provided. Must be either 1 or 2.}
      return -code error -errorinfo MSCF16::EnablePulser $msg
    }

    return [$self _Transaction "P$index"]
  }

  ## @brief Disable the pulser
  #
  # @returns response from the device
  method DisablePulser {} {
    return [$self _Transaction "P0"]
  }

  ## @brief Inquire as to whether the pulser is on or off
  # 
  # @returns boolean
  # @retval 0 - off
  # @retval 1 - on
  method PulserEnabled {} { 
    return [$self _GetBooleanParam {Test pulser} {Off 0 On 1}]
  }

  ## @brief retrieve firmware and software version
  #
  # @returns response from device to "V" command
  method ReadFirmware {} {
    return [$self _Transaction "V"]
  }

  ## @brief Set into or remove from remote control
  #
  # @param on   boolean to enable or disable rc mode
  #
  # @returns response from the device
  method EnableRC {on} {
    if {![string is boolean $on]} {
      set msg "Invalid argument provided. Must be a boolean value."
      return -code error -errorinfo MCFD16USB::EnableRC $msg
    }

    if {[string is true $on]} {
      return [$self _Transaction "ON"]
    } else {
      return [$self _Transaction "OFF"]
    }
  }

  ## @brief Set individual or common configuration mode
  #
  # @param mode   Configuration mode (either "individual" or "common")
  #
  # @returns response from the device
  # @throws error if invalid argument
  method SetMode {mode} {
    if {$mode ni [list individual common]} {
      set msg {Invalid argument provided. Must be either "individual" or }
      append msg {"common".}
      return -code error -errorinfo MCFD16USB::SetMode $msg
    }

    if {$mode eq "individual"} {
      return [$self _Transaction "MI"]
    } else {
      return [$self _Transaction "MC"]
    }
  }

  method GetMode {} {
    return [$self _GetBooleanParam {Operating mode} \
                {Individual individual Common common}]
  }

  ## @brief Set the a trigger source
  #
  # @param trigId   the trigger id to target (0, 1, or 2)
  # @param source   the source to use for the trigger (or, multiplicity, pair_coinc, mon, pat_or_0, pat_or_1, gg)
  # @param veto     whether to enable vetoing (boolean)
  #
  # This is just high level implementation of the TR command
  #
  # @returns result of _Transaction
  method SetTriggerSource {trigId source veto} {
    if {$trigId ni [list 0 1 2]} {
        set msg "Invalid trigger id argument provided. Must be 0, 1, or 2."
        return -code error -errorinfo MCFD16USB::SetTriggerSource $msg
    }

    set sourceBits [dict create or 1 multiplicity 2 pair_coinc 4 mon 8 pat_or_0 16 pat_or_1 32 gg 128]
    if {$source ni [dict keys $sourceBits]} {
        set msg "Invalid source provided. Must be or, multiplicity, pair_coinc, mon, pat_or_0, pat_or_1, or gg."
        return -code error -errorinfo MCFD16USB::SetTriggerSource $msg
    }

    set value [dict get $sourceBits $source]
    if {[string is true $veto]} {
        set value [expr {$value + 0x40}]
    }

    return [$self _Transaction "TR $trigId $value"]
  }

  ## @brief Retrieve the trigger source
  #
  # If necessary, this will update the cached state of the device.
  #
  # @param trigId   the trigger index (must be 0, 1, or 2)
  #
  # @returns list. first element is source, second element is veto enabled
  method GetTriggerSource {trigId} {

    if {$trigId ni [list 0 1 2]} {
        set msg "Invalid trigger id argument provided. Must be 0, 1, or 2."
        return -code error -errorinfo MCFD16USB::GetTriggerSource $msg
    }

    if {$m_needsUpdate} {
      $self Update
    }

    set code [dict get $m_moduleState Trig${trigId}_src]

    set vetoEnabled [expr {($code&0x40)!=0}]
    set source      [expr {$code&0xbf}]

    # SW 2.18 supports not having bit set in the source mask.
    
    
    set sourceNameMap [dict create 0 none  1 or 2 multiplicity 4 pair_coinc 8 \
                                    mon 16 pat_or_0 32 pat_or_1 128 gg]
    set sourceName [dict get $sourceNameMap $source]

    return [list $sourceName $vetoEnabled]
  }

  ## @brief Set which channels contribute to the OR
  #
  # @param trigId   the or pattern to set (0 or 1)
  # @param pattern  channels to set (must be in range [0, 65535])
  #
  # The pattern should specify the channels to use by setting bits. Each bit corresponds to 
  # a channel. Bit 0 --> Channel 0, Bit 1 --> Channel 1, etc. 
  # 
  # @returns result of last transactions
  method SetTriggerOrPattern {trigId pattern} {
    if {$trigId ni [list 0 1]} {
        set msg "Invalid pattern id argument provided. Must be 0 or 1."
        return -code error -errorinfo MCFD16USB::SetTriggerOrPattern $msg
    }

    if {![Utils::isInRange 0 0xffff $pattern]} {
      set msg {Invalid bit pattern provided. Must be in range [0,65535].}
      return -code error -errorinfo MCFD16USB::SetTriggerOrPattern $msg
    }

    set lowBits [expr {$pattern & 0xff}]
    set highBits [expr {($pattern>>8) & 0xff}]

    set trigOffset [expr {$trigId*2}]

    set result [$self _Transaction "TP $trigOffset $lowBits"]
    return [$self _Transaction "TP [expr {$trigOffset+1}] $highBits"]
  }


  ## @brief Retrieve the configurable OR pattern
  #
  # This updates the cached state if necessary.
  #
  # @param patternId    index of the pattern (must be 0 or 1)
  #
  # @returns integer whose set bits represent the channel states.
  #
  method GetTriggerOrPattern {patternId} {

    if {$patternId ni [list 0 1]} {
        set msg "Invalid pattern id argument provided. Must be 0 or 1."
        return -code error -errorinfo MCFD16USB::GetTriggerOrPattern $msg
    }

    if {$m_needsUpdate} {
      $self Update
    }

    return [dict get $m_moduleState or${patternId}_pattern]
  }


  ## @brief Write the fast veto register
  #
  # This turns on and off direct vetoing of the discriminators
  #
  # @param onoff  boolean value 
  #
  # @returns repsonse of the device
  method SetFastVeto {onoff} {
    return [$self _Transaction "SV [string is true $onoff]"]
  }

  ## @brief Read the fast veto register
  #
  #  If needed ,this will update the internally cached state of 
  #  the module.
  #
  # @returns  boolean
  #
  method GetFastVeto {} {

    if {$m_needsUpdate} {
      $self Update
    }

    return [dict get $m_moduleState fast_veto]
  }

  ## @brief Check whether in remote control mode
  #
  # @returns boolean
  # @retval 0 - not in remote control
  # @retval 1 - remote control mode enabled
  method RCEnabled {} {
    return [$self _GetBooleanParam {Remote Control} {Off 0 On 1}]
  }

  ## @brief Send the DS command and parse response
  #
  method Update {} {
    set response [$self _Transaction "DS"]

    set m_moduleState [$self _ParseDSResponse $response]

    set response [$self _Transaction "DT"]
    
    set triggerDict [$self _ParseDTResponse $response]
    dict for {key val} $triggerDict {
      dict set m_moduleState $key $val
    }

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
    if {[lindex $script 0] ni [list DP DT DS V ? H]} { 
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

    # keep trying to read until mcfd-16> is found in response
    while {![string match "mcfd-16>" [string range $totalResponse end-7 end]]} {
      after 25 
      set response [chan read $m_serialFile]
      append totalResponse $response
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

  ## @brief Throws when index is not in range [0,8] 
  # 
  #  @param chanPair  index of a channel pair 
  #
  #  @returns ""
  #  @throws error if channel pair is out of range
  method _ThrowOnBadChannelPair {chanPair} {
    if {![Utils::isInRange 0 8 $chanPair]} {
      set msg {Invalid channel pair provided. Must be in range [0,8].}
      return -code error -errorinfo MCFD16USB::SetPolarity $msg
    }
  }

  ## @brief Store a dictionary containing the parsed module state 
  #
  # The user should pass a dictionary of the following characteristics.
  # It should have keys pointing a list of minimum length. These are tabulated
  # below:
  # Key             Length
  # --------------  ------
  # Threshold        17 
  # Gain              9
  # Width             9
  # Deadtime          9
  # Delay             9
  # Fraction          9
  # Polarity          9
  # Mask register     1
  # Discrimination    1
  # Operating mode    1
  # Bandwidth limit   1
  # Remote control    1
  # Test pulser       1
  # 
  # @param stateDict  the dictionary of the parameters
  #
  # @returns ""
  method _CacheModuleState {stateDict} {
    set m_moduleState $stateDict
  }

  # ------ PARSING UTILITIES ---------------------------------------------#

  ## @brief The full parsing algorithm for a DS response
  #
  # @param response from a DS command
  #
  method _ParseDSResponse {response } {
    set lines [split $response "\n"]

    set parsedResponse [list]

    # first line should be DS
    if {[lindex $lines 0] ne "DS"} {
      set msg "Response being processed does not correspond to DS parameter"
      return -code error -errorinfo MCFD16USB::_ParseDSResponse $msg
    }

    # line 1 is a space

    # parse threhsolds
    lappend parsedResponse [$self _ParseThresholds [lrange $lines 2 3]] 

    # parse the gain
    lappend parsedResponse [$self _ParseChanPairLine [lindex $lines 4]]
    # parse the width
    lappend parsedResponse [$self _ParseChanPairLine [lindex $lines 5] 1]
    # parse the deadtime
    lappend parsedResponse [$self _ParseChanPairLine [lindex $lines 6] 1]
    # parse the delay
    lappend parsedResponse [$self _ParseChanPairLine [lindex $lines 7] 1]
    # parse the fraction
    lappend parsedResponse [$self _ParseChanPairLine [lindex $lines 8] 1]
    # parse the polarity
    lappend parsedResponse [$self _ParseChanPairLine [lindex $lines 9]]

    # line 10 is a space

    # parse the mask
    lappend parsedResponse [$self _ParseMaskReg [lindex $lines 11]]
    # parse the discrim mode
    lappend parsedResponse [$self _ParseSimpleLine [lindex $lines 12]]

    # we don't really care about the gate/delay gen state
    # lappend parsedResponse [$self _ParseSimpleLine [lindex $lines 13]]
    # we don't really care about the coinc time
    # lappend parsedResponse [$self _ParseSimpleLine [lindex $lines 14]]

    # operating mode
    lappend parsedResponse [$self _ParseSimpleLine [lindex $lines 15]]
    # bandwidth
    lappend parsedResponse [$self _ParseSimpleLine [lindex $lines 16]]
    # rc mode
    lappend parsedResponse [$self _ParseSimpleLine [lindex $lines 17]]

    # # frequency monitor
    # lappend parsedResponse [$self _ParseSimpleLine [lindex $lines 18]]

    # pulser
    lappend parsedResponse [$self _ParseSimpleLine [lindex $lines 19]]

    ## hardware settings
    #    lappend parsedResponse [$self _ParseSimpleLine [lindex $lines 20]]

    set stateDict [$self _TransformToDict $parsedResponse]
    
    set widths [dict get $stateDict Width]
    set list [$self _ConvertTimeWidthsToRawWidths $widths]
    dict set stateDict WidthRaw $list

    set deadtimes [dict get $stateDict Deadtime]
    set list [$self _ConvertDeadtimesToRaw $deadtimes]
    dict set stateDict DeadtimeRaw $list

    return $stateDict
  }

  method _ParseDTResponse {response} {
    set parsedResponse [list]

    set responseLines [split $response "\n"]

    lappend parsedResponse [$self _ParseFastVeto [lindex $responseLines 4]]

    set orPatterns [$self _ParseOredPattern [lindex $responseLines 7]]
    set parsedResponse [concat $parsedResponse [concat $orPatterns]]

    lappend parsedResponse [$self _ParseTriggerSource [lindex $responseLines 11]]
    lappend parsedResponse [$self _ParseTriggerSource [lindex $responseLines 12]]
    lappend parsedResponse [$self _ParseTriggerSource [lindex $responseLines 16]]

    set stateDict [$self _TransformToDict $parsedResponse]
    return $stateDict
  }

  method _ParseFastVeto {line} {
    set result [$self _SplitAndTrim $line ":"]

    set state 1
    if {[lindex $result 1] eq "disabled"} {
      set state 0
    }

    return [dict create name fast_veto values $state]
  }


  method _ParseOredPattern line {
    set parse1 [$self _SplitAndTrim $line ","]

    set or0Pattern [lindex [$self _SplitAndTrim [lindex $parse1 0] ":"] 2]
    set or1Pattern [lindex [$self _SplitAndTrim [lindex $parse1 1] ":"] 1]

    set or0Pattern "0x[string trimleft $or0Pattern 0]"
    set or1Pattern "0x[string trimleft $or1Pattern 0]"

    if {$or0Pattern eq "0x"} {
      set or0Pattern 0
    } else {
      set or0Pattern [expr $or0Pattern]
    }


    if {$or1Pattern eq "0x"} {
      set or1Pattern 0
    } else {
      set or1Pattern [expr $or1Pattern]
    }

    return [list [dict create name or0_pattern values $or0Pattern] \
                 [dict create name or1_pattern values $or1Pattern] ]
  }

  method _ParseTriggerSource line {
    set results [$self _SplitAndTrim $line ":"]
    set values [split [lindex $results 1] " "]

    set bits [list]
    foreach value $values {
      if {$value ne {}} {
        lappend bits $value
      }
    }

    set bits [lreverse $bits]

    set value 0
    for {set i 0} {$i<[llength $bits]} {incr i} {
        set value [expr {$value | ([lindex $bits $i]<<$i)}]
    }

    return [dict create name "[lindex $results 0]_src" values $value]
  }

  ## @brief Split a line into tokens that are trimmed
  #
  # Examplified behavior...
  #   $self _SplitAndTrim " dogs : bark : here "
  # would return:
  #   [[dogs] [bark] [here]]  (sq brackets should actually be curly)
  #
  # @param line the string to tokenize
  # @param del  the splitting delimiter
  #
  # @returns list of trimmed tokens
  method _SplitAndTrim {line del} {

  # split at the colon
    set split [split $line $del]

    # trim each part
    set result [list]
    foreach token $split {
      lappend result [string trim $token]
    }

    return $result
  }

  ## @brief Remove all dashes from a string
  #
  # Example behavior:
  #   $self _RemoveDash "3 4 5 - 7"
  # would return
  # "3 4 5 7"
  #
  # @param line the string to operate on
  #
  # @returns new string without dashes
  method _RemoveDash {line} {

  # basically we just split at all of the dashes and then replace it with a space
    return [join [$self _SplitAndTrim $line "-"] " "]
  }

  ## @brief Handle keys with a single element list
  #
  # Can split into two element and optionally strip units too.
  # Stripping the unit causes only the first word of the name to be returned.
  #
  # Example with no strip unit:
  #   $self _ParseSimpleLine "help (word): me"
  # returns
  # [name [help (word)] values me] (sq brackets should be curly)
  #
  # Example with strip unit:
  #   $self _ParseSimpleLine "help (word): me" 1
  # returns
  # [name help values me] (sq brackets should be curly)
  #
  # @param line   string to process
  # @param stripUnit whether to further break down name
  method _ParseSimpleLine {line {stripUnit 0}} {
    set split [$self _SplitAndTrim $line ":"]

    set name [lindex $split 0]
    if {$stripUnit} {
      set name [$self _StripUnit $name]
    }

    # generate the dict to return
    return [dict create name $name values [lindex $split 1]]
  } 

  ## @brief Retrieve first word of a string
  #
  # @param str  the string operate on
  #
  # @returns  first word of string
  method _StripUnit str {
    return [lindex [split $str " "] 0]
  }

  ## @brief Convenience algorithm for parsing parameter for channel pair
  #
  # This combines a few utility functions into this more complex one. Behavior
  # is best shown by example.
  #
  #  $self _ParseChanPairLine "ABC : 0 1 2 3 4 5 6 7 - 8"
  # returns
  #  [name ABC values [0 1 2 3 4 5 6 7 8]] (sq brackets should be curly)
  # 
  # It is possible to strip the unit as well. See _ParseSimpleLine...
  #
  # @param line string to operate on
  # @param stripUnit  boolean whether to use only first word of name
  #
  # @returns dict with keys : name and values
  method _ParseChanPairLine {line {stripUnit 0}} {

    set splitLine [$self _SplitAndTrim $line ":"]

    set name [lindex $splitLine 0]
    if {$stripUnit} {
      set name [$self _StripUnit $name]
    }

    set valStr [lindex $splitLine 1]

    set valStr [$self _RemoveDash $valStr]

    # locate the dash and remove it with its trailing space
    # convert to a list
    set vals [split $valStr " "]

    # form the response
    return [dict create name $name values $vals]
  }

  ## @brief Convenience parsing for the threhsolds
  # 
  # Because the threshold parameters are returned in a 2 line
  # format, the format must be handled uniquely. Another example
  #
  #   set lines [[Threshold:  0 1 2 3 4 5 6 7]
  #              [            8 9 0 1 2 3 4 5 - 6]] (sq brkts --> curly)
  #   $self _ParseThresholds $lines
  # returns
  #   [name Threshold values [0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6]] 
  #
  # @param lines  the string to parse
  #
  method _ParseThresholds {lines} {

    set line0 [lindex $lines 0]
    set splitLine [$self _SplitAndTrim $line0 ":"]

    set name [lindex $splitLine 0]
    set valStr0 [lindex $splitLine 1]

    set line1 [string trim [lindex $lines 1]]
    set valStr1 [$self _RemoveDash $line1]

    set vals [concat [split $valStr0 " "] [split $valStr1 " "]]

    return [dict create name $name values $vals]
  }

  ## @brief Convenience parsing for the mask reg
  #
  # We effectively convert a bitfield response to an integer. 
  #
  # @param line   string to be parsed
  #
  # @returns dict of keys: name and values
  method _ParseMaskReg {line} {
    set splitLine [split $line ":"]
    if {[llength $splitLine]!=3} {
      set msg "Failed to understand parse : \"$line\""
      return -code error -errorinfo MCFD16USB::_ParseMaskReg $msg
    }

    # name is the first eleement of list
    set name [string trim [lindex $splitLine 0]]

    # we still have some work to do to get the value of the mask
    # it should be something like " 00000000 (0)" at this point. 
    # Trim it and split it at all spaces. The value is in the first element
    set valStr [string trim [lindex $splitLine 2]]
    set valStr [lindex [$self _SplitAndTrim $valStr " "] 0]

    set valStr [string reverse $valStr]
    binary scan [binary format b32 $valStr] i val
    return [dict create name $name values $val]
  }

  ## @brief Collapse of list of dicts into one big dict
  #
  # Given a list of dicts each with 2 keys. This will create a new 
  # dict whose keys are the first key of each list and the value
  # of that key is the value of the second key in the list element. 
  #
  # Best shown by example:
  #  $self _TransformToDict [[name N0 values V0] [name N1 values V1]] (sq [ -->  curly)
  # returns
  #  [N0 V0 N1 V1]
  # 
  # @param list   list of dicts each with the two keys: name and values
  #
  # @returns flattened dict
  method _TransformToDict {list} {

  # assume that we have a bunch of dicts with two keys each: name and values

    set flatDict [dict create]
    foreach d $list {
      dict set flatDict [dict get $d name] [dict get $d values]
    }

    return $flatDict
  }

  ## @brief Convenience method for retrieving a parameter for a channel pair
  #
  method _GetChannelPairParam {key chpair} {
    $self _ThrowOnBadChannelPair $chpair

    if {$m_needsUpdate} {
      $self Update
    }

    # get the value 
    return [lindex [dict get $m_moduleState $key] $chpair]
  }


  ## @brief Convenience method for retrieving value of param with boolean val
  #
  # This converts the value stored in the dict via a conversion table. 
  #
  # @param key        name of parameter 
  # @param convTable  dict where key is value returned from module, key is what to return for it
  #
  # @returns the value of the convTable whose key matched the state of the module
  #
  # @throws error if state of module is not a key in convTable
  method _GetBooleanParam {key convTable} {

  # update stored state if necessary
    if {$m_needsUpdate} {
      $self Update
    }

    # get the value stored for the requested parameter name 
    set state [dict get $m_moduleState $key]

    # check whether the conversion table provided has a key for the returned value
    if {$state ni [dict keys $convTable]} {
    # nope... we don't understand how to convert the value stored
      set msg "Device state ($state) for $key is not understood by this driver."
      return -code error $msg
    } else {
    # yep... return the converted value
      return [dict get $convTable $state]
    }
  }


  method _ConvertTimeWidthsToRawWidths {elements} {
    set res [list]
    foreach el $elements {
      if {[catch {$type {ConvertWidthToArg} $el} result]} {
        lappend res NA
      } else {
        lappend res $result
      }
    }
    return $res
  }

  method _ConvertDeadtimesToRaw {elements} {
    set res [list]
    foreach el $elements {
      if {[catch {$type {ConvertDeadtimeToArg} $el} result]} {
        lappend res NA
      } else {
        lappend res $result
      }
    }
    return $res
  }

  # Some typevariables and utilities for setting them up.
  # These are for the purpose of converting between values read back from
  # the module that have been converted to some other form. Width and 
  # deadtime are subject to this.
  typevariable reverseWidthTable 
  typevariable reverseDeadtimeTable 

  typeconstructor {
    $type _ConstructWidthTable 
    $type _ConstructDeadtimeTable 
  }

  typemethod _ConstructWidthTable {} {
    set reverseWidthTable [dict create]

    set values [list]
    for {set val 19} {$val <= 222} {incr val} {
      lappend values $val
    }

    set t [list \
       6    9  10  12  13  15  17  18  20  21  23  25  26 \
       28  30  31  33  35  36  38  40  41  43  45  46  48  50  52  53 \
       55  57  59  61  62  64  66  68  70  71  73  75  77  79  81  83 \
       85  87  88  90  92  94  96  98 100 102 104 106 108 110 112 114 \
      116 118 121 123 125 127 129 131 133 135 138 140 142 144 146 149 \
      151 153 156 158 160 162 165 167 170 172 174 177 179 182 184 186 \
      189 191 194 197 199 202 204 207 209 212 215 217 220 223 226 228 \
      231 234 237 239 242 245 248 251 254 257 260 263 266 269 272 275 \
      278 281 285 288 291 294 298 301 304 308 311 314 318 321 325 328 \
      332 336 339 343 347 350 354 358 362 366 370 374 378 382 386 390 \
      394 399 403 407 412 416 421 425 430 434 439 444 449 454 459 464 \
      469 474 479 484 490 495 501 506 512 518 524 530 536 542 548 554 \
      561 567 574 581 587 594 602 609 616 624 632 639 647 656 664]

    set index 0
    foreach key $t {
      dict set reverseWidthTable $key [lindex $values $index]
      incr index
    }
  }

  typemethod _ConstructDeadtimeTable {} {
    
    set reverseDeadtimeTable [dict create]

    # create a list of all the raw parameter values
    set values [list]
    for {set val 27} {$val <= 222} {incr val} {
      lappend values $val
    }

    # create list of all the dead time values (in nanoseconds) that are 
    # mapped to by raw values
    set dt [list \
      20  21  23  25  26\
      28  30  31  33  35  36  38  40  41  43  45  46  48  50  52  53 \
      55  57  59  61  62  64  66  68  70  71  73  75  77  79  81  83 \
      85  87  88  90  92  94  96  98 100 102 104 106 108 110 112 114 \
     116 118 121 123 125 127 129 131 133 135 138 140 142 144 146 149 \
     151 153 156 158 160 162 165 167 170 172 174 177 179 182 184 186 \
     189 191 194 197 199 202 204 207 209 212 215 217 220 223 226 228 \
     231 234 237 239 242 245 248 251 254 257 260 263 266 269 272 275 \
     278 281 285 288 291 294 298 301 304 308 311 314 318 321 325 328 \
     332 336 339 343 347 350 354 358 362 366 370 374 378 382 386 390 \
     394 399 403 407 412 416 421 425 430 434 439 444 449 454 459 464 \
     469 474 479 484 490 495 501 506 512 518 524 530 536 542 548 554 \
     561 567 574 581 587 594 602 609 616 624 632 639 647 656 664 ]

   # build the table zippering the two lists together.
    set index 0
    foreach key $dt {
      dict set reverseDeadtimeTable $key [lindex $values $index]
      incr index
    }

  }

  typemethod ConvertWidthToArg {val} {
    return [dict get $reverseWidthTable $val]
  }

  typemethod ConvertDeadtimeToArg {val} {
    return [dict get $reverseDeadtimeTable $val]
  }


}

