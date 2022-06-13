


package require tcltest

package require cccusb 
package require Itcl


tcltest::test pkgrequire-0 {Test tat we can require the package
} {package require trigger2367} 11.0


# define a proc for setting up a CC-USB
proc acquireCCUSB {} {

  set devices [::cccusb::CCCUSB_enumerate]
  if {[cccusb::usb_device_vector_size $devices]==0} {
    return -code error "Failed to establish that at least one CC-USB is connected"
  }

  set device [::cccusb::usb_device_vector_get $devices 0] 
  set ctlr [::cccusb::CCCUSBusb aCCUSB $device]
  return $ctlr
}






set slot   11
set fwpath /home/tompkins/caesar/newdaq/vmusb/usbtrig.bit
set ctlr [acquireCCUSB]



proc setup {} {
    ATrigger2367 ::dev $::ctlr $::slot
}

proc tearDown {} {
  itcl::delete object ::dev
}

proc tcltest::cleanupTestsHook {} {
  $::ctlr -delete
}


proc decodeData {value} {
  set data [expr {$value&0xffffff}]
  set q    [expr {($value>>24)&0x1}]
  set x    [expr {($value>>25)&0x1}]
  return [list $data $q $x]
}



tcltest::test configure-0 { Test that we can configure the module
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev Configure $::fwpath
} -result ""


tcltest::test go-0 { Test that we can set and read the go bit
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list] 
  ::dev Go 1
  lappend result [decodeData [::dev ReadGo]]
  ::dev Go 0
  lappend result [decodeData [::dev ReadGo]]
  ::dev Go 1
  lappend result [decodeData [::dev ReadGo]]

  set result
} -result "{1 1 1} {0 1 1} {1 1 1}"


tcltest::test select-0 { Test that we can set and read the tstamp mode bit
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list] 
  ::dev Select 0 
  lappend result [decodeData [::dev ReadSelect]]
  ::dev Select 1 
  lappend result [decodeData [::dev ReadSelect]]
  ::dev Select 2
  lappend result [decodeData [::dev ReadSelect]]
  ::dev Select 3
  lappend result [decodeData [::dev ReadSelect]]

  set result
} -result "{0 1 1} {1 1 1} {2 1 1} {3 1 1}"



tcltest::test s800gdgdelay-0 { Test that we can set and read the s800gdgdelay
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list] 
  ::dev SetS800GDGDelay 32
  lappend result [decodeData [::dev GetS800GDGDelay]]
  ::dev SetS800GDGDelay 43 
  lappend result [decodeData [::dev GetS800GDGDelay]]
  ::dev SetS800GDGDelay 128
  lappend result [decodeData [::dev GetS800GDGDelay]]
  ::dev SetS800GDGDelay 255
  lappend result [decodeData [::dev GetS800GDGDelay]]

  set result
} -result "{32 1 1} {43 1 1} {128 1 1} {255 1 1}"





tcltest::test s800gdgwidth-0 { Test that we can set and read the s800gdgwidth
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list] 
  ::dev SetS800GDGWidth 32
  lappend result [decodeData [::dev GetS800GDGWidth]]
  ::dev SetS800GDGWidth 43 
  lappend result [decodeData [::dev GetS800GDGWidth]]
  ::dev SetS800GDGWidth 128
  lappend result [decodeData [::dev GetS800GDGWidth]]
  ::dev SetS800GDGWidth 255
  lappend result [decodeData [::dev GetS800GDGWidth]]

  set result
} -result "{32 1 1} {43 1 1} {128 1 1} {255 1 1}"






tcltest::test secondarygdgdelay-0 { Test that we can set and read the secondarygdgdelay
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list] 
  ::dev SetSecondaryGDGDelay 32
  lappend result [decodeData [::dev GetSecondaryGDGDelay]]
  ::dev SetSecondaryGDGDelay 43 
  lappend result [decodeData [::dev GetSecondaryGDGDelay]]
  ::dev SetSecondaryGDGDelay 128
  lappend result [decodeData [::dev GetSecondaryGDGDelay]]
  ::dev SetSecondaryGDGDelay 255
  lappend result [decodeData [::dev GetSecondaryGDGDelay]]

  set result
} -result "{32 1 1} {43 1 1} {128 1 1} {255 1 1}"





