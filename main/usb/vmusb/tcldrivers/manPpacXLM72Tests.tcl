
package require tcltest
package require cvmusb
package require ppacxlm72

proc acquireVMUSB {} {
  set modules [::cvmusb::CVMUSB_enumerate]
  set nmods [::cvmusb::usb_device_vector_size $modules]
  for {set index 0} {$index < $nmods} {incr index} {
    set module [::cvmusb::usb_device_vector_get $modules $index]
    set sn [cvmusb::string_to_char [::cvmusb::CVMUSB_serialNo $module]]
    set res [scan $sn {VM%d} number]
    if {$res==1} {
      return [::cvmusb::CVMUSBusb %AUTO% $module]
    }
  }
  error "Failed to find a VMUSB"
}

set ::ctlr  [acquireVMUSB]
set fw [file join /home tompkins s800conv ppac2v.bit]
set first 1
proc setup {} {

  APpacXLM72 ::dev $::ctlr 19 

  if {$::first} {
    puts -nonewline "Configuring..."
    flush stdout
    ::dev Configure $::fw 
    set ::first 0
    puts "Done"
  }

  ::dev AccessBus 0x10001
}

proc tearDown {} {
  ::dev ReleaseBus
   itcl::delete object ::dev
}


proc cleanupTestsHook {} {
  $::ctlr -delete
}


tcltest::test period-0 {Test that we can configure the period
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list]
  for {set val 0} {$val < 4} {incr val} {
    ::dev WritePeriod $val 
    lappend result [::dev ReadPeriod]
  }
  set result
} -result {0 1 2 3} 


tcltest::test delay-0 {Test that we can configure the delay
} -setup {
  setup
} -cleanup {
  tearDown
} -body {

  set result [list]

  for {set val 0} {$val < 16} {incr val} {
    ::dev WriteDelay $val 
    lappend result [::dev ReadDelay]
  }
  set result
} -result {0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15} 



tcltest::test width-0 {Test that we can configure the width 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {

  set result [list]

  for {set val 0} {$val < 64} {incr val} {
    ::dev WriteWidth $val 
    lappend result [::dev ReadWidth]
  }
  set result
} -result [list  0  1  2  3  4  5  6  7  8  9  \
                10 11 12 13 14 15 16 17 18 19 \
                20 21 22 23 24 25 26 27 28 29 \
                30 31 32 33 34 35 36 37 38 39 \
                40 41 42 43 44 45 46 47 48 49 \
                50 51 52 53 54 55 56 57 58 59 \
                60 61 62 63 ]



tcltest::test shift-0 {Test that we can configure the shift 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  # there are too many to test by hand (256 total). We will just test a few
  # and assume they work thereafter.
  set res [list]
  ::dev WriteShift 9
  lappend res [::dev ReadShift]
  ::dev WriteShift 8
  lappend res [::dev ReadShift]
  ::dev WriteShift 28
  lappend res [::dev ReadShift]
  ::dev WriteShift 121
  lappend res [::dev ReadShift]
  ::dev WriteShift 200
  lappend res [::dev ReadShift]
  ::dev WriteShift 255
  lappend res [::dev ReadShift]
  set res
} -result {9 8 28 121 200 255}





tcltest::cleanupTests


proc w {type val} {
  return [::dev Write$type $val]
}
proc r {type} {
  return [::dev Read$type]
}

proc wr {type val} {
  w $type $val
  return [r $type]
}

