#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file ui.tcl
# @brief Readout GUI User interface elements.
# @author Ron Fox <fox@nscl.msu.edu>
catch {package require Tk};    # In batch tests there's no display!
package require snit
package require RunstateMachine
package require StateManager
package require DataSourceUI
package require dialogwrapper
package require img::png

package provide ui   1.0
package provide ReadoutGUIPanel 1.0
package require Utils
package require textprompter


namespace eval ReadoutGUIPanel {
    
}

snit::macro getDefaultLogging {} {
    return  [dict create \
        output [list]           \
        log [list]              \
        error [list -background white -foreground red] \
        warning [list -foreground magenta]              \
    ]
}

##
# ReadoutGUIPanel::defaultLogging
#   Return the default value for -showlog
#
# @return dict
#
proc ::ReadoutGUIPanel::getDefaultLogging {} {
    return [dict create \
        output [list]           \
        log [list]              \
        error [list -background white -foreground red] \
        warning [list -foreground magenta]              \
    ]
}
    

#------------------------------------------------------------------------------
# Menubar:
#


##
# @class readoutMenubar
#
#  This class provides a framework for managing the ReadoutGUI's menubar.
#  The class is used by both the standard UI and the the API to make the
#  ReadoutGUI menu and maintain it.
#
# METHODS:
#   addMenu     - Add a new menu. This is a cascade command on the
#                 hull.
#   lookupMenu  - Look up a menu widget path by the text name.
#   listMenus   - returns the names of all menus.
#   addSeparator- Add a separator to to a menu
#   addCommand  - Add a command item to a menu
#   addMenuItem - Add a generic menu item to a menu (when the above
#                 convenience methods don't work.
#
# OPTIONS
#   All options supported by the menu tk command are supported.
snit::widgetadaptor readoutMenubar {
    
    delegate option * to hull
    delegate method * to hull
    
    # This variable contains a lookup table for the submenus
    # The index is the text in the menu (e.g. File) the value is the
    # menu widget path.
    #
    variable menus -array [list]
    variable menucount 0
    
    ##
    # constructor
    #
    #  Installs the hull as a menu.
    #
    # @param args - configuration parameters..
    #
    constructor args {
        installhull using menu -tearoff 0
        
        $self configurelist $args
    }
    
    #--------------------------------------------------------------------------
    #
    # Public methods.
    #
    
    ##
    #   addMenu
    #     Add a new menu. This is a cascade command on the hull.
    #
    # @param text - The menu item label.
    #
    # @note the menu widget is assigned and the menu is added to the menus
    #       array.
    #
    method addMenu text {
        set widget [menu $win.menu[incr menucount] -tearoff 0]
        $hull add cascade -label $text -menu $widget
        
        set menus($text) $widget
    }
    ##
    #   lookupMenu
    #      Look up a menu widget path by the text name.  This is provided in
    #      case there are special needs that are not met by the methods provided.
    #
    # @param text         - The label on the drop down menu.
    # @return widget-path - The path to the menu that is associated with that
    #                       cascade. If no menu with this label exists,
    #                       an empty string is returned.
    #
    method lookupMenu text {
        if {[array names menus $text] ne ""} {
            return $menus($text)
        } else {
            return ""
        }
    }
    ##
    # listMenus
    #    Lists the names of all  menus, in no particular order.
    #
    # @return list
    #
    method listMenus {} {
        return [array names menus]
    }
    ##
    #   addSeparator
    #      Add a separator to to a menu
    #
    # @param name - Name of the menu.
    # @throw If the name does not correspond to an existing menu.
    #
    method addSeparator name {
        $self addMenuItem $name separator
    }
    ##
    #   addCommand
    #     Add a command item to a menu
    #
    # @param name    - Menu name.
    # @param label   - Label for the command item.
    # @param command - script to run when menu itme is clicked.
    #
    method addCommand {name label command} {
        $self addMenuItem $name command -label $label -command $command
    }
    ##    
    #   addMenuItem - Add a generic menu item to a menu (when the above
    #                 convenience methods don't work.
    # @param name - The name of the menu in which the add is done.
    # @param itemtype - The type of entitity that is being added.
    # @param args - The -option value pairs that further describe the item.
    #
    method addMenuItem {name itemtype args} {
        set menu [$self lookupMenu $name]
        if {$menu eq ""} {
            error "There is no menu labeled $name"
        }
        $menu add $itemtype {*}$args
    }
}

#------------------------------------------------------------------------------
#  API elements related to menus.
# ASSUMPTIONS:
#   *    The menu object has been instantiated and associated with the toplevel.
#   *    The main window is the toplevel.
#



##
# ::ReadoutGUIPanel::addUserMenu
#
#  Adds a new user menu.
#
# @param ident - Ignored but present for compatibility sake.
# @param label - The Label on the menu.
#
# @return Window path to the menu item.
#
proc ::ReadoutGUIPanel::addUserMenu {ident label} {
    set menu [. cget -menu]
    $menu addMenu $label
    return [$menu lookupMenu $label]
}


#-------------------------------------------------------------------------------
#
#  Run identification widget and API  - Run number and title

##
# @class RunIdentification
#
#   This widget supplies an entry for the run title and a second for providing
#   the run number.  The run number is validated to ensure it is an integer >= 0.
#   validation is done when we are reasonably certain the run number has been
#   completely entered (on focus change not on every keystroke).
#   If an invalid run number is entered, a dialog pops and the old run number
#   is restored.
#
#   Note that since not all data sources can set a title or run number,
#   -haverun and -havetitle allow the client to run off those widgets.
#
# OPTIONS:
#    -haverun   - True if the run number widget should be displayed.
#                 false otherwise.
#    -havetitle - True if the title should be displayed else false.
#
#    -title     - Current title (read/write).
#    -run       - Current run number (read/write) subject to validation.
#    -state     - State to set the subwidgets.   This is used to disable
#                 modification of the widgets when a run is going on
#                 (Active or Paused).
#
snit::widgetadaptor RunIdentification {
    option -haverun   -default 1   -configuremethod _rebuild
    option -havetitle -default 1 -configuremethod _rebuild
    option -state     -default normal -configuremethod _setState
    
    option -title -default ""   -configuremethod _setTitle -cgetmethod _getTitle
    option -run   -default "0"  -configuremethod _setRun   -cgetmethod _getRun
    
    
    variable lastRunNumber 0
    
    ##
    # constructor
    #
    #   Install the hull as a ttk::frame and process the configuration.
    #   once processed, invoke _rebuild to ensure the widget gets displayed
    #   correctly (because I'm honestly not sure the configure methods will get
    #   called for default values of the options).
    #
    #  @parameter args - the -option values  that modify the initial default
    #                    configuration of the widget.
    #
    constructor args {
        installhull using ttk::frame
        $self configurelist $args
        $self _rebuild -haverun $options(-haverun)
    }
    #--------------------------------------------------------------------------
    # Configuration processing (private).
    
    ##
    # _rebuild
    #
    #   Called when either -haverun or -havetitle are modified.
    #   In this case the widget contents must be rebuilt depending on the new
    #   values of those options.
    #
    # @param optname - Name of the new option.
    # @param value   - New value (must be boolean).
    #
    method _rebuild {optname value} {
        if {![string is boolean -strict  $value]} {
            error "$optname can only take valid boolean values: '$value' is not boolean"
        }
        set options($optname) $value
        
        #  Destroy the existing widgets
        
        foreach widget [winfo children $win] {
            destroy $widget
        }
        
        #  Create the appropriate set:
        
        set havetitle $options(-havetitle)
        set haverun   $options(-haverun)
        set widgets   [list]
        
        if {$havetitle} {
            lappend widgets [textprompt $win.title          \
                -text Title -width 60 -validate key        \
                -validatecommand [mymethod _LimitTitle %P ] \
            ]
            #lappend widgets [ttk::label $win.tlabel -text Title: ]
            #lappend widgets [ttk::entry $win.title -width 60 -validate key -validatecommand [mymethod _LimitTitle %P]]
            $win.title insert end $options(-title)
        }
        if {$haverun} {
            lappend widgets [textprompt $win.run                 \
                -text {Run Number:} -width 6 -validate focusout \
                -validatecommand [mymethod _validate]           \
            ]
            #lappend widgets [ttk::label $win.rlabel -text {Run Number:} ]
            #lappend widgets [ttk::entry $win.run \
            #    -width 6 -validate focusout \
            #    -validatecommand [mymethod _validate]]
            $win.run insert end $options(-run)
            bind $win.run <Return> [mymethod _validate]
        }
    
        
        # Grid them in a horizontal strip;
        
        if {[llength $widgets] > 0} {
            grid {*}$widgets -sticky new
        }
        grid rowconfigure $win 0 -weight 0
        
    }
    ##
    # _LimitTitle
    #
    #  Called when something interesting has happend with the title.  We ensure the title
    #  is no more than 75 characters long (that's actually giving a bit of slop as I think)
    #  the title length limit is 79.
    #
    # @param proposedString The new title sdtring if accepted.
    # @return boolean true  if the resulting string is an acceptable length.
    #
    method _LimitTitle proposedString {
        if {[string length $proposedString] <= 75} {
            return 1
        } else {
            bell
            return 0
        }
    }
    
    ##
    #  _setState
    #
    #    Called to process a change in the widget state.  Note that only
    #    entry classed objects will be modified.
    #
    #  @param optname - name of the option being modified (-state).
    #  @param value   - new state.  Must be one of 'normal', 'disabled',
    #                   or readonly.  See ttk::entry(3tk) for the meaning
    #                   of these states.
    # 
    method _setState {optname value} {
        set allowedStates [list normal disabled readonly]
        if {$value ni $allowedStates} {
            error "$opname must be one of {[join $allowedStates {, }]} was $value"
        }
        foreach widget [winfo children $win] {
            if {[winfo class $widget] eq "TEntry"} {
                $widget configure -state $value
            }
        }
        set options($optname) $value
    }
    ##
    # _setTitle
    #   Set a new title for the widget.  Note this just saves the title if
    #   -havetitle is not true.
    #
    # @param optname  Name of the option being set (-title).
    # @param value    New value.
    #
    method _setTitle {optname value} {
        set options($optname) $value
        if {$options(-havetitle)} {
            $win.title delete 0 end
            $win.title insert end $value
        }
    }
    ##
    # _setRun
    #   set a new run number for the widget.  Note this just saves the
    #   run number if -haverun is not true.
    #
    # @param optname - Name of the option being configured (-run)
    # @param value   - New run number which must be an integer >= 0.
    #
    method _setRun   {optname value} {
        if {![string is integer -strict $value] || ($value < 0)} {
            error "$optname must be an integer >= 0 and was '$value'"
        }
        set options($optname) $value
        set lastRunNumber $value
        if {$options(-haverun)} {
            $win.run delete 0 end
            $win.run insert end $value
        }
    }
    ##
    # _getTitle
    #    If there is a title widget, the string is read from it into
    #    options($optname).  Regardless, the value of $options($optname) is
    #    returned.
    #
    # @param optname - the name of the option being queried (-title).
    # @return string - The current title value.
    #
    method _getTitle optname {
        if {$options(-havetitle)} {
            set options($optname) [$win.title get]
        }
        return $options($optname)
        
    }
    ##
    # _getRun
    #
    #   If there is a run number widget, the string is read from it into
    #   options($optname) regardless, $options($optname) is returned.
    #
    # @param optname - The option being fetched.
    # @return integer - Run number.
    #
    method _getRun   optname {
        if {$options(-haverun)} {
            set options($optname) [$win.run get]
        }
        return $options($optname)
    }
    #--------------------------------------------------------------------------
    # Action handlers
    #
    method _validate {} {
        set now [$win.run get]
        if {![string is integer -strict $now] || ($now < 0)} {
            tk_messageBox -title {Bad Run Number} -icon error \
                -message {Invalid run number - must be an integer >= 0} \
                -type ok -parent $win
            $win.run delete 0 end
            $win.run insert end $lastRunNumber
            return 0
        } else {
            set lastRunNumber $now
            return 1
        }
    }
}
#------------------------------------------------------------------------------
# Run identification API.  We need to cooperate with the
# main application to make this work.. ReadoutGUIPanel::runIdWidget must be
# set to the widget path of the RunIdentification widget.
#
namespace eval ::ReadoutGUIPanel {
    variable runIdWidget ""
}

