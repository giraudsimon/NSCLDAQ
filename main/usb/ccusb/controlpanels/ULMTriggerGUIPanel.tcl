#===================================================================
# class ATrigger2367
#===================================================================

package provide ULMTriggerGUI 1.0

package require ccusbcamac
package require Itcl
package require ScalerClient 
package require TclServer 
package require portAllocator 

set RunState "*Unknown*"

itcl::class ATrigger2367 {
  private variable reg            ;#< the device registry to use.
  private variable sclClientPID  -1 ;#< pid of the sclclient  
  private variable server         ;#< tclserver 

	private variable parameters
	private variable Fpara
	private variable Apara
	private variable wtrigger
	
    constructor {b c host port module slot {ring ""}} {

    if {[catch {set reg [Connect $b $c $host $port $module $slot]} msg]} {
      set msg "Slow-controls server at $host:$port is either not listening or accepting connections. "
      append msg "Make sure that CCUSBReadout is running and the host and port are "
      append msg "correct for it."
      tk_messageBox -icon error -message $msg
      exit
    }
   
#    set sclClientPID -1
    set port [startServer ]
    if {$ring eq ""} {
	set ring $::tcl_platform(user);    #More portable/reliable than the env.
    }
    puts "TclServer started on port = $port $ring"
    set sclClientPID [startScalerClient localhost $port \
                                        $host $ring]
    # if we made it here, then the connection was made. 

		set parameters [list PCDelay PCWidth SCDelay SCWidth PSDelay CCWidth SSDelay \
                          Bypasses PDFactor SDFactor TriggerBox ClockBox Inspect1 \
                          Inspect2 Inspect3 Inspect4 ADCWidth QDCWidth TDCWidth \
                          CoincidenceWidth]
		set Fpara [list 16 16 16 16 16 16 16 16 16 16 16 16 17 17 17 17 18 18 18 18]
		set Apara [list 0 1 2 3 4 5 6 7 8 9 10 12 0 1 2 3 0 1 2 3]
		DeclareVariables
	}
	
	destructor {
    OnExit 
  }
	
  public method OnExit {} {
    if {$sclClientPID != -1} {
      puts $sclClientPID
      exec kill $sclClientPID
      set sclClientPID -1
    }

    destroy $wtrigger
  }
  private method Connect {b c host port module slot}
	public method wienercfsa {m f args}
	public method SetupGUI {parent filename}
	public method FreeGUI {}
	public method ReadData {f a}
	public method WriteData {f a d}
	public method DeclareVariables {}
	public method DrawTrigger {c title}
	public method BindWires {c}
	public method DynamicColor {c w}
	public method DrawAndGate {c x y}
	public method DrawInput {c x y t}
	public method DrawOutput {c x y t}
	public method DrawLatch {c x y}
	public method DrawLatches {c x y}
	public method DrawGateGenerator {c x y wid var}
	public method DrawDelayGenerator {c x y did dvar bid bvar}
	public method DrawGateDelayGenerator {c x y did dvar wid wvar bid bvar}
	public method DrawDownscaler {c x y fid fvar}
	public method DrawCheckButton {c x y id var label color}
	public method Toggle {var}
	public method DrawEntryTrigger {c x y id var label color}
	public method IncrementTrigger {var}
	public method SendData {var}
	public method DecrementTrigger {var}
	public method StopRepeatTrigger {}
	public method ConnectWire {c w ch}
	public method UpdateWires {c}
	public method ReadStatus {}
	public method WriteConfigFile {}
	public method ReadConfigFile {}
	public method LockGUI {lockStatus}
	public method DrawButtons {}


  private method startServer {}
  private method OnRunStateChange {name1 name2 op}
  private method SynchronizeState {}
}


## @brief Connect to the server or bust
#
# @returns tcl list
# @retval {cccusb::CCCUSBRemote_handle slot}
itcl::body ATrigger2367::Connect {b c host port module slot} {
  # register connection information
  ccusbcamac::cdconn $b $c $host $port $module

  # check if we are online
  if {[ccusbcamac::isOnline $b $c]} {
    return [ccusbcamac::cdreg $b $c $slot]
  } else {
    set msg "ATrigger2367::Connect slow-controls server is not online "
    append msg "at $host:$port"
    return -code error $msg 
  }
}

# This method replaces the original wienercfsa
itcl::body ATrigger2367::wienercfsa {m f args} {
  if {[llength $args] == 1} {
    set res [::ccusbcamac::cfsa $reg $f $args]
    return $res
  } elseif {[llength $args] == 2} {
    set res [::ccusbcamac::cfsa $reg $f [lindex $args 0] [lindex $args 1]]
    return $res
  } else {
    return -code error "ATrigger2367::wienercfsa provided wrong number of arguments"
  }
}

