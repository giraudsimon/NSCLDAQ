package provide ALevel3XLM72GUI 1.0
package require Itcl
package require AXLM72GenericProxy
package require Tk
package require RunStateObserver


#===================================================================
# class ALevel3XLM72
#===================================================================
itcl::class ALevel3XLM72 {
	inherit AXLM72GenericProxy
	global l3
	global diagnostics

  variable runObserver

	constructor {module host port ring} {
		AXLM72GenericProxy::constructor $module $host $port


	} {
		global l3 diagnostics
# set initial variables
		set l3(module) "select XLM"
		set l3(selA) "select output"
		set l3(selB) "select output"
		set l3(selC) "select output"
		set l3(selD) "select output"
		set l3(varA) 1
		set l3(varB) 2
		set l3(varC) 4
		set l3(varD) 8
		set l3(loadfile) "none selected"
		set l3(bitdir) "~/server/fpga"
	# new bit file for 12011 and 11028
		set l3(bitfile) /user/sweeper/server/fpga/mlsc_timestamp_v3-1.bit
	# line below edited for e11027
	#		set l3(bitfile) $l3(bitdir)/mlsv_timestamp_v2-5-6.bit
		set l3(valuesfile) $l3(bitdir)/xlm_values_level3.dat
		set l3(readflag) false
		set l3(setflag) false
		set l3(state) ""
		
		array set diagnostics {
		    0x0    	""
		    0x1    	"latched_thin" 
		    0x2    	"latched_potscint"
		    0x4    	"latched_mona_trigger"
		    0x8    	"latched_lisa_trigger"
		    0x10   	"latched_mona_valid"
		    0x20   	"latched_lisa_valid"
		    0x40   	"sweeper_busy"
		    0x80  	"mona_busy"
		    0x100  	"lisa_busy" 
		    0x200  	"overlap"
		    0x400 	"system_trigger"
		    0x800 	"gate"
		    0x1000 	"system_clear"
		    0x2000 	"system_busy"
		    0x4000	"late_system_busy"
		    0x8000	"ts_counter_low(0)"
		    0x10000	"raw_thin"
		    0x20000	"raw_pot"
		    0x40000	"raw_mona"
		    0x80000	"raw_lisa"
		    0x100000	"raw_mona_valid"
		    0x200000	"raw_lisa_valid"
		    0x400000	"latched_overlap"
		    0x800000	"latched_overlap_endgate"
		    0x1000000	"or_of_busy"
		    0x2000000	"mona_lisa_trigger_busy"
		    0x4000000	"runstop"
		    0x8000000	"no_overlap_pulse"
		    0x10000000	"watchdog_pulse"
		    0x20000000	"latched_caesar_valid"
		    0x40000000	"latched_caesar_busy"
		    0x80000000	"latched_ds"
		}
		set l3(dssingles) 0
		set l3(triggeroption) 0


    if {$ring ne {}} {
      set runObserver [RunStateObserver %AUTO% -ringurl $ring \
                                                -onbegin [list $this OnBegin] \
                                                -onpause [list $this OnPause] \
                                                -onresume [list $this OnResume] \
                                                -onend [list $this OnEnd]]
      $runObserver attachToRing
    }

	}

	public method SetupGUI {main args}
	public method ControlMenu {loc}
	public method Exit {} {exit}
	public method ReadXLM {}
	public method SetValues {}
	public method Xget {name addr format}
	public method Xget16 {name addr format}
	public method Xset {name addr data} {Write  $addr $data}
	public method Xset16 {name addr data} {Write16  $addr $data}
	public method BG_yellow {wind} {$wind config -bg yellow}
	public method BG_green {wind} {$wind config -bg green}
	public method ReadFile {}
	public method WriteFile {}
	public method AreYouSure {}
	public method RunStop {stop}
	public method ResetTS {}
	public method SetDiagnostics {channel mask}
	public method Changed {argv}
	public method LoadBitfile {}
	public method newbitfile {level}
	public method CheckStatus {}
	public method UpdateState {}
	public method SelectTrigger {}
	public method ReadTrigger {}
	public method LockGUI {}
	public method Children {w state}
	public method FreeGUI {}
  public method OnBegin item
  public method OnEnd item 
  public method OnPause item
  public method OnResume item 

# stack methods

}

	
itcl::body ALevel3XLM72::Xget {name addr format} {
# adding bit masking to make fit with 64 bit system
# {expr [Read var $addr]&0xffffffff}
    switch $format {
        u {
#            $name get -l $addr
            expr [Read $addr]&0xffffffff
        }
        d {
#            format %\#d [$name get -l $addr]
            format %\#d [expr [Read $addr]&0xffffffff]
        }
        x {
#            format %\#x [$name get -l $addr]
            format %\#x [expr [Read  $addr]&0xffffffff]
        }
        default {
#            $name get -l $addr
            expr [Read $addr]&0xffffffff
        }
    }
}