##
#  ::ReadoutGUIPanel::getRunIdInstance
#
# return/create the runid instance.
#
# @param widget (only looked at the first time)
# @return widget path - to the run id widget.
#
proc ::ReadoutGUIPanel::getRunIdInstance {{widget {}}} {
    if {$::ReadoutGUIPanel::runIdWidget eq ""} {
        set ::ReadoutGUIPanel::runIdWidget [RunIdentification  $widget]
    }
    return $::ReadoutGUIPanel::runIdWidget
}

##
# ::ReadoutGUIPanel::ghostWidgets
#
#    Disables all the widgets that need to be turned off when he run is
#    either paused or active
#
# TODO:  Be sure to also get the recording enable checkbox when that's added.
#
proc ::ReadoutGUIPanel::ghostWidgets {} {
    $::ReadoutGUIPanel::runIdWidget configure -state readonly
}
##
# ::ReadoutGUIPanel::unghostWidgets
#
#  Enables all widges that need to be turned on when the run is halted.
#
#  TODO: Handle the recording checkbutton.
# 
proc ::ReadoutGUIPanel::unghostWidgets {} {
    $::ReadoutGUIPanel::runIdWidget configure -state normal
}
##
# ::ReadoutGUIPanel::getTitle
#
#   Return the run title
#
# @return string - the current run title string.
#
proc ::ReadoutGUIPanel::getTitle {} {
    return [$::ReadoutGUIPanel::runIdWidget cget -title]
}
##
# ::ReadoutGUIPanel::setTitle
#
#   Set a new title string in the widget:
#
# @param title - The new title string.
#
proc ::ReadoutGUIPanel::setTitle title {
    $::ReadoutGUIPanel::runIdWidget configure -title $title
}
##
# ::ReadoutGUIPanel::getRun
#
#  @return - The current run number.
#  @note   - If this is called early -- I mean really early,
#            during e.g. bundle load time, the runIdWidget might
#            not exist yet..
#            in that case, return -1... an invalid run number.
#
proc ::ReadoutGUIPanel::getRun   {} {
    if {$::ReadoutGUIPanel::runIdWidget eq ""} {
        return -1
    }
    return [$::ReadoutGUIPanel::runIdWidget cget -run]
}
##
#  ::ReadoutGUIPanel::setRun
#
#   Set a new run number.  Note that validation is done by the widget so
#   I'm not going to be concerned with that here.
#
# @param run - The new run number.
#
proc ::ReadoutGUIPanel::setRun run {
    $::ReadoutGUIPanel::runIdWidget configure -run $run
}
##
# ::ReadoutGUIPanel::incrRun
#
#  Adds 1 to the current run number.
#
proc ::ReadoutGUIPanel::incrRun {} {
    set now [::ReadoutGUIPanel::getRun]
    incr now
    ::ReadoutGUIPanel::setRun $now
}

##
# ErrorReporter widget
#
# A dialog that will take a list of errors associated with each callout bundle
# and organize them into a treeview. The error list is passed in as an option
# called -errorlist. For example, an error list might look like:
#
# @code
# [list [list bundle1 [list "bundle1 error #0" "bundle1 error #1"]] [list bundle2 [list "bundle2 error"]]]
# @endcode
#
# When a begin transition is attempted, the transition is prechecked for detectable 
# error conditions and if any are found, this dialog displays the error conditions.
#
# This is not a modal dialog because that allows the user to investigate without 
# being blocked.
#
snit::widget ErrorReporter {
  hulltype toplevel

  option -errorlist -default [list]


  constructor {args} {

    $self configurelist $args 

    $self _buildWidget
  }

  ## Assemble the megawidget
  method _buildWidget {} {

    wm title ${win} "Detected Errors"

      set msg "The errors listed below have been found. Please correct \nthese before attempting a transition."
      ttk::label ${win}.info -text $msg

      scrollbar ${win}.xscroll -orient horizontal
      ttk::treeview ${win}.msg -xscrollcommand "${win}.xscroll set"
      ${win}.xscroll configure -command "${win}.msg xview"

      foreach entry $options(-errorlist) {
        set bundleName [lindex $entry 0]
        set errors [lindex $entry 1]
        set id [${win}.msg insert {} end -text $bundleName -open true]


        foreach msg $errors {
          ${win}.msg insert $id end -text $msg
        }
      }

      ${win}.msg column #0 -minwidth 1000 -stretch true

      ttk::frame ${win}.buttons
      ttk::button ${win}.buttons.dismiss -text "Dismiss" -command [mymethod _close]
      grid ${win}.buttons.dismiss -sticky e

      grid ${win}.info -padx 9 -pady 9 -sticky new
      grid ${win}.msg -padx 9 -pady 9 -sticky nsew
      grid ${win}.xscroll -sticky new
      grid ${win}.buttons -padx 9 -pady 9 -sticky sew 

      grid columnconfigure ${win} all -weight 1
      grid rowconfigure ${win} {1} -weight 1

  }

  ## Callback for the dismiss button to close the dialog
  method _close {} {
    destroy ${win}
  }
}


#-----------------------------------------------------------------------------
#
#  The next widget and API are involved in providing a UI for the run
#  state machine.
#


##
# @class RunControl
#    This megawidget provides a control panel for the run state machine.
#    buttons are enabled and disabled depending on the actual run state.
#    as reflected by the run state machine singleton.  We therefore
#    register methods as callout bundles in order to be notified of state
#    transitions so that the UI can be updated.
#    This going to be a pair of snit clases:
#   * RunControl - which provides the widget.
#   * RunControlSingleton - which provides a way for the RunstateMachine
#     callback bundle to locate the object to trigger state changes.
#     RunControlSingleton will be responsible for registering the callback
#     bundle as well.
#
# LAYOUT:
#    +-----------------------------------------------------+
#    |                [Start (button)]                     |
#    | [Begin (button)]                 [Pause (button)]   |
#    |                 [ ] Record                          |
#    +-----------------------------------------------------+
#
# OPTIONS:
#    -pauseable     - Boolean indicating if the pause button should be displayed.
#    -recording     - State of the record checkbox.
#    -state         - Set the state of the widgets (disabled/normal)
#
#  @note - In order to support more than one module wanting to control
#          the -state, there is a disable counter so that:
#
# <verbatim>

#   .buttons configure -state disabled
#    ...
#   .buttons  configure -state disabled
#   ...
#   .buttons  configure -state normal;    # Still disabled.
#   ...
#   .buttons configures -state normal;    # _Now_ normal state.
#   
#
snit::widgetadaptor RunControl {
    
    option -pauseable -default 1 -configuremethod _changePauseVisibility
    option -recording -default 0
    option -state     -default normal -configuremethod _setState
    
    #
    #  The variable below is an array indexed by state the values are two
    #  element lists of widget subnames and the state of each widget when
    #  we are in that state.
    #
   variable widgetStates -array [list \
        NotReady [list {start normal} {beginend disabled} {pauseresume disabled} \
                  {record normal}]                                                \
        Starting [list {start disabled} {beginend disabled} {pauseresume disabled} \
                  {record normal}]                                                \
        Halted   [list {start disabled} {beginend normal} {pauseresume disabled}  \
                  {record normal}]                                                \
        Active   [list {start disabled} {beginend normal} {pauseresume normal}    \
                  {record disabled}]                                              \
        Paused   [list {start disabled} {beginend normal} {pauseresume normal}     \
                    {record disabled}]                                            \
    ]
     
    ##
    #  The variable below contains an array indexed by state whose values
    #  are lists of 2 element sublists.  Each sublist contains the subname of a
    #  widget and the -text value it should have in that state.
    #
    
    variable widgetLabels -array [list   \
        NotReady [list {beginend Begin} {pauseresume Pause}]                \
        Starting [list {beginend Begin} {pauseresume Pause}]                \
        Halted   [list {beginend Begin} {pauseresume Pause}]                \
        Active   [list {beginend End}   {pauseresume Pause}]                \
        Paused   [list {beginend End}   {pauseresume Resume}]               \
    ]
    
    variable slave 0
    
    variable disableCount 0;                 # Support for nested disable.
    
    ##
    # constructor
    #
    #   Construct and configure the widget.
    #   - Install a ttk::frame as the hull.#
    #   - create and layout the widgets.
    #   - configure any options provided at instantiation time.
    #
    # METHODS
    #     slave - Set slave mode by making the buttons go away
    #     master- Set master mode by relaying out the UI.
    #     isSlave - Return slave state.
    #
    # @param args - The -option value configuration options provided at
    #               instantiation time.
    #
    constructor args {
        installhull using ttk::frame
        
        ttk::button $win.start       -text Start -command [mymethod _start]
        ttk::button $win.beginend    -text Begin -command [mymethod _beginend]
        ttk::button $win.pauseresume -text Pause -command [mymethod _pauseresume]
        ttk::checkbutton $win.record -text Record -onvalue 1 -offvalue 0 \
            -variable [myvar options(-recording)]
        ttk::label $win.slavemode -text "Currently  in slave mode (NotReady)"
        
        $self _layoutWidgets
        
        $self _updateAppearance
        
        $self configurelist $args
        
    }
    ##
    # destructor
    #
    destructor {
    }
    
    method slave {} {
        if {!$slave} {
            set slave 1
            grid forget  $win.start $win.beginend $win.record $win.pauseresume
            grid $win.slavemode
        }
    }
    method master {} {
        if {$slave} {
            set slave 0
            grid forget $win.slavemode
            $self _layoutWidgets
            $self _changePauseVisibility -pauseable $options(-pauseable)
            $self _updateAppearance
        }
        
    }
    method isSlave {} {
        return $slave
    }
    #---------------------------------------------------------------------------
    #  Configuration methods
    
    ##
    # _setState
    #    Set begin/end/paus button states.
    #

    method _setState {optname value} {

        set setState 0;             # Assume we make no state change.
        if {$value eq "disabled"} {
            incr disableCount
            set setState 1
        } elseif {$value eq "normal"} {
            incr disableCount -1
            if {$disableCount <= 0} {
                set setState 1
                set disableCount 0;             # In case they go too far.
            }
        } else {
            error "Invalid state must be disabled or normal was $value"
        }

        if {$setState} {
            foreach b [list $win.beginend $win.pauseresume] {
                $b configure -state $value
            }
        }
	set options($optname) $value
    }
    ##
    # _changePauseVisibility
    #
    #   Determines if the pause button is visible.  This can be turned off
    #   if not all data sources can pause a run.
    #
    # @param optname - Name of the configuration option that controls this.
    # @param value   - New value, must be boolean.
    method _changePauseVisibility {optname value} {
        if {![string is boolean -strict $value]} {
            error "$optname takes a boolean value and you gave me $value"
        }
        set options($optname) $value
        
        # See if pause is already gridded:
        
        set gridded [expr {"$win.pauseresume" in [grid slaves $win -row 1]}]

        if {$value} {
            if {!$gridded} {
                grid $win.pauseresume -row 1 -column 1
            }
            
        } else {
            if {$gridded} {
                grid forget $win.pauseresume
            }
        }
    }
    
    #--------------------------------------------------------------------------
    #  Button event handlers.
    
    ##
    # _start
    #   Start the data sources in the RunstateMachine.  Note that
    #   the appearance of the buttons will change only as a result of the callback
    #   bundle eventually invoking the _udpateAppearance method..which will look
    #   at the actual state machine and set button states accordingly.
    # 
    #
    method _start {} {
      set stateMachine [RunstateMachineSingleton %AUTO%]

      RunControlSingleton::updateStateBundleOrder

      start

      $stateMachine destroy 
    }
    ##
    #  _beginend
    #
    #  Invoked when the begin/end button is clicked.  The transition we actually
    #  initiate depends on the current  runstate:
    #  *  Halted -> Active
    #  * {Paused, Active} -> Halted.
    #  * Other    - A bug error.
    # @note The callback bundle will be responsible for invoking _updateAppearance
    #       to change the appearance of the button itself.
    # 
    method _beginend {} {
      set stateMachine [RunstateMachineSingleton %AUTO%]
      set state [$stateMachine getState]

      RunControlSingleton::updateStateBundleOrder

      if {$state eq "Halted"} {
        set responses [$stateMachine precheckTransitionForErrors Active]
        if {[llength $responses] == 0} {
          begin
        } else {
          $self reportTransitionFailures $responses
          }
        } elseif {$state in [list Paused Active]} {
          end
        } else {
          error "ERROR: begin/end button clicked when state is $state which should not happen"
        }

      $stateMachine destroy
    }


    method reportTransitionFailures {responses} {
      
      #toplevel .errors
      #ErrorReporter .errors.dialog -errorlist $responses
      #pack .errors.dialog -expand 1 -fill both
      ErrorReporter .errors -errorlist $responses
    }

    ##
    # _pauseresume
    #
    #  Invoked when the pause/resume button is clicked.
    #  Action depends on the state:
    #  *  Paused -> Active
    #  *  Active -> Paused
    #  * Any other -> Bug error
    method _pauseresume {} {
        set stateMachine [RunstateMachineSingleton %AUTO%]
        set state [$stateMachine getState]
        $stateMachine destroy

        RunControlSingleton::updateStateBundleOrder

        if {$state eq "Paused"} {
          resume
        } elseif {$state eq "Active"} {
          pause
        } else {
          error "ERROR: pause/resume button clicked when state is $state which should not happen"
        }
    }
    
    #---------------------------------------------------------------------------
    #  Other private methods:
    #
    ##
    # _layoutWidgets
    #    Called to do the initial layout of the widgets (modulo pause/resume)
    #
    method _layoutWidgets {} {
        grid $win.start -columnspan 2 -sticky n
        grid $win.beginend $win.pauseresume -sticky n
        grid $win.record -columnspan 2 -sticky n

        grid configure $win -padx 5 -pady 5
        grid rowconfigure $win 0 -weight 0        
        grid rowconfigure $win 1 -weight 0        
        grid rowconfigure $win 2 -weight 0        
        
    }
    ##
    # _updateAppearance
    #   Called to make the widget appearance consistent with the current run
    #   state.  See the variables widgetLabels and widgetStates for more
    #   information.
    #
    method _updateAppearance {} {
        set stateMachine [RunstateMachineSingleton %AUTO%]
        set state [$stateMachine getState]
        $stateMachine destroy
        
        $win.slavemode configure -text "Currently  in slave mode ($state)"
        
        # Widget states:
        
        if {[array names widgetStates $state] eq ""} {
            error "ERROR: State is $state but there's no corresponding set of widget states!"
        }
        foreach stateInfo $widgetStates($state) {
            set widget $win.[lindex $stateInfo 0]
            $widget configure -state [lindex $stateInfo 1]
        }
        
        # Widget labels:
        
        if {[array names widgetLabels $state] eq ""} {
            error "ERROR: State is $state but there's no corresponding set of widget labels"
        }
        foreach labelInfo $widgetLabels($state) {
            set widget $win.[lindex $labelInfo 0]
            $widget configure -text [lindex $labelInfo 1]
        }
    }
}

