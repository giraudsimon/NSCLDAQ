#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#    Jeromy Tompkins
#	   NSCL
#	   Michigan State University
#	   East Lansing, MI 48824-1321

package provide mcfd16guiapp 1.0

package require Tk
package require snit
package require mcfd16factory
package require mcfd16gui
package require FrameSequencer

## @brief Application options for the Gui
#
# A class that encapsulates the application options used by the MCFD16GuiApp.
# Some other instances are used for this. It is treated kind of like a struct,
# and the MCFD16GuiApp delegate settings of these option to an instance of this
# type.
#
snit::type MCFD16AppOptions {
  option -protocol
  option -serialfile
  option -module
  option -host
  option -port 
  option -devno
  option -configfile

  ## @brief Parse and set options
  constructor {args} {
    $self configurelist $args 
  }
}


## @brief Main snit::type that manages top level configs.
#
# The MCFD16GuiApp is mainly responsible for setting up the various subsystems
# and maintaining the top level entities. It does the following:
# 1. Keeps hold of the load and save dialog widgets and provides mechanism for
#     switching to them.
# 2. Sets up the menu bar
# 3. Holds the global option object and keeps the options passed in at the cmd
#     line
# 4. Holds the frame sequencer and registers each of the frames the user might
#    interact with to the sequencer in a proper order.
# 5. It holds the main device handle for the entire GUI.
#
snit::type MCFD16GuiApp {
  option -optionarray -default ::params ;# name of the array of options
  option -widgetname -default .app      ;# name of view controlled by this

  component _handle           ;# MCFD16 type device driver
  component _options          ;# MCFD16AppOptions
  
  delegate option * to _options ;# automatically forwards cget/configure to
                                ;# _options if not locally defined

  variable _controlPrsntr  {} ;# the MCFD16ControlPanel instance
  variable _configFr {}       ;# the window name of MCFD16ControlPanel
                              ;# at the moment this is the same as
                              ;# _controlPrsntr
  variable _saveFr {}         ;# Widget name for the save dialog
  variable _loadFr {}         ;# widget name for the load dialog
  variable _enableFr {}       ;# name for the ChannelEnableDisableView
  variable _enable {}         ;# name for the ChannelEnableDisablePresenter
  variable _orPatternFr {}    ;# name for the ORPatternConfigView
  variable _sequencer {}      ;# the FrameSequencer


  ## @brief Initialize the MCFD16 driver and the gui
  #
  # @param args   the option-value pairs
  #
  constructor {args} {
      install _options using MCFD16AppOptions %AUTO% 
      
      $self configurelist $args
      
      # Constructs the mcfd16 driver
      $self processCmdlineOptions
      
      $self setUpMenu
      $self BuildGUI
      if {[$_options cget -configfile] ne ""} {
	  set path [$_options cget -configfile]
	  if {[file exists $path]} {
	      $self LoadConfigFile $path
	  } else {
	      set msg "Cannot load config file because "
	      append msg "$path does not exist."
	      tk_messageBox -icon error -message $msg
	  }
      }

  }

  ## @brief Cleanup the device driver
  destructor {
    catch {$_handle destroy}
  }

  ## @brief Constructs the MCFD16 based on the command line options
  #
  # If there is a failure while creating the device, the application exits and
  # then prints the message detailing the error.
  method processCmdlineOptions {} {

    # Create the driver that serves as the backend of the gui
    MCFD16Factory factory $_options

    set res [catch {
      set paramDict [array get [$self cget -optionarray]]
      set protocol [dict get $paramDict -protocol]
      if {$protocol eq "usb"} {
        set serialFile [dict get $paramDict -serialfile]
        if {![file exists $serialFile]} {
          puts "Serial file \"$serialFile\" provided but does not exist."
          exit
        }

        install _handle using factory create usb
        flush stdout
      } elseif {$protocol eq "mxdcrcbus"} {
      # at this point the only other option is mxdcrcbus because 
      # assertProtocolDependencies would have exited otherwise.
        install _handle using factory create mxdcrcbus
      } elseif {$protocol eq "test"} {

        install _handle using factory create memorizer

        $self loadTestState

      } else {
        return -code error "Invalid protocol name specified for --protocol argument"
      }
    } msg]

    if {$res == 1} {
      puts "Failed to create an MCFD16 device driver with error : $msg"
      exit
    }
  }

  ## @brief Build the menu
  # 
  # Adds the File and Configure drop-down menus.
  #
  # @todo Make this capable of building a menu if the toplevel is not ".".
  #
  method setUpMenu {} {

    option add *tearOff 0

    # get the menu for the toplevel
    set menu [[winfo toplevel [$self cget -widgetname]] cget -menu]
    if {$menu eq ""} {
      set m .menu
      menu $m
      menu $m.file 
      $m.file add command -command [mymethod ToSaveAs] -label "Save as..."
      $m.file add command -command [mymethod ToLoad] -label "Load settings..."

      menu $m.config
      $m.config add command -command [mymethod ToEnableDisable] -label "Enable/disable..."
      $m.config add command -command [mymethod ToORPatternConfig] -label "Trigger OR..."

      $m add cascade -menu $m.file -label "File"
      $m add cascade -menu $m.config -label "Configure"

      . configure -menu $m
    }
  }

  ## @brief Build up the GUI piecemeal
  #
  # There are 4 main widgets that needs to be constructed and added to the frame
  # sequencer. The _configFr (the frame name of the MCFD16ControlPanel instance)
  # is the default fallback location for the visible frame. The menu will take
  # the user to the frame for enabling/disabling, save, and load frames. This
  # method simply constructs them and then loads them into the frame sequencer
  # properly.
  #
  method BuildGUI {} {
    set win [$self cget -widgetname]

    ttk::label $win.title -text "MCFD-16 Controls" -style "Title.TLabel"

    set _sequencer $win.frames
    FrameSequencer $_sequencer

    # build the frames    
    set _configFr [$self BuildControlFrame $_sequencer]
    set _saveFr [$self BuildSaveAsFrame $_sequencer]
    set _loadFr [$self BuildLoadFrame $_sequencer]
    set _enableFr [$self BuildEnableDisableFrame $_sequencer]
    set _orPatternFr [$self BuildOrPatternFrame $_sequencer]

      # load the frames into the FrameSequencer in a proper order.
    $_sequencer staticAdd config $_configFr {}
    $_sequencer staticAdd save $_saveFr config
    $_sequencer staticAdd load $_loadFr config
    $_sequencer staticAdd enable $_enableFr config
    $_sequencer staticAdd orPattern $_orPatternFr config
    $_sequencer select config

    grid $_sequencer -sticky nsew -padx 8 -pady 8

  }

  ## @brief Build the MCFD16ControlPanel
  #
  # This includes the MCFD16ControlPanel and the textual info at the top of the
  # window.
  #
  # @param top  the parent widget
  #
  # @returns the name of the MCFD16ControlPanel instance
  method BuildControlFrame {top} {
    set configFr $top.config
    ttk::frame $configFr
    set title [ttk::label $configFr.title -text "MCFD-16 Controls" \
                                          -style "Title.TLabel"]
    set infoFrame [$self constructInfoFrame $configFr]
    set _controlPrsntr $configFr.ctl
    set control [MCFD16ControlPanel $_controlPrsntr -handle $_handle]

    grid $title -sticky nsew -padx 8 -pady 8
    grid $infoFrame -sticky nsew -padx 8 -pady 8
    grid $control -sticky nsew -padx 8 -pady 8
    grid rowconfigure $configFr {2} -weight 1
    grid columnconfigure $configFr 0 -weight 1

    return $configFr
  }

  ## @brief Construct the SaveToFileForm
  #
  # @param top  the parent widget
  #
  # @returns the name of the SaveToFileForm instance
  method BuildSaveAsFrame {top} {
    set saveFr $top.save
    SaveToFileForm $top.save $self

    return $saveFr
  }

  ## @brief Construct the LoadFromFileForm
  #
  # @param top  the parent widget
  #
  # @returns the name of the LoadFromFileForm instance
  method BuildLoadFrame {top} {
    set loadFr $top.load
    LoadFromFileForm $top.load
    set load [LoadFromFilePresenter %AUTO% $_controlPrsntr -view $top.load]

    return $loadFr
  }


  ## @brief Construct the ChannelEnableDisable controls
  #
  # @param top  the parent widget
  #
  # @returns the name of the ChannelEnableDisableView widget
  method BuildEnableDisableFrame {top} {
    set enableFr $top.enable
    ChannelEnableDisableView $top.enable ;# the view
    
    # construct the presenter and pass the view in to it... this is a more
    # loosely-couopled and testable form of the MVP design.
    set _enable [ChannelEnableDisablePresenter %AUTO% -view $top.enable \
                                                      -handle $_handle]

    return $enableFr
  }

  method BuildOrPatternFrame {top} {
    set orPatternFr $top.orPattern
    ORPatternConfiguration $orPatternFr -handle $_handle
    return $orPatternFr
  }

  ## @brief Construct the contextual info for the chosen protocol
  #
  # Depending on whether the user is speaking over usb or mxdcrcbus, this will
  # display the appropriate connection information.
  #
  # @todo If this is to become more complicated, consider factoring the logic of
  # this method out into a separate snit::type.
  #
  # @param top  the parent widget
  #
  # @returns the name of the ttk::frame containing the info
  method constructInfoFrame {top} {
    set paramDict [array get [$self cget -optionarray]]
    set protoLbl ""
    ttk::frame $top.info -style "Info.TFrame"
    if {[dict get $paramDict -protocol] eq "usb"} {
      set protoLbl "Protocol : USB"
      set serialFile "Serial file : [dict get $paramDict -serialfile]"

      ttk::label $top.info.protoLbl -text $protoLbl
      ttk::label $top.info.serialFile -text $serialFile

      grid $top.info.protoLbl $top.info.serialFile -sticky nsew
      grid columnconfigure $top.info {0 1} -weight 1
    } elseif {[dict get $paramDict -protocol] eq "mxdcrcbus"} {
      set protoLbl "Protocol : MxDC-RCbus"
      set module "Module name : [dict get $paramDict -module]"
      set host "Server host : [dict get $paramDict -host]"
      set port "Server port : [dict get $paramDict -port]"
      set devno "Device address : [dict get $paramDict -devno]"

      ttk::label $top.info.protoLbl -text $protoLbl
      ttk::label $top.info.module -text $module
      ttk::label $top.info.host -text $host
      ttk::label $top.info.port -text $port
      ttk::label $top.info.devno -text $devno

      grid $top.info.protoLbl $top.info.host -sticky nsew
      grid $top.info.module $top.info.port -sticky nsew
      grid $top.info.devno x -sticky nsew
      grid columnconfigure $top.info {0 1} -weight 1
    } else {
      set protoLbl "TEST MODE - NO HARDWARE INTERACTION"
      ttk::label $top.info.protoLbl -text $protoLbl
      grid $top.info.protoLbl -sticky nsew
      grid columnconfigure $top.info {0} -weight 1
    }


    return $top.info
  }


  ## @brief Callback for the "Save as..." menu button
  # 
  method ToSaveAs {} {
    $_sequencer select save
  }

  ## @brief Callback for the "Load settings..." menu button
  #
  method ToLoad {} {
    $_sequencer select load
  }

  ## @brief Callback for the "Enable/disable..." menu button
  #
  method ToEnableDisable {} {
    $_sequencer select enable
  }

  ## @brief Callback for the "OR Pattern..." menu button
  #
  method ToORPatternConfig {} {
    $_sequencer select orPattern
  }

  ## @brief Getter for the device handle
  #
  # @returns the name of the MCFD16 device driver in use
  method GetHandle {} {
    return $_handle
  }

  ## @brief Getter for the MCFD16ControlPanel instance
  #
  # @returns the name of the MCFD16ControlPanel in use
  method GetControlPresenter {} {
    return $_controlPrsntr
  }

  method GetEnableDisablePresenter {} {
    return $_enable
  }

  method GetOrPatternPresenter {} {
    return $_orPatternFr
  }

  ## @brief Getter for the MCFD16AppOptions 
  #
  # @returns the instance of the options that this snit::type delegates to.
  method GetOptions {} {
    return $_options
  }

  method loadTestState {} {
    for {set ch 0} {$ch <= 16} {incr ch} {
      $_handle SetThreshold $ch 100
    }
    for {set ch 0} {$ch <= 8} {incr ch} {
      $_handle SetPolarity $ch neg
      $_handle SetGain $ch 1 
      $_handle SetWidth $ch 100 
      $_handle SetDeadtime $ch 30
      $_handle SetDelay $ch 3 
      $_handle SetFraction $ch 20 
    }

    $_handle SetMode individual
    $_handle SetChannelMask 255
  }

  ## @brief Load a config file into the GUI
  #
  # @param top  the parent widget
  #
  # @returns the name of the LoadFromFileForm instance
    method LoadConfigFile {path} {
	LoadFromFilePresenter loader $_controlPrsntr
	loader Load $path
  }

}