# The following methods are imported from the original trigger.tcl code
itcl::body ATrigger2367::SetupGUI {parent filename} {
  global RunState firstupdate configfilename
  set configfilename $filename
 
  set top $parent
  if {$parent eq "."} {
    set top ""
  }
	set wtrigger $top.canvas
#	CheckModuleTrigger
#	DeclareVariables
	canvas $wtrigger -width 1150 -height 370
	pack $wtrigger -expand 1 -fill both
	DrawTrigger $wtrigger "S800 Trigger"
	BindWires $wtrigger
#	DrawRunInfo
	DrawButtons
# Lock GUI by default and wait 1 second in case a run is in progress
	LockGUI 1

  trace add variable ::RunState write [itcl::code "$this OnRunStateChange"]
  set callback [itcl::code "$this SynchronizeState"]
	set firstupdate [after 2000 $callback]
}

itcl::body ATrigger2367::FreeGUI {} {
	LockGUI 0
	ReadStatus
}

itcl::body ATrigger2367::ReadData {f a} {
	global module
	set data [lindex [wienercfsa $module $f $a] 0]
	return $data
}

itcl::body ATrigger2367::WriteData {f a d} {
	global module
	wienercfsa $module [expr $f+16] $a $d
	set check [ReadData $f $a]
	return $check
}

itcl::body ATrigger2367::DeclareVariables {} {
	global variable increment lowerlimit upperlimit function address bit locked module

	set locked 0

	set variable(Bypasses) 0
	set function(Bypasses) 0
	set address(Bypasses) 7
	set variable(TriggerBox) 0
	set function(TriggerBox) 0
	set address(TriggerBox) 10
#	set variable(BusyBox) 0
#	set function(BusyBox) 0
#	set address(BusyBox) 13
	set variable(ClockBox) 0
	set function(ClockBox) 0
        set address(ClockBox) 12

    set variable(SyncBox) 0
    set function(SyncBox) 0
    set address(SyncBox)  13
    
	set variable(Inspect1) 0
	set variable(Inspect2) 0
	set variable(Inspect3) 0
	set variable(Inspect4) 0
	set function(Inspect1) 1
	set function(Inspect2) 1
	set function(Inspect3) 1
	set function(Inspect4) 1
	set address(Inspect1) 0
	set address(Inspect2) 1
	set address(Inspect3) 2
	set address(Inspect4) 3
	
	# Primary trigger coincidence Gate & Delay
	set variable(PCDelay) 0
	set increment(PCDelay) 25
	set lowerlimit(PCDelay) 25
	set upperlimit(PCDelay) [expr 255*25]
	set function(PCDelay) 0
	set address(PCDelay) 0
	set variable(PCWidth) 0
	set increment(PCWidth) 25
	set lowerlimit(PCWidth) 25
	set upperlimit(PCWidth) [expr 255*25]
	set function(PCWidth) 0
	set address(PCWidth) 1
	set variable(PCBypass) 0
	set bit(PCBypass) 0
	
	# Primary trigger singles Delay
	set variable(PSDelay) 0
	set increment(PSDelay) 25
	set lowerlimit(PSDelay) 25
	set upperlimit(PSDelay) [expr 255*25]
	set function(PSDelay) 0
	set address(PSDelay) 4
	set variable(PSBypass) 0
	set bit(PSBypass) 2
	
	# Coincidence Gate
	set variable(CCWidth) 0
	set increment(CCWidth) 25
	set lowerlimit(CCWidth) 25
	set upperlimit(CCWidth) [expr 255*25]
	set function(CCWidth) 0
	set address(CCWidth) 5

	# Primary trigger singles Downscaler
	set variable(PDFactor) 0
	set increment(PDFactor) 1
	set lowerlimit(PDFactor) 1
	set upperlimit(PDFactor) 1023
	set function(PDFactor) 0
	set address(PDFactor) 8
	
	# Secondary trigger coincidence Gate & Delay
	set variable(SCDelay) 0
	set increment(SCDelay) 25
	set lowerlimit(SCDelay) 25
	set upperlimit(SCDelay) [expr 255*25]
	set function(SCDelay) 0
	set address(SCDelay) 2
	set variable(SCWidth) 0
	set increment(SCWidth) 25
	set lowerlimit(SCWidth) 25
	set upperlimit(SCWidth) [expr 255*25]
	set function(SCWidth) 0
	set address(SCWidth) 3
	set variable(SCBypass) 0
	set bit(SCBypass) 1

	# Secondary trigger singles Delay
	set variable(SSDelay) 0
	set increment(SSDelay) 25
	set lowerlimit(SSDelay) 25
	set upperlimit(SSDelay) [expr 255*25]
	set function(SSDelay) 0
	set address(SSDelay) 6
	set variable(SSBypass) 0
	set bit(SSBypass) 3

	# Secondary trigger singles Downscaler
	set variable(SDFactor) 0
	set increment(SDFactor) 1
	set lowerlimit(SDFactor) 1
	set upperlimit(SDFactor) 1023
	set function(SDFactor) 0
	set address(SDFactor) 9

	# Trigger box switches
	set variable(S800Trigger) 0
	set bit(S800Trigger) 0
	set variable(CoincidenceTrigger) 0
	set bit(CoincidenceTrigger) 1
	set variable(External1Trigger) 0
	set bit(External1Trigger) 2
	set variable(External2Trigger) 0
	set bit(External2Trigger) 3
	set variable(SecondaryTrigger) 0
	set bit(SecondaryTrigger) 4
	
	# Busy
#	set variable(Enable1Busy) 0
#	set variable(Enable2Busy) 0
#	set variable(Enable3Busy) 0
#	set variable(Enable4Busy) 0
#	set bit(Enable1Busy) 0
#	set bit(Enable2Busy) 1
#	set bit(Enable3Busy) 2
#	set bit(Enable4Busy) 3
#	set variable(BusyBypass) 0
#	set bit(BusyBypass) 4

	# Time stamp clock
	set variable(SelectClock) 0
	set bit(SelectClock) 0

    # External sync

    set variable(SelectExtSync) 0
    set bit(SelectExtSync) 0
    
	# ADC Gate width
	set variable(ADCWidth) 0
	set increment(ADCWidth) 100
	set lowerlimit(ADCWidth) 100
	set upperlimit(ADCWidth) [expr 255*100]
	set function(ADCWidth) 2
	set address(ADCWidth) 0

	# QDC Gate width
	set variable(QDCWidth) 0
	set increment(QDCWidth) 25
	set lowerlimit(QDCWidth) 25
	set upperlimit(QDCWidth) [expr 255*25]
	set function(QDCWidth) 2
	set address(QDCWidth) 1

	# TDC Start width
	set variable(TDCWidth) 0
	set increment(TDCWidth) 25
	set lowerlimit(TDCWidth) 25
	set upperlimit(TDCWidth) [expr 255*25]
	set function(TDCWidth) 2
	set address(TDCWidth) 2

	# Coincidence register Gate width
	set variable(CoincidenceWidth) 0
	set increment(CoincidenceWidth) 25
	set lowerlimit(CoincidenceWidth) 25
	set upperlimit(CoincidenceWidth) [expr 255*25]
	set function(CoincidenceWidth) 2
	set address(CoincidenceWidth) 3

# Added 02/2012 for compatibility
	set module dummy
}

