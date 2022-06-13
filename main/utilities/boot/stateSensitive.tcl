##
# state sensitive tcl script.
# Expects the evn vars:
#  TRANSITION_SUBSCRIPTION_URI
#  TRANSITION_REQUEST_URI
#  DAQROOT to be set.
#
#  Establishes a connection to the state manager publication and
#  exits when the state transition to NotReady.

set TclLibs [file join $env(DAQROOT) TclLibs]
lappend auto_path $TclLibs

package require removetcllibpath
package require statemanager

statemanager::statemonitor start $env(TRANSITION_REQUEST_URI) $env(TRANSITION_SUBSCRIPTION_URI)




proc exiter {from to} {
    
    if {[string toupper $to] eq "NOTREADY"} {
	exit
    }
}

statemanager::statemonitor register NotReady exiter

vwait forever
