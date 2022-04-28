#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#      NSCL DAQ Development Team 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#
# @author Jeromy Tompkins
# @note XLM72ScalerGUI layout and basic logic based on original design
#       by Daniel Bazin.

##
# @file xlm72scalerpanel.tcl
#
#
# Herein contains the source code for running the XLM72ScalerGUI.
# It is only half of the application as it requires the user to
# to have setup a VMUSBReadout to have registered an instance of
# the XLM72ScalerControl to the slow controls server as a tcl 
# module. 
# 
# The code is broken up into two distinct classes. The XLM72ScalerGUI
# class is an itcl megawidget that provides the UI widgets and layout.
# It owns a reference to an XLM72SclrControl class that handles the
# interaction of the GUI with the VMUSBReadout slow controls server. 
# The XLM72ScalerGUI is not purely a passive view, but rather handles
# the logic for refreshing itself when live updating is enabled.  
# 
#

package provide xlm72scalerpanel 1.0

package require Itcl
package require snit 
package require Tk 
package require usbcontrolclient 
package require TclServer
package require portAllocator 
package require InstallRoot
package require ScalerClient
package require ChannelLabel

# Set a default value for RunState. The sclclient manipulates
# this parameter when it is running but we will set a default
# value in case it hasn't set it to anything yet 
set RunState "*Unknown*" 



########################################################################
########################################################################
########################################################################

