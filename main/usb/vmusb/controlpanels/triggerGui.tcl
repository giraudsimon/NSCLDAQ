package require Tk

#
# Update a single label with the width it contains
#
# @param widget - the widget whose label will be changed.
# @param format - Format string into which the ns will be encoded.
# @param register - Value of the associated width register.  ns = (register+1)*10 according to 
#                   Carlo Tintori's guide.
#
#
proc updateWidth {widget format register} {
    set ns [expr {($register + 1)*10}]
    $widget configure -text [format $format $ns]
}

#
#  Update the width labels to show the ns of delay selected.  Note
#  This is the delay that will be set if the commit button will be pressed
#  or alternatively, the actual widths if the commit button does not need
#  to be pressed.
#
#
proc updateLabels {} {
    if {[winfo exists .short]} {
	set short [.short get]
	if {$short ne ""} {
	    updateWidth .shortl {Short width (%d ns)} $short
	}
    }

    if {[winfo exists .long]} {
	set long [.long get]
	if {$long ne ""} {
	    updateWidth .longl {Long width (%d ns)} $long
	}
    }
}
#
#  - Commit the current set of settings to the device.
#  - Set the background color of the commit button to normal.
#  - Write the current settings out to the config file so that we'll
#    start with them next time.
#
#  @note - if the commit fails, the file is still written and the user is informed of this.
#          in that case the commit button remains red to inform the user
#          the GUI does not match the device.
#
proc doCommit {} {
    set result [commit]
    if {[lindex $result 0] eq "ERROR"} {
	tk_messageBox -title {Failed to set device} \
	    -message "Unable to commit settings to the device: $result. Settings file will be updated anyway"
    } else {
	set bg [. cget -background]
	.commit configure -background $bg
    }

    set fd [open $::configFile w]
    foreach  getter [list getModuleName getHost getShortGate getLongGate getMask getGates]  \
	varname [list name host shortGate longGate mask control] {
	    puts $fd "[list set $varname [$getter]]"
	}
    close $fd
}
#
#  Sets the color of the Commit button to red.  This is done anytime there's been some
#  change to the GUI state to flag that the user must commit these changes to the device
#  for them to take effect.  This is done to minimize the communication with the
#  Readout GUI.
#
proc changeColor {} {
    if {[winfo exists .commit]} {
	.commit configure -background red
    }
}

#
#  The Trigger module requires that the long gate not be shorter or the same 
#  length as the short gate. This proc is therefore invoked anytime the 
#  short gate length is changed. It ensures the long gate is at least one greater than the
#  short.
#  It also updates the labels above the boxes to reflect real units for the gate widths.
#
# @param new - the new proposed value of the short gate.
#
# @return true - Since this is a validation command it must return true to allow the modification.
#
proc checkLongGate {new} {
    global  longgate
    if {$new >= $longgate} {
	set longgate [expr $new +1]
    }
    setLongGate $longgate
    setShortGate $new

    changeColor

    after 1 updateLabels
    return true

}

#
#  See checkLongGate, this ensures the short gate is always at lest longgate -1
#  It also updates the labels above the width boxes to reflect the true gate widths in ns.
#  
# @param new - new proposed value of the long gate.
#
# @return true - Since this is a validation command it must return true to allow the modification.
#
proc checkShortGate {new} {
    global shortgate
    if {($new <= $shortgate) && ($new > 0)} {
	set shortgate [expr $new -1]
    }
    setShortGate $shortgate
    setLongGate $new
    changeColor
    after 1 updateLabels
    return true
}

#
#  Puts the current values of the g0/g1 select variables [test point selectors]
#  in the g0select and g1select variables.
#
proc getGateVariables {} {
    global g0select
    global g1select

    set g0g1 [getGates]
    set g0select [expr $g0g1 &0xf]
    set g1select [expr ($g0g1 & 0xf0) >> 4]
}
#
#  Given the values of the test point selections prepares them for 
#  the commit.
#
proc setTestPoints {} {
    global g0select g1select

    setG0 $g0select
    setG1 $g1select
    changeColor
    
}
#
#  Processes the current value of the channel enable mask and turns it into the 
#  maskBits array which has one element per bit..set to one or zero depending on t
#  the state of that bit.
#
proc setMaskBits {} {
    global maskBits
    set mask [getMask]
    for {set i 0} {$i < 20} {incr i} {
	if {($mask & (1 << $i)) != 0} {
	    set maskBits($i) 1
	} else {
 	    set maskBits($i) 0
	}
    }
}
#
#  Set the enables mask from the individual bit values in maskBits(i).
#
proc setChMask {} {
    global maskBits

    set mask 0
    for {set i 0} {$i < 20} {incr i} {
	if {$maskBits($i)} {
	    set mask [expr $mask | (1 << $i)]
	}
    }
    setMask $mask
    changeColor
}