itcl::body ATrigger2367::DrawTrigger {c title} {
	set xpos [expr [$c cget -width] / 2]
	set bigFont [font create -family geneva -size 24]
	$c create text $xpos 20 -text $title -font $bigFont
	set lwidth 2

	DrawInput $c 75 150 "S800"

	DrawDelayGenerator $c 315 100 psdelay PSDelay psbypass PSBypass
	$c create line 85 150 125 150 -width $lwidth -tags wire1

	DrawGateDelayGenerator $c 125 150 pcdelay PCDelay pcwidth PCWidth pcbypass PCBypass
	$c create line 100 150 100 100 315 100 -width $lwidth -tags wire1

	DrawInput $c 75 250 "Secondary"

	DrawGateDelayGenerator $c 125 250 scdelay SCDelay scwidth SCWidth scbypass SCBypass
	$c create line 85 250 125 250 -width $lwidth -tags wire4

	DrawAndGate $c 315 200
	$c create line 275 150 300 150 300 185 315 185 -width $lwidth -tags wire2
	$c create line 275 250 300 250 300 215 315 215 -width $lwidth -tags wire3

	DrawGateGenerator $c 390 200 ccwidth CCWidth
	$c create line 375 200 390 200 -width $lwidth -tags wire23

	DrawAndGate $c 530 185
	$c create line 510 200 530 200 -width $lwidth -tags wire6
	$c create line 435 100 475 100 475 170 530 170 -width $lwidth -tags wire5

	DrawDownscaler $c 500 100 pdfactor PDFactor
	$c create line 475 100 500 100 -width $lwidth -tags wire5
	$c create line 600 100 650 100 -width $lwidth -tags wire8
	$c create line 590 185 600 185 600 150 650 150 -width $lwidth -tags wire9

	DrawDelayGenerator $c 315 300 ssdelay SSDelay ssbypass SSBypass
	$c create line 100 250 100 300 315 300 -width $lwidth -tags wire4

	DrawDownscaler $c 500 300 sdfactor SDFactor
	$c create line 435 300 500 300 -width $lwidth -tags wire7
	$c create line 600 300 650 300 -width $lwidth -tags wire12

	DrawInput $c 580 230 "External1"

	DrawInput $c 580 250 "External2"
	$c create line 590 230 600 230 600 200 650 200 -width $lwidth -tags wire10
	$c create line 590 250 650 250 -width $lwidth -tags wire11

	# Draw trigger box	
	set color gray
	$c create rectangle 650 70 755 330 -width 3 -fill $color
	$c create text 700 80 -text "Trigger Box"
	DrawCheckButton $c 655 100 s800single S800Trigger "S800" $color
	DrawCheckButton $c 655 150 coincidence CoincidenceTrigger "Coincidence" $color
	DrawCheckButton $c 655 200 external1 External1Trigger "External1" $color
	DrawCheckButton $c 655 250 external2 External2Trigger "External2" $color
	DrawCheckButton $c 655 300 secondary SecondaryTrigger "Secondary" $color
	

#	DrawInput $c 800 350 "Clear Busy"
	DrawInput $c 850 315 "S800 source\ndelayed (2us)"
	DrawAndGate $c 920 300
	$c create oval 915 282 920 287
	$c create line 860 315 920 315 -width $lwidth -tags wire24
	$c create line 980 300 1070 300 -width $lwidth -tags wire25
	$c create line 910 200 910 285 915 285 -width $lwidth -tags wire19

	DrawLatches $c 800 180
    DrawCheckButton $c 500 350 clock SelectClock "External time\nstamp clock" lightgray
    DrawCheckButton $c 620 350 sync SelectExtSync "External sync\n select" lightgray

	$c create line 755 150 800 150 -width $lwidth -tags wire15
	$c create line 775 150 775 90 800 90 -width $lwidth -tags wire15
	DrawOutput $c 800 90 "Raw Trigger"

#	$c create line 710 350 775 350 775 210 800 210 -width $lwidth -tags wire16
	$c create line 775 170 800 170 -width $lwidth -tags wire26
	$c create line 775 190 800 190 -width $lwidth -tags wire27
	$c create line 775 210 800 210 -width $lwidth -tags wire28
	$c create line 775 230 800 230 -width $lwidth -tags wire29
	$c create line 775 250 800 250 -width $lwidth -tags wire30

	$c create line 900 250 1070 250 -width $lwidth -tags wire19
	DrawOutput $c 1070 250 "Live Trigger/\nRequest\nEvent"
	
	DrawGateGenerator $c 920 100 adcwidth ADCWidth

	DrawGateGenerator $c 920 50 qdcwidth QDCWidth

	DrawGateGenerator $c 920 150 tdcwidth TDCWidth

	DrawGateGenerator $c 920 200 coincidencewidth CoincidenceWidth

	$c create line 910 200 910 165 910 100 920 100 -width $lwidth -tags wire19
	$c create line 300 100 300 50 920 50 -width $lwidth -tags wire1
	$c create line 910 200 910 235 910 200 920 200 -width $lwidth -tags wire19
	$c create line 910 150 920 150 -width $lwidth -tags wire19

	DrawOutput $c 1070 100 "ADC Gate"

	DrawOutput $c 1070 50 "QDC Gate"

	DrawOutput $c 1070 150 "TDC Start"

	DrawOutput $c 1070 200 "Coincidence\nRegister Gate"

	DrawOutput $c 1070 300 "Fast Clear"

	$c create line 1040 50 1070 50 -width $lwidth -tags wire18
	$c create line 1040 100 1070 100 -width $lwidth -tags wire17
	$c create line 1040 150 1070 150 -width $lwidth -tags wire20
	$c create line 1040 200 1070 200 -width $lwidth -tags wire21
}