## XLM772ScalerGUI UI 
#
# Provides the main UI for the XLM72ScalerGUI. It also handles the logic
# for performing live updates. The class sends all requests created by 
# user actions to the mediator (an instance of XLM72SclrControl) that
# ultimately performs the communication with the VMUSBReadout slow controls
# server. On exit, this also stores the state of itself into a file
# that then is reloaded when constructed again.
#
# 
itcl::class XLM72ScalerGUI {
  
  private variable parentWidget ;#< the parent widget
  private variable mediator     ;#< the XLM72SclrGUIControl that handles communication
 
  private variable around       ;#< const value = 2^24
	private variable cancel ""    ;#< script for scheduling for period updates

	public variable scaler        ;#< array of total scaler values
	public variable rate          ;#< array computed rate values
	public variable enable        ;#< stores whether scalers are enabled
	public variable trigger       ;#< array storing whether channels contribute to trigger output
	public variable wrap          ;#< array storing the number of times that channels have wrapped
	public variable name          ;#< array storing name of channels
	public variable live          ;#< bool that contains state of live updates
	public variable frequency     ;#< int period of live update

  ##
  # @brief Constructor
  # 
  # The arrays of the value are initialized to zero values.
  # Builds the UI.
  # A reference to the XLM72SclrGUICtlr is obtained for communication
  # Determines whether the XLM72 has the proper firmware loaded
  # If all of the previous step succeed it loads the previous state from a file.
  constructor {parent modulename host port} {
    set parentWidget $parent
		set enable 0
	  set live 0
    set frequency 1
		set around [expr 1<<24]
		for {set i 0} {$i < 32} {incr i} {
  		set scaler($i) 0
			set rate($i) 0
  	 	set trigger($i) 0
  	 	set wrap($i) 0
		}


		# look for module and update from it
    set mediator [::XLM72SclrGUICtlr %AUTO% -host $host -port $port -name $modulename]

    # If we are in a running state just stop...that saves a lot of headache.
    if {[RunStateIsActive]} {
      tk_messageBox -icon error \
                   -message "XLM72ScalerGUI : Unable to startup when run is active. Please stop the run and try again."
      exit
    }

    $this BuildGUI $parentWidget
    $mediator configure -widget $this 


    # First let's see if it even makes sense to run this thing
    if {[IsGoodFirmware 0xdaba0002]} {
      # look for a file saved from before 
      LoadSavedSettings 
      UpdatePanel 1
    } else {
      tk_messageBox -icon error \
                   -message "Failed to read good firmware id from module. Maybe the firmware needs to be loaded?"
      exit
    }

    
  }
 
  ## 
  # @brief Checks that the firmware signature matches a value
  # 
  # This simply reads the firmware from the device and then compares it
  # to a value. It ensures that first the fw value is a valid integer
  # and then if that succeeds, it does the comparison. The signature
  # that is checked when this is called is 0xdaba0002.
  # 
  # @param signature the value to compare the firmware id against 
  method IsGoodFirmware {signature}

  ##
  # @brief Build the GUI
  # 
  # This calls a number of constructor methods to build the varios
  # pieces of the UI and then grids them all together. All of the 
  # widgets are put into a ttk::frame that is not itself gridded. 
  # To grid the complete GUI, the user should obtain the name of 
  # the megawidget using the GetTopFrame method and then grid it.
  # 
  # @param parent the parent widget in which to build the UI
  #
  method BuildGUI {parent}

  ## 
  # @brief Build one of the two panels containing the actual scaler values.
  # This creates 16 rows of elements with indices provided by some offset
  # By the time this returns, all of the child widgets of the parent are
  # gridded but the parent is not. 
  #
  # @param parent widget in which to build the panel
  # @param name   name of the frame that is being built
  # @param offset index offset used to start labeling
  # 
  # @return string
  # @retval the name of the widget created ($parent.$name)
  #
  method BuildPanel {parent name offset}

  ## 
  # @brief Constructs the two top panels containing all ofthe scaler data
  # 
  # This is a convenience function that just calls the BUildPanel
  # method for both of the top scaler panels and grids them within
  # a frame. It also creates a ttk::separator to separate the two
  # panels. 
  #
  # @param parent widget to hold the top panel section
  # @param paren  name name of the frame that will hold the panels 
  #
  # @return string
  # @retval name of the fully built widget ($parent.$name)
  method BuildPanels {parent name}

  ## 
  # @brief Build the labelframe containing scaler controls
  # 
  # This constructs the little box that holds the enable checkbox
  # and the reset button. It creates a label frame, fills it 
  # with these widgets, and then grids them.
  # 
  # @param parent widget that will hold the newly built frame
  # @param name   name of the frame that is being filled with widgets
  #
  # @return string
  # @retval name of the fully built widget ($parent.$name)
  method BuildSclrControlBox {parent name}
  
  ##
  # @brief Build the labelframe containing update controls 
  #
  # This constructs the frame that holds the enable checkbox for the
  # live update feature and also the spinbox that sets the value of the
  # update period.
  #
  # @param parent widget that will hold the newly built frame
  # @param name   name of the frame that is being filled with widgets
  #
  # @return string
  # @retval name of the fully built widget ($parent.$name)
  method BuildUpdateControlBox {parent name}

  ## 
  # @brief Sets the background color for the various widget using the ttk::style
  # 
  # Location where all of the style specifications occur
  # 
  # @return nothing
  method SetupStyle {}

  ##
  # @brief Retrieves the parent widget name stored during construction
  #
  # @return string
  # @retval name of parent widget
  method GetParent {} {return $parentWidget} 

  ## 
  # @brief Save current state to a file
  # 
  # The scaler, wrap, enable, live, and trigger 
  # values are written to a file named: XLM72Scaler_$this.tcl
  # with the colons trimmed off
  # 
  # @return nothing
  method SaveSettings {}

  ## 
  # @brief Update panel information
  # 
  # The mechanism for obtaining new data. Depending on the argument
  # this will either only update the values once or it will continue
  # to update them periodically at a user defined interval (@see frequency)
  # 
  # The update only results in a call to the VMUSB if the RunState 
  # variable is not set to Active. If it is not, then the first thing
  # that the update does is query the VMUSBReadout slow controls for the
  # run state to ensure that the sclclient lagged in receiving its information.
  # This is a protection against always going into and out of the data
  # taking mode more than once. If after these levels of checks, it is 
  # still true that the data taking is not in effect, then the values 
  # are actually updated. However, if the run is active, then the display
  # is disabled.
  # 
  # @param once whether to update once or turn on live updating
  # 
  # @return nothing
  method UpdatePanel {once}
 
  ##
  # @brief Turn on/off the live update feature
  # 
  # The behavior of this method relies on whether the variable "live" is 0 or 1.
  # if the value is 0, then the live updates are turned off. On the other hand, if the
  # value is 1, then live updates are turned on.
  method SetLive {}
 
  ##
  # @brief Handle an exit event
  # 
  # On an exit, the state of the gui is saved by the SaveSettings method,
  # the live updates are canceled and removed from the tcl event loop,
  # the XLM72SclrGUICntlr receives an OnExit (to kill off the sclclient),
  # and then the parent window is killed.
  #
  method OnExit {}
  
  ##
  # @brief Disable or enable the widgets in the panels except for the exit button
  # 
  method SetChildrenState {state}
 
  ##
  # @brief Update the actual scaler values
  # 
  # This retrieves new scaler values from the XLM72 and then computes the
  # correct values for the rate based on the most recent increment. The update 
  # is then used to determine wehther the scaler counters have wrapped around. 
  #
  # @return nothing
  method UpdateValues {}

  ## 
  # @brief Parse the trigger register 
  #
  # When reading the trigger register, the state of all channels is encoded in
  # a single 32-bit integer. Because the GUI deals with the trigger states as 
  # an array, the integer needs to be unpacked and the decoded values stored in
  # the proper array locations.
  # 
  # @param value the integer containing encoded trigger states for each channel
  #
  # @return nothing
  
  method ParseTriggerRegister {value}
  
  ## 
  # @brief Reset the scaler counters
  # 
  # This simply causes the scaler counters to be cleared.
  # 
  # @return nothing
  method OnReset {}
  
  ## 
  # @brief Enable/Disable all scalers to count
  # 
  # Depending on the state of the "enable" variable, this will either cause 
  # the scaler values to be disabled or enabled. The value of the endable
  # variable is tied to the checkbutton.
  #
  # @return nothing
  method OnEnable {}

  ## 
  # @brief Handle when a trigger checkbutton has been toggled
  # 
  # Depending on state ofthe "trigger($ch)" variable, the channel is either
  # added to the trigger OR or removed from it.
  # 
  # @param ch   channel to operate on 
  #
  # @return nothing
  method OnTriggerToggle {ch}

  ## 
  # @brief Get the name of the frame containing the entire megawidget
  #  
  # @return string
  # @retval $parentWidget.topframe
  method GetTopFrame {} { 
    if {$parentWidget ne "."} {
      return $parentWidget.topframe
    } else {
      return .topframe
    }
  }

  ## 
  # @brief Retrieve the values of the settings from a file
  # 
  # If the file XLM72Scaler_$this.tcl exists, then it is
  # executed to return the state of the gui to what it was 
  # before shutting down.
  # 
  # @return nothing
  method LoadSavedSettings {} 

  ## @brief Check whether run is not idle
  # 
  # First checks to see whether RunState is Active. If that
  # is not true, then it asks the slow-controls server what 
  # its runstate is. 
  #
  # @returns boolean
  # @retval  0 - RunState != Active and runstate is not idle or paused
  # @retval  1 - Otherwise 
  #
  method RunStateIsActive {} 
}


