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



package require tcltest

package require mscf16usb

set serialFile /dev/ttyUSB1

proc setup {} {
  MSCF16USB ::dev $::serialFile
}

proc tearDown {} {
  ::dev destroy
}

tcltest::test gain-0 { Set gain
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  puts "gain-0"
  set resList [list]

  ::dev SetMode individual
  for {set ch 0} {$ch<4} {incr ch} {
    ::dev SetGain $ch [expr $ch*2]
    lappend resList [::dev GetGain $ch]
  }
  
  ::dev SetMode common
  ::dev SetGain 4 12
  lappend resList [::dev GetGain 4]

  set resList
} -result {0 2 4 6 12}


tcltest::test shaping-0 {Set/get shaping time
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  puts "shaping-0"
  set resList [list]

  ::dev SetMode individual
  for {set ch 0} {$ch<4} {incr ch} {
    ::dev SetShapingTime $ch [expr $ch%3]
    lappend resList [::dev GetShapingTime $ch]
  }
  
  ::dev SetMode common
  ::dev SetShapingTime 4 2
  lappend resList [::dev GetShapingTime 4]

  set resList
} -result {0 1 2 0 2}

if {0} {

tcltest::test thresh-0 {Set/get thresholds
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  puts "thres-0"
  set resList [list]

  ::dev SetMode individual
  for {set ch 0} {$ch<16} {incr ch} {
    puts -nonewline "$ch "
    flush stdout
    ::dev SetThreshold $ch [expr $ch*10]
    lappend resList [::dev GetThreshold $ch]
  }
  
  ::dev SetMode common
  ::dev SetThreshold 16 255 
  lappend resList [::dev GetThreshold 16]

  set resList
} -result {0 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150 255}


tcltest::test polezero-0 {Set/get pole zero
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  puts "polezero-0"
  set resList [list]

  ::dev SetMode individual
  for {set ch 0} {$ch<16} {incr ch} {
    puts -nonewline "$ch "
    flush stdout
    ::dev SetPoleZero $ch [expr $ch*10]
    lappend resList [::dev GetPoleZero $ch]
  }
  
  ::dev SetMode common
  ::dev SetPoleZero 16 255 
  lappend resList [::dev GetPoleZero 16]

  set resList
} -result {0 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150 255}


} 



tcltest::test monitor-0 {Set/get monitor
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  puts "monitor-0"
  set resList [list]

  ::dev SetMode individual
  ::dev SetMonitor 2
  lappend resList [::dev GetMonitor]
  
  ::dev SetMode common
  ::dev SetMonitor 15
  lappend resList [::dev GetMonitor]

  set resList
} -result {2 15}



tcltest::test enablerc-0 {Enable/disable rc
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  puts "enablerc-0"
  set resList [list]

  ::dev SetMode individual
  ::dev EnableRC on
  lappend resList [::dev RCEnabled]
  ::dev EnableRC off
  lappend resList [::dev RCEnabled]
  
  ::dev SetMode common
  ::dev EnableRC on
  lappend resList [::dev RCEnabled]
  ::dev EnableRC off
  lappend resList [::dev RCEnabled]

  set resList
} -result {1 0 1 0}


tcltest::test mode-0 {Set/get mode
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  puts "mode-0"
  set resList [list]

  ::dev SetMode individual
  lappend resList [::dev GetMode]
  ::dev SetMode common
  lappend resList [::dev GetMode]

  set resList
} -result {individual common}



tcltest::cleanupTests
