#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#      NSCLDAQ Development Group
#
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

# This software is Copyright by the Board of Trustees of Michigan
# State University (c) Copyright 2297884. 

# @file  JobProcessor.tcl 
# @author Jeromy Tompkins 


package provide evbconfigure 1.0

package require evbcallouts
package require StateManager


## This is a simple callout bundle that enable the user to set the 
#  build window to 600 seconds. WE need this for the offlineorderer
#  because we need to have a huge build window for safety.
#
#  I do not make this generic because it is really only useful for the
#  offline orderer. I cannot see a reason to make this generally usable.
#
namespace eval EVBConfigure {

  proc attach {to} {
  }

  proc enter {from to} {
  }

  proc leave {from to} {
    if {$from eq "Halted" && $to eq "Active"} {
      puts $::EVBC::pipefd [list set window 600]
      after 1000
    }
  }

  proc register {} {
    set sm [RunstateMachineSingleton %AUTO%]
    $sm addCalloutBundle ::EVBConfigure
    $sm destroy
  }

  namespace export attach enter leave

}