tcltest::test secondarygdgwidth-0 { Test that we can set and read the secondarygdgwidth
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list] 
  ::dev SetSecondaryGDGWidth 32
  lappend result [decodeData [::dev GetSecondaryGDGWidth]]
  ::dev SetSecondaryGDGWidth 43 
  lappend result [decodeData [::dev GetSecondaryGDGWidth]]
  ::dev SetSecondaryGDGWidth 128
  lappend result [decodeData [::dev GetSecondaryGDGWidth]]
  ::dev SetSecondaryGDGWidth 255
  lappend result [decodeData [::dev GetSecondaryGDGWidth]]

  set result
} -result "{32 1 1} {43 1 1} {128 1 1} {255 1 1}"









tcltest::test s800delay-0 { Test that we can set and read the s800delay
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list] 
  ::dev SetS800Delay 32
  lappend result [decodeData [::dev GetS800Delay]]
  ::dev SetS800Delay 43 
  lappend result [decodeData [::dev GetS800Delay]]
  ::dev SetS800Delay 128
  lappend result [decodeData [::dev GetS800Delay]]
  ::dev SetS800Delay 255
  lappend result [decodeData [::dev GetS800Delay]]

  set result
} -result "{32 1 1} {43 1 1} {128 1 1} {255 1 1}"






tcltest::test coincidenceWidth-0 { Test that we can set and read the coincidenceWidth
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list] 
  ::dev SetCoincidenceWidth 32
  lappend result [decodeData [::dev GetCoincidenceWidth]]
  ::dev SetCoincidenceWidth 43 
  lappend result [decodeData [::dev GetCoincidenceWidth]]
  ::dev SetCoincidenceWidth 128
  lappend result [decodeData [::dev GetCoincidenceWidth]]
  ::dev SetCoincidenceWidth 255
  lappend result [decodeData [::dev GetCoincidenceWidth]]

  set result
} -result "{32 1 1} {43 1 1} {128 1 1} {255 1 1}"




tcltest::test secondarydelay-0 { Test that we can set and read the secondarydelay
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list] 
  ::dev SetSecondaryDelay 32
  lappend result [decodeData [::dev GetSecondaryDelay]]
  ::dev SetSecondaryDelay 43 
  lappend result [decodeData [::dev GetSecondaryDelay]]
  ::dev SetSecondaryDelay 128
  lappend result [decodeData [::dev GetSecondaryDelay]]
  ::dev SetSecondaryDelay 255
  lappend result [decodeData [::dev GetSecondaryDelay]]

  set result
} -result "{32 1 1} {43 1 1} {128 1 1} {255 1 1}"








tcltest::test bypasses-0 { Test that we can set and read the bypasses
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list] 
  ::dev SetBypasses 32
  lappend result [decodeData [::dev GetBypasses]]
  ::dev SetBypasses 43 
  lappend result [decodeData [::dev GetBypasses]]
  ::dev SetBypasses 128
  lappend result [decodeData [::dev GetBypasses]]
  ::dev SetBypasses 255
  lappend result [decodeData [::dev GetBypasses]]

  set result
} -result "{32 1 1} {43 1 1} {128 1 1} {255 1 1}"





tcltest::test s800downscale-0 { Test that we can set and read the s800downscale
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list] 
  ::dev SetS800DownscaleFactor 32
  lappend result [decodeData [::dev GetS800DownscaleFactor]]
  ::dev SetS800DownscaleFactor 43 
  lappend result [decodeData [::dev GetS800DownscaleFactor]]
  ::dev SetS800DownscaleFactor 128
  lappend result [decodeData [::dev GetS800DownscaleFactor]]
  ::dev SetS800DownscaleFactor 255
  lappend result [decodeData [::dev GetS800DownscaleFactor]]

  set result
} -result "{32 1 1} {43 1 1} {128 1 1} {255 1 1}"