itcl::body ALevel3XLM72::Xget16 {name addr format} {
    switch $format {
        u {
#            $name get -l $addr
            Read16 $addr
        }
        d {
#            format %\#d [$name get -l $addr]
            format %\#d [Read16  $addr]
        }
        x {
#            format %\#x [$name get -l $addr]
            format %\#x [Read16  $addr]
        }
        default {
#            $name get -l $addr
            Read16 $addr
        }
    }
}

itcl::body ALevel3XLM72::CheckStatus {} {
    global l3
    set l3(xlmstatus) "o.k."

#    MapXLM 0 $slot XLM
#    RequestBus XLM X
	AccessBus 0x10000

####!!!!!!!!!!!!!!change this for single slot !!!!!
####!!!!!!!!!!!!FPGA code should be modified for different tag
####!!!!!!!!!!!Need to coordinate this

    set signature [Xget XLM 0x400000 x]
    puts "signature $signature"
#   prevous signature 0x54533131
    if {$signature == "0xdeba2014"} {
	puts "XLM Level3 o.k."
	$l3(page1_status) config -bg green
	$l3(page2_status) config -bg green
	$l3(page3_status) config -bg green
    } else {
	puts "XLM LEVEL 3 not properly loaded"
	set l3(xlmstatus) "error"
	$l3(page1_status) config -bg red
	$l3(page2_status) config -bg red
	$l3(page3_status) config -bg red
    }

#    ReleaseBus XLM
#    UnmapXLM XLM
	ReleaseBus
}
itcl::body ALevel3XLM72::UpdateState {}	{
    global l3	

    if {$l3(stoppedstate) == 0} {
    	set l3(state) "running"
    } else {
    	set l3(state) "stopped"
    }


}
itcl::body ALevel3XLM72::SelectTrigger {}	{
    global l3
    if {$l3(triggeroption) == 0} {
    # Sweeper standalone
    	set l3(mona_lisa_disable) 1
    	set l3(caesar_disable) 1
    	set l3(ds_disable) 1
    	set l3(singlesmode) 1
    } elseif {$l3(triggeroption) == 1} {
    # Sweeper singles plus MoNA-LISA, CAESAR, DS
    	set l3(mona_lisa_disable) 0
    	set l3(caesar_disable) 0
    	set l3(ds_disable) 0
    	set l3(singlesmode) 1
    } elseif {$l3(triggeroption) == 2} {
    # Sweeper coincidence MoNA/LISA
    	set l3(mona_lisa_disable) 0
    	set l3(caesar_disable) 0
    	set l3(ds_disable) 0
    	set l3(singlesmode) 0
    }	
    Changed yes
}
itcl::body ALevel3XLM72::ReadTrigger {}	{
    global l3
    if {$l3(mona_lisa_disable) == 1 && \
	$l3(caesar_disable) == 1 && \
	$l3(ds_disable) == 1 && $l3(singlesmode) == 1} {
    # Sweeper standalone
        set l3(triggeroption) 0
    } elseif {$l3(mona_lisa_disable) == 0 && \
	$l3(caesar_disable) == 0 && \
	$l3(ds_disable) == 0 && $l3(singlesmode) == 1} {
    # Sweeper singles plus MoNA-LISA, CAESAR, DS
    	set l3(triggeroption) 1
    } elseif {$l3(mona_lisa_disable) == 0 && \
	$l3(caesar_disable) == 0 && \
	$l3(ds_disable) == 0 && $l3(singlesmode) == 0} {
    # Sweeper coincidence MoNA/LISA
    	set l3(triggeroption) 2
    } else {
    # one or more systems disabled: undefined trigger option
	set l3(triggeroption) 3
    }
}