itcl::body ATrigger2367::BindWires {c} {
	global color inspect nwires
	set nwires 30
	set color(0) black
	set color(1) orange
	set color(2) green
	set color(3) red
	set color(4) blue
	for {set i 1} {$i <= $nwires} {incr i} {
		set inspect(wire$i) 0
		$c bind wire$i <Enter> "$c itemconfigure wire$i -fill white"
		$c bind wire$i <Leave> "$this DynamicColor $c $i"
#		$c bind wire$i <ButtonPress> "InvokeInspectMenu $c $i %X %Y"
		$c bind wire$i <ButtonPress> "tk_popup $c.menuwire$i %X %Y"
		menu $c.menuwire$i -tearoff False
		$c.menuwire$i add radiobutton -label Disconnect -value 0 -variable inspect(wire$i) -command "$this ConnectWire $c $i 0"
		$c.menuwire$i add radiobutton -label Inspect1 -value 1 -variable inspect(wire$i) -command "$this ConnectWire $c $i 1"
		$c.menuwire$i add radiobutton -label Inspect2 -value 2 -variable inspect(wire$i) -command "$this ConnectWire $c $i 2"
		$c.menuwire$i add radiobutton -label Inspect3 -value 3 -variable inspect(wire$i) -command "$this ConnectWire $c $i 3"
		$c.menuwire$i add radiobutton -label Inspect4 -value 4 -variable inspect(wire$i) -command "$this ConnectWire $c $i 4"
	}
}

itcl::body ATrigger2367::DynamicColor {c w} {
	global color inspect
	$c itemconfigure wire$w -fill $color($inspect(wire$w))
}

itcl::body ATrigger2367::DrawAndGate {c x y} {
	set w 3
	$c create line \
	[expr $x+35] [expr $y+25] \
	[expr $x] [expr $y+25] \
	[expr $x] [expr $y-25] \
	[expr $x+35] [expr $y-25] -width $w
	$c create arc \
	[expr $x+10] [expr $y+25] [expr $x+60] [expr $y-25] \
	-start 270 -extent 180 -style arc -width $w
	$c create text [expr $x+25] $y -text "AND\nGATE" -justify center
}

itcl::body ATrigger2367::DrawInput {c x y t} {
	$c create text $x $y -text $t -anchor e -justify center
	$c create line $x [expr $y-5] [expr $x+10] $y $x [expr $y+5]
}

itcl::body ATrigger2367::DrawOutput {c x y t} {
	$c create text $x $y -text $t -anchor w -justify center
	$c create line [expr $x-10] [expr $y-5] $x $y [expr $x-10] [expr $y+5]
}

