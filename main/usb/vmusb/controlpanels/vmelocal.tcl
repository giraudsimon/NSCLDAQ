##
#  Simple VME local access via a VM-USB.
#


package provide vmelocal 1.0

#
#  Assume daqsetup has been invoked.  Note the swig wrapper packages
#  are in DAQLIB
#


lappend auto_path $::env(DAQLIB)

package require cvmusb
package require cvmusbreadoutlist

package require snit

# enumerateVMUSB
#
#  @return list - of pairs of VMUSB serial numbers and devices.
#
#
proc enumerateVMUSB  {} {
    set devVector [::cvmusb::CVMUSB_enumerate]
    set nUsb      [::cvmusb::usb_device_vector_size $devVector]
    set result [list]

    for {set i 0} {$i < $nUsb} {incr i} {
	set dev [::cvmusb::usb_device_vector_get $devVector $i]
	lappend result [list [::cvmusb::CVMUSB_serialNo $dev] $dev]
    }
    return $result
}


##
#  VMUSB type to make using cvmusb::CVMUSBusb a bit (a lot)? simpler.
#
# OPTIONS:
#    -device - the VMUSB device used to instantiate a CVMUSBusb object.
#    -amod   - Address modifier used to perform operations.
snit::type VMUSB {
    option -device -readonly 1
    option -amod -default a32
    option -persist;		# So that we're compatible with vmeremote.

    variable controller
    variable amods -array [list]

    ##
    # constructor configure the device and if possible the address modifier.
    # construct the controller and save it in congtroller
    #
    constructor args {
	$self configurelist $args

	set controller [::cvmusb:::CVMUSBusb ${self}.dev  $options(-device)]
	array set amods  [list \
     a32 $::cvmusbreadoutlist::CVMUSBReadoutList_a32UserData \
     a24 $::cvmusbreadoutlist::CVMUSBReadoutList_a24UserData \
     a16 $::cvmusbreadoutlist::CVMUSBReadoutList_a16User \
			      ]
    }
    destructor { catch {$controller destroy}}
    ##
    # Methods to access controller and VME:
    #

    #
    #  Firmware id
    #
    method readFirmwareID {} {
	return [$controller readFirmwareID]
    }
    # 32 bit transfers:
    
    method vmeRead32 {address} {
	return [$controller vmeRead32 $address $amods($options(-amod))]
	
    }
    method vmeWrite32 {address data} {
	$controller vmeWrite32 $address $amods($options(-amod)) $data
    }
    # 16 bit transfers:
    
    method vmeRead16 {address} {
	return [$controller vmeRead16 $address $amods($options(-amod))]
    }
    method vmeWrite16 {address data} {
	$controller vmeWrite16 $address $amods($options(-amod)) $data
    }
    # 8 bit transfers:

    method vmeRead8 {address} {
	return [$controller vmewRead8 $address $amods($options(-amod))]
    }
    method vmeWrite8 {address data} {
	$controller vmeWrite8 $address $amods($options(-amod)) $data
    }
}