#---------------------------------------------------------------------------------------------
#
#  Initialization and GUI setup.
#


# Read the configuratino file

source [file join [file dirname [info script]] triggerRegs.tcl]


set configFile [lindex $argv 0]
source $configFile


wm title . $configFile

setModuleName $name;		# Selects the module within the set of potential trigger modules.

if {[info globals $host] ne ""} {
    setHost $host
}


# get current values.

getValues

# Override with any settings in the configuration file:

foreach proc [list setShortGate setLongGate setMask setGates] varname [list shortGate longGate mask control] {
    if {[info globals $varname] ne ""} {
	$proc [set $varname]
    }
}
commit;				# Try to force the settings into the box.


set shortgate [getShortGate]
set longgate [getLongGate]

##
#  Create and layout the GUI Widgets 
#

#
#  The gui contains a spinbox for each of the
#  delays.. .the shortgate spinbox -vcmd is used to ensure the
#  longgate is longer.
#
#  A radio button for each of the G0/G1 signals selects the 
#  test point.
# 
#  Checkboxes disable channels.
#
#

label .shortl -text {Short width}
label .longl  -text {Long width}
spinbox .short -from 0 -to 0xfe -increment 1 -vcmd [list checkLongGate %P] -textvariable shortgate \
    -validate all
spinbox .long -from  1 -to 0xff -increment 1 -vcmd [list checkShortGate %P] -textvariable longgate \
    -validate all



grid .shortl .longl
grid .short  .long
# The test point radio buttons. The test points represent:
#  1 Majority >= 1   (M1)
#  2 Majority >= 1 50ns output widdth. (M1-50)
#  3 Majority >= 1 250ns output width. (M1-250)
#  5 Majority >= 2 or Majority of inputs 10-19 >= 1. (M2)
#  6 Majority >= 2 or Majority of inputs 10-19 >= 1. 50ns output width (M2-50)
#  7 Majority >= 2 or Majority of inputs 10-19 >= 1. 250ns output width. (M2-250)
#


frame .g0sel -relief groove -borderwidth 2
frame .g1sel -relief groove -borderwidth 2

label .g0sel.l -text {Test point G0}
label .g1sel.l -text {Test point G1}

grid .g0sel.l -columnspan 6
grid .g1sel.l -columnspan 6

set c 0
foreach label [list Off M1 M1-50ns M1-250ns M2 M2-50ns M2-250ns] \
    value [list 0 1 2 3 5 6 7] {
	set r0 [radiobutton .g0sel.g0$label -text $label -value $value -variable g0select \
		    -command [list setTestPoints]]
	set r1 [radiobutton .g1sel.g1$label -text $label -value $value -variable g1select \
		    -command [list setTestPoints]]
	grid $r0 -row 1 -column $c
	grid $r1 -row 1 -column $c

	incr c

}
grid .g0sel -columnspan 2
grid .g1sel -columnspan 2

getGateVariables

#
#  The channel masks  These boxes are checked for channels that are enabled.
#
#

frame .enables -relief groove -borderwidth 2


array set maskBits [list]
setMaskBits

set r 0
set c 0
for {set i 0} {$i < 20} {incr i} {
    label        .enables.l$i     -text In[format %02d $i]
    checkbutton .enables.e$i -variable maskBits($i) -command setChMask -onvalue 1 -offvalue 0
   
    grid .enables.l$i -row $r -column $c
    grid .enables.e$i -row [expr $r+1] -column $c

    if {$i == 9} {
	incr r 2
	set c 0
    } else {
	incr c
    }
}

grid .enables

button .commit -text {Commit} -command [list doCommit]
grid .commit