itcl::body ALevel3XLM72::SetValues {} {
	global l3
    set l3(setflag) true
    global page1_set
    global page1_writefile
    global page2_set
    global page2_writefile
    global page3_set
    global page3_writefile

    BG_yellow $l3(page3_writefile)
    BG_green  $l3(page3_set)
    BG_yellow $l3(page2_writefile)
    BG_green  $l3(page2_set)
    BG_yellow $l3(page1_writefile)
    BG_green  $l3(page1_set)

    if {$l3(readflag) == true} {
	BG_green $l3(page1_writefile)
        BG_green $l3(page2_writefile)
        BG_green $l3(page3_writefile)
    }

#    MapXLM 0 $slot XLM
#    RequestBus XLM X
	AccessBus 0x10000


#    Xset XLM 0x400004 $vme1
#    Xset XLM 0x400008 $vme2
#  Xset XLM 0x40000c $vme3

#    Xset XLM 0x40000c $l3(coincgate)
#    puts "right after: [Xget XLM  0x40000c d]"

#    Xset XLM 0x400010 $stoppedstate
    Xset XLM 0x400014 $l3(singlesmode)
# set mona_lisa_disable, caesar_disable, and ds_disable:
    Xset XLM 0x400018 $l3(mona_lisa_disable)
    Xset XLM 0x40001c $l3(caesar_disable)
    Xset XLM 0x400020 $l3(ds_disable)
    
    Xset XLM 0x400038 $l3(varA)
    Xset XLM 0x40003c $l3(varB)
    Xset XLM 0x400040 $l3(varC)
    Xset XLM 0x400044 $l3(varD)
    Xset XLM 0x400060 $l3(dssingles)

    Xset XLM 0x40000c $l3(coincgate)

#    puts "$slot values provided ( gate vme1 vme2 vme3):"
    puts [Xget XLM  0x40000c d]
#	    puts [Xget XLM  0x400004 d]
#	    puts [Xget XLM  0x400008 d]
#	    puts [Xget XLM  0x40000c d]

    puts "set stoppedstate, singlesmode, dssingles"
    puts [Xget XLM  0x400010 d]
    puts [Xget XLM  0x400014 d]
    puts [Xget XLM  0x400060 d]

    puts "set mona_lisa_disable, caesar_disable, ds_disable"
    puts [Xget XLM  0x400018 d]
    puts [Xget XLM  0x40001c d]
    puts [Xget XLM  0x400020 d]

    puts "diagnostic mask values:"
    puts [Xget XLM  0x400038 d]
    puts [Xget XLM  0x40003c d]
    puts [Xget XLM  0x400040 d]
    puts [Xget XLM  0x400044 d]

	
#    ReleaseBus XLM
#    UnmapXLM XLM
	ReleaseBus
}

itcl::body ALevel3XLM72::ReadXLM {} {
	global l3 diagnostics

    set l3(setflag) true

    BG_yellow $l3(page3_writefile)
    BG_green  $l3(page3_set)
    BG_yellow $l3(page2_writefile)
    BG_green  $l3(page2_set)
    BG_yellow $l3(page1_writefile)
    BG_green  $l3(page1_set)
    
#    MapXLM 0 $slot XLM
#    RequestBus XLM X
	AccessBus 0x10000

#	    set vme1      [Xget XLM 0x400004 d]
#	    set vme2      [Xget XLM  0x400008 d]
#	    set vme3      [Xget XLM  0x40000c d]
    set l3(coincgate) [Xget XLM  0x40000c d]
    
    set l3(stoppedstate) [Xget XLM  0x400010 d]
    UpdateState
    set l3(singlesmode)        [Xget XLM  0x400014 d]
    set l3(mona_lisa_disable)  [Xget XLM  0x400018 d]
    set l3(caesar_disable)     [Xget XLM  0x40001c d]
    set l3(ds_disable)         [Xget XLM  0x400020 d]
    
    set l3(varA)  [Xget XLM  0x400038 x]
#    puts "varA [format %d $varA]"
    set l3(selA)  $diagnostics($l3(varA))
    set l3(varB)  [Xget XLM  0x40003c x]
    set l3(selB)  $diagnostics($l3(varB))
    set l3(varC)  [Xget XLM  0x400040 x]
    set l3(selC)  $diagnostics($l3(varC))
    set l3(varD)  [Xget XLM  0x400044 x]
    set l3(selD)  $diagnostics($l3(varD))

    set l3(timestamp) [Xget XLM  0x400048 x]
    set l3(dssingles) [Xget XLM  0x400060 d]

    puts "reading stoppedstate, singlesmode, dssingles"
    puts [Xget XLM  0x400010 d]
    puts [Xget XLM  0x400014 d]
    puts [Xget XLM  0x400060 d]

    puts "reading mona_lisa_disable, caesar_disable, ds_disable"
    puts [Xget XLM  0x400018 d]
    puts [Xget XLM  0x40001c d]
    puts [Xget XLM  0x400020 d]
   
    ReadTrigger

#    ReleaseBus XLM
#    UnmapXLM XLM
	ReleaseBus

}