## A callout bundles to disable the control widget state
#
#  This unconditionally disables the control widgets at the start
#  of any transition.
namespace eval RunControlDisable {

  variable prevState normal

  ##
  #  attach
  #   Called when the bundle is attached to the run control state machine.
  #   Caches the state of the run control buttons.
  #
  # @param to - the current machine state.
  proc attach {to} {
    variable prevState
    set prevState [::RunControlSingleton::getState]
  }

##
#  leave
#   Called when the state machine leaves a state. 
#   Sets the state of the widgets to disabled every
#   time.
#
  proc leave {from to} {
    set rc [RunControlSingleton::getInstance]
    $rc configure -state disabled
  }

  proc enter {from to} {}

  ## 
  # register bundle
  #
  # Inserts the callout bundle at the very start of the callout bundle
  # list.
  proc register {} {
    set sm [RunstateMachineSingleton %AUTO%]
    set bundles [$sm listCalloutBundles]
    $sm addCalloutBundle RunControlDisable [lindex $bundles 0]
    $sm destroy
  }

  ##
  # Unregisters bundle from state machine
  #
  # Besides unregistering, this also sets the state of the 
  # widgets to the state they were at registration. This is just
  # a cleanup step.
  proc unregister {} {
    variable prevState

    RunControlSingleton::setState $prevState
    RunStateMachineConvenience::removeBundle RunControlDisable
    
  }

  namespace export attach leave enter
}

## A callout bundle to enable the control widget state
#
# This unconditionally enables the control widget the the
# end of any transition
#
namespace eval RunControlEnable {

  variable prevState normal

  #  attach
  #   Called when the bundle is attached to the run control state machine.
  # @param to - the current machine state.
  # 
  proc attach {to} {
    variable prevState
    set prevState [::RunControlSingleton::getState]
    set rc [RunControlSingleton::getInstance]
    $rc _updateAppearance
  }

  ##
  #  leave
  #   Called whenn the state machine leaves a state (unused)
  #
  proc leave {from to} {
  }

  ##
  # enter
  #    Called when the state machine enters a new state. 
  #    This only adjusts the appearance if the transition is to
  #    Starting or Halted.
  #
  # @param from - old state.
  # @param to   - Current state
  #
  proc enter {from to} {
    set rc [RunControlSingleton::getInstance]
    $rc _updateAppearance
  }

  ## 
  # register bundle
  #
  # Inserts the callout bundle at the very start of the callout bundle
  # list.
  proc register {} {
    set sm [RunstateMachineSingleton %AUTO%]
    $sm addCalloutBundle RunControlEnable 
    $sm destroy
  }

  ##
  # Unregisters bundle from state machine
  #
  # Besides unregistering, this also sets the state of the 
  # widgets to the state they were at registration. This is just
  # a cleanup step.
  proc unregister {} {
    variable prevState
    RunControlSingleton::setState $prevState
    RunStateMachineConvenience::removeBundle RunControlEnable
    
  }

  namespace export attach leave enter
}


##
#   The functions/namespace below implement the run control singleton pattern.
#
namespace eval ::RunControlSingleton {
    variable theInstance ""
    namespace export attach enter leave
}

##
# Register (or reregister) the RunControlEnable/Disable bundles 
#
# This unregisters and then registers the RunControlEnable and 
# RunControlDisable bundles to ensure that the Disable budnle
# is first and the Enable bundle is last.
#
proc ::RunControlSingleton::updateStateBundleOrder {} {
  catch {RunControlEnable::unregister}
  catch RunControlDisable::unregister
    

  RunControlEnable::register
  RunControlDisable::register

}
##
#  ::RunControlSingleton::getInstance
#
#     Returns the instance of the widget.
#
# @param path - (defaults to .runcontrol)  the widget path of the instance.
# @param args - (defaults to "") additional configuration options.
#
proc ::RunControlSingleton::getInstance {{path ""} args} {
    if {$::RunControlSingleton::theInstance eq ""} {
        set ::RunControlSingleton::theInstance [RunControl $path {*}$args]

       RunControlSingleton::updateStateBundleOrder

    } elseif {[llength $args] > 0} {
        $::RunControlSingleton::theInstance configure {*}$args
    }
    return $::RunControlSingleton::theInstance
}
##
# ::RunControlSingleton::getState
#
#    Get the state of the run control singelton
#
# @return string.
#
proc ::RunControlSingleton::getState {} {
    variable prevState
    set rc [RunControlSingleton::getInstance]
    return [$rc cget -state]

    
}
##
# ::RunControlSingleton::setState
#
#   Set the state of the run control singleton
#
#  @param state  - new state.
#
proc ::RunControlSingleton::setState {state} {
    set rc [RunControlSingleton::getInstance]
    $rc configure -state $state
}



##
#  attach
#   Called when the bundle is attached to the run control state machine.
# @param state - the current machine state.
# 
proc ::RunControlSingleton::attach {state} {
}

##
# enter
#    Called when the state machine enters a new state. 
#    This only adjusts the appearance if the transition is to
#    Starting or Halted.
#
# @param from - old state.
# @param to   - Current state
#
proc ::RunControlSingleton::enter {from to} {
}

##
#  leave
#   Called whenn the state machine leaves a state (unused)
#
proc ::RunControlSingleton::leave {from to} {
}

##
#  API elements for all this fun:
#

##
#  ::ReadoutGUIPanel::recordOff
#
#   Request that the next run not record events.  This can be issued
#   during an active run, however it might give the mistaken impression
#   that recording is active now...The state of the recording checkbox
#   only matters when the run starts.
#
proc ::ReadoutGUIPanel::recordOff {} {
    set r [::RunControlSingleton::getInstance]
    $r configure -recording 0
}

##
# ::ReadoutGUIPanel::recordOn
#
#   Request that the next run record events.  See recordOff above, however
#
proc ::ReadoutGUIPanel::recordOn {} {
    set r [::RunControlSingleton::getInstance]
    $r configure -recording 1
}
##
#  ::ReadoutGUIPanel::recordData
#     Returns the state of the recording flag inthe readout GUI.  Note that this
#     reflects the state of the record checkbutton.  If application specific
#     code invoked one of the two procs above, this may not indicate whether
#     a currently active run is recording.
#
proc ::ReadoutGUIPanel::recordData {} {
    set r [::RunControlSingleton::getInstance]
    $r cget -recording
}

#--------------------------------------------------------------------------------
#
#  Elapsed run time widget and related API elements.
#


