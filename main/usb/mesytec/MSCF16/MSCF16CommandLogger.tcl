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

package provide mscf16commandlogger 1.0

package require snit

snit::type MSCF16CommandLogger {
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

  method SetGain {ch val} {
    chan puts $_logFile "[$self cget -name] SetGain $ch $val"
  }

  method GetGain {ch} {
    chan puts $_logFile "[$self cget -name] GetGain $ch"
  }

  method SetShapingTime {ch val} {
    chan puts $_logFile "[$self cget -name] SetShapingTime $ch $val"
  }

  method GetShapingTime {ch} {
    chan puts $_logFile "[$self cget -name] GetShapingTime $ch"
  }

  method SetPoleZero {ch val} {
    chan puts $_logFile "[$self cget -name] SetPoleZero $ch $val"
  }

  method GetPoleZero {ch} {
    chan puts $_logFile "[$self cget -name] GetPoleZero $ch"
  }


  method SetThreshold {ch val} {
    chan puts $_logFile "[$self cget -name] SetThreshold $ch $val"
  }

  method GetThreshold {ch} {
    chan puts $_logFile "[$self cget -name] GetThreshold $ch"
  }

  method SetMonitor {ch} {
    chan puts $_logFile "[$self cget -name] SetMonitor $ch"
  }
  method GetMonitor {} {
    chan puts $_logFile "[$self cget -name] GetMonitor"
  }

  method SetMode {mode} {
    chan puts $_logFile "[$self cget -name] SetMode $mode"
  }
  method GetMode {} {
    chan puts $_logFile "[$self cget -name] GetMode"
  }

  method EnableRC {on} {
    chan puts $_logFile "[$self cget -name] EnableRC $on"
  }
  method RCEnabled {} {
    chan puts $_logFile "[$self cget -name] RCEnabled"
  }
}