itcl::body ALevel3XLM72::ControlMenu {loc} { 
	global l3
    frame $loc
    button $loc.exit -text "Exit" -command "$this Exit" -state disable
########!!!!!!!!!! fix for single slot level 3 !!!!!!!

    button $loc.readxlm  -text "Read XLM" -command "$this ReadXLM"
    button $loc.setxlm  -text "Set XLM" -command "$this SetValues"
    button $loc.readfile -text "Read File" -command "$this ReadFile"
    button $loc.writefile -text "Write File" -command "$this AreYouSure"
    button $loc.checkxlm -text "Check XLM" -command "$this CheckStatus"
    label  $loc.status -textvariable l3(xlmstatus) -width 8
    
    pack $loc.exit \
	$loc.readxlm \
	$loc.setxlm \
	$loc.readfile \
	$loc.writefile \
	$loc.checkxlm \
	$loc.status -side left
}

itcl::body ALevel3XLM72::RunStop {stop} {
	global l3

#    MapXLM 0 $slot XLM
#    RequestBus XLM X
	AccessBus 0x10000

    set l3(stoppedstate) $stop
    Xset XLM 0x400010 $l3(stoppedstate)
    UpdateState
#    ReleaseBus XLM
#    UnmapXLM XLM
	ReleaseBus
}

itcl::body ALevel3XLM72::ResetTS  {} {
#    MapXLM 0 $slot XLM
#    RequestBus XLM X
	AccessBus 0x10000

    Xset XLM 0x400050 1
    Xset XLM 0x400050 0
    Xset XLM 0x40004c 1; # Set this register to 1 to enable TS counter!

#    ReleaseBus XLM
#    UnmapXLM XLM
	ReleaseBus
}


itcl::body ALevel3XLM72::SetDiagnostics {channel mask} {
#    MapXLM 0 $slot XLM
#    RequestBus XLM X
	AccessBus 0x10000

    if     {$channel == "1"} {
	set address 0x400038
    } elseif {$channel == "2"} {
	set address 0x40003c
    } elseif {$channel == "3"} {
	set address 0x400040
    } elseif {$channel == "4"} {
	set address 0x400044
    } else {
	puts "invalid channel, abort"
#	ReleaseBus XLM
#	UnmapXLM XLM
	ReleaseBus
    }

    Xset XLM $address $mask

    puts "diagnostic mask value channel $channel:"
    puts [Xget XLM  $address x]

#    ReleaseBus XLM
#    UnmapXLM XLM
	ReleaseBus
}

itcl::body ALevel3XLM72::Changed {argv} {
    global l3

    set l3(readflag) false
    set l3(setflag) false

    BG_yellow $l3(page3_writefile)
    BG_yellow  $l3(page3_set)
    BG_yellow $l3(page2_writefile)
    BG_yellow  $l3(page2_set)
    BG_yellow $l3(page1_writefile)
    BG_yellow  $l3(page1_set)
}

itcl::body ALevel3XLM72::LoadBitfile {} {
	global l3

# load bit file
    puts "loading $l3(bitfile)"
#    LoadXLM 0 $slot ASRAMA $bitfile
	Configure $l3(bitfile)
}