itcl::body ATrigger2367::DrawLatch {c x y} {
	set color white
	$c create rectangle $x [expr $y+23] [expr $x+60] [expr $y-20] -width 3 -fill $color
	$c create text [expr $x+30] [expr $y-10] -text "Latch"
	$c create text [expr $x+5] $y -text "Start" -anchor w
	$c create text [expr $x+5] [expr $y+10] -text "Stop" -anchor w
}

itcl::body ATrigger2367::DrawLatches {c x y} {
	set color white
	$c create rectangle $x [expr $y+83] [expr $x+100] [expr $y-60] -width 3 -fill $color
	$c create text [expr $x+45] [expr $y-50] -text "Busy"
	$c create text [expr $x+5] [expr $y-30] -text "Start" -anchor w
	$c create text [expr $x+5] [expr $y-10] -text "Busy1" -anchor w
	$c create text [expr $x+5] [expr $y+10] -text "Busy2" -anchor w
	$c create text [expr $x+5] [expr $y+30] -text "Busy3" -anchor w
	$c create text [expr $x+5] [expr $y+50] -text "Busy4" -anchor w
#	$c create text [expr $x+50] [expr $y-30] -text "Enable" -anchor w
	$c create text [expr $x+5] [expr $y+70] -text "Vetoes" -anchor w
#	DrawCheckButton $c [expr $x+50] [expr $y-10] enable1 Enable1Busy "L1" $color
#	DrawCheckButton $c [expr $x+50] [expr $y+10] enable2 Enable2Busy "L2" $color
#	DrawCheckButton $c [expr $x+50] [expr $y+30] enable3 Enable3Busy "L3" $color
#	DrawCheckButton $c [expr $x+50] [expr $y+50] enable4 Enable4Busy "L4" $color
}

itcl::body ATrigger2367::DrawGateGenerator {c x y wid var} {
	set color yellow
	$c create rectangle $x [expr $y+23] [expr $x+120] [expr $y-20] -width 3 -fill $color
	$c create text [expr $x+60] [expr $y-10] -text "Gate Generator"
	DrawEntryTrigger $c [expr $x+60] [expr $y+10] $wid $var "width (ns)" $color
}

itcl::body ATrigger2367::DrawDelayGenerator {c x y did dvar bid bvar} {
	set color pink
	$c create rectangle $x [expr $y+33] [expr $x+120] [expr $y-30] -width 3 -fill $color
	$c create text [expr $x+60] [expr $y-20] -text "Delay Generator"
	DrawEntryTrigger $c [expr $x+60] [expr $y+0] $did $dvar "delay (ns)" $color
	DrawCheckButton $c [expr $x+5] [expr $y+20] $bid $bvar "bypass" $color
}

itcl::body ATrigger2367::DrawGateDelayGenerator {c x y did dvar wid wvar bid bvar} {
	set color lightgreen
	$c create rectangle $x [expr $y+43] [expr $x+150] [expr $y-40] -width 3 -fill $color
	$c create text [expr $x+75] [expr $y-30] -text "Gate & Delay Generator"
	DrawEntryTrigger $c [expr $x+75] [expr $y-10] $did $dvar "delay (ns)" $color
	DrawEntryTrigger $c [expr $x+75] [expr $y+10] $wid $wvar "width (ns)" $color
	DrawCheckButton $c [expr $x+5] [expr $y+30] $bid $bvar "bypass" $color
}

itcl::body ATrigger2367::DrawDownscaler {c x y fid fvar} {
	set color lightblue
	$c create rectangle $x [expr $y+23] [expr $x+100] [expr $y-20] -width 3 -fill $color
	$c create text [expr $x+50] [expr $y-10] -text "Downscaler"
	DrawEntryTrigger $c [expr $x+50] [expr $y+10] $fid $fvar "factor" $color
}

itcl::body ATrigger2367::DrawCheckButton {c x y id var label color} {
	global variable
	checkbutton $c.$id -variable variable($var) -text $label -command "$this Toggle $var" -background $color
	$c create window $x $y -window $c.$id -anchor w
}