#
# There is a little hole here... 
itcl::body XLM72ScalerGUI::IsGoodFirmware {signature} {
  set fw [$mediator GetFirmware]
  if {[string length $fw]>0 && [string is integer -strict $fw]} {
    if {$fw==$signature} {
      return 1 
    }  else {
       return 0
    }
  } else {
    return 0
  }
}

itcl::body XLM72ScalerGUI::LoadSavedSettings {} {
  if {[file exists "XLM72Scaler_[string trimleft $this :].tcl"]} {
    source "XLM72Scaler_[string trimleft $this :].tcl"
  } else {
    for {set i 0} {$i < 32} {incr i} {
      set wrap($i) 0
      set name($i) ""
    }
  }
  # Establish consistency
  if {$enable} {
    $this OnEnable
  }

  if {$live} {
    $this SetLive
  }
}

itcl::body XLM72ScalerGUI::BuildGUI {parent} {
  SetupStyle
  ## Build the left column
  if {$parent eq "." } { set parent "" }
  set top $parent.topframe
  ttk::frame $top -style TFrame
  

  # Build the big pieces
  set panels [BuildPanels $top panels]
  set sclrctrls [BuildSclrControlBox $top sclrctrl]
  set updatectrls [BuildUpdateControlBox $top updatectrl]

  SetChildrenState active

  ## Build the button box
  set w $top.buttons
  ttk::frame $w -style TFrame
  ttk::button $w.exit -text Exit -command "$this OnExit"

  grid $w.exit -sticky news -padx 3 -pady 6
  grid rowconfigure $w 0 -weight 1 
  grid columnconfigure $w 0 -weight 1 

  ## Put it all together
  grid $panels -columnspan 2 -sticky news -padx 3
#  grid $sclrctrls $updatectrls -sticky nsew -padx 3 -pady 6
  grid $top.buttons -columnspan 2 -sticky nsew

}