itcl::body ALevel3XLM72::newbitfile {level} {
    global l3

    set newfilename [tk_getOpenFile -initialdir $l3(bitdir)]
    
    if {$level == 2} {
	set l3(bitfile) $newfilename
	$l3(page2_bitfile) config -text $l3(bitfile)
    }
    if {$level == 0} {
	set l3(valuesfile) $newfilename
	$l3(page2_valuesfile) config -text $l3(valuesfile)
    }
}


itcl::body ALevel3XLM72::SetupGUI {main args} {
    set bw 100
    global l3 diagnostics

    set l3(xlmstatus) "?"
    set l3(toplevel) $main
    set entry "100"
    #frame .loader
#    wm title . "TS level 3 XLM control"
    set root 0
    set base 0
#    label $main
#    pack $main
	if {$main ne "."} {
		toplevel $main;    # don't create if it's .
		wm title $main "TS level 3 XLM control"
	} else {
		 set main ""
		 wm title . "TS Level 3 XLM control"
	}
    
#    puts "$main"

#    source $InstallDir/tabnbook.tcl
#    source $InstallDir/notebook.tcl
    
    
#    set notebk [tabnotebook_create $main.book]
	set notebk $main.book
	ttk::notebook $notebk -padding 5
    pack $notebk -expand 1 -fill both

#    set page(1) [tabnotebook_page $notebk "control page"]
#    puts "$page(1)"
	set page(1) [frame $notebk.page1]
	$notebk add $page(1) -text "control page"
    set base $page(1)
    set root $page(1)
    set WindowMap(1) $root

    frame $base.1

    ControlMenu $base.1.x

    set l3(page1_writefile) $base.1.x.writefile
    set l3(page1_set) $base.1.x.setxlm
    set l3(page1_status) $base.1.x.status 


    frame $base.1.a
#    frame $base.1.a.left
#    frame $base.1.a.right

############6/28/16
#    button $base.1.a.left -text "RUN" -command "$this RunStop 0" -width 10

#    button $base.1.a.center -text "STOP" -command "$this RunStop 1" -width 10
    
#    label  $base.1.a.right -textvariable l3(state) -pady 4 -width 10 
############

    set page1_state  $base.1.a.right

    label $base.1.a.label -text "Run/Stop Control" -fg blue
############6/28/16
#    pack $base.1.a.label -side top -anchor w
#    pack $base.1.a.left $base.1.a.center $base.1.a.right -side left -anchor n
############
	frame $base.1.f
    label $base.1.f.label1 -text "Trigger Detector" -fg blue
#    checkbutton $base.1.f.coinc -text "Coincidence Mode" \
#	-variable l3(singlesmode) -onvalue 0 -offvalue 1 -command "$this Changed yes"
#    label $base.1.f.label2 -text "Auxiliary Singles Trigger" -fg red
    radiobutton $base.1.f.swt0 -text "Sweeper dE scintillator (standard trigger: thin_LU)" -value 0  \
	-variable l3(dssingles) -command "$this Changed yes"
    radiobutton $base.1.f.swt1 -text "Timing scintillator (pot_scint, not recommended)" -value 1 \
	-variable l3(dssingles) -command "$this Changed yes"

    label $base.1.f.label2 -text "System Trigger Condition" -fg blue
    radiobutton  $base.1.f.trg0 -text "Sweeper standalone (disables all other triggers)" -value 0 \
     -variable l3(triggeroption) -command "$this SelectTrigger"
    radiobutton  $base.1.f.trg1 -text "Sweeper singles (will read MoNA-LISA & CAESAR)" -value 1 \
     -variable l3(triggeroption) -command "$this SelectTrigger"
    radiobutton  $base.1.f.trg2 -text "Coincidence Sweeper - requires MoNA or LISA or CAESAR or DS" -value 2 \
     -variable l3(triggeroption) -command "$this SelectTrigger"

    pack $base.1.f.label1 $base.1.f.swt0 $base.1.f.swt1 \
         $base.1.f.label2 $base.1.f.trg0 $base.1.f.trg1 $base.1.f.trg2 \
       -side top -anchor w

#    frame $base.1.g
#    label $base.1.g.label -text "Disabling Systems (overrides Trigger Condition)" -fg blue
#    checkbutton $base.1.g.dis0 -text "Disable MoNA-LISA" \
#	-variable l3(mona_lisa_disable) -onvalue 1 -offvalue 0 -command "$this Changed yes"
#    checkbutton $base.1.g.dis1 -text "Disable CAESAR" \
#	-variable l3(caesar_disable) -onvalue 1 -offvalue 0 -command "$this Changed yes"
#    checkbutton $base.1.g.dis2 -text "Disable downscale" \
#	-variable l3(ds_disable) -onvalue 1 -offvalue 0 -command "$this Changed yes"
#    checkbutton $base.1.g.dis3 -text "Singles mode" \
#	-variable l3(singlesmode) -onvalue 1 -offvalue 0 -command "$this Changed yes"

#    pack $base.1.g.label $base.1.g.dis0 $base.1.g.dis1 $base.1.g.dis2 $base.1.g.dis3 -side top -anchor w


    frame $base.1.t
    label $base.1.t.ts -textvariable l3(timestamp)
    button $base.1.t.r -text "TS reset" -command "$this ResetTS"
    pack $base.1.t.r $base.1.t.ts -side left
    
    pack $base.1.x $base.1.a $base.1.f         $base.1.t -side top
    pack $base.1 -in $root
   

#    set page(2) [tabnotebook_page $notebk "FPGA configure"]
	set page(2) [frame $notebk.page2]
	$notebk add $page(2) -text "FPGA configure"
	
    set base $page(2)
    set root $page(2)
    set WindowMap(2) $root

#
# define command buttons
#

    frame $base.2

    ControlMenu $base.2.x

    set l3(page2_writefile) $base.2.x.writefile
    set l3(page2_set) $base.2.x.setxlm
    set l3(page2_status) $base.2.x.status 


    frame  $base.2.a
#    menubutton   $base.2.a.select \
#	-textvariable module -menu $base.2.a.select.menu -relief sunken -width 12
#    set cm [menu $base.2.a.select.menu -tearoff 1]
#    $cm add radio -label "slot [expr 10]" -variable slot \
#	-value [expr 10] \
#	-command {
#	    puts $slot
#	    set module "XLM slot $slot"
#	}
    

    button $base.2.a.load -width 12 -text "Load" -command "$this LoadBitfile"
    button $base.2.a.boot -width 12 -text "Reset" \
	-command "$this SetFPGABoot 0x0; $this BootFPGA; puts \"Level3 XLM has been rebooted, you have to reload configuration.\""
#	    BootXLM 0 $slot flash0
    button $base.2.a.readsn -width 12 -text "Read s/n" \
	-command "puts \"S/N is [$this Read  0x820048]\""
    pack $base.2.a.load $base.2.a.boot $base.2.a.readsn -side left
    
#    frame  $base.2.b
#    button $base.2.b.loadall -width 26 -text "Load all FPGAs" \
#	-command {LoadBitfile {10}}
#   pack    $base.2.b.loadall
    frame  $base.2.c
    
    frame $base.2.space -height 10

    # define scales
    frame  $base.2.d
    
    scale  $base.2.d.coincgate -from 1 -to 500 -length 200 \
	-variable l3(coincgate) -orient horizontal -label "Coincidence gate" -command "$this Changed"
#    scale  $base.2.d.vme1 -from 1 -to 70 -length 200 \
	-variable vme1 -orient horizontal -label "not used" -command Changed
#   scale  $base.2.d.vme2 -from 0 -to 500 -length 200 \
	-variable vme2 -orient horizontal -label "not used" -command Changed
#    scale  $base.2.d.vme3 -from 1 -to 200 -length 200 \
	-variable vme3 -orient horizontal -label "not used" -command Changed

   pack $base.2.d.coincgate -side top
#$base.2.d.vme1 $base.2.d.vme2 $base.2.d.vme3
    #
    frame $base.2.g
    label $base.2.g.null -text "  " -pady 20
#    frame $base.2.g.one
#    puts [Xget XLM  0x40000c d]    
    frame $base.2.g.two
    frame $base.2.g.three
#    label $base.2.g.one.label1 -text "LV1 bitfile:" -width 12
    label $base.2.g.two.label2 -text "LV3 bitfile:" -width 12
    label $base.2.g.three.label3 -text "values file:" -width 12
#    button $base.2.g.one.level1 -text $bitfile1 -relief sunken -command {newbitfile "1"} -width 55 
    button $base.2.g.two.level2 -text $l3(bitfile) -relief sunken -command "$this newbitfile 2" -width 55
    button $base.2.g.three.file -text $l3(valuesfile) -relief sunken -command "$this newbitfile 0" -width 55

    pack $base.2.g.null
#    pack $base.2.g.one.label1    $base.2.g.one.level1 -side left
    pack $base.2.g.two.label2    $base.2.g.two.level2 -side left
    pack $base.2.g.three.label3  $base.2.g.three.file -side left
#    pack $base.2.g.null $base.2.g.one $base.2.g.two $base.2.g.three -side top
    pack $base.2.g.null $base.2.g.two $base.2.g.three -side top
    pack  $base.2.x $base.2.a $base.2.c $base.2.space $base.2.d \
	$base.2.g -side top

    pack $base.2 -in $root
    set l3(page2_bitfile) $base.2.g.two.level2
    set l3(page2_valuesfile) $base.2.g.three.file


#    set page(3) [tabnotebook_page $notebk "Signal routing"]
	set page(3) [frame $notebk.page3]
	$notebk add $page(3) -text "Signal routing"
    set base $page(3)
    set root $page(3)
    set WindowMap(3) $root
  
    frame $base.3

    ControlMenu $base.3.x

    set l3(page3_writefile) $base.3.x.writefile
    set l3(page3_set) $base.3.x.setxlm
    set l3(page3_status) $base.3.x.status 

    pack $base.3.x

    frame  $base.3.a

    label  $base.3.a.null -text "  " -width 15
    label  $base.3.a.label -text "diagnostics "
    pack $base.3.a.null $base.3.a.label  -side left
    pack $base.3.a

    frame   $base.3.b
    set loc $base.3.b
    label $loc.label -text "channel 1:" -width 15
    menubutton   $loc.select \
	-textvariable l3(selA) -menu $loc.select.menu -relief sunken -width 20
    set cm [menu $loc.select.menu -tearoff 0]
    foreach {maskvalue} [lsort -integer [array names diagnostics]] {
	$cm add radio -label $diagnostics($maskvalue) \
	    -variable l3(varA) -value $maskvalue \
	    -command "set l3(selA) $diagnostics($maskvalue);$this SetDiagnostics 1 $maskvalue"
    }
    pack $loc.label $loc.select -side left
    pack $loc

    frame   $base.3.c
    set loc $base.3.c
    label $loc.label -text "channel 2:" -width 15
    menubutton   $loc.select \
	-textvariable l3(selB) -menu $loc.select.menu -relief sunken -width 20
    set cm [menu $loc.select.menu -tearoff 0]
    foreach {maskvalue} [lsort -integer [array names diagnostics]] {
	$cm add radio -label $diagnostics($maskvalue) \
	    -variable l3(varB) -value $maskvalue \
	    -command "set l3(selB) $diagnostics($maskvalue);$this SetDiagnostics 2 $maskvalue"
    }
    pack $loc.label $loc.select -side left
    pack $loc

    frame   $base.3.d
    set loc $base.3.d
    label $loc.label -text "channel 3:" -width 15
    menubutton   $loc.select \
	-textvariable l3(selC) -menu $loc.select.menu -relief sunken -width 20
    set cm [menu $loc.select.menu -tearoff 0]
    foreach {maskvalue} [lsort -integer [array names diagnostics]] {
	$cm add radio -label $diagnostics($maskvalue) \
	    -variable l3(varC) -value $maskvalue \
	    -command "set l3(selC) $diagnostics($maskvalue);$this SetDiagnostics 3 $maskvalue"
    }
    pack $loc.label $loc.select -side left
    pack $loc

    frame   $base.3.e
    set loc $base.3.e
    label $loc.label -text "channel 4:" -width 15
    menubutton   $loc.select \
	-textvariable l3(selD) -menu $loc.select.menu -relief sunken -width 20
    set cm [menu $loc.select.menu -tearoff 0]
    foreach {maskvalue} [lsort -integer [array names diagnostics]] {
	$cm add radio -label $diagnostics($maskvalue) \
	    -variable l3(varD) -value $maskvalue \
	    -command "set l3(selD) $diagnostics($maskvalue);$this SetDiagnostics 4 $maskvalue"
    }
    pack $loc.label $loc.select -side left
    pack $loc

    pack $base.3 -in $root 
}