##
# @class Stopwatch
#
#  This non graphical type is used to provide a self contained timer/stopwatch:
#
# METHODS:
#    start       - Start the timer from the last known initial time.
#    stop        - Stop the timer.
#    reset       - Reset the elapsed time to zero.
#    addAlarm    - add a Callback invoked at the global level
#                  when the elapsed time reaches a specified value.
#    removeAlarm - Remove an alarm.
#    isRunning   - Tests for the timer running.
#    elapsedTime - Get the elapsed time.
#
# NOTE:
#    There is the assumption the event loop is working since this all uses after.
#
snit::type Stopwatch {
    
    ##
    #  elapsedTime - Number of milli-seconds we've been running.
    #  afterId     - ID of the tcl after command/scheduler...-1 if not running.
    #  alarms      - Array of callbacks.  Array inidices are elapsed times in seconds
    #                values are lists of commands called when that time is hit.
    #
    variable elapsedTimeMs 0
    variable startMs       0;    # When we start counting.
    variable afterId    -1
    variable alarms -array [list]
    variable calledAlarms -array [list]
    
    variable timerResolution 250;   # Ms per tick.
    variable stopTimeMs 0
    
    ##
    # destructor needs to kill off any after scheduled so it does not call into
    # a deleted object:
    
    destructor {
        $self _stopTimer
    }
    #-------------------------------------------------------------------------
    # Public methods.
    #
    
    ##
    # Start the timer.  Note the elapsed time is not reset.  This is because
    # we are allowed to pause the timer (stop/restart)...e.g. if a run is paused.
    #
    # @param resetStartTime   boolean indicating whether to reset the start time
    #
    # @throw  if the timer is already running
    #
    method start {{resetStartTime 1}} {
        if {![$self isRunning] } {
            $self _startTimer
            if {$resetStartTime} {
              set startMs [clock milliseconds]
            }
            array set calledAlarms [list]
        } else {
            error "Timer is already running!"
        }
    }

    ##
    # Resume the timer. This is the same as starting the timer, but it does
    # not reset the startMs. This makes sure that we continue right where 
    # things left off.
    #
    method resume {} {
      set now [clock milliseconds]
      set startMs [expr $startMs + ($now - $stopTimeMs)] 
      $self start 0
    }

    ##
    # Stop the timer.
    #
    # @throw  if the timer is already halted.
    #
    method stop {} {
        if {[$self isRunning]} {
          $self _stopTimer
          set stopTimeMs [clock milliseconds]
        } else {
#            error "Timer is already stopped"
        }
    }
    ##
    # Reset the elapsed time to zero.  Like a stopwatch this is allowed when
    # the timer is running, however in the run control case, this will typically
    # happen when the run state machine leaves halted for active.
    #
    method reset {} {
        set elapsedTimeMs 0
        set startMs [clock milliseconds]
        set stopTimeMs $startMs
        array unset calledAlarms 
        array set calledAlarms [list]
    }
    ##
    #  addAlarm
    #
    #  Add an alarm to the stopwatch.  This is normally used to
    #  force an end to a timed run.  The alarm is called when the elapsedTimeMs
    #  is at least when seconds of total time.
    #
    # @param when   - When in seconds the alarm script should fire.
    # @param script - The script to run (at global level) if the alarm fires.
    #
    # @note several alarms can be set...even on the same time.
    # @note recurring alarms are not yet supported, however a script can
    #       remove itself and schedule a later alarm for itself.
    # @throw if when is not an integer or is less than zero.
    #
    method addAlarm {when script} {
        if {![string is integer -strict $when] || ($when < 0)} {
            error "Alarms must be set on an integer number of seconds > 0: $when"
        }
        set when [expr {$when*1000}]
        lappend alarms($when) $script
    }
    ##
    # removeAlarm
    #
    #  Removes an existing alarm.
    #
    # @param when - the time the alarm was set form.#
    # @param script - The script to remove.
    #
    # @throw - If when is illegal (see addAlarm)
    # @throw - If there are no alarms for when.
    # @throw - If the script is not registered at the time specified by when.
    #
    method removeAlarm {when script} {
        if {![string is integer -strict $when] || ($when < 0)} {
            error "Alarms can only be removed from an integer number of ssecnods > 0: $when"
        }
        set whenMs [expr {$when*1000}]
        if {[array names alarms $whenMs] eq ""} {
            error "There are no alarms set for $when"
        }
        
        set whichOne [lsearch -exact $alarms($whenMs) $script]
        if {$whichOne == -1} {
            error "The script specified was not registerd as an alarm for $when :\n $script"
        }
        set alarms($whenMs) [lreplace $alarms($whenMs) $whichOne $whichOne]
        if {[llength $alarms($whenMs)] == 0} {
            array unset alarms $whenMs;     # eliminate empty lists.
        }
    }
    ##
    # isRunning
    #
    # @return bool - True if the timer is timing false otherwise.
    #
    method isRunning {} {
        return [expr {$afterId != -1}]
    }
    ##
    # elapsedTime
    #   Give the elapsed time in milliseconds.
    method elapsedTime {} {
        return $elapsedTimeMs
    }
    #--------------------------------------------------------------------------
    # Private methods
    
    
    ##
    # _startTimer
    #
    #   Set up the first after.  Note that the caller is supposed to ensure
    #   we are running.
    #
    method _startTimer {} {
        set afterId [after $timerResolution [mymethod _tick]]
    }
    ##
    # _stopTimer
    #   Stops the timer.  The caller is supposed to ensure it's not already stopped.
    #
    method _stopTimer {} {
        after cancel $afterId
        set afterId -1
    }
    ##
    # _tick
    #    called when timer tick must be counted.
    #
    method _tick {} {
        set now [clock milliseconds]
        set elapsedTimeMs  [expr {$now - $startMs}]

        $self _startTimer;     # schedules the next one.
        $self _callScripts;    # this order gives a more stable time.
    }
    ##
    # _callScripts
    #    Invokes all the scripts set for this elapsed time.
    #
    method _callScripts {} {

        foreach name [array names alarms] {
            if {$name ni [array names calledAlarms]} {
                if {$elapsedTimeMs > $name} {
                    foreach script $alarms($name) {
                        uplevel #0 $script
                    }
                    set calledAlarms($name) 1
                }
            }
        }
        return
    
    
        if {[array names alarms $elapsedTimeMs] ne ""} {
            foreach script $alarms($elapsedTimeMs) {
                uplevel #0 $script
            }
        }
    }
}
##
# @class ElapsedTimeDisplay
#
#    UI element that shows an elapsed time.  Note the timing functions are
#    performed by a Stopwatch component which has its full user interface
#    exposed however reset is intercepted to re-set our first alarm.
#
snit::widgetadaptor ElapsedTimeDisplay {
    component clock
    delegate method * to clock
    
    variable formattedNow "0 00:00:00"
    variable nextAlarm     1
    
    ##
    #  constructor
    #    * Hull is a tk::frame.
    #    * The contents are a label that tracks formattedNow.
    #    * We establish our first alarm handler at 1 second to update now.
    #    * No configuration processing is done.
    constructor args {
        installhull using ttk::frame
        install clock using Stopwatch %AUTO%
        
        ttk::label $win.time -textvariable [myvar formattedNow]
        ttk::label $win.label -text {Active Run Time}
        grid $win.time $win.label -sticky nsew 
        grid rowconfigure $win 0 -weight 0
        grid columnconfigure $win 0 -weight 0
        
        $clock addAlarm 1 [mymethod _tick]
    }
    
    destructor {
        destroy $clock
    }
    ##
    # reset
    #
    #  Intercept the reset method so that we can kill off our nextAlarm
    #  and reset the alarm back to 1.
    #
    method reset {} {
        $clock removeAlarm $nextAlarm [mymethod _tick]
        $clock addAlarm     1         [mymethod _tick]
        set nextAlarm       1
        
        set formattedNow "0 00:00:00"
        
        $clock reset
    
    }
    #--------------------------------------------------------------------------
    # private methods
    
    ##
    # _tick
    #    Alarm handler from the stopwatch.
    #    - Compute the new formattedNow from the elapsed time.
    #    - Kill off the alarm that fired.
    #    - Set up our next alarm.
    #
    method _tick {} {
        set now [$clock elapsedTime]
        $self _formatTime $now
        
        # Reschedule:
        
        $clock removeAlarm $nextAlarm [mymethod _tick]
        incr nextAlarm
        $clock addAlarm $nextAlarm [mymethod _tick]
    }
    ##
    # _formatTime
    #     compute the new value for formatted now
    #
    # @param now - Now in ms.
    #
    method _formatTime {now} {
        
        # Convert to now in ms to seconds:
        
        set now [expr {int($now/1000)}]
        
        return [Utils::formatDeltaTime $now]
    }
}
##
# Encapsulate the elapsed time GUI element as a singleton so that
# it can be accessed consistently by the API elements in the ReadoutGUIPanel
# package.
#
namespace eval  ::ElapsedTime {
    variable theInstance ""
    variable previousState ""
    namespace export attach enter leave
}

##
#  ::ElapsedTime::getInstance
#    Returns the elapsed time instance singleton:
#    *   If it does not yet exist it is created with the widget path provided.
#    *   Regardless the single instance is returned.
#
# @param  w - The widget path (optional unless this creates) to the widget.
# @return string -widget path to the singleton.
#
proc ::ElapsedTime::getInstance {{w ""}} {
    if {$::ElapsedTime::theInstance eq ""} {
        set ::ElapsedTime::theInstance [ElapsedTimeDisplay $w]
        set rstate [RunstateMachineSingleton %AUTO%]
        $rstate addCalloutBundle ElapsedTime
        $rstate destroy
    }
    return $::ElapsedTime::theInstance
}
##
# ::ElapsedTime::attach
#
#   Called at attach time.  If the run is active we're going to start
#   the clock even though the elapsed time won't be right.
#
# @param state - The state we are in at the time of attachment.
#
proc ::ElapsedTime::attach {state} {
    if {$state eq "Active"} {
        set timer [::ElapsedTime::getInstance]
        $timer start
    }
}

##
# ::ElapsedTime::enter
#
#  Called when a state is entered:
#  *  Halted -> Active : clear and start timer.
#  *  Paused  -> Active : start.
#  *  Active -> Anything other than active : Stop timer
#
# @param from - Prior state.
# @param to   - Current state.
#
proc ::ElapsedTime::enter  {from to} {
    set timer [::ElapsedTime::getInstance]
    if {($from eq "Halted") && ($to eq "Active")} {
        $timer reset
        $timer start
    } elseif {($from eq "Paused") && ($to eq "Active")} {
        $timer resume
    }
}
##
# ::ElapsedTime::leave
#
#   Called when a state is being left.  This is not used by us.
#
# @param from - state being left.
# @param to   - State we are going to enter.
proc ::ElapsedTime::leave  {from to} {
  
    set timer [::ElapsedTime::getInstance]
    if {$from  eq "Active"} {
        $timer stop
    } elseif {$to eq "NotReady"} {
      catch {$timer stop; $timer reset }
    }
}



##
# ReadoutGUIPanel::getRunTime
#
#   Returns the current time into the run in seconds.
#
# @return integer
#
proc ::ReadoutGUIPanel::getRunTime {} {
    set w [::ElapsedTime::getInstance]
    return [expr {int([$w elapsedTime]/1000)}]
}
#---------------------------------------------------------------------------
#
# Timed run controls and API
#  Note that there's the usual singleton and a registration bundle that
#  makes it possible, when a timed run is active to register an alarm with the
#  elapsed time that ends the run.
# 
#

