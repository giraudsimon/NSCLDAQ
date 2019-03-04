
puts $auto_path

package require tcltest
package require scalerxlm72
package require cvmusb 

proc acquireVMUSB {} {
  set modules [::cvmusb::CVMUSB_enumerate]
  set nmods [::cvmusb::usb_device_vector_size $modules]
  puts $nmods
  for {set index 0} {$index < $nmods} {incr index} {
    set module [::cvmusb::usb_device_vector_get $modules $index]
    set sn [cvmusb::string_to_char [::cvmusb::CVMUSB_serialNo $module]]
    puts $sn
    set res [scan $sn {VM%d} number]
    puts "Scan result : $res"
    if {$res==1} {
      return [::cvmusb::CVMUSBusb %AUTO% $module]
    }
  }
  error "Failed to find a VMUSB"
}

 
set ctlr [acquireVMUSB]

tcltest::test firmware-0 {Test that GetFirmware returns something 
} -setup {
  AXLM72Scaler sclr 5
} -cleanup {
  itcl::delete object sclr
} -body {
  set value [sclr GetFirmware $::ctlr]
  puts [format "%x" $value]
  string is integer $value
} -result 1 -returnCodes 0 

tcltest::test settrigger-0 {Test that we can set trigger bits 0
} -setup {
  puts "Begin Set Trigger-0"
  AXLM72Scaler sclr 5
} -cleanup {
  itcl::delete object sclr
} -body {
  puts "Starting loop"
  for {set ch 0} {$ch<32} {incr ch} {
    sclr SetTriggerBit $::ctlr $ch 0
  } 
  set value [sclr ReadTrigger $::ctlr]
} -result 0 -returnCodes 0 

tcltest::test settrigger-1 {Test that we can set trigger bits 
} -setup {
  AXLM72Scaler sclr 5
} -cleanup {
  itcl::delete object sclr
} -body {
  # Make sure that all bits are zero
  for {set ch 0} {$ch<32} {incr ch} {
    sclr SetTriggerBit $::ctlr $ch 0
  } 
  
  sclr SetTriggerBit $::ctlr 1 1
  set value [sclr ReadTrigger $::ctlr]
} -result [expr 0x2] -returnCodes 0 

tcltest::test settrigger-2 {Test that we can set trigger bits 
} -setup {
  AXLM72Scaler sclr 5
} -cleanup {
  itcl::delete object sclr
} -body {
  # Make sure that all bits are zero
  sclr SetTriggerBits $::ctlr 0x12345678 
  sclr ReadTrigger $::ctlr
} -result [expr 0x12345678] -returnCodes 0 

tcltest::test settrigger-3 {Test that SetTrigger returns 0
} -setup {
  AXLM72Scaler sclr 5
} -cleanup {
  itcl::delete object sclr
} -body {
  # Make sure that all bits are zero
  sclr SetTrigger $::ctlr 1 
} -result 0 -returnCodes 0 


tcltest::test settrigger-4 {Test that SetTriggerBit returns 0
} -setup {
  AXLM72Scaler sclr 5
} -cleanup {
  itcl::delete object sclr
} -body {
  # Make sure that all bits are zero
  sclr SetTriggerBit $::ctlr 1 1 
} -result 0 -returnCodes 0 

tcltest::test settrigger-6 {Test that SetTriggerBits returns 0
} -setup {
  AXLM72Scaler sclr 5
} -cleanup {
  itcl::delete object sclr
} -body {
  # Make sure that all bits are zero
  sclr SetTriggerBits $::ctlr 0x0002 
} -result 0 -returnCodes 0 


tcltest::test readall-0 {Test that we can read all 32-channels
} -setup {
  AXLM72Scaler sclr 5
} -cleanup {
  itcl::delete object sclr
} -body {
  sclr Reset $::ctlr 

  sclr ReleaseBus $::ctlr
  sclr ReadAll $::ctlr
} -result "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0" -returnCodes 0 

tcltest::test setenable-0 {Test that we can set the enable 
} -setup {
  AXLM72Scaler sclr 5
} -cleanup {
  itcl::delete object sclr
} -body {
  sclr SetEnable $::ctlr 1
  set val [sclr ReadEnable $::ctlr]
  expr {$val == 1}
} -result 1 -returnCodes 0 

tcltest::test setenable-1 {Test that we can set the enable 
} -setup {
  AXLM72Scaler sclr 5
} -cleanup {
  itcl::delete object sclr
} -body {
  sclr SetEnable $::ctlr 0
  set val [sclr ReadEnable $::ctlr]
  expr {$val == 0}
} -result 1 -returnCodes 0 


tcltest::test setenable-2 {Test that we can set the enable with 0 return status
} -setup {
  AXLM72Scaler sclr 5
} -cleanup {
  itcl::delete object sclr
} -body {
  sclr SetEnable $::ctlr 0
} -result 0 -returnCodes 0 


tcltest::cleanupTests
