#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

lappend auto_path [file normalize [file join [file dirname [info script]] .. .. lib]]

package require snit
package require VMUSBDriverSupport
package require cvmusb
package require cvmusbreadoutlist

package provide SIS330XDriver 1.0

##
# @file sis3340xdriver.tcl 
#
#   Provides a Tcl/Snit VM-USB device driver for the 
#   SIS330x Flash adc module.  The options are closely patterned
#   after the methdos of the sis3300.cpp class which implements
#   SBS support for this module.
#
#
#  OPTIONS:
#     -base            - Module base address in VME
#     -clocksource     - Module clock source.. See clocksourceValues bellow for the valid list.
#     -startdelay      - Boolean.  If true the start delay is enabled and:
#     -startdelayticks - this is the number of samples in the start delay.
#     -stopdelay       - Boolean. IF true, the stop dela is enabled and:
#     -stopdelayticks  - this is the number of sample in the stop delay.
#     -stoptrigger     - The module stop is the trigger.
#     -gatemode        - Module is or is not in gate mode (start stop from gate input fall/rise)
#     -lemostartstop   - Start/Stop come from lemo inputs.
#     -p2startstop     - Backplane signals on the p2 connector provide start/stop.
#     -hirarandomclock - If true, HiRA random clock mode is enabled.
#     -randomclock     - If true front panel provides non-periodic clock (not as random as
#                        HiRA's clock).
#     -samplesize      - Size of sample buffers.
#     -wrap            - Buffers are  in wrap mode (normally used if start/stop is longer than
#                        samplesize.
#     -thresholdslt    - Thresholds represent a  level _below_ which the conversion must fall.
#     -thresholds      - Threshold values for all 8 channels.
#     -groupsread      - list of flags indicating which groups of ADCs will be read (the module
#                        is organized into 4 groups of 2 ADCs each).
#  PUBLIC METHODS:
#     Initialize     -  Initialize the module in accordance with the current configuration.
#     addReadoutList -  Adds elements to the readout list that read the module.   