itcl::body ALevel3XLM72::ReadFile {} {
	global l3
    set l3(setflag) false
    set l3(readflag) true
    set file $l3(valuesfile)

    BG_green $l3(page3_writefile)
    BG_yellow  $l3(page3_set)
    BG_green $l3(page2_writefile)
    BG_yellow  $l3(page2_set)
    BG_green $l3(page1_writefile)
    BG_yellow  $l3(page1_set)

    puts "reading from $file"
    set f [open $file "r"]
    while {[gets $f varname] > 0} {
	    puts $varname
	    gets $f value
	    puts [set l3($varname) $value]
	    }
    close $f
    #update trigger display
    ReadTrigger
    UpdateState
    
    
 #   set f [open $file "r"]
 #   set coincgate [gets $f]
#    set vme1 [gets $f]
#    set vme2 [gets $f]
#    set vme3 [gets $f]

 #   set varA [gets $f]
 #   set varB [gets $f]
 #   set varC [gets $f]
 #   set varD [gets $f]

#    set stoppedstate [gets $f]
  #  set singlesmode [gets $f]
  #  set dssingles [gets $f]

   # close $f
 
}

itcl::body ALevel3XLM72::AreYouSure {} {
    global l3
    set choice [tk_messageBox -type yesno -default no -message "Update $l3(valuesfile) ?" -icon question ]
    if {$choice == yes} {WriteFile}
}