itcl::body ATrigger2367::Toggle {var} {
	global variable bit module function address
	if {[string match *Bypass $var] == 1} {
		set mask [expr 1<<$bit($var)]
		if {$variable($var) == 1} {
			set data [expr $variable(Bypasses) | $mask]
		} else {
			set data [expr $variable(Bypasses) & ~$mask]
		}
		wienercfsa $module [expr $function(Bypasses)+16] $address(Bypasses) $data
		set data [lindex [wienercfsa $module $function(Bypasses) $address(Bypasses)] 0]
		set variable(Bypasses) $data
		set variable($var) [expr ($data & $mask)>>$bit($var)]
	}
	if {[string match *Trigger $var] == 1} {
		set mask [expr 1<<$bit($var)]
		if {$variable($var) == 1} {
			set data [expr $variable(TriggerBox) | $mask]
		} else {
			set data [expr $variable(TriggerBox) & ~$mask]
		}
		wienercfsa $module [expr $function(TriggerBox)+16] $address(TriggerBox) $data
		set data [lindex [wienercfsa $module $function(TriggerBox) $address(TriggerBox)] 0]
		set variable(TriggerBox) $data
		set variable($var) [expr ($data & $mask)>>$bit($var)]
	}
#	if {[string match *Busy $var] == 1} {
#		set mask [expr 1<<$bit($var)]
#		if {$variable($var) == 1} {
#			set data [expr $variable(BusyBox) | $mask]
#		} else {
#			set data [expr $variable(BusyBox) & ~$mask]
#		}
#		wienercfsa $module [expr $function(BusyBox)+16] $address(BusyBox) $data
#		set data [lindex [wienercfsa $module $function(BusyBox) $address(BusyBox)] 0]
#		set variable(BusyBox) $data
#		set variable($var) [expr ($data & $mask)>>$bit($var)]
#	}
	if {[string match *Clock $var] == 1} {
		set mask [expr 1<<$bit($var)]
		if {$variable($var) == 1} {
			set data [expr $variable(ClockBox) | $mask]
		} else {
			set data [expr $variable(ClockBox) & ~$mask]
		}
	    wienercfsa $module [expr $function(ClockBox)+16] \
		    $address(ClockBox) $data
	    set data [lindex [wienercfsa $module $function(ClockBox) \
				  $address(ClockBox)] 0]
		set variable(ClockBox) $data
		set variable($var) [expr ($data & $mask)>>$bit($var)]
	}
    	if {[string match *Sync $var] == 1} {
		set mask [expr 1<<$bit($var)]
		if {$variable($var) == 1} {
			set data [expr $variable(SyncBox) | $mask]
		} else {
			set data [expr $variable(SyncBox) & ~$mask]
		}
	    wienercfsa $module [expr $function(SyncBox)+16] \
		    $address(SyncBox) $data
	    set data [lindex [wienercfsa $module $function(SyncBox) \
				  $address(SyncBox)] 0]
		set variable(SyncBox) $data
		set variable($var) [expr ($data & $mask)>>$bit($var)]
	}
}

itcl::body ATrigger2367::DrawEntryTrigger {c x y id var label color} {
	global variable
	$c create text $x $y -text $label -anchor e
	label $c.$id -textvariable variable($var) -width 4 -background $color
	$c create window $x $y -window $c.$id -anchor w
	set up [$c create polygon [expr $x+35] [expr $y-2] \
	[expr $x+45] [expr $y-2] [expr $x+40] [expr $y-10] -fill white -outline black]
	set down [$c create polygon [expr $x+35] [expr $y+2] \
	[expr $x+45] [expr $y+2] [expr $x+40] [expr $y+10] -fill white -outline black]
	$c bind $up <Enter> "$c itemconfigure $up -fill black"
	$c bind $up <Leave> "$c itemconfigure $up -fill white"
	$c bind $up <Button-1> "$this IncrementTrigger $var"
	$c bind $up <ButtonRelease> "$this StopRepeatTrigger"
	$c bind $down <Enter> "$c itemconfigure $down -fill black"
	$c bind $down <Leave> "$c itemconfigure $down -fill white"
	$c bind $down <Button-1> "$this DecrementTrigger $var"
	$c bind $down <ButtonRelease> "$this StopRepeatTrigger"
	return $c.$id
}

itcl::body ATrigger2367::IncrementTrigger {var} {
	global variable increment upperlimit function address
	global firstClick repeatId locked
	if {[info exist firstClick] == 0} {
		set firstClick 1
	}
	if {$firstClick == 1} {
		set repeatId [after 500 $this IncrementTrigger $var]
		set firstClick 0
	} else {
		set repeatId [after 50 $this IncrementTrigger $var]
	}
	if {$locked == 1} {
		return
	}
	set variable($var) [expr $variable($var) + $increment($var)]
	if {$variable($var) > $upperlimit($var)} {
		set variable($var) $upperlimit($var)
	}
	SendData $var
}

itcl::body ATrigger2367::SendData {var} {
	global variable increment function address
	# Send data to module
	if {[info exist increment($var)] == 1} {
		set inc $increment($var)
	} else {
		set inc 1
	}
	set data [expr $variable($var) / $inc]
	set data [WriteData $function($var) $address($var) $data]
	set variable($var) [expr $data * $inc]
}

itcl::body ATrigger2367::DecrementTrigger {var} {
	global variable increment lowerlimit function address
	global firstClick repeatId locked
	if {[info exist firstClick] == 0} {
		set firstClick 1
	}
	if {$firstClick == 1} {
		set repeatId [after 500 $this DecrementTrigger $var]
		set firstClick 0
	} else {
		set repeatId [after 50 $this DecrementTrigger $var]
	}
	if {$locked == 1} {
		return
	}
	set variable($var) [expr $variable($var) - $increment($var)]
	if {$variable($var) < $lowerlimit($var)} {
		set variable($var) $lowerlimit($var)
	}
	SendData $var
}

itcl::body ATrigger2367::StopRepeatTrigger {} {
	global firstClick repeatId
	after cancel $repeatId
	set firstClick 1
}