tcltest::test secondarydownscale-0 { Test that we can set and read the secondarydownscale
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list] 
  ::dev SetSecondaryDownscaleFactor 32
  lappend result [decodeData [::dev GetSecondaryDownscaleFactor]]
  ::dev SetSecondaryDownscaleFactor 43 
  lappend result [decodeData [::dev GetSecondaryDownscaleFactor]]
  ::dev SetSecondaryDownscaleFactor 128
  lappend result [decodeData [::dev GetSecondaryDownscaleFactor]]
  ::dev SetSecondaryDownscaleFactor 255
  lappend result [decodeData [::dev GetSecondaryDownscaleFactor]]

  set result
} -result "{32 1 1} {43 1 1} {128 1 1} {255 1 1}"






tcltest::test triggerbox-0 { Test that we can set and read the triggerbox
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list] 
  ::dev SetTriggerBox 32
  lappend result [decodeData [::dev GetTriggerBox]]
  ::dev SetTriggerBox 43 
  lappend result [decodeData [::dev GetTriggerBox]]
  ::dev SetTriggerBox 128
  lappend result [decodeData [::dev GetTriggerBox]]
  ::dev SetTriggerBox 255
  lappend result [decodeData [::dev GetTriggerBox]]

  set result
} -result "{32 1 1} {43 1 1} {128 1 1} {255 1 1}"






tcltest::test adcgatewidth-0 { Test that we can set and read the adcgatewidth
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list] 
  ::dev SetADCGateWidth 32
  lappend result [decodeData [::dev GetADCGateWidth]]
  ::dev SetADCGateWidth 43 
  lappend result [decodeData [::dev GetADCGateWidth]]
  ::dev SetADCGateWidth 128
  lappend result [decodeData [::dev GetADCGateWidth]]
  ::dev SetADCGateWidth 255
  lappend result [decodeData [::dev GetADCGateWidth]]

  set result
} -result "{32 1 1} {43 1 1} {128 1 1} {255 1 1}"






tcltest::test qdcgatewidth-0 { Test that we can set and read the qdcgatewidth
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list] 
  ::dev SetQDCGateWidth 32
  lappend result [decodeData [::dev GetQDCGateWidth]]
  ::dev SetQDCGateWidth 43 
  lappend result [decodeData [::dev GetQDCGateWidth]]
  ::dev SetQDCGateWidth 128
  lappend result [decodeData [::dev GetQDCGateWidth]]
  ::dev SetQDCGateWidth 255
  lappend result [decodeData [::dev GetQDCGateWidth]]

  set result
} -result "{32 1 1} {43 1 1} {128 1 1} {255 1 1}"







tcltest::test tdcgatewidth-0 { Test that we can set and read the tdcgatewidth
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list] 
  ::dev SetTDCGateWidth 32
  lappend result [decodeData [::dev GetTDCGateWidth]]
  ::dev SetTDCGateWidth 43 
  lappend result [decodeData [::dev GetTDCGateWidth]]
  ::dev SetTDCGateWidth 128
  lappend result [decodeData [::dev GetTDCGateWidth]]
  ::dev SetTDCGateWidth 255
  lappend result [decodeData [::dev GetTDCGateWidth]]

  set result
} -result "{32 1 1} {43 1 1} {128 1 1} {255 1 1}"







tcltest::test coincreggategatewidth-0 { Test that we can set and read the 
                                        coincreggategatewidth
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set result [list] 
  ::dev SetCoincRegGateWidth 32
  lappend result [decodeData [::dev GetCoincRegGateWidth]]
  ::dev SetCoincRegGateWidth 43 
  lappend result [decodeData [::dev GetCoincRegGateWidth]]
  ::dev SetCoincRegGateWidth 128
  lappend result [decodeData [::dev GetCoincRegGateWidth]]
  ::dev SetCoincRegGateWidth 255
  lappend result [decodeData [::dev GetCoincRegGateWidth]]

  set result
} -result "{32 1 1} {43 1 1} {128 1 1} {255 1 1}"






tcltest::cleanupTests