snit::type sis330xDriver {
    option -base            -default 0              -configuremethod configureInt
    option -clocksource     -default 100Mhz         -configuremethod configureClockSource
    option -startdelay      -default off            -configuremethod configureBool
    option -startdelayticks -default   0            -configuremethod configureInt
    option -stopdelay       -default off            -configuremethod configureBool
    option -stoptrigger     -default off            -configuremethod configureBool
    option -stopdelayticks  -default   0            -configuremethod configureInt
    option -gatemode        -default off            -configuremethod configureBool
    option -lemostartstop   -default on             -configuremethod configureBool
    option -p2startstop     -default off            -configuremethod configureBool
    option -hirarandomclock -default off            -configuremethod configureBool
    option -randomclock     -default off            -configuremethod configureBool
    option -triggeronstop   -default off            -configuremethod configureBool
    option -samplesize      -default 128K           -configuremethod configureSampleSize
    option -wrap            -default off            -configuremethod configureBool
    option -thresholdslt    -default false          -configuremethod configureBool
    option -thresholds      -default [list 0x3fff 0x3fff 0x3fff 0x3fff 0x3fff 0x3fff 0x3fff 0x3fff] -configuremethod configureThresholds
    option -groupsread      -default [list true true true true] -configuremethod configureGroupsRead

    package require sis3300defs
sy    #-----------------------------------------------------------------------
    #
    # constructor:
    #
    # Provides a chance to configure and create in one step:
    #
    constructor args {
	$self configurelist $args
    }
    #-----------------------------------------------------------------------
    #
    # Public methods that implement the driver interface:
    #

    # Initialize:
    #
    #  Initialize the device to the configuration defined by the configuration
    #  options.
    #
    # @param vmusb - VM-USB controller object handle.  This must be converted to
    #                a cvmusb object.
    #
    method Initialize vmusb  {
	set controller [::VMUSBDriverSupport::convertVmUSB $vmusb]
	set base       $options(-base)

	# Reset as a single shot operation then 

	$controller vmeWrite32 [expr $base +  $Reset] $setupAmod 0

	# Build a list of operations to complete the init:

	set list       [::cvmusbreadoutlist::CVMUSBReadoutList]

	# Figure out and set the  initial CSR.

	set csrValue   [expr $CRUserOutputOff | \
			    $CRLedOff        | \
			    $CREnableTriggerOutput | \
			    $CRNormalTriggerOutput]

	if {$options(-stoptrigger)} {
	    set csrValue [expr $csrValue | $CRTriggerOnArmedAndStarted]
	} else {
	    set csrValue [expr $csrValue | $CRTriggerOnArmed]
	}
	$list addWrite32 [expr $base + $CR] $setupAmod $csrValue
	puts "CR Value written as [format %08x $csrValue]"

	# Turn off all bits in the ACQ register:

	$list addWrite32 [expr $base + $DAQControl] $setupAmod 0xffff0000
	$list addDelay 20

	# Figure out what needs turning on in the acquisition register (having
	# first turned everything off) ( and program related parameters if necessary)

	set acqValue 0

	# start delay also implies programing the start delay value:

	if {$options(-startdelay)} {
	    set acqValue [expr $acqValue | $DAQStartDelayOn]
	    $list addWrite32 [expr $base + $ExternStartDelay] $setupAmod $options(-startdelayticks)
	}
	# stop delay also implies programming the stop delay value:

	if {$options(-stopdelay)} {
	    set acqValue [expr $acqValue | $DAQStopDelayOn]
	    $list addWrite32 [expr $base + $ExternStopDelay] $setupAmod $options(-stopdelayticks)
	}
	if {$options(-gatemode)} {
	    set acqValue [expr $acqValue | $DAQEnableGateMode]
	}
	if {$options(-randomclock)} {
	    set acqValue [expr $acqValue | $DAQEnableRandomClock]
	}
	if {$options(-lemostartstop)} {
	    set acqValue [expr $acqValue | $DAQEnableLemoStartStop]
	}
	if {$options(-p2startstop)} { 
	    set acqValue [expr $acqValue | $DAQEnableP2StartStop]
	}
	if {$options(-hirarandomclock)} {
	    set acqValue [expr $acqValue | $DAQEnableHiRARCM | $DAQEnableRandomClock]
	}
 
	set acqValue [expr $acqValue | ($clockSourceValues($options(-clocksource)) << $DAQClockSetShiftCount)]
	
	$list addWrite32 [expr $base + $DAQControl] $setupAmod $acqValue
	puts "ACQ Value [format %04x $acqValue]"
	

	# Configure the global event configuration register.

	set evtConfig [expr $sampleSizeValues($options(-samplesize)) << $ECFGPageSizeShiftCount]
	if {$options(-wrap)} {
	    set evtConfig [expr $evtConfig | $ECFGWrapMask]
	}
	if {$options(-randomclock)} {
	    set evtConfig [expr $evtConfig | $ECFGRandomClock]
	}
	puts "Evt config register [format %08x $evtConfig]" 
	$list addWrite32 [expr $base + $CommonInfo + $EventConfiguration] $setupAmod $evtConfig

	
	# Channel thresholds:

	puts "Setting channel enables"

	for {set chan 0} {$chan < 8} {incr chan 2} {
	    set even [lindex $options(-thresholds) $chan]
	    if {$options(-thresholdslt)} {
		set even [expr $even | $THRLt]
	    }
	    set odd [lindex $options(-thresholds) [expr $chan + 1]]
	    if {$options(-thresholdslt)} {
		set odd [expr $odd | $THRLt]
	    }
	    set registerValue [expr ($odd << $THRChannelShift) | $even]
	    puts "Writing threshold [format "%08x to %08x" $registerValue [expr $base +  $EventBases([expr $chan/2]) + $TriggerThreshold]]"

	    $list addWrite32 \
		[expr $base +  $EventBases([expr $chan/2]) + $TriggerThreshold] \
		$setupAmod $registerValue
	}
	puts "Done setting channel thresholds"

	# Disarm both banks first:

	$list addWrite32 [expr $base + $DAQControl] $setupAmod \
	    [expr $DAQSampleBank1Off | $DAQSampleBank2Off]


	
	# Execute the list:
	
	puts "Executing list"

	$controller executeList $list 1000
#	$inputData destroy
	$list -delete
	puts "Returning from init"

	# Dump the values of some key registers:

	set cr [$vmusb vmeRead32 [expr $base + $CR] $setupAmod]
	puts "Control register [format %08x $cr]"

	set daqctl [$vmusb vmeRead32 [expr $base + $DAQControl] $setupAmod]
	puts "DAQ control register  [format %08x $daqctl]"


	set evconfig [$vmusb vmeRead32 [expr $base + $EventInfo1] $setupAmod]
	puts "Event config: [format %08x $evconfig]"

	# Do some one shot stuff at the end of the bulk initialization.
	#  - Clear the data
	#  - arm bank 1
	#  - If the configuration justifies, start sampling.

	$self _SetArm $controller 1 on

	if {!$options(-gatemode)} {
	    puts "Turning on sampling now!"
	    $self _StartSampling $controller
	}
	set daqctl [$vmusb vmeRead32 [expr $base + $DAQControl] $setupAmod]
	puts "DAQ control register(after arming)  [format %08x $daqctl]"	
    }
    ##
    # addReadoutList 
    #   Add elements to the readout list that responds to the trigger for the stack
    #   an instance of this class belongs to.
    #
    #   The resulting data structure is:
    #    +----------------------+
    #    | group mask           | 1's for each group read.
    #    +----------------------+
    #    | group longword count | (for lowest ordered bit in the group mask).
    #    +----------------------+
    #    | Samples for group... |
    #    \                      \ See SIS manual for the format here.
    #    /                      /
    #    +---------------------+
    #      ...
    #
    #
    # @param list - Swig handle to a cvmusbreaoutlist::VMUSBReadoutList object.
    #               This must be converted to a swig object.
    #
    #
    method addReadoutList list {
	set base $options(-base); # using this frequently so...
	set list [VMUSBDriverSupport::convertVmUSBReadoutList $list]

	# Stop the adc sample clock:

	$list addWrite32 [expr $base + $VMEStop] $setupAmod 1
	$list addWrite32 [expr $base + $DAQControl] $setupAmod $DAQSampleBank1Off

	# Prefix the output with a mask of the groups we'll read:

	set mask 0 
	$list addMarker 0xfadc
	foreach bit [list 1 2 4 8] enable $options(-groupsread) {
	    if {$enable} {
		set mask [expr $mask | $bit]
	    }
	}
	$list addMarker $mask
	$list addDelay  1

	# Read each enabled group using the address register to provide a count.

	foreach enable $options(-groupsread) \
	    evInfoBase [list $EventInfo1 $EventInfo2 $EventInfo3 $EventInfo4] \
	    evBuffer [list $Bank1Group1 $Bank1Group2 $Bank1Group3 $Bank1Group4] {
		if {$enable} {
		    puts "Adding maske count read from \
[format %08x [expr $base + $evInfoBase + $EventDirectory1]] mask : [format %08x $sampleMaskValues($options(-samplesize))]" 
		    $list addBlockCountRead32 \
			[expr $base + $evInfoBase + $EventDirectory1] \
			[expr $sampleMaskValues($options(-samplesize))] $setupAmod; # read count..
		    
		    $list addMaskedCountBlockRead32 [expr $base + $evBuffer] $blockXfer; # the read itself.
		    $list addMarker 0xaaaa
		    $list addDelay   1
		}
	    }

	#  Clear daq, re-arm and start sampling.
	#

	$list addWrite32 [expr $base + $DAQControl] $setupAmod $DAQSampleBank1On
	$list addWrite32 [expr $base + $VMEStart]   $setupAmod 1


    }

    method onEndRun vmusb {

    }

    #-----------------------------------------------------------------------
    #
    # Private utilties:


    ##
    #  method _Clear controller
    #
    #  Clears the data in the module
    #
    # @param controller - VMUSB controller object.
    #
    method _Clear controller {
	$controller vmeWrite32 [expr $options(-base) +$DAQControl] \
	    $setupAmod $DAQSampleBank1On
    }
    ##
    # method _SetArm controller which state
    #
    # Arms or disarms an adc bank:
    #
    # @param controller - VMUSB controller object.
    # @param which      - Which bank to arm/disarm (1 or 2).
    # @param state      - What to do: true - arm, false disarm
    #
    method _SetArm {controller which state} {
	array set arms    [list 1 $DAQSampleBank1On  2 $DAQSampleBank2On]
	array set disarms [list 1 $DAQSampleBank1Off 2 $DAQSampleBank2Off]

	set addr [expr $options(-base) + $DAQControl]
	if {$state} {
	    set value $arms($which)
	} else {
	    set value $disarms($which)
	}
	puts "Arm/disarm: [format "%08x @ %08x" $value $addr]"
	$controller vmeWrite32 $addr $setupAmod $value
    }
    #
    # _StartSampling controller
    #
    # Start the ADC Sampling.
    #
    # @param controller - VM-USB controller object.
    #
    method _StartSampling {controller} {
	$controller vmeWrite32 [expr $options(-base) + $VMEStart] \
	    $setupAmod 1;	# Key register could write anything.
    }


    ##

    #-----------------------------------------------------------------------
    #
    # Configuration setters:

    ##
    # configureClockSource:
    #
    #  Configure the clock source selector.  Array indices of clockSourceValues
    #  are the set of valid value strings.
    #
    # @param option - name of the option to set.
    # @param value  - Proposed new value.
    #
    method configureClockSource {option value} {
	::VMUSBDriverSupport::validEnum $value [array names clockSourceValues]
	set options($option) $value
    }
    ##
    # configureBool:
    #
    # Common function to validate and configure a boolean parameter.
    #
    # @param option - name of the option to set.
    # @param value  - Proposed new value.
    #
    method configureBool {option value} {
	::VMUSBDriverSupport::validBool $value
	set options($option) $value
    }
    ##
    # configureInt
    #   
    # Common code to configure an integer that has no limits
    #
    # @param option - name of the option to set.
    # @param value  - Proposed new value.
    #
    method configureInt {option value} {
	::VMUSBDriverSupport::validInt $value
	set options($option) $value
    }
    ##
    # configureSampleSize:
    #   Configure the number of samples of data to take.
    #   This is an enumerated parameter whose legal values are the
    #   array indices of sampleSizeValues
    #
    # @param option - name of the option to set.
    # @param value  - Proposed new value.
    #
    method configureSampleSize {option value} {
	::VMUSBDriverSupport::validEnum $value [array names sampleSizeValues]
	set options($option) $value
    }

    ##
    # configureThresholds:
    #   configure the channel threshold values.  These are an 8 element list of values.
    #   in the range 0 - 3fff (SIS 3301 limits are used).
    #
    # @param option - name of the option to set.
    # @param value  - Proposed new value.
    #
    method configureThresholds {option value} {
	::VMUSBDriverSupport::validIntList $value 8 8 0 0x3ff
	set options($option) $value
    }
				     
    ##
    #  configureGroupsRead:
    #    Configures the set of groups to read.  This is a 4 element list of bools
    # 
    # @param option - name of the option to set.
    # @param value  - Proposed new value.
    #
    method configureGroupsRead {option value} {
	::VMUSBDriverSupport::validBoolList $value 4 4
	set options($option) $value
    }
			    
}