itcl::body XLM72ScalerGUI::SetupStyle {} {
  ttk::style configure TFrame -background lightblue
  ttk::style configure TLabel -background lightblue
  ttk::style configure TLabelframe -background lightblue
  ttk::style configure TLabelframe.Label -background lightblue
  ttk::style configure TCheckbutton -background lightblue

  ttk::style map TFrame -background [list disabled lightblue active lightblue]
  ttk::style map TLabel -background [list disabled lightblue active lightblue]
#  ttk::style map TLabelframe -background [list disabled lightblue active lightblue]
#  ttk::style map TLabelframe.Label -background [list disabled lightblue active lightblue]
  ttk::style map TCheckbutton -background [list disabled lightblue active lightblue]

}

itcl::body XLM72ScalerGUI::BuildPanel {parent name offset} {
  set top $parent 

  set w $top.$name
  ttk::frame $w 

  ttk::label $w.chanLbl -text "Ch#"
  ttk::label $w.nameLbl -text "Name"
  ttk::label $w.rateLbl -text "Rate"
  ttk::label $w.totalLbl -text "Total"
  ttk::label $w.enableLbl -text "Trig"

  set begin $offset
  set nrows 16
  set end [expr $offset+$nrows]
  for {set i $begin} {$i < $end} {incr i} {

    set trigger($i) 0
#    ttk::label $w.ch$i -text [format "%02d" $i] -width 2
    ChannelLabel $w.name$i -defaultstring [format "Ch %02d" $i] \
                          -textvariable [itcl::scope name($i)] -width 20
    #ttk::entry $w.name$i   -textvariable [itcl::scope name($i)] -width 8 \
    #                  ;#-validatecommand SaveSettings
    ttk::label $w.scaler$i -textvariable [itcl::scope scaler($i)]  \
                          -padding 2 -width 10 -anchor e
    ttk::label $w.rate$i   -textvariable [itcl::scope rate($i)] \
                          -padding 2 -width 8 -anchor e
    ttk::checkbutton $w.trigger$i -variable [itcl::scope trigger($i)] \
                             -command "$this OnTriggerToggle $i" \
                             -takefocus 0

  }

  ## Grid the panel
  grid $w.nameLbl $w.enableLbl -stick nsew
#  grid $w.chanLbl $w.nameLbl $w.totalLbl $w.rateLbl $w.enableLbl -stick nsew
  for {set i $begin} {$i < $end} {incr i} {
    grid $w.name$i $w.trigger$i -sticky news
#    grid $w.ch$i $w.name$i $w.scaler$i $w.rate$i $w.trigger$i -sticky news
    grid rowconfigure $w [expr ($i-$begin)+1] -weight 1
  }
  grid columnconfigure $w {1 2 3} -weight 1 


  return $w
}

itcl::body XLM72ScalerGUI::BuildPanels {parent name} {
  set frmname $parent.$name

  ttk::frame $frmname

  set leftPanel [$this BuildPanel $frmname s1 0]
  set rightPanel [$this BuildPanel $frmname s2 16]
  ttk::separator $frmname.vs -orient vertical

  grid $leftPanel $frmname.vs $rightPanel -sticky news -padx 3

  return $frmname
}

itcl::body XLM72ScalerGUI::BuildSclrControlBox {parent name} {

  set w $parent.$name

  # Create the frame and configure it
  ttk::labelframe $w -text "Scaler Controls"
  set lblfrmClass [winfo class $w]

  # make the widgets
  ttk::checkbutton $w.enable -text "Enable" -variable [itcl::scope enable] \
                        -command "$this OnEnable" 
  ttk::button $w.reset -text "Reset Scalers" -command "$this OnReset"

  grid $w.enable $w.reset -sticky nsew -padx 3 -pady 3
  grid columnconfigure $w {0 1} -weight 1 

  return $w
}

itcl::body XLM72ScalerGUI::BuildUpdateControlBox {parent name} {

  set w $parent.$name

  # create the frame and configure the background color
  ttk::labelframe $w  -text "Live Update"

  # create the widgets
  ttk::checkbutton $w.live -text "Enable" -variable [itcl::scope live] \
                      -command "$this SetLive"
  ttk::spinbox $w.freq -textvariable [itcl::scope frequency] -width 3 -increment 1 \
                  -from 1 -to 10
  ttk::label $w.freqLbl -text "Update Period (s)"

  # grid it 
  grid $w.live $w.freqLbl $w.freq -sticky nsew -padx 3 -pady 3
  grid columnconfigure $w 0 -weight 1

  return $w
}