##
# @class TimedRunControls
#
#   This control consists of a mechanism to choose a run duration along
#   with a checkbutton that is lit when a run should be timed.
#   
# LAYOUT
#   +-----------------------------------------------------+
#   |  [ ] timed run  [ ] days  [ ] hrs [ ] min [ ] sec   |
#   +-----------------------------------------------------+
#
#   Days is a spin box while hrs/min/sec are comboboxes. Like the 
#   RunControl megawidget, this can be displayed differently if the
#   ReadoutGUI has been enslaved. If the GUI is enslaved, the widgets
#   are not displayed. Otherwise they are displayed.
#
#  OPTIONS:
#    -state  - State of the controls (disabled, normal, readonly e.g.).
#    -days   - Number of days
#    -hours  - Number of hours
#    -mins   - Number of minutes
#    -secs   - Number of seconds.
#    -timed  - If the run is timed.
#
snit::widgetadaptor TimedRunControls {
    option -state -default normal -configuremethod _changeState
    option -days  -default 0 -configuremethod _changeDays
    option -hours 0
    option -mins  0
    option -secs  0
    option -timed 0
    
    #  List of widget path tails to the widgets controlled by the
    #  -state option.
    
    variable editableWidgets [list onoff days hrs min secs]

    variable slave 0
    variable wasTimedBeforeLastEnslavement 0
    
    ##
    # constructor
    #    Build the widget and lay it out.
    #
    # @param args -the -option/value configuration pairs.
    #
    constructor args {
        installhull using ttk::frame
        
        $self _layoutTimeWidgets $win.time ;# normal widget display
        $self _layoutBlank $win.blank ;# create what we will display when enslaved
        
        $self configurelist $args

        # we want to display our time widgets so long as we are not enslaved
        grid $win.time -sticky nsew

        grid rowconfigure $win 0 -weight 1
        grid columnconfigure $win 0 -weight 1
        
    }

    ## 
    # Layout the normal widgets that would be displayed when in 
    # master mode. The method simply creates a frame and stuffs
    # it with the widgets. This makes it easy to make the 
    # widgets appear and disappear at will.
    #
    # @param name   name of the frame to fill with widgets
    # 
    method _layoutTimeWidgets name {

        
        # Make a list that runs 0-59
        for {set i 0} {$i < 60} {incr i} {
            lappend values $i
        }
        
        set top [ttk::frame $name]

        ttk::checkbutton $top.onoff  \
            -text {Timed Run} -onvalue 1 -offvalue 0 -variable [myvar options(-timed)]
        
        ttk::spinbox $top.days -from 0 -to 36500 -increment 1 \
            -command [mymethod _updateDays] -format "%6.0f"  -width 6
        $top.days set 0
        
        ttk::label $top.daysep -text "-"
        ttk::combobox $top.hrs -values [lrange $values 0 23] \
            -textvariable [myvar options(-hours)] -width 2
        ttk::label $top.hrsep -text ":"
        ttk::combobox $top.min -values $values -width 2 \
            -textvariable [myvar options(-mins)]
        ttk::label $top.minsep -text ":"
        ttk::combobox $top.secs -values $values -width 2 \
            -textvariable [myvar options(-secs)]

        grid $top.onoff $top.days $top.daysep $top.hrs $top.hrsep $top.min $top.minsep $top.secs -sticky nsew
        grid rowconfigure $top 0 -weight 0
        grid configure $top -ipadx 5 -ipady 5

    }

    ##
    # Creates the widgets to be displayed when in slave mode
    #
    # In reality, this only makes an empty ttk::frame.
    #
    # @param name   name of frame to create
    #
    method _layoutBlank name {
      ttk::frame $name
    }

    #---------------------------------------------------------------------------
    #
    #  Configuration operations:
    
    ##
    # _changeState
    #    Modifies the state of all of the widgets that arec changeable.
    #    This is used when e.g. the run is active, to prevent the values and
    #    enable from being modified.
    #
    # @param optname - the name of the option that is being modified (-state).
    # @param value   - the new value which is one of normal or disabled.
    #
    method _changeState {optname value} {
        set options($optname) $value
        foreach tail $editableWidgets {
            $win.time.$tail configure -state $value
        }
        
    }
    ##
    # _changeDays
    #   Called when the -days value was configured.  Updates the
    #   widget contents.
    #
    # @param optname - the option being configured.
    # @param value   -  new value.
    #
    method _changeDays  {optname value} {
        set options($optname) $value
        $win.time.days set $value
    }
    #---------------------------------------------------------------------------
    #  Action handlers.
    #
    
    ##
    # Called when the days widget is modified.  This modification is propagated
    # to options(-days)
    #
    method _updateDays {} {
        set options(-days) [$win.time.days get]
    }

    ##
    # Called when you want to enslave the ReadoutGUI. This will update the 
    # displayed widgets of the instance
    #
    # @param value  a boolean value (true=slave mode, false=local mode)
    method setSlave value {
      set slave $value
      if {$slave} {
        set wasTimedBeforeLastEnslavement [$self cget -timed]
        $self configure -timed 0
        # get rid of time controls
        if {[winfo viewable $win.time]} {
          grid forget $win.time
        }
        grid $win.blank -sticky nsew

      } else {
        $self configure -timed $wasTimedBeforeLastEnslavement 

        # we are becoming master again... show the time controls
        #
        if {[winfo viewable $win.blank]} {
          grid forget $win.blank
        }
        grid $win.time -sticky nsew
      }
      grid rowconfigure $win 0 -weight 1
      grid columnconfigure $win 0 -weight 1
    }

    ##
    # Query whether the widgets are displayed according to slave mode 
    # or not.
    #
    # @returns boolean
    # @retval 0 - not slave mode
    # @retval 1 - slave mode
    #
    method isSlave {} {
      return $slave
    }
    
}
#-----------------------------------------------------------------------------
# Singleton pattern implementation and API:

namespace eval ::TimedRun {
    variable theInstance ""
    variable lastAlarmTime 0
    namespace export attach enter leave
}

##
# TimedRun::getInstance
#    Returns the singleton instance, creating it if needed.
#
# @param win - optional parameter that is the path to the widget.
# @param args - optional parameter that are the configuration option/value pairs.
#
# @return win - Path to the object.
#
proc ::TimedRun::getInstance {{win ""} args} {
    if {$::TimedRun::theInstance eq ""} {
        set ::TimedRun::theInstance [TimedRunControls $win]
        if {[llength $args] > 0} {
            $win configure {*}$args
        }
        set sm [::RunstateMachineSingleton %AUTO%]
        $sm addCalloutBundle TimedRun
    }
    return $::TimedRun::theInstance
}
##
#   TimedRun::_setWidgetState
# Set the timed run state depending on the
# run state.
#
# @param state - run state to use..
#
proc ::TimedRun::_setWidgetState {state} {
    set w  [::TimedRun::getInstance]
    if {$state in [list Paused Active]} {
        set state disabled
    } else {
        set state normal
    }
    $w configure -state $state
    
}
##
#   TimedRun::_formatWidgetTime
#
#    Given a widget that has -secs, -mins, -hours and -days
#    returns the formatted time string represented by that widget.
#
# @param w  - Widget.
# @return string - formatted string.
#
proc ::TimedRun::_formatWidgetTime {w} {
    set secs [$w cget -secs]
    set mins [$w cget -mins]
    set hrs  [$w cget -hours]
    set days [$w cget -days]
    
    return [expr {
        (($days*24 + $hrs)*60 + $mins)*60 + $secs
    }]
}

#  Callback bundle:

##
# ::TimedRun::attach
#
#    Called when the singleton instance is attached to the state machine.
#    set the widget state appropriately:
#    *   {Paused, Active} -> disabled.
#    *    other           -> normal
#
proc ::TimedRun::attach {state} {
    
    ::TimedRun::_setWidgetState $state
}

##
# ::TimedRun::leave
#    Called when a state is left (not used).
#
# @param from - the state we are leaving
# @param to   - The next state.
#
proc ::TimedRun::leave {from to} {}
##
#  ::TimedRun::enter
#
#    Called when a new state has been entered.
#    *  any -> {Active, Paused} -> state disabled.
#    *  any other transitions   -> state normal
#    *  Halted -> Active and timed run button set, set an alarm to end the run.
#    *  {Paused, Active} ->Halted and timed run button set, cancel the alarm
proc ::TimedRun::enter {from to} {
    
    set w [::TimedRun::getInstance]
    
    # normal/disabled state handling:
    
    ::TimedRun::_setWidgetState $to
    
    #  Timed run handling:
    
    if {[$w cget -timed]} {
        set et [::ElapsedTime::getInstance]
        if {($from eq "Halted") && ($to eq "Active")} {
            
            # Compute the run length, save it in ::TimedRun::lastAlarmTime
            # and setup an _alarm for tha time.
            
            set runTime [::TimedRun::_formatWidgetTime $w]
            
            $et addAlarm  $runTime ::TimedRun::_alarm
            set ::TimedRun::lastAlarmTime $runTime
            
        } elseif {($from in [list Paused Active]) && $to eq ("Halted")} {
            # Remove the last alarm set.
            
            $et removeAlarm $::TimedRun::lastAlarmTime ::TimedRun::_alarm
        }
    }
}

##
# ::TimedRun::_alarm
#
#   Called when a timed run time expires.  This is just going to request
#   a transition to halted.  The state machine callbacks do the rest of the work.
#
proc ::TimedRun::_alarm {} {
  end
}

# API Functions for the length of the run.

##
# ReadoutGUIPanel::isTimed
#
# @return boolean - if the GUI's timed button is checked.
#
proc ::ReadoutGUIPanel::isTimed {} {
    set w [::TimedRun::getInstance]
    return [$w cget -timed]
}
##
# ReadoutGUIPanel::setTimed
#
#   Sets the state of teh GUI time checkbutton.
#
# @param state - new state (boolean)
#
proc ::ReadoutGUIPanel::setTimed {state} {
    set w [::TimedRun::getInstance]
    $w configure -timed $state
}
##
# ::ReadoutGUIPanel::getRequestedRunTime
#
# @return integer - number of seconds in the d-hh:mm:ss request.
#
proc ::ReadoutGUIPanel::getRequestedRunTime {} {
    set w [::TimedRun::getInstance]
    return [::TimedRun::_formatWidgetTime $w]
    
}
##
# ::ReadoutGUIPanel::setRequestedRunTime
#
#   Set the length of a timed run.  This only has meaning
#   *  If the timed run is enabled (::ReadoutGUIPanel::setTimed 1)
#   *  At the start of the next run.
#
# @param secs  - number of seconds desired for the next run.
#
proc ::ReadoutGUIPanel::setRequestedRunTime secs {
    set seconds   [expr {$secs % 60}]
    set remainder [expr {int($secs / 60)}]
    set mins      [expr {$remainder % 60}]
    set remainder [expr {int($remainder /60)}]
    set hours     [expr {$remainder % 24}]
    set days      [expr {int($remainder / 24) }]
    
    set w [::TimedRun::getInstance]
    $w configure -secs $seconds -mins $mins -hours $hours -days $days
}
#------------------------------------------------------------------------------
#  Logging/output

