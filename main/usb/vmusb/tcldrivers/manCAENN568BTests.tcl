#===================================================================
##
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#      NSCL DAQ Development Group 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#
# @file   manCAENN568BTests.tcl
# @author Jeromy Tompkins
# @note   Tests to run for testing an N568B device controlled through a V288.

package require tcltest
package require cvmusb 
package require caenv288
package require Itcl
package require snit


# make sure we can require the package...
tcltest::test pkgrequire-0 {Test that we can require the package
} { package require caenn568b } 11.0


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
proc setup {} {
#   cvmusb::CMockVMUSB ::ctlr
   ACAENV288 ::proxy $::ctlr 0x000800
   ACAENN568B ::dev ::proxy 1 
}

proc tearDown {} {
   itcl::delete object ::proxy 
   itcl::delete object ::dev

}


tcltest::test moduleIdentifier-0 {Test that we can read the correct id
} -setup {
setup
} -cleanup {
tearDown
} -body {
 set id [::dev ReadModuleIdentifier]
  
 regexp {^N568 Version \d+\.\d+$} $id
} -result 1 




# Setting every parameter in the module and then checking the result is a bit
# much for a unit test. I will instead check to make sure that we have the
# expected number of returned params. The params for each channel are parsed.
# (16 channels * 6 params/ch) + offset = 50 elements
tcltest::test ReadAll-0 {Test that we have 
} -setup {
setup
} -cleanup {
tearDown
} -body {
 llength [::dev ReadAllParameters]
} -result 97 



tcltest::test fgain-0 {Test that setting the fgain parameter works
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev SetParameter 3 fgain 123
  ::dev ReadFineGain 3
} -result {123}

tcltest::test fgain-1 {Test that bad gain parameter produces error 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev SetParameter 3 fgain 256 
} -result {::dev: fgain value out of range. Must be in range [0,255].} \
-returnCodes 1





tcltest::test cgain-0 {Test that setting the cgain parameter works
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev SetParameter 3 cgain 5
  ::dev ReadCoarseGain 3
} -result {5}

tcltest::test cgain-1 {Test that bad gain parameter produces error 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev SetParameter 2 cgain 8
} -result {::dev: cgain value out of range. Must be in range [0,7].} \
-returnCodes 1





tcltest::test pzero-0 {Test that setting the pzero parameter works
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev SetParameter 3 pzero 178 
  ::dev ReadPoleZero 3
} -result {178}

tcltest::test pzero-1 {Test that bad pzero parameter produces error 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev SetParameter 15 pzero 256 
} -result {::dev: pzero value out of range. Must be in range [0,255].} \
-returnCodes 1




tcltest::test shape-0 {Test that setting the shape parameter works
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set res [list]
  ::dev SetParameter 3 shape 2
  lappend res [::dev ReadShape 3]
  ::dev SetParameter 3 shape 3 
  lappend res [::dev ReadShape 3]
  ::dev SetParameter 3 shape 1
  lappend res [::dev ReadShape 3]
  ::dev SetParameter 3 shape 0
  lappend res [::dev ReadShape 3]
  set res
} -result {2 3 1 0}

tcltest::test shape-1 {Test that bad shape parameter produces error 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev SetParameter 12 shape 4
} -result {::dev: shape value out of range. Must be in range [0,3].} \
-returnCodes 1



tcltest::test polarity-0 {Test that setting the polarity parameter works
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set res [list]
  ::dev SetParameter 3 polar 1 
  lappend res [::dev ReadPolarity 3]
  ::dev SetParameter 3 polar 0 
  lappend res [::dev ReadPolarity 3]
  ::dev SetParameter 3 polar 1
  lappend res [::dev ReadPolarity 3]
  set res
} -result {1 0 1}

tcltest::test polarity-1 {Test that bad polarity parameter produces error 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev SetParameter 13 polar 2 
} -result {::dev: polar value out of range. Must be in range [0,1].} \
-returnCodes 1



tcltest::test out-0 {Test that setting the output configuration parameter works
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  set res [list]
  ::dev SetParameter 3 out 1 
  lappend res [::dev ReadOutputConfiguration 3]
  ::dev SetParameter 3 out 0 
  lappend res [::dev ReadOutputConfiguration 3]
  ::dev SetParameter 3 out 1
  lappend res [::dev ReadOutputConfiguration 3]
  set res
} -result {1 0 1}

tcltest::test out-1 {Test that bad output configuration parameter produces error 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev SetParameter 13 out -1 
} -result {::dev: out value out of range. Must be in range [0,1].} \
-returnCodes 1




tcltest::test offset-0 {Test that we can set and retrieve offset
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev SetOffset 253 
  ::dev ReadOffset 

} -result {253} 

tcltest::test offset-1 {Test that bad offset value will cause error
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev SetOffset 256 
} -result "Offset value out of bounds. Must be in range \[0,255\]." \
-returnCodes 1


tcltest::test setParameters-0 {Test that bad channel value will fail
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev SetParameter 16 fgain 0 
} -result "::dev: channel number out of bounds. Must be in range \[0,15\]." \
-returnCodes 1


tcltest::test ReadMUXStatusAndLastChAccess-0 {Test that reading mux status and last chn access is sensible 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev SetParameter 4 cgain 5
  ::dev EnableMUX 

  ::dev ReadMUXStatusAndLastChAccess 
} -result [list 1  4]


tcltest::test parseStatusRegister-0 {Test that we properly parse the data 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev ParseStatusRegister [expr (1<<6)|(0<<5)|(3<<3)|5]
} -result {1 0 3 5}

tcltest::cleanupTests