itcl::body XLM72ScalerGUI::SaveSettings {} {
  set file [open "XLM72Scaler_[string trimleft $this :].tcl" w]
  for {set i 0} {$i < 32} {incr i} {
    puts $file "set trigger($i) $trigger($i)"
    puts $file "set name($i) \"$name($i)\""
    puts $file "set wrap($i) $wrap($i)"
  }
  puts $file "set enable $enable"
  puts $file "set live $live"
  puts $file "set frequency $frequency"
  close $file
  return 1
}

##
# UpdatePanel
#
# THis handles the updates to the GUI. In this case, it will relay an Update
# call through the mediator and then retrieve the updated values through some
# Get commands
#
itcl::body  XLM72ScalerGUI::UpdatePanel {once} {
  global RunState
#	set top ".aXLM72Scaler_[string trimleft $this :]"

  set active [RunStateIsActive] 
  if {$active} {
    SetChildrenState disabled
  } else {
    SetChildrenState "!disabled"
    UpdateValues ;# Update the values
  }

  # If needed reschedule the next update
  if {!$once} {
    set cancel [after [expr $frequency*1000] "$this UpdatePanel 0"]
  }
}

itcl::body XLM72ScalerGUI::RunStateIsActive {} {

  # First check to see if the RunState knows that we are 
  # active
  set active [expr {$::RunState eq "Active"}]

  # If we think we aren't active, then look a little closer, because
  # if the period between refreshes is shorter than it takes
  # for data to be observed by the sclclient and set RunState 
  # us, then we end up getting through to the slow controls server
  # when the run is active. This then pauses the run and it is 
  # annoying. So we can at least prevent it from happening more than once
  # by asking for the state.
  if {! $active} {
    set state [$mediator GetRunState]
    set active [expr {($state ne "idle") && ($state ne "paused")}]
  }

  return $active
}

# Update state of child widgets
itcl::body XLM72ScalerGUI::SetChildrenState {state} {
  set top [GetTopFrame] 
#  foreach c [winfo children $top.panels] {$c configure -state $state}
  foreach c [winfo children $top.panels.s1] {$c state $state}
  foreach c [winfo children $top.panels.s2] {$c state $state}
  foreach c [winfo children $top.sclrctrl] {$c state $state}
  foreach c [winfo children $top.updatectrl] {$c state $state}
  update
}

itcl::body XLM72ScalerGUI::UpdateValues {} {
    $mediator Update
    set enable  [$mediator GetEnable]
    set trigreg [$mediator GetAllTriggers]
    ParseTriggerRegister $trigreg
    set values   [$mediator GetAllSclrValues]
    if {[llength $values] != 32 } {
      tk_messageBox -icon error -message "Failed to read all 32 scaler values"
    } else {
      for {set i 0} {$i < 32} {incr i} {
        set value [lindex $values $i]
        # determine whether the number has wrapped
        if {$value < $scaler($i)} {
          incr wrap($i)
        }
        # compute the new scaler value
        set updated [expr $wrap($i)*$around + $value]
        set rate($i) [format %.1f [expr ($updated-$scaler($i))/$frequency]]
        set scaler($i) $updated
      }
    }
}

itcl::body XLM72ScalerGUI::SetLive {} {
  if {$live} {
    $this UpdatePanel 0
  } else {
    after cancel $cancel
  }
}

itcl::body XLM72ScalerGUI::OnExit {} {
  after cancel $cancel
  SaveSettings
  $mediator OnExit
  destroy [GetParent] 
}

itcl::body XLM72ScalerGUI::ParseTriggerRegister {value} {
	for {set i 0} {$i < 32} {incr i} {
		set trigger($i) [expr ($value&(1<<$i))>>$i]
	}
}

itcl::body XLM72ScalerGUI::OnReset {} {
  if {![RunStateIsActive]} {
    $mediator Reset
    for {set i 0} {$i < 32} {incr i} {
      set wrap($i) 0
      set scaler($i) 0
      set rate($i) 0
    }
  }
}

itcl::body XLM72ScalerGUI::OnEnable {} {
  if {![RunStateIsActive]} {
    $mediator SetEnable $enable
  }
}

