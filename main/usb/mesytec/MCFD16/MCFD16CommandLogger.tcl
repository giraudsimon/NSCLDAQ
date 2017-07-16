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


package provide mcfd16commandlogger 1.0

package require snit


## @brief A snit::type that acts like an actual device driver but
#         writes commands to a file instead
#
#  This class does not actually own the file that it writes to but rather
#  accepts it as an argument and then uses it. In this way, it is merely
#  responsible for adding text into the file whenever a call is made.
#
#  The utility of the snit::type is to provide a mechanism for logging calls.
#  Consider the following example:
#
#  \begincode
#  set logFile [open "log.tcl" w]
#  MCFD16CommandLogger logger $logFile -dev ::foo
#
#  logger SetThreshold 0 23
#  logger SetGain 0 4
#  logger GetWidth 4
#
#  \endcode
#
#  In the above example, the log file will have the lines:
#  \beginverbatim
#  ::foo SetThreshold 0 23
#  ::foo SetGain 0 4
#  ::foo GetWidth 4
#  \endverbatim
#
#  As with all io operations on channel objects, it may be necessary to flush
#  the channel to ensure that the added text makes it into the file.
#
#  It useful to note that unlike some of the other actual drivers, this performs
#  no parameter checking. It simply writes whatever parameter values passed into
#  it to the file. As a result, the responsibility is left to the caller to pass
#  good parameter values. 
#
#  USAGE: This is primarily used for saving the GUI state of the MCFD16Control
#  program into a file for later execution.
#
snit::type MCFD16CommandLogger {
  option -name -default ::dev ;# name of device to use when writing

  variable _logFile ;# the file channel to write to

  ## @brief Constructor
  #
  # @param channel  an open, writable file channel 
  # @param args     option-value pairs
  constructor {channel args} {
    set _logFile $channel

    $self configurelist $args
  }
  
  ## @brief No-op...
  #
  destructor {
  }

  ## @brief Log a SetThreshold to the file
  #
  method SetThreshold {ch val} {
    chan puts $_logFile "[$self cget -name] SetThreshold $ch $val"
  }

  ## @brief Log a GetThreshold to the file
  #
  method GetThreshold {ch} {
    chan puts $_logFile "[$self cget -name] GetThreshold $ch"
  }

  ## @brief Log a SetGain to the file
  #
  method SetGain {ch val} {
    chan puts $_logFile "[$self cget -name] SetGain $ch $val"
  }

  ## @brief Log a GetGain to the file
  method GetGain {ch} {
    chan puts $_logFile "[$self cget -name] GetGain $ch"
  }

  ## @brief Log a SetWidth to the file
  method SetWidth {ch val} {
    chan puts $_logFile "[$self cget -name] SetWidth $ch $val"
  }

  ## @brief Log a GetWidth to the file
  method GetWidth {ch} {
    chan puts $_logFile "[$self cget -name] GetWidth $ch"
  }

  ## @brief Log a SetDeadtime to the file
  method SetDeadtime {ch val} {
    chan puts $_logFile "[$self cget -name] SetDeadtime $ch $val"
  }

  ## @brief Log a GetDeadtime to the file
  method GetDeadtime {ch} {
    chan puts $_logFile "[$self cget -name] GetDeadtime $ch"
  }

  ## @brief Log a SetDelay to the file
  method SetDelay {ch val} {
    chan puts $_logFile "[$self cget -name] SetDelay $ch $val"
  }

  ## @brief Log a GetDelay to the file
  method GetDelay {ch} {
    chan puts $_logFile "[$self cget -name] GetDelay $ch"
  }

  ## @brief Log a SetFraction to the file
  method SetFraction {ch val} {
    chan puts $_logFile "[$self cget -name] SetFraction $ch $val"
  }

  ## @brief Log a GetFraction to the file
  method GetFraction {ch} {
    chan puts $_logFile "[$self cget -name] GetFraction $ch"
  }

  ## @brief Log a SetPolarity to the file
  method SetPolarity {ch val} {
    chan puts $_logFile "[$self cget -name] SetPolarity $ch $val"
  }

  ## @brief Log a GetPolarity to the file
  method GetPolarity {ch} {
    chan puts $_logFile "[$self cget -name] GetPolarity $ch"
  }

  ## @brief Log a SetMode to the file
  method SetMode {mode} {
    chan puts $_logFile "[$self cget -name] SetMode $mode"
  }

  ## @brief Log a GetMode to the file
  method GetMode {} {
    chan puts $_logFile "[$self cget -name] GetMode"
  }

  ## @brief Log an EnableRC to the file
  method EnableRC {state} {
    chan puts $_logFile "[$self cget -name] EnableRC $state"
  }

  ## @brief Log an RCEnabled to the file
  method RCEnabled {} {
    chan puts $_logFile "[$self cget -name] RCEnabled"
  }

  ## @brief Log a SetChannelMask to the file
  method SetChannelMask {mask} {
    chan puts $_logFile "[$self cget -name] SetChannelMask $mask"
  }

  ## @brief Log a GetChannelMask to the file
  method GetChannelMask {} {
    chan puts $_logFile "[$self cget -name] GetChannelMask"
  }

  ## @brief Log an EnablePulser  to the file
  method EnablePulser {ptype} {
    chan puts $_logFile "[$self cget -name] EnablePulser $ptype"
  }

  ## @brief Log a DisablePulser  to the file
  method DisablePulser {} {
    chan puts $_logFile "[$self cget -name] DisablePulser"
  }

  ## @brief Log a PulserEnabled  to the file
  method PulserEnabled {} {
    chan puts $_logFile "[$self cget -name] PulserEnabled"
  }

  method SetTriggerSource {trigId source veto} {
    chan puts $_logFile "[$self cget -name] SetTriggerSource $trigId $source $veto"
  }

  method GetTriggerSource {trigId} {
    chan puts $_logFile "[$self cget -name] GetTriggerSource $trigId"
  }

  method SetTriggerOrPattern {patternId pattern} {
    chan puts $_logFile "[$self cget -name] SetTriggerOrPattern $patternId $pattern"
  }

  method GetTriggerOrPattern {patternId} {
    chan puts $_logFile "[$self cget -name] GetTriggerOrPattern $patternId"
  }

  method SetFastVeto {onoff} {
    chan puts $_logFile "[$self cget -name] SetFastVeto $onoff"
  }

  method GetFastVeto {} {
    chan puts $_logFile "[$self cget -name] GetFastVeto"
  }


  ## @brief Convenience wrapper around a "chan flush" command
  method Flush {} {
    chan flush $_logFile
  }

}
