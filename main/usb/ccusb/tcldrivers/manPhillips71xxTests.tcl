


package require tcltest

package require cccusb 
package require Itcl


tcltest::test pkgrequire-0 {Test tat we can require the package
} {package require phillips71xx} 11.0


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






set slot   12
set fwpath /home/tompkins/caesar/newdaq/vmusb/usbtrig.bit
set ctlr [acquireCCUSB]



proc setup {} {
    APhillips71xx ::dev $::ctlr $::slot
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



tcltest::test controlRegister-0 { Test that we can set,reset, and read cntrl reg
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set res [list]
  ::dev SetControlRegister 0x3
  lappend res [decodeData [::dev ReadControlRegister]]

  ::dev ResetControlRegister 0x1
  lappend res [decodeData [::dev ReadControlRegister]]

  ::dev SetControlRegister 0x2
  lappend res [decodeData [::dev ReadControlRegister]]

  ::dev ResetControlRegister 0x2
  lappend res [decodeData [::dev ReadControlRegister]]

  ::dev ResetControlRegister 0x1
  lappend res [decodeData [::dev ReadControlRegister]]

  set res
} -result "{3 1 1} {2 1 1} {2 1 1} {0 1 1} {0 1 1}"



tcltest::test writePedestals-0 { Test that we can write pedestals properly 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set peds [list -2 -1 0 1 2 3 4 5 6 7 8 9 0 1 4094 4095]
  ::dev WritePedestals $peds
} -result ""



tcltest::test writeSetLowerThreshold-0 { Test that we can write lower thres properly 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set llt [list 0 1 2 3 4 5 6 7 8 9 10 11 12 13 4094 4095]
  ::dev WriteLowerThresholds $llt
} -result ""



tcltest::test writeSetUpperThreshold-0 { Test that we can write upper thres properly 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set ult [list 0 1 2 3 4 5 6 7 8 9 10 11 12 13 4094 4095]
  ::dev WriteUpperThresholds $ult
} -result ""



tcltest::cleanupTests