##
# @class OutputWindow
#   Provides a widget that can be used to output lines of text. Two interesting
#   features of the output window are the history limit and logging classes.
#   The -history option allows the client to configure the number of lines of
#   text that will be retained on the widget.  This prevents the memory storage
#   of the widget from growing without bounds.
#
#  Logging classes are used with the log method.   The -logclasses option
#  defines a list of log item types that the log method will accept (an
#  error is thrown if log is called with a log type not in that list).
#  -showlog defines a list of log item types the log method will actually
#  display on the screen.  The initial set of log classes are:
#  *   output - intended to display output from some source (e.g. a data source program).
#  *   log    - intended to log some interesting event (e.g. the run started).
#  *   error  - intended to log some error condition.
#  *   warning- intended to log some condition that could be a problem.
#  *   debug  - intended for debugging output.
#
#  -showlog defines how or if items will be displayed.  It consists of a
#      list in the form accepted by array set.  The keys to the array are
#      log classes.  If a log class is not in the list it will not be displayed.
#      the elements of the array are lists of option/value pairs where each
#      option is an option accepted by the text widget tag configure operation.
#      As you might guess, log entries are given a tag that matches their logclass
#      Therefore changes to the -showlog are dynamic with the exception of the
#      addition/removal of classes from the list which only affect future log
#      operations. By default these are (note debug is not displayed):
#      *  output [list]
#      *  log    [list]
#      *  error  -foreground red -background white
#      *  warning -foreground yellow
#     @note a dict could be just as easily used as a list with keys the option names
#           and values the option values.
#
# LAYOUT:
#   +---------------------------------------------------------------+
#   | +---------------------------------------------+--+            |
#   | |  text widget                                |sb|            |
#   | +---------------------------------------------+--+            |
#   | |    scrollbar                                |               |
#   | +---------------------------------------------+
#   +---------------------------------------------------------------+
#
# OPTIONS:
#    -foreground - Widget foreground color
#    -background - Widget background color
#    -width      - Width of widget in characters.
#    -height     - Height of widget in characters.
#    -history    - Number of lines of historical text that are retained.
#    -logclasses - Defines the set of log classes accepted by the log method.
#    -showlog    - Defines which of the log classes will actually be displayed.
#    -monitorcmd - Defines a command to call when stuff is output to the window.
#                  the text of the output is appended to the command.
# METHODS:
#   puts         - puts data to the widget
#   clear        - clear the entire text and history.
#   log          - Make a timestamped log entry.
#   get          - Returns all of the characters in windowm, visible and historic
#   open         - Opens a log file for the widget.  The file is opened for
#                  append access. From this point on all data output will be logged
#                  to this file.
#   close        - closes the log file.
#
snit::widgetadaptor OutputWindow {
    component text
    
    delegate option -foreground to text
    delegate option -background to text
    delegate option -width      to text
    delegate option -height     to text

    option -history    -default 1000
    option -logclasses -default [list output log error warning debug]
    option -showlog    -default [getDefaultLogging] \
        -configuremethod _updateTagOptions
    option -monitorcmd [list]
    
    # If non empty, this is the log file fd.
    
    variable logfileFd  ""
    
    ##
    # constructor
    #   Builds the text widget along with horizontal and vertical scrollbars.
    #   the widget is set to -state disabled so that users can't type at it.
    #
    # @param args - The configuration options for the text widget.
    #
    constructor args {
        installhull using ttk::frame
        
        # Widget creation
        
        install text using text $win.text -xscrollcommand [list $win.xsb set] \
            -yscrollcommand [list $win.ysb set] -wrap word -background grey -state disabled
        ttk::scrollbar $win.ysb -orient vertical   -command [list $text yview]
        ttk::scrollbar $win.xsb -orient horizontal -command [list $text xview]
        
        # widget layout:
        
        grid $text $win.ysb -sticky nsew
        grid $win.xsb       -sticky new
        grid rowconfigure $win 0 -weight 1
        grid columnconfigure $win 0 -weight 1
        
        
        $self configurelist $args
        $self _updateTagOptions -showlog $options(-showlog)
    }
    ##
    # destructor
    #   Ensure we don't leak file descriptors:
    #
    destructor {
        if {$logfileFd ne ""} {
            close $logfileFd
        }
    }
    
    #---------------------------------------------------------------------------
    # Public methods:
    #
    
    ##
    # puts
    #   Puts simple text to the output window.
    #
    # @param args - This can be one or two parameters:
    #               *  text       - the text is output followed by a newline.
    #               * -nonewline  - The text is output with no trailing newline.
    #
    method puts  {args}       {

        if {[llength $args] == 1} {
            set line "[lindex $args 0]\n"
        } elseif {([llength $args] == 2) && ([lindex $args 0] eq "-nonewline")} {
            set line [lindex $args 1]
        } else {
            error "use puts ?-nonewline Text"
        }
        $self _output "__notag___" $line
    }
    ##
    # clear
    #    Clears the entire widget.
    #
    method clear {}  {
        $text config -state normal
        $text delete 0.0 end
        $text config -state disabled
    }
    ##
    # log
    #    Make a log like message.  A log message gets a timestamp and is
    #   associated with log class (-logclasses).  Further more, -showlog can
    #   associate rendering options with these messages.
    #
    # @param class - The log class associated with the message (must be in
    #                -logclasses)
    # @param msg   - The messagde to log.
    method log   {class msg} {
        set timestamp [clock format [clock seconds] -format {%D %T} ]
        if {$class ni $options(-logclasses)} {
            error "'$class' is not one of the known classes: {$options(-logclasses)}"
        }
        array set logRenditions $options(-showlog)
        if {$class in [array names logRenditions]} {
            $self _output $class "$timestamp : $class : $msg\n"
        }
    }
    ##
    # get
    #   Return the text in the widget.  No effor is made to return information
    #   about the rendering of the text... only the text itself is returned.
    #
    # @return - string containing all of the text in the output window.
    #
    method get   {}           {
        return [$text get 0.0 end]
    }
    ##
    # open
    #   Open a file and log subsequent output to it as well as to the window.
    #   *   If a file is already open it is closed.
    #   *   the file is opened for append access (a+).
    #   *   Output will be logged to the file until close is called or until
    #       a different file is opened.
    #
    # @param filename  - Name of the file to open.
    #
    # @throw if the file is not writable.
    #
    method open  {filename}   {
    
        #  Close any open file:
        
        if {$logfileFd ne ""} {
            $self close
        }
        #  Open the new file
        
        set logfileFd [open $filename "a+"]
    }
    ##
    # Close the log file.
    # @throw it is an error to close when no log file is active.
    #
    method close {}           {
        if {$logfileFd eq ""} {
            error "No logfile is open"
        } else {
            close $logfileFd
            set logfileFd ""
        }
    }
    
    #--------------------------------------------------------------------------
    #  Configuration processors.
    
    
    ##
    # _updateTagOptions
    #
    #  Processes a new set of -showlog values.  These get turned into tag
    #  configuration.
    #
    # @param optname  - The option being configured
    # @param value    - The new value for this option.
    #
    # @note see the class comments for more informationabout the value.
    #
    method _updateTagOptions {optname value} {
        #
        #  By putting the values into an array whose values are treated as dicts
        #  locally we
        #  - make it easy to iterate over the tags.
        #  - Syntax check the dictionary definition lists.
        
        array set tagInfo $value
        
        foreach tag [array names tagInfo] {
            set tagConfig $tagInfo($tag)
            if {([llength $tagConfig] % 2) == 1} {
                error "log option lists must be even but the one for '$tag' is not: {$tagConfig}"
            }
            dict for {tagopt tagoptValue} $tagConfig {
                $text tag configure $tag $tagopt $tagoptValue
            }
        }
        
        #
        # If we got here, everything is legal:
        #
        set options($optname) $value
    }
    #--------------------------------------------------------------------------
    # Private methods
    
    ##
    # _output
    #    Centralized output/file-logging/history-trimming output method:
    #
    # @param tag  - Tag to associate with the output.
    # @param text - Data to output.
    #
    method _output {tag data} {
        
        # Output the data to the widget:
        
	$text configure -state normal
        $text insert end $data  $tag
        $text yview -pickplace end
	$text configure -state disabled
        
        # Limit the number of lines that can appear.
        
        set lines [$text count -lines 0.0 end]
        if {$lines > $options(-history)} {
            $text delete 0.0 "end - $options(-history) lines"
        }
        # Log to file if we must:
        
        if {$logfileFd != ""} {
            puts -nonewline $logfileFd $data
            flush $logfileFd
        }
        # If there is a non null -monitorcmd hand it the text:
        
        set cmd $options(-monitorcmd)
        if {$cmd ne ""} {
            uplevel #0 $cmd "{$data}"
        }
    }
}