itcl::body ATrigger2367::ConnectWire {c w ch} {
	global variable function address module
# if channel=0 that means we want to disconnect the wire
	if {$ch == 0} {
		for {set channel 1} {$channel <= 4} {incr channel} {
			if {$variable(Inspect$channel) == $w} {
				set variable(Inspect$channel) 0
				wienercfsa $module [expr $function(Inspect$channel)+16] $address(Inspect$channel) 0				
			}
		}
# otherwise we connect it to the channel
	} else {
		set variable(Inspect$ch) $w
		wienercfsa $module [expr $function(Inspect$ch)+16] $address(Inspect$ch) $w
	}
	UpdateWires $c
}

itcl::body ATrigger2367::UpdateWires {c} {
	global nwires inspect variable color
# loop on wires to update wires
	for {set w 1} {$w <= $nwires} {incr w} {
# first reset all wires disconnected
		set inspect(wire$w) 0
		DynamicColor $c $w
# then find which are connected to which channel
		for {set ch 1} {$ch <= 4} {incr ch} {
			if {$w == $variable(Inspect$ch)} {
				set inspect(wire$w) $ch
				DynamicColor $c $w
			}
		}
	}
# loop on channels to update labels
	for {set ch 1} {$ch <= 4} {incr ch} {
# first delete all labels
		$c delete inspect$ch
# then display the active ones
		if {$variable(Inspect$ch) != 0} {
			set y [expr 300+$ch*15]
			$c create line 10 $y 30 $y -width 2 -fill $color($ch) -tags inspect$ch
			$c create text 40 $y -text "Channel $ch: wire$variable(Inspect$ch)" -anchor w -tags inspect$ch
		}
	}
}

itcl::body ATrigger2367::ReadStatus {} {
	global module function address
	global variable bit inspect increment
	# Read all available data

	foreach v [array names address] {
		set variable($v) [lindex [wienercfsa $module $function($v) $address($v)] 0]
	}
	# Scale to appropriate units
	foreach v [array names increment] {
		set variable($v) [expr $variable($v) * $increment($v)]
	}
	# Decode bit registers
	foreach v [array names variable] {
		if {[string match *Bypass $v] == 1} {
			set mask [expr 1<<$bit($v)]
			set variable($v) [expr ($variable(Bypasses) & $mask)>>$bit($v)]
		}
		if {[string match *Trigger $v] == 1} {
			set mask [expr 1<<$bit($v)]
			set variable($v) [expr ($variable(TriggerBox) & $mask)>>$bit($v)]
		}
#		if {[string match *Busy $v] == 1} {
#			set mask [expr 1<<$bit($v)]
#			set variable($v) [expr ($variable(BusyBox) & $mask)>>$bit($v)]
#		}
		if {[string match *Clock $v] == 1} {
			set mask [expr 1<<$bit($v)]
			set variable($v) [expr ($variable(ClockBox) & $mask)>>$bit($v)]
		}
	    if {[string match *Sync $v] == 1} {
		set mask [expr 1<<$bit($v)]
		set variable($v) [expr ($variable(SyncBox) & $mask)>>$bit($v)]
	    }
	}
	UpdateWires $wtrigger
}

itcl::body ATrigger2367::WriteConfigFile {} {
	global variable address configfilename increment
	if { ! [info exist configfilename] } {
		set configfilename [tk_getSaveFile \
      -title "Please select the Trigger configuration file" -filetypes {{"Tcl file" {.tcl}}}]
	} elseif { $configfilename eq {} } {
		set configfilename [tk_getSaveFile \
      -title "Please select the Trigger configuration file" -filetypes {{"Tcl file" {.tcl}}}]
  }
  if {$configfilename eq {}} {
    # there is nothing to do because we have no file...
    return
  }
	set ch [open $configfilename w]
	puts $ch "set TRIGGER(configFileName) /user/s800/server/fpga/usbtrig.bit"
	puts $ch "set TRIGGER(configuration) 5800"
	foreach v [array names address] {
		if {[info exist increment($v)] == 1} {
			puts $ch "set TRIGGER($v) [expr $variable($v)/$increment($v)]"
		} else {
			puts $ch "set TRIGGER($v) $variable($v)"
		}
	}
	close $ch
}

