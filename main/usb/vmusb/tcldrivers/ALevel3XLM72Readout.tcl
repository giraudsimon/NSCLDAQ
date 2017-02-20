#
#   ALEvel3XLM72 for readout:
#

package provide ALevel3XLM72Readout 1.0
package require Itcl
package require xlm72

itcl::class ALevel3XLM72 {
    inherit AXLM72

    global stoppedstate

    constructor {de sl} {
	AXLM72::constructor $de $sl
    } {}

    method XSet {dev offset value}

    method ResetTS {}

    method RunStop {stop}
    method sStamp  {stack}
    method sTrigger {stack}
}

itcl::body ALevel3XLM72::ResetTS {} {
    AccessBus 0x10000

    XSet XLM 0x400050 1;	# Toggle the reset bit.
    XSet XLM 0x400050 0
    XSet XLM 0x40004c 1;	# Enable the timestamp counter.
    
    ReleaseBus
}

itcl::body ALevel3XLM72::RunStop stop {
    AccessBus 0x10000

    XSet XLM 0x400010 $stop

    ReleaseBus
}

itcl::body ALevel3XLM72::sStamp stack {
    sAccessBuss $stack 0x10000
    sRead       $stack fpga 72
    sRead       $stack fpga 84
    sReleaseBus $stack
}

itcl::body ALevel3XLM72::sTrigger stack {
    sAccessBus  $stack 0x10000
    sRead       $stack fpga 88
    sReleaseBus $stack
}

itcl::body ALevel3XLM72::XSet {name address data} {
    Write base $addr $data
}