##
# @class OuputWindowSettings
#
#   For to prompt the output window settings. This is normally wrapped
#   in a dialog.
#
# LAYOUT:
#   +----------------------------------------------------------------+
#   | Rows: [^V]   Columns [^V] History [^V] [ ] show debug messages |
#   +----------------------------------------------------------------+
#
#   * Plain text are labels.
#   * [^V] are spinboxes
#   * [ ] is a checkbutton.
#
# OPTIONS:
#   *  -rows    - Number of displayed rows of text.
#   *  -columns - Number of displayed text columns
#   *  -history - Number of lines of history text displayed.
#   *  -debug   - boolean on to display debug log messages.
#
#
snit::widgetadaptor OutputWindowSettings {
    option -rows    -configuremethod  _setSpinbox -cgetmethod _getSpinbox
    option -columns -configuremethod  _setSpinbox -cgetmethod _getSpinbox
    option -history -configuremethod _setSpinbox -cgetmethod _getSpinbox
    option -debug
    
    ##
    # constructor
    #    Create the widgets, bind them to the options and lay them out
    #    As shown in the LAYOUT comments section
    #
    # @args configuration options.
    #
    constructor args {
        installhull using ttk::frame
        
        # Widget creation:
        
        ttk::label $win.rowlabel -text {Rows: }
        ttk::spinbox $win.rows -from 10 -to 40 -increment 1 -width 3
        
        ttk::label $win.collabel -text {Columns: }
        ttk::spinbox $win.columns -from 40 -to 132 -increment 1 -width 4
        
        ttk::label $win.historylabel -text {History lines: }
        ttk::spinbox $win.history -from 500 -to 10000 -increment 100 -width 6
        
        ttk::checkbutton $win.debug -text {Show debugging output} \
            -variable [myvar options(-debug)] -onvalue 1 -offvalue 0
        
        # widget layout:
        
      #  grid $win.rowlabel $win.rows $win.collabel $win.columns \
      #      $win.historylabel $win.history $win.debug
	grid $win.historylabel $win.history $win.debug; # Lazy way to remove row/col.
 
        $self configurelist $args
        
    }
    #--------------------------------------------------------------------------
    #
    #  Configuration get/set operations.
    
    ##
    # _setSpinBox
    #
    #  Sets the spinbox associated with an option.  The spinbox must be
    #  named win.optionname where optionname is the option name with the -
    #  stripped.  e.g. for the -rows option the spinbox is named
    #  $win.rows
    #
    # @param optname - Name of the option.
    # @param value   - New value for the option.
    #
    method _setSpinbox {optname value} {
        set name [string range $optname 1 end];    # Strip off the leading$
        $win.$name set $value
    }
    ##
    # _getSpinbox
    #
    #   Gets the value of an option associated with a spinbox.  See
    #   _setSpinbox above for naming requirements
    #
    # @param optname - name of the option being queried.
    #
    method _getSpinbox optname {
        set name [string range $optname 1 end]
        return [$win.$name get]
    }
}
##
# @class TabbedOutput
#
#   This widget is a tabbed notebook containing output windows.  The idea is that
#   there is a main window which shows all puts and Logs that don't have a data  source.
#   As Logs are done with a data source, new tabs are created and the output is then segregated
#   into those tabs.  Tab names are:
#   Main - The window with non-sourced data.
#   Srcname - The tab for the data source Srcname
#  
#  Furthermore if output is directed to a windows that is not visible the number of outputs
#  is shown as a (n) after the tab name.  Displaying that tab eliminates the (n).
#
#
# OPTIONS
#    -foreground - The foreground color for all of the widgets.
#    -background - The background color for all of the widgets.
#    -width      - Width in characters of all widgets.
#    -height     - height of all widgets in lines of text.
#    -history    - number of lines of historical data each widget has.
#    -logclasses - Define the set of log classes each widget accepts.
#    -showlog    - Defines which log classes are displayed and how.
#    -monitorcmd - Defines the monitor command for all windows.
# METHODS
#  puts  - Outputs something to the main window.
#  clear - Clears display and history of all windows.
#  log   - Logs to the main window.
#  logFrom - logs from a specific data source.
#  open    - Turns on logging for all windows.
#  close   - Closes logging for all windows.
#  get     - returns a list of pairs.  The first element of each pair is the name of a source
#            ('main' or a data source name),  The second element of each pair is the contents of that window.
#
snit::widgetadaptor TabbedOutput {

    # wish we could formally delegate but we are a fanout.

    option -foreground  -configuremethod _RelayOption
    option -background  -configuremethod _RelayOption
    option -width       -configuremethod _RelayOption
    option -height      -configuremethod _RelayOption
    option -history     -configuremethod _RelayOption -default 1000
    option -logclasses  -configuremethod _RelayOption -default [list output log error warning debug]
    option -showlog     -configuremethod _RelayOption \
        -default [getDefaultLogging]
    option -monitorcmd  -configuremethod _RelayOption -default [list]
    option -errorclasses [list error warning]

    
    
    delegate option * to hull
    delegate method * to hull

    # Options that don't get fanned out:
    
    variable localOptions [list -errorclasses]

    #
    # List of Output widgets managed by us...this is in index order.
    # Index 0 is the Main widget.

    variable outputWindows [list]

    # Array of dicts indexed by widget namee each Dict containing:
    #      name - source name.
    #      lines- Unseen puts of text.
    #

    variable tabInfo       -array [list]
    
    # Used to uniquify output windows:
    
    variable outIndex     0
    
    # log file if open [list] if not...used to bring new windows into the log:
    
    variable logFile [list]

    ##
    #  constructor
    #
    #  *   Install a ttk::notebook as the hull.
    #  *   Create an output window $win.main as the main widget... in the notebook.
    #  *   Add the main output window to the outputWindows list.
    #  *   Populate -foreground, -background, -height, -width from the main window.
    #  *   Process the configuration options.
    #

    constructor args {
      installhull using ttk::notebook


        lappend outputWindows [OutputWindow $win.main]
        set tabInfo($win.main) [dict create name main lines 0]
        $hull add $win.main -text main
        $hull tab $win.main -sticky nsew
        set options(-foreground) [$win.main cget -foreground]
        set options(-background) [$win.main cget -background]
        set options(-width)      [$win.main cget -width]
        set options(-height)     [$win.main cget -height]

        $self configurelist $args

        grid rowconfigure $win 0 -weight 1
        grid columnconfigure $win 0 -weight 1

# When the selected tab has changed we need to update its tab to indicate
# It's lines have been read.

        bind $win <<NotebookTabChanged>> [mymethod _TabChanged]
    }

    #---------------------------------------------------------------------------------
    #
    #  Configuration methods


    ##
    # _RelayOption
    #     Called when a delegated option is modified.
    # @param opt - Name of the option.
    # @param val - New value.
    #
    method _RelayOption {opt value} {
	set options($opt) $value
	$self _DoAll [list configure $opt $value]
    }

    #---------------------------------------------------------------------------------
    # Public methods.
    #

    ##
    # puts
    #    Output a string to the main window.  If the main window is not current it gets a (n) with the number
    #    of outputs done since the last time it was current.
    #
    # @param args  -  Args see OutpuWindow puts.
    #
    method puts args {
	set widget [lindex $outputWindows 0];                   # Always main.
	$widget puts {*}$args

	$self _UpdateTabText $widget
    }
    ##
    # clear
    #    Clears the text/history from all windows.
    #    The tabs are set back to just the source names.
    #
    method clear {} {
        $self _DoAll clear
        foreach win $outputWindows {
            $hull tab $win -text [dict get $tabInfo($win) name]
        }
    }
    ##
    # log
    #    Logs to the main window.
    #    This basically delegates to the main windows log method.
    #
    # @param args - the parameters that would be passed to the log method normally.
    #
    # @note - The _UpdateTabText is called to ensure the tab title is updated if
    #         the window is not displated.
    #
    method log args {
        set widget [lindex $outputWindows 0]
        $self _LogToWidget $widget {*}$args
    }
    ##
    # logFrom
    #   Logs to a specific source window (creating it as needed).
    #
    # @param source - source of the log message.  If there is no output window
    #                 for this source, one is created.
    # @param args   - The stuff that goes into the source's window log method.
    #
    # @note - _UpdateTabText is invoked to ensure the tab title is updated if needed.
    #
    method logFrom {source args} {
        set widget [$self _GetSourceWindow $source]
        $self _LogToWidget $widget {*}$args
    }
    ##
    # open
    #   Open logging in all windows.  These go to a common log file.
    #
    # @param filename - Path of file to log to.
    #
    method open filename {
        $self _DoAll [list open $filename]
        set logFile $filename
    }
    ##
    # close
    #   Close the log file in all windows.
    #
    method close {} {
        $self _DoAll close
        set logFile [list]
    }
    ##
    # get
    #   Return the contents of all windows.
    #
    # @return list of pairs - each pair consists of a source name and the
    #                         data from the window associated with that source.
    #
    
    method get {} {
        set result [list]
        foreach widget $outputWindows {
            set name [dict get $tabInfo($widget) name]
            set contents [$widget get]
            lappend result [list $name $contents]
        }
        return $result
    }
    #--------------------------------------------------------------------------------
    #
    # Private utility methods.
    #

    ##
    #  _DoAll
    #    Do the same command in all windows.
    # cmd - List that is the command to perform.
    #
    method _DoAll cmd {
	foreach win $outputWindows {
	    $win {*}$cmd
	}
    }
    ##
    # _UpdateTabText
    #   If the widget is not the currently displayed one, increment its number of unseen
    #   lines and modify the tab text.
    #
    # @param widget - The widget to modify.
    # @param class  - The log class used
    #
    method _UpdateTabText {widget {class output}} {
	set windex [$hull index $widget]
	set cindex [$hull index current]

	if {$windex != $cindex} {
	    dict incr tabInfo($widget) lines
	    set name  [dict get $tabInfo($widget) name]
	    set lines [dict get $tabInfo($widget) lines]
	    set tabText [format "%s (%d)" $name $lines]
	    $hull tab $windex -text $tabText
            if {$class in $options(-errorclasses)} {
                $hull tab $windex -image output_error -compound left
            }

		      
	}
    }
    ##
    # _GetSourceWindow
    #    Returns the widget associated with a data source output window.
    #    If the data source does not have an output window, one is created
    #    and that widget is returned.
    #
    # @param source - Name of the source who's window we want
    #
    # @return window - Path to the wndow for that source.
    #
    method _GetSourceWindow {source} {
        foreach widget $outputWindows {
            if {[dict get $tabInfo($widget) name] eq $source} {
                return $widget
            }
        }
        #  Need to create a new one:
        
        set widget $win.source[incr outIndex]
        lappend outputWindows $widget
        set tabInfo($widget) [dict create name $source lines 0]
        $hull add [OutputWindow $widget] -text $source
        
        # Propagate the settings to the new window.
        
        foreach option [array names options] {
            if {$option ni $localOptions} {
                $widget configure $option $options($option)
            }
        }
        if {$logFile ne ""} {
            $widget open $logFile
        }
        
        # Return it as the output window:
        
        return $widget
    }
    
    ##
    # _LogToWidget
    #   Given an output widget log to it.
    #
    # @param widget - The widget to log to.
    # @param args   - log parameters
    #
    method _LogToWidget {widget args} {
        $widget log {*}$args
        set class [lindex $args 0];             #log class.
        $self _UpdateTabText $widget $class
    }
    ##
    # _TabChanged
    #   Called when the tab changes.  We must figure out the current widget
    #   and reset its tab name to just the source name (removing any
    #   unseen entry count.
    #
    #
    method _TabChanged {} {
        set idx [$hull index current]
        set widget [lindex $outputWindows $idx]
        set source [dict get $tabInfo($widget) name]
        $hull tab $idx -text $source -image [list]
        dict set tabInfo($widget) lines 0;    # No output is unseen.
    }
    
}
#
#  Singleton and bundle to ensure state transitions get logged.
#
namespace eval ::Output {
    variable theInstance ""
    namespace export enter leave attach
    variable errorFilename [file join [file dirname [info script]] error.png]
}

image create photo output_error -file $::Output::errorFilename

##
#  Load the output error icon:
#

##
# Output::getInstance
#
#   Return the singleton instance of the OutputWindow creating it if needed.
#
# @param win - (Only required for the creation). Window path for the output window.
# @param args- (Optional, only seen at creation). Configuration parameters for
#               the output window.
#
proc Output::getInstance { {win {}} args} {
    if {$::Output::theInstance eq ""} {
        set ::Output::theInstance [TabbedOutput $win {*}$args]
        set sm [::RunstateMachineSingleton %AUTO%]
        $sm addCalloutBundle Output
        $sm destroy
      
        grid $::Output::theInstance -sticky nsew
        grid rowconfigure $win 0 -weight 1
        grid columnconfigure $win 0 -weight 1
    }
    return $::Output::theInstance
}
#
# Callout bundle methods.. These log stuff to the singleton OutputWindow.
#

##
# ::Output::attach
#   Outputs a debug message indicating attached and which state.
#
# @param state - The current state.
#
proc ::Output::attach {state} {
    set w [::Output::getInstance]
    $w log debug "Attached to state machine state is: $state"
}
##
# ::Output::enter
#   State machine finished a transition.
#   Make a log level entry indicating we completed a state transition.
#
# @param from - the state we left.
# @param to   - The state we are now in.
#
proc ::Output::enter  {from to} {
    set w [::Output::getInstance]
    $w log log "Run state changed: $from -> $to"
}
##
# ::Output::leave
#   Called when a state transition is beginning.
#   Make a debug entry indicating this.
#
# @param from - State we are leaving.
# @param to   - State we are headed for
#
proc ::Output::leave  {from to} {
    set w [::Output::getInstance]
    $w log debug "Run leaving state $from heading for state $to"
}

##
# ::Output::_isDebugging
#
#  Determins if the set of items in -showlog for an output widget
#  is consistent with displaying debug output. Specifically is there
#  A 'debug' entry.
#
# @param output - widget command for an Output window.
#
# @return boolean - true if debugging is being displayed else false.
#
proc ::Output::_isDebugging {output} {
    array set logRenditions [$output cget -showlog]
    return [expr {[array names logRenditions debug] ne ""}]
}
##
# ::Output::_setShowDebug
#
#  Sets an output widget to show or not show debug log entries depending
#  on the state
#
# @param widget - ouput widget's command.
# @param state - bool true if debugging should be displayed and false otherwise.
#
proc ::Output::_setShowDebug {widget state} {
    array set logRenditions [$widget cget -showlog]
    if {$state} {
        set logRenditions(debug) [list]
    } else {
        array unset logRenditions debug; 
    }
    $widget configure -showlog [array get logRenditions]
}

##
#  Prompt for new settings. The following settings are possible:
#  * -width
#  * -height
#  * -history
#  * enable/disable debug output.
#
proc ::Output::promptSettings {} {
    
    # Make the toplevel and wrapper.
    
    toplevel .outputsettings
    set w [DialogWrapper .outputsettings.dialog]
    set frame [$w controlarea]
    
    # Create the form and stock it with the current values from the
    # output singleton.  Note that the effort to compute the debug option
    # is delegated to a private proc.
    
    set output [::Output::getInstance]
    set form [OutputWindowSettings $frame.form \
        -rows [$output cget -height] -columns [$output cget -width] \
        -history [$output cget -history]]
    $form configure -debug [::Output::_isDebugging $output]
    
    $w configure -form $form

    # Do th layout and wait for the user:
    
    pack $w
    set action [$w modal]
    if {$action eq "Ok"} {
        $output configure -width [$form cget -columns] -height [$form cget -rows]
        $output configure -history [$form cget -history]
        ::Output::_setShowDebug $output [$form cget -debug]
    }
    destroy .outputsettings
    
}
#
#  API for the output window.
#

##
#  ::ReadoutGUIPanel::isRecording
#     Set the text widget to the recording colors.
#     (dark green background with white foreground).
# 
proc ::ReadoutGUIPanel::isRecording {} {
    set w [::Output::getInstance]
    $w configure -background darkgreen -foreground white
}
##
# ::ReadoutGUIPanel::notRecording
#   set the text widget to the non-recording colors of background grey
#   foreground black.
#
proc ::ReadoutGUIPanel::notRecording {} {
    set w [::Output::getInstance]
    $w configure -background grey -foreground black
}
##
# ::ReadoutGUIPanel::normalColors
#
#  Synonym for notRecording.
#
proc ::ReadoutGUIPanel::normalColors {} {
    ::ReadoutGUIPanel::notRecording   
}
##
# ::ReadoutGUIPanel::outputText
#    Output a line of text to the output widget.
#
# @param text - text to write,  a newline is appended to it.
#
proc ::ReadoutGUIPanel::outputText {text} {
    set w [::Output::getInstance]
    $w puts $text
}
##
# ::ReadoutGUIPanel::log
#
#   Creates a log entry on on the text window.
#
# @param src  - The creator of the log.
# @param class - The log class (see -logclasses option on OutputWindow)
# @param msg   - Meat of the message.
#
proc ::ReadoutGUIPanel::Log {src class msg} {
    set w [::Output::getInstance]
    $w logFrom $src $class "$src: $msg"
    
#    update idletasks
#    update idletasks;        # Ensure the UI is updated in case
#    update idletasks;        # the log is prior to some event loop dead wait.
#    update idletasks
}