itcl::body ATrigger2367::ReadConfigFile {} {
	global variable address increment function bit inspect configfilename locked
	if { ! [info exist configfilename] } { 
		set configfilename [tk_getOpenFile \
	                        	-title "Please select the Trigger configuration file" \
                            -filetypes {{"Tcl file" {.tcl}}}]
	} elseif {$configfilename eq {}} {
		set configfilename [tk_getOpenFile \
	                        	-title "Please select the Trigger configuration file" \
                            -filetypes {{"Tcl file" {.tcl}}}]
  } elseif {! [file exists $configfilename] } {
    set msg "Config file $configfilename does not yet exist. "
    append msg "Maybe you need to write the state to file first"
    tk_messageBox -icon error -message $msg
    return
  }
  if {$configfilename eq {}} {
    # there is nothing to do because we have no file...
    return
  }
	set ch [open $configfilename r]
	gets $ch str
	gets $ch str
	foreach v [array names address] {
		gets $ch str
		set fmt "set TRIGGER($v) %d"
		scan $str $fmt variable($v)
		if {[info exist increment($v)] == 1} {
			set variable($v) [expr $variable($v)*$increment($v)]
		}
		if {$locked == 0} {
			SendData $v
		}
	}
	close $ch
	# Decode bit registers
	foreach v [array names variable] {
		if {[string match *Bypass $v] == 1} {
			set mask [expr 1<<$bit($v)]
			set variable($v) [expr ($variable(Bypasses) & $mask)>>$bit($v)]
		}
		if {[string match *Trigger $v] == 1} {
			set mask [expr 1<<$bit($v)]
			set variable($v) [expr ($variable(TriggerBox) & $mask)>>$bit($v)]
		}
#		if {[string match *Busy $v] == 1} {
#			set mask [expr 1<<$bit($v)]
#			set variable($v) [expr ($variable(BusyBox) & $mask)>>$bit($v)]
#		}
		if {[string match *Clock $v] == 1} {
			set mask [expr 1<<$bit($v)]
		    set variable($v) [expr ($variable(ClockBox) & $mask)>>$bit($v)]
		}
		if {[string match *Sync $v] == 1} {
			set mask [expr 1<<$bit($v)]
			set variable($v) [expr ($variable(SyncBox) & $mask)>>$bit($v)]
		}		
	    
	}
	UpdateWires $wtrigger
}

itcl::body ATrigger2367::LockGUI {lockStatus} {
	global locked
	set locked $lockStatus
	set c $wtrigger
	if {$locked == 1} {
		foreach w [winfo children $c] {
			if {[string match *menuwire* $w] == 1} {
				for {set i 0} {$i < 5} {incr i} {
					$w entryconfigure $i -state disabled
				}
			} else {
				if {[string match *run* $w] == 0 && [string match *file* $w] == 0\
				&& [string match *ratio* $w] == 0 && [string match *average* $w] == 0\
				&& [string match raw $w] == 0 && [string match live $w] == 0} {
					$w configure -state disabled
				}
			}
		}
		set xpos [expr [$c cget -width] / 2]
		set bigFont [font create -family times -size 24]
		$c create text $xpos 50 -text "GUI Locked!" -font $bigFont -fill red -tag lock
	} else {
		foreach w [winfo children $c] {
			if {[string match *menuwire* $w] == 1} {
				for {set i 0} {$i < 5} {incr i} {
					$w entryconfigure $i -state normal
				}
			} else {
				$w configure -state normal
			}
		}
		$c delete lock
	}
}

itcl::body ATrigger2367::DrawButtons {} {
	button $wtrigger.savefile -text "Save to File" -command "$this WriteConfigFile" -width 16
	button $wtrigger.readfile -text "Read from File" -command "$this ReadConfigFile" -width 16
	button $wtrigger.update -text "Update from Module" -command "$this ReadStatus" -width 16
	$wtrigger create window 100 20 -anchor w -window $wtrigger.savefile
	$wtrigger create window 100 50 -anchor w -window $wtrigger.readfile
	$wtrigger create window 100 80 -anchor w -window $wtrigger.update
	$wtrigger create text 350 20 -text "Save to\nmake Active!" -anchor e -fill red -justify center
}

#####################################################################
####################################################################
#
# Some methods for the TclServer
#

## @brief Method called for connections
#
# Prints message containing connection information
# This also always returns true and therefore will always
# accept a connection request.
#
# @returns boolean
# @retval  true
proc _OnTclServerConnect {fd ip port } {
  gets $fd user
  puts "TclServer connection request $ip:$port by $user"
  return true
}

## @brief Method called when error occurs
#
# Simply prints the error message provided
#
# @return boolean
# @retval true
proc _OnTclServerError {chan command msg } {
  puts "Error when executing \"$command\" : $msg"
  return true
}

## @brief Starts up the TclServer
#
# Allocates a port and then starts the server on it.
# 
# @returns int
# @retval  port that TclServer listens on
itcl::body ATrigger2367::startServer {} {
  ::portAllocator create allocator 
  set port [allocator allocatePort XLM72ScalerGUI]
  set server [TclServer %AUTO% -port $port -onconnect _OnTclServerConnect]
  $server configure -onerror _OnTclServerError
  $server start

  return $port
}

itcl::body ATrigger2367::OnRunStateChange {name1 name2 op} {
  upvar $name1 state
  puts "RunState now : $state"
  if {$state eq "Active"} {
    LockGUI 1
  } else {
    LockGUI 0
  }

}

itcl::body ATrigger2367::SynchronizeState {} {
  puts "RunState now : $::RunState"
  if {$::RunState eq "Active"} {
    LockGUI 1
  } else {
    LockGUI 0
  }

}
#################################################################
##################################################################
##################################################################
##
## ScalerDisplay hooks that we have to define but don't use.

##
proc Update {} {}
proc BeginRun {} {}
proc EndRun {} {}
proc PauseRun {} {}
proc ResumeRun {} {}
proc RunInProgress {} {}
proc UpdateRunTime {src time} {}
proc SourceElapsedTime {time} {}


