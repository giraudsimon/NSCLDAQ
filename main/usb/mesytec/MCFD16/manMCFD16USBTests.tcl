

package require mcfd16usb
package require tcltest

set serialFile /dev/ttyUSB0

MCFD16USB dev $serialFile


tcltest::test polarity-0 { Writing polarity should be effective
} {
  for {set chpr 0} {$chpr<9} {incr chpr} {
    if {($chpr%2)==0} {
      ::dev SetPolarity $chpr neg
    } else {
      ::dev SetPolarity $chpr pos
    }
  }

  set state [list]
  for {set chpr 0} {$chpr<9} {incr chpr} {
    lappend state [::dev GetPolarity $chpr]
  }
  set state
} {neg pos neg pos neg pos neg pos neg}




tcltest::test gain-0 { Writing gain should work
} {
  for {set ch 0} {$ch<9} {incr ch} {
    set gain [lindex {0 1 2} [expr $ch%3]]
    ::dev SetGain $ch $gain
  }

  set state [list]
  for {set ch 0} {$ch<9} {incr ch} {
    lappend state [::dev GetGain $ch]
  }
  set state
} {0 1 2 0 1 2 0 1 2}


tcltest::test bwl-0 { Set bandwidth should be responsive
} {
  # all fails if not in RC mode
  ::dev EnableRC 1

  set state [list]
  ::dev EnableBandwidthLimit on
  lappend state [::dev GetBandwidthLimit]
  ::dev EnableBandwidthLimit false
  lappend state [::dev GetBandwidthLimit]
  set state
} {1 0}

tcltest::test discriminator-0 {We should be able to set the disc mode
} {
  # this is necessary for it to work
  ::dev EnableRC 1

  set state [list]
  set res [catch {
  ::dev SetDiscriminatorMode cfd
  lappend state [::dev GetDiscriminatorMode]
  ::dev SetDiscriminatorMode led
  lappend state [::dev GetDiscriminatorMode]
  ::dev SetDiscriminatorMode cfd
  lappend state [::dev GetDiscriminatorMode]
  } msg]

  if {$res == 1} {
    puts $msg
  }

  set state
} {cfd led cfd}

tcltest::test threshold-0 { Writing thresholds should work
} {
  for {set ch 0} {$ch<17} {incr ch} {
      ::dev SetThreshold $ch $ch
  }

  set state [list]
  for {set ch 0} {$ch<17} {incr ch} {
    lappend state [::dev GetThreshold $ch]
  }
  set state
} {0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16}

tcltest::test width-0 { We should be able to write widths
} {
  
  for {set ch 0} {$ch<9} {incr ch} {
    ::dev SetWidth $ch [expr $ch+100]
  }

  set state [list]
  for {set ch 0} {$ch<9} {incr ch} {
    lappend state [::dev GetWidth $ch]
  }
  set state
} {100 101 102 103 104 105 106 107 108}

tcltest::test delay-0 { make sure we can set the delay and read it back
} {
  set res [catch {
  for {set ch 0} {$ch<9} {incr ch} {
    set delay [lindex {0 1 2 3 4} [expr $ch%5]]
    ::dev SetDelay $ch $delay
  }

  set state [list]
  for {set ch 0} {$ch<9} {incr ch} {
    lappend state [::dev GetDelay $ch]
  }
  } msg]
  if {$res} {
    puts $msg
  }
  set state
} {0 1 2 3 4 0 1 2 3}

tcltest::test fraction-0 { make sure we can set the delay and read it back
} {
  for {set ch 0} {$ch<9} {incr ch} {
    set frac [lindex {20 40} [expr $ch%2]]
    ::dev SetFraction $ch $frac
  }

  set state [list]
  for {set ch 0} {$ch<9} {incr ch} {
    lappend state [::dev GetFraction $ch]
  }

  set state
} {20 40 20 40 20 40 20 40 20}

tcltest::test pulser-0 {Make sure we can enable/disable pulser
} {
  set state [list]
  ::dev EnablePulser 1
  lappend state [::dev PulserEnabled]
  ::dev DisablePulser
  lappend state [::dev PulserEnabled]
  ::dev EnablePulser 2
  lappend state [::dev PulserEnabled]

  set state
} {1 0 1}

tcltest::test rc-0 { Set rc mode should work
} {
  set state [list]
  ::dev EnableRC 1
  lappend state [::dev RCEnabled]
  ::dev EnableRC 0
  lappend state [::dev RCEnabled]
  set state
} {1 0}


tcltest::test setTriggerSource {Set the trigger source
} {
  set state [list]
  ::dev SetTriggerSource 0 or 1
  lappend state [::dev GetTriggerSource 0]
  ::dev SetTriggerSource 1 pat_or_0 0
  lappend state [::dev GetTriggerSource 1]
  ::dev SetTriggerSource 2 gg 1
  lappend state [::dev GetTriggerSource 2]

  set state
} {{or 1} {pat_or_0 0} {gg 1}}

tcltest::test setTriggerOrPattern {Set the trigger or pattern
} {
  set state [list]
  ::dev SetTriggerOrPattern 0 0xa0a0
  lappend state [::dev GetTriggerOrPattern 0]
  ::dev SetTriggerOrPattern 1 0x1234
  lappend state [::dev GetTriggerOrPattern 1]

  set state
} {41120 4660}

tcltest::cleanupTests