itcl::body ALevel3XLM72::WriteFile {} {
	global l3
    set l3(readflag) false
    set file $l3(valuesfile)

    BG_green $l3(page3_writefile)
    BG_yellow  $l3(page3_set)
    BG_green $l3(page2_writefile)
    BG_yellow  $l3(page2_set)
    BG_green $l3(page1_writefile)
    BG_yellow  $l3(page1_set)

    if {$l3(setflag) == true} {
        BG_green $l3(page1_set)
        BG_green $l3(page2_set)
        BG_green $l3(page3_set)
    }

    set HM 14

    puts "writing to $file"
    set f [open $file "w"]

    puts $f "coincgate"
    puts $f "$l3(coincgate)"
    puts $f "varA"
    puts $f "$l3(varA)"
    puts $f "varB"
    puts $f "$l3(varB)"
    puts $f "varC"
    puts $f "$l3(varC)"
    puts $f "varD"
    puts $f "$l3(varD)"
    puts $f "singlesmode"
    puts $f "$l3(singlesmode)"
    puts $f "dssingles"
    puts $f "$l3(dssingles)"
    puts $f "mona_lisa_disable"
    puts $f "$l3(mona_lisa_disable)"
    puts $f "caesar_disable"
    puts $f "$l3(caesar_disable)"
    puts $f "ds_disable"
    puts $f "$l3(ds_disable)"
    close $f

  #  BG_green .loader.f.writefile
}

itcl::body ALevel3XLM72::LockGUI {} {
	global l3
	Children $l3(toplevel) disabled
}

itcl::body ALevel3XLM72::Children {w state} {
	set childlist [winfo children $w]
	if {[llength $childlist] == 0} {
		if {![string match *exit* $w]} {
			catch {$w configure -state $state}
		}
	} else {
		catch {$w configure -state $state}
		foreach ww $childlist {Children $ww $state}
	}
}

itcl::body ALevel3XLM72::FreeGUI {} {
	global l3
	Children $l3(toplevel) normal
}
itcl::body ALevel3XLM72::OnBegin item {
  LockGUI
}
itcl::body ALevel3XLM72::OnEnd item {
  FreeGUI
}
itcl::body ALevel3XLM72::OnPause item {
  FreeGUI
}
itcl::body ALevel3XLM72::OnResume item {
  LockGUI
}
