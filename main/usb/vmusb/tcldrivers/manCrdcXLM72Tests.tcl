lappend auto_path [file join $::env(DAQROOT) TclLibs]
lappend auto_path [file join $::env(DAQLIB)]

package require tcltest
package require cvmusb
package require crdcxlm72

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
set fw [file join /home tompkins s800conv crdc5v.bit]
set first 1
proc setup {} {
   ACrdcXLM72 ::dev $::ctlr 8

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

proc sequence {first last} {
  set list [list]

  while {$first <= $last} {
    lappend list $first
    incr first
  }

  return $list
}

tcltest::test samples-0 {Test that we can configure the samples
} -setup {
  setup
} -cleanup {
  tearDown
} -body {

  set results [list]
  for {set index 0} {$index < 512} {incr index} {
    ::dev WriteSamples $index 
    lappend results [::dev ReadSamples]
  }
  set results
} -result [sequence 0 511]


tcltest::test period-0 {Test that we can configure the period
} -setup {
  setup
} -cleanup {
  tearDown
} -body {

  set results [list]
  for {set index 0} {$index < 4} {incr index} {
    ::dev WritePeriod $index 
    lappend results [::dev ReadPeriod]
  }
  set results
} -result [sequence 0 3]


tcltest::test delay-0 {Test that we can configure the delay
} -setup {
  setup
} -cleanup {
  tearDown
} -body {

  set results [list]
  for {set index 0} {$index < 16} {incr index} {
    ::dev WriteDelay $index 
    lappend results [::dev ReadDelay]
  }
  set results
} -result [sequence 0 15]



tcltest::test width-0 {Test that we can configure the width 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {

  set results [list]
  for {set index 0} {$index < 64} {incr index} {
    ::dev WriteWidth $index 
    lappend results [::dev ReadWidth]
  }
  set results
} -result [sequence 0 63]



tcltest::test shift-0 {Test that we can configure the shift 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {


  set results [list]
  for {set index 0} {$index < 256} {incr index} {
    ::dev WriteShift $index 
    lappend results [::dev ReadShift]
  }
  set results
} -result [sequence 0 255]



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

