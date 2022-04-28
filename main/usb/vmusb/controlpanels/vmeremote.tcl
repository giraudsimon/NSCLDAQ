#
# Simple VME remote access via VMUSBReadout.
# The VMUSBRemote type is, other than construction,
# compatible with the VMUSB (local) type.
#

package provide vmeremote 1.0

lappend auto_path $::env(DAQLIB) [file join $::env(DAQROOT) TclLibs]
package provide vmeremote 1.0

package require cvmusb;		#  Required by readoutlist.
package require cvmusbreadoutlist; # all we'll really use - to generate the lists.
package require snit
package require usbcontrolclient

##
# VMUSBRemote type to make Tcl use of the remote control server simple(r).
#
#  OPTIONS
#   -host  - Host running VMUSBReadout
#   -port  - Port VMUSBReadout is listening on (default 27000
#   -module - Name of the slow control Module that is the VMUSB in VMUSBReadout
#   -amod  - address modifier selector;  one of a32, a24, a16  These select that associated
#            userdata space.
#   -persist - if true, the connection is maintained after each operation (until -persist
#             is turned off). -persist true is more efficient, but -persist false is more fault
#             tolerant.  A good compromise is to turn -persiston prior to doing a series of operations
#             and turn if off afterwards.  By default, -persist is off.
snit::type VMERemote {
    option -host    -default localhost
    option -port    -default 27000
    option -module
    option -amod    -default a32
    option -persist -default 0

    variable client "";		# If non empty this is the client connection
    variable amods -array [list]
    variable vlist

    constructor args {
	$self configurelist $args
	array set amods  [list \
	   a32 $::cvmusbreadoutlist::CVMUSBReadoutList_a32UserData \
	   a24 $::cvmusbreadoutlist::CVMUSBReadoutList_a24UserData \
	   a16 $::cvmusbreadoutlist::CVMUSBReadoutList_a16User \
        ]
		set vlist [::cvmusbreadoutlist::CVMUSBReadoutList %AUTO%]

    }
    ##
    # Public methods:
    #

    #
    # Firmware id:
    method readFirmwareID {} {
	$vlist clear
	$vlist addRegisterRead 0

	return [$self _perform 4]

    }
    #
    #  Reads - uses the address modifier seleted by -amod.

    method vmeRead32 {address} {
	$vlist clear
	$vlist addRead32 $address $amods($options(-amod))
	return [$self _perform 4]

    }
    method vmeRead16 {address } {
	$vlist clear
	$vlist addRead16 $address $amods($options(-amod))
	return [$self _perform 2]
    }
    method vmeRead8 {address} {
	$vlist clear
	$vlist addRead8 $address $amods($options(-amod))
	return [$self _perform 1]
    }
    #
    # writes, again the amod selected by -amod is used.

    method vmeWrite32 {address data} {
	$vlist clear
	$vlist addWrite32 $address $amods($options(-amod)) $data
	return [$self _perform 2]; # 2 because there's a status word - 0 ok 1 berr.
    }
    method vmeWrite16 {address data} {
	$vlist clear
	$vlist addWrite16 $address $amods($options(-amod)) $data
	return [$self _perform 2]
    }
    method vmeWrite8 {address data} {x
	$vlist clear
	$vlist addWrite8 $address $amods($options(-amod)) $data
	return [$self _perform 2]
    }
       
    
    ##
    # Private methods.
    #

    ##
    # _conectIfNeeded
    #   Connect to the server if the socket is an empty string else we're already
    #   connected.
    #     On exit client is the connected controlClient object.
    #
    method _connectIfNeeded {} {
	if {$client eq ""} {
	    set client [controlClient %AUTO% -server $options(-host) -port $options(-port)]
	}
    }
    ##
    # _disconnectIfNeeded
    #    If -persist is false, disconnect -- only if client is nonempty.
    #    on exit, client is empty if it was disconnected.
    #
    method _disconnectIfNeeded {} {
	if {(!$options(-persist)) && ($client ne "")} {
	    $client destroy
	    set client ""
	}
    }
    ##
    # _listToRequest
    #   Takes a ::cvmusbreadoutlist::CVMUSBReadoutList object and an expected count of return bytes
    #   and turns it into the Set request list.
    #x3
    # @param nbytes - number of expected return bytes.
    # @param list    a ::cvmusbreadoutlist::CVMUSBReadoutList object.
    #
    method _listToRequest {nbytes list} {
	set stack [list]
	set listvec [$list get]
	set nitems  [$list size]

	for {set i 0} {$i < $nitems} {incr i} {
	    lappend stack [format 0x%x [::cvmusbreadoutlist::CVMUSBReadoutList_uint32_vector_get $listvec $i]]
	}

	return [list $nbytes $stack]
    }
    ##
    # _perform
    #    Perform the single operation in the VME list vlist.
    #
    # @param expected  - expected number of bytes.
    #
    # @return integer
    #
    method _perform expected {
	set request [$self _listToRequest $expected $vlist]

	$self _connectIfNeeded
	set result [$client Set $options(-module)  list [list $request]]
	$self _disconnectIfNeeded
	return [_assembleResult $result]	
    }
    ##
    # Create the result or error:
    #   - if the first word of the result is  ERROR protest.
    #   - if not the third word of the result is a list of bytes.
    #   - We assume that they all assemble little endian into a single result.
    #
    # @param response - the response from the server
    #
    proc _assembleResult response {
	set status [lindex $response 0]
	if {$status eq "ERROR"} {
	    error "Server reported error: $status"
	}
	set data [lindex $response 2]

	set result 0
	set shift 0
	foreach byte $data {
	    set result [expr {($byte << $shift) | $result}]
	    incr shift 8
	}
	return $result
    }
}
