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


package provide mscf16memorizer 1.0

package require snit

##############################################################################
# Setting up Fake Objects to test the presenter with...

# A simple fake object for the device handle
snit::type MSCF16Memorizer {

  variable _memory

  constructor {args} {
#    $self configurelist $args

    set _memory [dict create]
    
    dict set _memory threshold [dict create]
    for {set ch 0} {$ch < 17} {incr ch} {
      dict set _memory threshold $ch NA
    }

    dict set _memory polezero [dict create]
    for {set ch 0} {$ch < 17} {incr ch} {
      dict set _memory polezero $ch NA
    }

    dict set _memory gain [dict create]
    for {set ch 0} {$ch < 5} {incr ch} {
      dict set _memory gain $ch NA
    }

    dict set _memory shaping [dict create]
    for {set ch 0} {$ch < 5} {incr ch} {
      dict set _memory shaping $ch NA
    }

    dict set _memory monitor NA
    dict set _memory mode NA
    dict set _memory rc NA
  }

  method SetThreshold {ch val} { dict set _memory threshold $ch $val }
  method GetThreshold {ch} { return [dict get $_memory threshold $ch] }

  method SetPoleZero {ch val} { dict set _memory polezero $ch $val }
  method GetPoleZero {ch} { return [dict get $_memory polezero $ch] }

  method SetGain {ch val} { dict set _memory gain $ch $val }
  method GetGain {ch} { return [dict get $_memory gain $ch] }

  method SetShapingTime {ch val} { dict set _memory shaping $ch $val }
  method GetShapingTime {ch} { return [dict get $_memory shaping $ch] }

  method SetMode {val} { dict set _memory mode $val }
  method GetMode {} { return [dict get $_memory mode] }

  method SetMonitor {val} { dict set _memory monitor $val }
  method GetMonitor {} { return [dict get $_memory monitor] }

  method EnableRC {val} { dict set _memory rc $val }
  method RCEnabled {} { return [dict get $_memory rc]}
}
