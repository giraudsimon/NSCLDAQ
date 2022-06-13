#
#   Snit megawidget to control an Ph7106 leading edge discriminator
#   via the tcp port.
#
package provide ph7106Widget 1.0
package require Tk
package require snit

snit::widget ph7106Widget {
    variable socket
    variable mode      local
    variable threshold 55
    variable enables -array { 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
    }


    option   -name "led";	# Module name in configuration file.
    option   -host  localhost
    option   -port 27000

    constructor args {
	variable socket
	variable mode
	variable threshold
	variable enables

	$self configurelist $args
        set host $options(-host)
        set port $options(-port)

	set socket [socket $host $port]
	fconfigure $socket -buffering line
	

	# Initialize the state from the hardware.

	label $win.title -text "PH7106 control for [$self cget -name]"

	label $win.threshl -text "Threshold: "
	spinbox $win.thresh -from 0 -to 1023 -textvariable [myvar threshold] \
	    -command [mymethod setThreshold] -width 5
	

	label $win.maskl -text "Channel Enables"
	for {set i 0} {$i < 16} {incr i} {
	    checkbutton $win.mask$i -text $i -variable [myvar enables($i)] \
		-command [mymethod setMask]
	}
	label $win.state -textvariable [myvar mode]

	# Layout the widgets.

	grid $win.title -row 0 -column 0 -columnspan 10 
	grid $win.state -row 0 -column 10

	grid $win.threshl -columnspan 10
	grid $win.thresh  -columnspan 10

	grid $win.maskl -columnspan 10
	for {set i 0} {$i < 8} {incr i} {
	    grid $win.mask$i -row 4 -column $i
	}
	for {set i 8} {$i < 16} {incr i} {
	    set col [expr $i - 8]
	    grid $win.mask$i -row 5 -column $col
	}

	# Update once and assume the user won't futz with us.

	$self update

    }

    # Performs a transaction with the controller
    # returns the response... which is transaction dependent.
    #
    method transaction command {
	variable socket

	puts $socket $command
	flush $socket
	return [gets $socket]

    }
    #
    # Return mode: (camac or local)
    #
    method getMode {} {
	set result [$self transaction "Get [$self cget -name] mode"]
	return $result
    }
    #  Return threshold:

    method getThreshold {} {
	set result [$self transaction "Get [$self cget -name] threshold"]
	return $result
    }
    #  Update the enables array:

    method updateEnables {} {
	variable enables

	set result [$self transaction "Get [$self cget -name] mask"]
	for {set i 0} {$i < 16} {incr i} {
	    if { $result & (1 << $i)} {
		set enables($i) 1
	    } else {
		set enables($i) 0
	    }
	}
    }
    method setThresholdValue newValue {
	variable threshold
	set threshold $newValue
	$self setThreshold
    }

    method setThreshold {} {
	variable threshold
	set result [$self transaction "Set [$self cget -name] threshold $threshold"]
    }
    method setMaskValue mask {
        set enables $mask
        $self setMask
    }
    method setMask {} {
	variable enables

	set mask 0

	for {set i 0} {$i < 16} {incr i} {
	    if {$enables($i)} {
		set mask [expr $mask | (1 << $i)]
	    }
	}
	set result [$self transaction "Set [$self cget -name] mask $mask"]

    }
    method update {} {
	variable threshold
	variable mode
	variable timerId
	$self updateEnables
	set mode [$self getMode]
	set threshold [$self getThreshold]
	$win.thresh set $threshold

    }
}