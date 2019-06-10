package provide chicotrigger 1.0

namespace eval trigger {
    variable host localhost
    variable port 27000

    variable name trigger
    variable shortWidth
    variable longWidth
    variable controlRegister
    variable mask
}

#------------------------------------------------------------------------
#
#  These just manipulate the local shadow data.
#  see commit and getValues for the proc's that actually communicate with the
#  server.

##
#  Set the module name for later use:
#
#  @param name - name of the module in configuration database.
#

proc setModuleName name {
    set ::trigger::name $name
}
proc getModuleName {} {
    return $::trigger::name
}

##
# Set the host on which the VMUSBReadout is running.
#
# @param name -dns host name or dotted ip address.
# 
proc setHost name {
    set  ::trigger::host $name
}
proc getHost {} {
    return $::trigger::host
}
##
# Set the short gate width of the module
#
# @param width - new short width.
#
proc setShortGate width {
    set ::trigger::shortWidth $width
}
##
# Return the saved shortGate width value. This does not
# perform any actual I/O, but just returns the last remembered value.
#
proc getShortGate {} {
    return $::trigger::shortWidth
}
##
# set the long gate width of the module.
# 
# @param width - new width value
#
proc setLongGate width {
    set ::trigger::longWidth $width
}
##
# Returns the last remembered long gate width value:
#
proc getLongGate {} {
    return $::trigger::longWidth
}
##
#  Set the new channel enable mask value
#
# @param mask - new mask value.
#
proc setMask mask {
    set ::trigger::mask $mask
}
##
# get the last mask value
#
proc getMask {} {
    return $::trigger::mask
}
##
# set the G0  test point
#
#  @param which - the new test point selector.
#
proc setG0 which {
    set which [expr $which & 0xf]
    set current $::trigger::controlRegister
    set current [expr ($current & 0xfffffff0) | $which]
    set ::trigger::controlRegister  $current
}
##
# set the G1 test point selector.
#
# @param which - selects the test point that G1 monitors.
#
proc setG1 which {
    set which [expr $which & 0xf]
    set current $::trigger::controlRegister
    set current [expr ($current & 0xffffff0f) | ($which << 4)]
    set ::trigger::controlRegiseter $current
}
##
# set the G0/G1 outputs to be TTL:
#
proc ttl {} {
    set current $::trigger::controlRegister
    set current [expr ($current & 0x000000ff) | (0x100)]
    set ::trigger::controlRegister $current
}
##
# Set the G0/G1 outputs to be nim
#
proc nim {} {
    set current [trigger_unit get -l 0x101c]
    set current [expr ($current & 0x000000ff)]
    set ::trigger::controlRegister $current

}
##
# Get the last remembered control register value.
#
proc getGates {} {
    return $::trigger::controlRegister
}
##
# Set the entire gate register:
#
proc setGates value {
    set ::trigger::controlRegister $value
}
#------------------------------------------------------------
#
#  Actual I/O.

##
#  Connect to the server and read the current values into the
# control variables:
#
# @return string -
# @retval "OK" success
# @retval "ERROR - message" if an error
proc getValues {} {
    if {[catch {socket $::trigger::host $::trigger::port} fd]} {
	return "ERROR - Unable to connect to server"
    }

    fconfigure $fd -buffering line
    puts $fd "Get $::trigger::name all"
    gets $fd value
    close $fd
    if {[lindex $value 0] eq "ERROR"} {
	return $value
    }
    set values [lindex $value 1]
    foreach value $values name [list shortWidth longWidth mask controlRegister] {
	set ::trigger::$name $value
    }
    return "OK"
	
}
##
#  commit the current set of values to the server.
#
#
proc commit {} {
    if {[catch {socket $::trigger::host $::trigger::port} fd]} {
	return "ERROR - Unable to connect to server"
    }

    fconfigure $fd -buffering line
    puts $fd "Set $::trigger::name all {$::trigger::shortWidth $::trigger::longWidth  \
                                 $::trigger::mask $::trigger::controlRegister}"
    gets $fd reply

    close $fd
    return $reply
}