#------------------------------------------------------------------------------
# Status area -- This will be a set of very customizable lines.
#

##
# @class StatusArea
#
#   Provides a very customizable widget for displaying status information.
#   The status information is divided into several status lines.
#   Each status line is internally managed but has contents that are externally
#   controlled.
#
# METHODS:
#    addWidget   - Add an arbitrary widget to the bottom of the status area.
#    addMessage  - Add a message line to the bottom of the status area.
#    setMessage  - Set the text of a message widget
#    statusItems - List the set of status items that are managed.
#    messageHandles - List the message handles.
#
snit::widgetadaptor StatusArea {
    delegate option * to hull
    
    variable index 0
    variable messages [list]
    
    ##
    # constructor
    #    Construct the container
    constructor args {
        installhull using ttk::frame
        
        $self configurelist $args
    }
    ##
    #  addWidget
    #    Adds as widget and grid's it at the bottom of the status
    #    line.  The widget is made sticky nsew.
    #
    # @param args - Widget creation command line minus the window name.
    #               e.g. to add a label widget that tracks the
    #               value of some global varialbe 'theText':
    #               $sa addWidget label -textvariable ::theText
    #
    # @return widget path - the created widget.
    #    
    method addWidget args {
        set command [lindex $args 0]
        set config  [lrange $args 1 end]
        set window  $win.child[incr index]
        
        $command $window {*}$config
        grid $window -sticky nsew
        grid rowconfigure $window 0 -weight 0
        
        return $window
    }
    ##
    # addMessage
    #    Convenience method that adds a label into which
    #    status text can be placed.
    #
    # @param msg - (optional) initial message contents.
    #
    # @return integer - handle that can be used in future calls to setMessage
    #
    method addMessage {{msg {}}} {
        set win [$self addWidget ttk::label -text $msg]
        set handle [llength $messages]
        lappend messages $win
        return $handle
    }
    ##
    # setMessage
    #
    #   Sets new contents for a status message line.
    #
    # @param handle - A message handle gotten from addMessage.
    # @param text   - The new text
    # @param args   - optional additional configuration options
    #                 e.g. -foreground -background
    #
    method setMessage {handle text args} {
        if {$handle >= [llength $messages]} {
            error "Invalid message handle"
        }
        set widget [lindex $messages $handle]
        
        $widget configure -text $text {*}$args
    }
    ##
    # statusItems
    #
    #  Returns the list of widgets that are managed as top level status
    #  items.  Note that if the client creates a frame stocked with widges,
    #  this only returns that frame not the contents of the frame.
    #  Further more, message objects are no different than any other widget
    #
    # @return list - List of widget windows.
    #
    method statusItems {} {
        return [winfo children $win]
    }
    ##
    # messageHandles
    #
    # @return list - of message object handles.
    #
    method messageHandles {} {
        return $messages
    }
    
}
##
#  Singleton pattern implementation for the status bar.
#
namespace eval ::StatusBar {
    variable theInstance ""
}

##
# ::StatusBar::getInstance
#
# Return the current instance of the status bar, creating it if needed.
#
# @param widget  -Path to the widget -- only looked at in the first call.
#
proc ::StatusBar::getInstance {{widget {}}} {
    if {$::StatusBar::theInstance eq ""} {
        set ::StatusBar::theInstance [StatusArea $widget]
    }
    return $::StatusBar::theInstance
}

#-----------------------------------------------------------------------------
#
#  Full Run control GUI base.  We build using the singletons so that
#  external bits and pieces of the run control system can find us.
#

##
# @class ReadoutGUI
#    The fully assembled readout control panel.
#
# LAYOUT:
#
#    +------------------------------------------------+
#    | Run identification                             |
#    | +-----------------------+                      |
#    | | Run controls          | Elapsed time         |
#    | |                       | Timed Run            |
#    | +-----------------------+                      |
#    | +-------------------------------------------+  |
#    | |    Output window.                         |  |
#                    ...
#    | +-------------------------------------------+  |
#    |    status lines                                |
#                          ...
#    +------------------------------------------------+
#
snit::widgetadaptor ReadoutGUI {
    component menubar
    component runid
    component runcontrol
    component elapsedtime
    component timedrun
    component output
    component statusbar
    
    delegate option * to hull
    
    ##
    # constructor
    #   All the work gets done here.  The remainder of the application
    #   just connects via the singletons and the run state machine.
    #
    #
    # @param args - hull (ttk::frame) options
    #
    constructor args {
        installhull using ttk::frame
        $self configurelist $args
        
        #  Make the menubar infrastructure.  We only handle the
        #  File menu.  See _populateFileMenu for more. There is an implicit
        #  assumption that we are in the top level widget.
        #  TODO:  - figure out how to not make that true.
        
        install menubar using readoutMenubar $win.menu
        . configure -menu $menubar
        $self _populateFileMenu
 
         # Install the components
        
        
        install runid using ::ReadoutGUIPanel::getRunIdInstance $win.runid
        install runcontrol using ::RunControlSingleton::getInstance $win.rctl
        install elapsedtime using ::ElapsedTime::getInstance $win.elapsed
        install timedrun    using ::TimedRun::getInstance $win.timed
        install output      using ::Output::getInstance $win.output
        install statusbar   using ::StatusBar::getInstance $win.status
        
        # Lay them all out:
       
        grid $runid       -sticky nsew -columnspan 2 -row 0 -column 0
        grid $runcontrol  -sticky nsw  -rowspan    2 -row 1 -column 0
        grid $elapsedtime -sticky se -row 1 -column 1 
        grid $timedrun    -sticky ne -row 2 -column 1
        grid $output      -sticky nsew -row 3 -column 0 -columnspan 2
        grid $statusbar   -sticky nsew -row 4 -column 0 -columnspan 2
        
        grid rowconfigure $win 0 -weight 0        
        grid rowconfigure $win 1 -weight 0        
        grid rowconfigure $win 2 -weight 0        
        grid rowconfigure $win 3 -weight 1        
        grid rowconfigure $win 4 -weight 0        
        grid columnconfigure $win 0 -weight 1
        grid columnconfigure $win 1 -weight 1

	bind $win <Destroy> [mymethod _OnDestroy %W]
	
        # configure
        
        $self configurelist $args

    }
    #--------------------------------------------------------------------------
    #  Private methods
    #
    
    ##
    # _populateFileMenu
    #   Populate the file menu which consists of the following:
    #   * Load        - Loads a script by doing a source at the global level.
    #   * Add Library - Adds a library directory to the auto_path.
    #   * <separator>
    #   * Log         - Select a log file for the output widget.
    #   * Log Disable - Turn off logging in the output widget.
    #   * <separator>
    #   * Exit        - Ends any active run and exits the application.
    #
    method _populateFileMenu {} {
        $menubar addMenu File
        $menubar addCommand   File Load... [mymethod _sourceFile]
        $menubar addCommand   File {Add Library...} [mymethod _extendAutoPath]
        $menubar addSeparator File
        $menubar addCommand   File Log... [mymethod _chooseLogfile]
        $menubar addCommand   File {Disable Log} [mymethod _stopLogging]
        $menubar addSeparator File
        $menubar addCommand   File {Exit...} [mymethod _Exit]
    }

    
    #--------------------------------------------------------------------------
    #  Private methods:
    #
    
    

    
    ##
    # _stopRun
    #     Ensure the run state machine is not in an non halted state
    #     If the state is Active or Paused, the machine is transitioned to
    #     Halted.
    #
    method _stopRun {} {
        set sm [::RunstateMachineSingleton %AUTO%]
        if {[$sm getState] in [list Active Paused]} {
            $sm transition Halted
        }
        $sm destroy
    }
    ##
    #  Stop all data sources.
    #
    method _stopDataSources {} {
        set mgr [DataSourcemanagerSingleton %AUTO%]
        set sm [RunstateMachineSingleton %AUTO%]
        
        if {[$sm getState] ne "NotReady"} {
	    puts "Stopping all data sources"
            $mgr stopAll
        }
        $sm destroy
        $mgr destroy
    }
    
    #--------------------------------------------------------------------------
    #  Menu bar handlers:
    #
    
    ##
    # _sourceFile
    #    Sources a file at the top level.  This can be used to add plugins
    #    to the ReadoutGUI.  Since this is a menubar function;
    #    the file sourced is gottenvia tk_getOpenFile.
    #    *  .tcl - is the normal default filetype and the default extension
    #    *  .tk  - is also an option for extension filtering.
    #    *  .*   - is an option for extension filtering.
    #
    method _sourceFile {} {
        set scriptPath [tk_getOpenFile -defaultextension .tcl -parent $win  \
            -title {Choose script file}             \
            -filetypes [list                        \
                [list {Tcl Scripts} {.tcl}]         \
                [list {Tk Scripts}  {.tk"}]         \
                [list {All Files}    *]             \
            ]]
        #
        #  The dialog returns "" if cancel is clicked
        #
        if {$scriptPath ne ""} {
            uplevel #0 source $scriptPath
        }
    }
    ##
    # _extendAutoPath
    #    Adds another directory to the ::auto_path list.  The directory
    #    is prompted fro using tcl_ChooseDirectory
    #
    method _extendAutoPath {} {
        set packageDirectory [tk_chooseDirectory -mustexist 1 -parent $win \
            -title {Choose Package directory}]
        if {$packageDirectory ne ""} {
            lappend ::auto_path $packageDirectory
        }
    }
    ##
    # _chooseLogfile
    #
    #   The output widget has the capability of logging all output to a file
    #   this menu item prompts for the log file name and initiate logging.
    #   Logging will continue until the _stopLogging menu item is invoked or
    #   alternatively some external code stops logging.   If the user
    #   requests a different log file, logging will continue in that file.
    #
    method _chooseLogfile {} {
        set logFilename [tk_getSaveFile -defaultextension log -parent $win \
            -title {Choose log file}                                        \
            -filetypes [list                                                \
                [list {Log files} .log]                                     \
                [list {Text files} .txt]                                    \
                [list {All Files}   *]                                      \
        ]]
        if {$logFilename ne ""} {
            $output open $logFilename
        }
    }
    ##
    # _stopLogging
    #    Close the output window logfile if any is active.
    #    At this point this is done just by catching a call to
    #    $output close.
    #    TODO:  The menu item should be disabled when logging is off
    #           and enabled otherwise.
    #
    method _stopLogging {} {
        catch {$output close}
    }
    ##
    # _Exit
    #   Prompt for confirmation and exit.
    #   If exit is confirmed then
    #   *  If the run state is one of the active ones (Active, Paused) the run
    #      state machine is asked to stop the run.
    #   *  Data sources are asked to exit.
    #   *  The state is then transitioned to NotReady to ensure that all
    #      state transition handlers run
    #   *  The exit command is performed with a 0 (normal) exit code.
    method _Exit {} {
        set confirm [tk_messageBox -default no -type yesno -parent $win \
            -title {Really Exit?} -icon question \
            -message {Are you sure you want to exit?} \
            ]
        if {$confirm eq "yes"} {
            $self _stopRun
            $self _stopDataSources
            set sm [RunstateMachineSingleton %AUTO%]
            if {[$sm getState] ne "NotReady"} {
                $sm transition NotReady
            }
            $sm destroy;                # though there's not much point to this:
            #
            #  Save the state
            #
            
            set state [StateManagerSingleton %AUTO%]
            $state save
            $state destroy
            
            exit 0;                     # since we're exiting now.
        }
    }
    method _OnDestroy {wid} {
    	   puts "OnDestroy $wid : $win"
    	   if {$wid eq $win} {
              puts "Stopping data sources"
	      $self _stopDataSources
	   }
    }
}

