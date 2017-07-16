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

package provide mcfd16memorizer 1.0

package require snit
package require Utils 


## @brief An MCFD16 driver that remember the last value written to each param
#
# The original intent of this type was for loading the state of the control
# panels using a a tcl script of API calls. It does so by recording the last
# value written for each unique parameter in a set operation and making it
# available on the next get operation. For examply, GetThreshold 0 would return
# 23 if previously SetThreshold 0 23 was called. If the caller sets a different
# value for an already set parameter, the former value is overwritten by the new
# one. So the snit::type's memory is shallow and it does not remember all
# parameter values passed in. However its memory is sufficient for what it was
# intended for. Another useful application of this snit::type is for testing
# because it can sense when values have been written and when they have not.
# Unfortunately, this was written late in the game and a number of tests were
# written using a different sensing scheme than this.
#
# Because most of the methods in this are identical it is not worth documenting
# them individually. Instead, the general scheme of these methods is that a set
# operation sets the appropriate value in the dictionary. A get operation
# retrieves the value from the dictionary. If a parameter has not been set yet,
# the default value returned will be "NA". There is never a check for the
# validity of parameter values. This will always record an argument no matter
# how crazy it is.
#
snit::type MCFD16Memorizer {

  component _memory ;# a dict maintaining a well defined set of keys.

  constructor {} {
    install _memory using dict create

    $self _SetUpMemory ;# setup the structure of the dict
  }

  method SetThreshold {ch val} { dict set _memory threshold $ch $val }
  method GetThreshold {ch} { return [dict get $_memory threshold $ch]}

  method SetGain {ch val} { dict set _memory gain $ch $val }
  method GetGain {ch} { return [dict get $_memory gain $ch]}

  method SetWidth {ch val} { dict set _memory width $ch $val }
  method GetWidth {ch} { return [dict get $_memory width $ch]}

  method SetDeadtime {ch val} { dict set _memory deadtime $ch $val }
  method GetDeadtime {ch} { return [dict get $_memory deadtime $ch]}

  method SetDelay {ch val} { dict set _memory delay $ch $val }
  method GetDelay {ch} { return [dict get $_memory delay $ch]}

  method SetFraction {ch val} { dict set _memory fraction $ch $val }
  method GetFraction {ch} { return [dict get $_memory fraction $ch]}

  method SetPolarity {ch val} { dict set _memory polarity $ch $val }
  method GetPolarity {ch} { return [dict get $_memory polarity $ch]}

  method SetMode {mode} { dict set _memory mode $mode }
  method GetMode {} { return [dict get $_memory mode]}

  method EnableRC {on} { dict set _memory rc [string is true $on] }
  method RCEnabled {} { return [dict get $_memory rc]}

  method SetChannelMask {mask} { dict set _memory mask $mask}
  method GetChannelMask {} { return [dict get $_memory mask]}

  method EnablePulser {ptype} { dict set _memory pulser $ptype}
  method DisablePulser {} { dict set _memory pulser 0}
  method PulserEnabled {} { return [dict get $_memory pulser]}

  method SetTriggerSource {trigId source veto} { 
    dict set _memory trig${trigId}_source [list $source $veto]
  }

  method GetTriggerSource {trigId} {
    set state [dict get $_memory trig${trigId}_source]

    return $state
  }
  method SetTriggerOrPattern {patternId pattern} {
    dict set _memory or${patternId}_pattern $pattern
  }

  method GetTriggerOrPattern {patternId} {
    set state [dict get $_memory or${patternId}_pattern]

    if {$state ne "NA"} {
      return [expr $state]
    } else {
      return $state
    }
  }

  method SetFastVeto {onoff} {
    dict set _memory fast_veto [string is true $onoff]
  }

  method GetFastVeto {} {
    return [dict get $_memory fast_veto]
  }

  ## @brief Construct the structure of the dictionary
  #
  # Each parameter name has a key associated with it. Parameters that have
  # multiple channels have a dictionary whose keys are the channel indices and
  # the values are the values.
  #
  method _SetUpMemory {} {
    # add the dict for threshold and fill with defaults
    $self _AddParamToMemory threshold 0 17

    # add the dict for gain and fill with defaults
    $self _AddParamToMemory gain 0 9

    # add the dict for width and fill with defaults
    $self _AddParamToMemory width 0 9

    # add the dict for deadtime and fill with defaults
    $self _AddParamToMemory deadtime 0 9

    # add the dict for delay and fill with defaults
    $self _AddParamToMemory delay 0 9

    # add the dict for fraction and fill with defaults
    $self _AddParamToMemory fraction 0 9

    # add the dict for polarity and fill with defaults
    $self _AddParamToMemory polarity 0 9

    # the following do not have any channels associated with them
    dict set _memory mode NA
    dict set _memory rc NA
    dict set _memory mask NA
    dict set _memory pulser NA
    dict set _memory trig0_source NA
    dict set _memory trig1_source NA
    dict set _memory trig2_source NA
    dict set _memory or0_pattern NA
    dict set _memory or1_pattern NA
    dict set _memory fast_veto NA
  }

  ## @brief Utilty method for adding a parameter with channel to the dict
  #
  # @param  name    name of key to add (typically name of the key)
  # @param  low     first channel value 
  # @param  nchan   number of channels
  method _AddParamToMemory {name low nchan} {
    dict set _memory $name [dict create]
    foreach ch [Utils::sequence $low $nchan 1] {
      dict set _memory $name $ch NA
    }

  }
}