itcl::body XLM72ScalerGUI::OnTriggerToggle {ch} {
  if {![RunStateIsActive]} {
    $mediator SetTrigger $ch $trigger($ch)
  }
}





###############################################################################
###############################################################################
###############################################################################
###############################################################################
###############################################################################


##
# @class XLM72SclrGUICtlr
# @brief Controller for XLM72ScalerGUI
#
# This is ultimately the piece that interacts with VMUSBReadout program.
# Any time a request is made in the GUI, it causes this to send out 
# requests to the VMUSBReadout slow controls server. Communication is performed
# through the usb controlclient provided by the usbcontrolclient package.
#
# It also manages a sclclient program that it starts up in the background. When
# the OnExit method is called, which should happens when the GUI's exit button is
# pressed, it kills off this process. To ensure that when the "x" button on the
# window (i.e. the window close button) this method is called, it is important
# to set the "wm protocol win_name WM_DELETE_WINDOW { script }" to ultimately
# call the OnExit of this. Usually this is done through the GUI's OnExit proc.
#
#
snit::type XLM72SclrGUICtlr {

  option -host       -default localhost ;#< host running the VMUSBReadout program
  option -port       -default 27000     ;#< port on which the slow controls server is running
  option -connection -default ""        ;#< usb controlClient instance for interacting \
                                            with the VMUSBReadout slow controls server
  option -name       -default ""        ;#< name of the module registered in the slow controls \
                                            server
  option -widget     -default ""        ;#< a reference to the XLM72SclrGUI
  option -onlost     -default ""        ;#< script to run when connection is lost
  
  variable sclclientPID   -1            ;#< PID of the sclclient process spawned by this
  variable server         -1            ;#< PID of the sclclient process spawned by this

  ##
  # @brief Constructor
  # 
  # Sets up all of the options and then connects to the slow controls server.
  # Furthermore, it starts up a sclclient program. 
  #
  # @param args a list of option value pairs
  constructor args {
    variable sclclientPID

    $self configurelist $args
    
    $self Reconnect $options(-host) $options(-port)

    set port [$self startServer ]
    puts "TclServer started on port = $port"
    set sclclientPID [startScalerClient localhost $port \
                                        localhost $::env(USER)]
  }
  
  ## @brief Destructor
  # 
  # Simply kill off the sclclient PID if it exists
  #  
  destructor {
    variable sclclientPID
    if {$sclclientPID != -1} {
      $self OnExit
    }
  }

  ##
  # @brief Handle an exit event
  # 
  # The only task that this has is to kill the sclclient process that is 
  # spawned by this.
  # 
  method OnExit {} {
    variable sclclientPID
    puts $sclclientPID
    exec kill -9 $sclclientPID
    set sclclientPID -1
  }

  ##
  # @brief Relay an enable command to the slow controls
  # 
  # @param enable boolean value for whether to enable or disable scalers 
  #
  # @return nothing
  method SetEnable {enable} {
    $self Set [list enable $enable]
  }

  ## 
  # @brief Relay a trigger event to slow controls
  # 
  # This mostly just formats the parameter name before sending it to the
  # slow controls server. The parameter name is encoded into the name by
  # appending it to the "trigger" string.
  # 
  # @param ch   channel to operate on
  # @param val  boolean value specifying whether to enable or disable trigger
  # 
  # @return nothing
  method SetTrigger {ch val} {
    $self Set [list [format "trigger%d" $ch ] $val]
  }

  ##
  # @brief Send a reset command to slow controls
  #
  # This always causes a scaler reset (aka. clear) .
  #  
  method Reset {} {
    $self Set [list reset 1]
  }

  ##
  # @brief Send a firmware id request to slow controls server
  # 
  # @return int
  # @retval the firmware id
  # 
  method GetFirmware {} {
    return [$self Get [list firmware]]
  }

  ## 
  # @brief Ask whether or not the scalers are enabled
  # 
  # @return boolean
  # @retval whether the scalers are enabled or disabled
  #
  method GetEnable {} {
    return [$self Get [list enable]]
  }

  ## 
  # @brief Send request for trigger register contents
  # 
  # Request the encoded integer that contains all of the trigger
  # bit settings. The bits in the integer correspond to each
  # channel's trigger settings. The trigger setting for ch.0 is 
  # encoded in bit 0, ch. 1 in bit 1, etc.
  # 
  # @return int
  # @retval bitset of all trigger setting bits
  method GetAllTriggers {} {
    return [$self Get [list alltriggers]]
  }

  ## 
  # @brief Retrieve a fresh reading of the scaler values
  #
  # This causes the XLM72 scalers to be latched and then read out.
  # The values are converted into a tcl list before returning.
  #
  # @return list
  # @retval 32 scaler values 
  #
  method GetAllSclrValues {} {
    return [$self Get [list allscalers]]
  }

  ## 
  # @brief Request the run state 
  # 
  # @return returns the runstate
  # @retval active
  # @retval idle
  # @retval starting
  # @retval stopping
  # @retval paused
  # 
  method GetRunState {} {
    return [$self Get [list runstate]]
  }

  ##
  # @brief Connect to the slow controls server
  # 
  # Repeatedly try to connect to the server until it succeeds. On failure to connectm
  # the user is presented with an error message. On success, the created 
  # controlClient is cached as -connection.
  # 
  # @param host host where the VMUSBReadout program is running
  # @param port port on which slow controls server is listening
  # 
  method Reconnect {host port} {

    while {[catch {controlClient %AUTO% -server $host -port $port} connection]} {
      tk_messageBox -icon info \
        -message "The control panel requires readout be running, please start it: $connection"
    }
    set options(-connection) $connection
  }

# perform  transactions with the device.
#

  ## 
  # @brief Setup the Get command
  # 
  # All commands that request information ultimately funnel through this command.
  # This simply formats a proper Get request and then initiates the transaction.
  # 
  # @param args a tcl list containing the parameter name to retrieve
  # 
  # @return the result
  # @retval depends on the request
  method Get args {
    set name $options(-name)
    set conn $options(-connection)
    set value [$self transaction [concat [list $conn Get $name] [lindex $args 0]]]
    return $value
  }


  ## 
  # @brief Initiate the Set command
  # 
  # Like the Get method, this formats the request to be sent for Set commands.
  # It then initiates the transaction with the slow controls server.
  # 
  # @param args a tcl list containing the parameter and value to write
  # 
  # @return the result
  # @retval depends on the request
  method Set {args}  {
    set name $options(-name)
    set conn $options(-connection)
    set args [lindex $args 0] 
    set command [concat [list $conn Set $name] $args]
    return [$self transaction $command]

  }

  ##
  # @brief Initiate the Update command
  # 
  # This doesn't actually get used but it is defined for the future
  # 
  method Update {} {
    set name $options(-name)
    set conn $options(-connection)

    return [$self transaction [list $conn Update $name]]
  }


  ## 
  # @brief Manage the transactions
  # 
  # Executes the request and then deals with the response.
  # If an exceptional return occurs with no result, then the
  # -onlost connection script is called. If that is not defined then
  # it just spits out a standard error. If the slow controls server
  # returns back an actual response beginning with "ERROR" then 
  # the method transforms the response into a TCL error. Otherwise,
  # a successful transaction just returns a value.
  #
  # @param script a properly formatted Set, Get, or Update command
  # 
  # @return depends on the request 
  # 
  method transaction {script} {
# loop until transaction works or error signalled.

    while {[catch {eval $script} msg] && ($msg ne "")} {
      if {$options(-onlost) ne ""} {# comm fail handler..
        eval $options(-onlost) $options(-connection)
        set script [lreplace $script 0 0 $options(-connection)]
      } else {			# no handler.
        error "Communication failure $msg"
      }
    }

    if {[string is list $msg]} {
      if {[lindex $msg 0] eq "ERROR"} {
        puts "THE CLIENT RECEIVED ERROR"
          error $msg;			# Transaction error.
      }
    }

  	return $msg

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
  method _OnTclServerConnect {fd ip port } {
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
  method _OnTclServerError {chan command msg } {
    puts "Error when executing \"$command\" : $msg"
    return true
  }

  ## @brief Starts up the TclServer
  #
  # Allocates a port and then starts the server on it.
  # 
  # @returns int
  # @retval  port that TclServer listens on
  method startServer {} {
    variable server
    ::portAllocator create allocator 
    set port [allocator allocatePort XLM72ScalerGUI]
    set server [TclServer %AUTO% -port $port -onconnect [mymethod _OnTclServerConnect]]
    $server configure -onerror [mymethod _OnTclServerError]
    $server start
  
    return $port
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


