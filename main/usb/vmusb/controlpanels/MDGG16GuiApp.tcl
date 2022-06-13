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

package provide mdgg16guiapp 1.0

package require snit
package require mdgg16gui
package require mdgg16proxy

## @brief A convenience snit::type for encapsulating the options
#
snit::type MDGG16AppOptions {
  option -module  -default {}
  option -host  -default localhost
  option -port  -default 27000
  option -configfile -default ""

  constructor {args} {
    $self configurelist $args
  }
}


## @brief The main snit::type that stitches all of the pieces together
#
# This owns the device handle, main view, and main presenter. It manages the set
# up of the menu bar by adding a "Save as..." button to it. It also handles the
# logic associated with the "Save as..." operation.
#
# Usage of this snit::type in a larger application is currently discouraged. It
# _should_ work fine as long as there is no other menu bar in the toplevel it is
# being embedded into. Should the user want to build a more complicated widget
# that manages no only the MDGG16 but other devices in a single Tk application,
# then it would be more appropriate to use this snit::type as a guide for how to
# assemble a functional application. they should just instantiate a new
# MDGG16View, MDGG16Presenter, and MDGG16Proxy of their own to add into their
# application.
#
snit::type MDGG16GuiApp {

  option -name -default {.view} ;#!< name to give view widget

  variable _menu ;#!< the main menu object

  component _options ;#! the option object
  component _proxy   ;#! the device handle  (probably an MDGG16Proxy)
  component _view    ;#! the main view
  component _presenter ;#! the presenter that controls the main view

  delegate option * to _options ;#!< All options should go to the 
                                ;#   MDGG16AppOptions

  ## @brief Construct, intialize components, set up
  #
  # This will install the components. A failure while constructing them is
  # fatal and this will bail with a message.
  # 
  # @param args   option value pairs to parse
  #
  constructor {args} {
    install _options using MDGG16AppOptions %AUTO%
    $self configurelist $args

    set res [catch {
      install _proxy using MDGG16Proxy %AUTO% -server [$self cget -host] \
        -port [$self cget -port] \
        -module [$self cget -module]

      install _view using MDGG16View [$self cget -name]
      install _presenter using MDGG16Presenter %AUTO% -view [$self cget -name] \
        -handle $_proxy

      if {[$self cget -configfile] ne {}} {
        $_presenter LoadStateFromFile [$self cget -configfile]
        $self SetWindowTitle "MDGG-16 Controls - [file tail [$self cget -configfile]]"
      }
    } msg]
    if {$res} {
      puts "MDGG16GuiApp failed to construct with error : $msg"
      puts "Exiting..."
      exit
    }

    $self configureMenu

    grid [$self cget -name] -sticky nsew -padx 8 -pady 8
    grid rowconfigure . 0 -weight 1
    grid columnconfigure . 0 -weight 1

  }

  ## @brief Destroy the components
  #
  destructor {
    catch {$_options destroy}
    catch {$_proxy destroy}
    catch {destroy $_view}
    catch {destroy $_menu}
    catch {$_presenter destroy}
  }


  ## @brief Set the menu on the toplevel widget to be this.
  #
  # At the moment, this will just override any existing menu. This should be
  # fine unless other people are looking to embed this with other megawidgets.
  # In which case, they can just build it themselves from the MDGG16View and
  # MDGG16Presenter. This instance is probably not the correct thing to use.
  #
  # @todo support proper handling of an existing menu on the toplevel.
  #
  method configureMenu {} {

    set top [winfo toplevel [$self cget -name]]
    if {$top eq "."} {
      set _menu [menu ${top}menu]
    } else {
      set _menu [menu $top.menu]
    }
    $_menu add command -label "Save" -command [mymethod Save]
    $_menu add command -label "Save as..." -command [mymethod SaveAs]

    . configure -menu .menu
  }

  ## @brief Logic to handle the "Save as..." operation
  #
  # This method just dispatches to the presenter after acquiring a path name.
  #
  method Save {} {
    if {[$self cget -configfile] eq {}} {
      set path [tk_getSaveFile -confirmoverwrite 1 -title {Save as} ] 
      if {$path ne {}} {
        $self configure -configfile $path
        $_presenter SaveCurrentStateToFile [$self cget -configfile]
        $_view SetStatus "Saved to [file tail [$self cget -configfile]]"
        $self SetWindowTitle "MDGG-16 Controls - [file tail [$self cget -configfile]]"
      } else {
        $_view SetStatus "Save operation cancelled."
      }
    } else {
      $_presenter SaveCurrentStateToFile [$self cget -configfile]
      $_view SetStatus "Saved to [file tail [$self cget -configfile]]"
      $self SetWindowTitle "MDGG-16 Controls - [file tail [$self cget -configfile]]"
    }
  }

  ## @brief Logic to handle the "Save as..." operation
  #
  # This method just dispatches to the presenter after acquiring a path name.
  #
  method SaveAs {} {
    set path [tk_getSaveFile -confirmoverwrite 1 -title {Save as} ] 
    if {$path ne {}} {
      $_presenter SaveCurrentStateToFile $path
      $_view SetStatus "Saved to [file tail $path]"
    } else {
      $_view SetStatus "Save operation cancelled."
    }
  }

  method SetWindowTitle {title} {
    set top [winfo toplevel [$self cget -name]]
    wm title $top $title
  }

  method GetWindowTitle {} {
    set top [winfo toplevel [$self cget -name]]
    return [wm title $top]
  }

}
