#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2015.
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


package provide mscf16guiapp 1.0

package require snit
package require mscf16usb
package require mscf16gui
package require mscf16statesaver
package require mscf16fileloader


## @brief Options for the MSCF16Control app.
#
# This encapsulates the options so that they could in principle be passed 
# around more easily.
#
snit::type MSCF16AppOptions {

  option -serialfile -default {}
  option -configfile -default {}
  option -widgetname -default {.app}

  constructor {args} {
    $self configurelist $args
  }
}

## @brief Toplevel snit::type that holds all the piece of the MSCF16Control
#
# This owns the handle, presenter, and view. It also deals with managing of the
# global data such as the channel names. At startup and shutdown of the
# application, this will store that state of these names to a user specified
# file.
#
snit::type MSCF16GuiApp {

  component _options    ;#!< the MSCF16AppOptions
  component _view       ;#!< a view
  component _presenter  ;#!< presenter for the view
  component _handle     ;#! a device handle

  # all option management is handled by the MSCF16AppOptions snit::type
  delegate option * to _options

  ## @brief COnstruct all of the pieces..
  #
  # The device handle, view, and presenter are all created so that they can work
  # together.  Furthermore, the config file for the channel names ia read if it
  # exists to restore previous channel names.  Failure during this cause the
  # whole application to halt "gracefully".
  #
  # @param args   list of option-value pairs
  constructor {args} {
    install _options using MSCF16AppOptions %AUTO%
    $self configurelist $args
    set res [catch {
      install _handle using MSCF16USB %AUTO% [$self cget -serialfile]
      install _view using MSCF16Form [$self cget -widgetname]

      install _presenter using MSCF16Presenter %AUTO% -view $_view \
                                     -handle $_handle

      $self SetUpMenu 

      # read in the names of the channels.
      if {[$self cget -configfile] ne {}} {
        $self LoadChannelNames [$self cget -configfile]
        $_view SetStatus "Channel names loaded from [$self cget -configfile]"
      }
    } msg]
    
    if {$res} {
      puts "Error during application startup. Reason given : \"$msg\""
      exit
    }

  }

  ## @brief Destroy all owned state and save channel names to file
  #
  destructor {
    # save state to file

    catch {$self SaveState [$self cget -configfile]}

    catch {$_options destroy}
    catch {$_handle destroy}
    catch {destroy $_view}

  }
  ## @brief Write the existing channel names to a file
  #
  # This not only writes the channel names, but also timestamps the creation of
  # the file.
  #
  #  @param path  path to the file to save
  #
  method SaveChannelNames {path} {
    set f [open $path w]
    for {set i 0} {$i < 16} {incr i} {
      chan puts $f [set ::MSCF16ChannelNames::chan$i] 
    }
    chan puts $f "Generated on [clock format [clock seconds]]"
    close $f
    exit
  }

  ## @brief Build the menu
  # 
  # Adds the File and Configure drop-down menus.
  #
  # @todo Make this capable of building a menu if the toplevel is not ".".
  #
  method SetUpMenu {} {

    option add *tearOff 0

    # get the menu for the toplevel
    set menu [[winfo toplevel [$self cget -widgetname]] cget -menu]
    if {$menu eq ""} {
      set m .menu
      menu $m
      menu $m.file 
      $m.file add command -command [mymethod ToSaveAs] -label "Save as..."
      $m.file add command -command [mymethod ToLoad] -label "Load settings..."

#      menu $m.config
#      $m.config add command -command [mymethod ToEnableDisable] -label "Enable/disable..."

      $m add cascade -menu $m.file -label "File"
#      $m add cascade -menu $m.config -label "Configure"

      . configure -menu $m
    }
  }

  method ToSaveAs {} {
    set path [tk_getSaveFile -confirmoverwrite 1 -defaultextension ".tcl" \
                    -title {Save as} ] 
    if {$path ne ""} {
      $self SaveState $path
      $_view SetStatus "State saved as [file tail $path]"
    }
  }

  method SaveState {path} {
    set saver [MSCF16StateSaver %AUTO% $_options $_presenter]
    $saver SaveState $path
    $saver destroy
  }

  method ToLoad {} {
    set path [tk_getOpenFile -defaultextension ".tcl" \
                    -title {Choose file to load} ] 
    if {$path ne ""} {
      $self LoadDeviceState $path
      $self LoadChannelNames $path
      $_view SetStatus "State loaded from [file tail $path]"
    }
  }

  method LoadDeviceState {path} {
    set devLoader [MSCF16FileLoader %AUTO% $_options $_presenter]
    $devLoader Load $path
    $devLoader destroy
  }

  ## @brief Attempt to read in config file with channel names
  #
  # This simply returns if the file at the path provided cannot be opened. So
  # the user understands what happened, it presents them with a dialogue
  # explaining what will happen.
  # 
  # @param path   path to the channel config
  #
  method LoadChannelNames {path} {
    set nameLoader [MSCF16NameLoader %AUTO%]
    $nameLoader Load $path
    $nameLoader destroy
  }


  method GetOptions {} {
    return $_options
  }

  method GetPresenter {} {
    return $_presenter
  }
  method SetPresenter {pres} {
    set oldPresenter $_presenter
    set _presenter $pres
    return $oldPresenter
  }

  method GetHandle {} {
    return $_handle
  }

  method SetHandle {handle} {
    set oldHandle $_handle
    set _handle $handle
    return $oldHandle
  }
} ;# end of MSCF16GuiApp snit::type
