#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file scaleControl.tcl
# @brief Megawidget to control scales of uhm...stuff.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide scaleControl 1.0
package require Tk
package require snit
package require textprompter

##
# @class ScaleControl
#    provides controls for axis scales.
#
# APPEARANCE:
# \verbatim
#   +------------------------------------+
#   |  +   -  [V zoom ]  Min:  [     ]   |
#   +------------------------------------+
#
# *  The + - are flush buttons that do a scale up or scale down.
# *  Is a pulldown menu with various zoom options.
# *  Min: - allows you to truncate the axis at the specified mininum value
#
# OPTIONS:
#    - -menulist   - List of items to put on the zoom pulldown menu.
#    - -zoomRange  - See -current as well - this sets the range of menu values
#                    over which the +/- buttons operate.  When the + button is
#                    clicked it will advance the zoom to the next item in the menu
#                    list that is within the range specified by -zoomRange.  Similarly
#                    "-" will  zoom to the prior item.  Suppose for example, the
#                    menulist is [list x1 x2 x4 x8 reset custom].
#                    Suppose that -zoomRange is [list 0 3].  Suppose -current is
#                    1x (item 0).  + will advance the zoom to 2x and - will
#                    do nothing as 0 - 1 is below the -zoomRange low limit of 0.
#   -  -current    - Current menu item text selected.  This can be modified by:
#                    * the configure subcommand  - there's no requirement then
#                      that the value be 'legal' from the point of view of
#                      +/-
#                    * the +/- buttons.
#                    * the drop down menu.
#   - -min         -  Value of the minimum.   This can be changed via explicit
#                     configuration or by modifying the Min entry.
#   - -command     -  Script invoked when -current changes.
#   - -mincommand  -  Script invoked when -min is changed.
#   - -enablemin   -  If true, min is enabled otherwise not.
#
#
#  SUBSTITUTIONS
#    The following substitutions are available for -command and -mincommand:
#
#   -   %W   - this widget.
#   -   %S   - current value.
#   -   %M   - min value

snit::widgetadaptor ScaleControl {
    component plus
    component minus
    component menubutton
    component menu
    component min
    
    option -menulist    -default [list] -configuremethod _configureMenu
    option -zoomrange   [list 0 0]
    option -current     -default ""     -configuremethod _configCurrent
    option -min         -default 0      -configuremethod _configMin
    option -command     [list]
    option -mincommand  [list]
    option -enablemin   -default true   -configuremethod _configMinWidget

    typeconstructor  {
        ttk::style configure flat.TButton -relief flat -padx 2
    }
    
    ##
    # construtor
    #    Create/layout the widgets, process our configuration
    #
    constructor args {
        installhull using ttk::frame
        
        # Make the widgets.
        
        install plus  using \
            ttk::button $win.plus  -text + -style flat.TButton -width 1 \
            -command [mymethod _plus]
        install minus \
            using ttk::button $win.minus -text - -style flat.TButton -width 1 \
            -command [mymethod _minus]
        
        install menubutton using \
            ttk::menubutton $win.zoombutton -text zoom \
            -menu $win.zoombutton.menu
        install menu using \
            menu $win.zoombutton.menu -tearoff 0;    # Filled in on -menulist config.
        
        #ttk::label $win.minlbl -text Min:
        #install min using ttk::entry $win.min -width 6 -validate focusout \
        #    -validatecommand [mymethod _validateMin %s]
        install min using textprompt $win.min -text Min: -width 6 -validate focusout \
            -validatecommand [mymethod _validateMin %s]
        $win.min insert end 0
        
        # Lay them out:
        
        grid $win.plus $win.minus $win.zoombutton  $win.min
        
        #  <Return> in $min forces validation ...which can dispatch as well.
        #
        
        bind $min <Return>   [list $min validate]
        bind $min <KP_Enter> [list $min validate]
        
        $self configurelist $args
    }
    #--------------------------------------------------------------------------
    #  Private methods:
    ##
    # _configMinWidget
    #    turn on/off the min widget's presence.
    #
    # @param optname - name of the option being configured.
    # @param value   - New boolean value - true min is displayed, false, not.
    #
    method _configMinWidget {optname value} {
        if {![string is boolean -strict $value]} {
            error "$optname must be configured with a boolean, was configured with $value"
        }
        
        
        # Only configure changes.
        if {$value != $options($optname)} {
            if {$value} {
                grid $win.minlbl -row 0 -column 3
                grid $min -row 0 -column 4
            } else {
                grid forget $min
            }
        }
        set options($optname) $value
        
    }
    ##
    # _configureMenu
    #   Called when -menulist is configures.  The existing menu is
    #   repopulated with the texts provided.
    #
    # @param optname - name of the option  (-meulist)
    # @param value   - List of options.
    #
    method _configureMenu {optname value} {
        
        #  Destroy the existing menu items:
        
        $menu delete 0 end
        
        # Each new lable is a command  item.
        
        foreach label $value {
            $menu add command -label $label \
                -command [mymethod _onMenuSelected $label]
        }
        
        
        # Update the option variable.
        
        set options($optname) $value
        
        # Set our current to the first allowed menu item:
        #  Doing it with configure also dispatches the -command.
        #
        set newCurrentIndex [lindex $options(-zoomrange) 0]
        
        $self configure -current [lindex $value $newCurrentIndex]
        
    }
    ##
    # _configCurrent
    #    Change the value of the -current option.
    #    - Set the menu button value.
    #    - set the option value.
    #    - Dispatch the user script (if any).
    #
    # @param optname - Name of the option being configured.
    # @param value   - New value.
    #
    method _configCurrent {optname value} {
        $menubutton configure -text $value
        set options($optname) $value
        $self _dispatch -command
    }
    ##
    # _configMin
    #   Configure the -min value. This just updates the option and
    #   dispatches the script.
    #
    # @param optname - Name of option being modified.
    # @param value   - New proposed value for the option.
    #
    method _configMin {optname value} {
        if {![string is double -strict $value] || ($value < 0.0)} {
            error "-min value must be a postitive number"
        }
        set options($optname) $value
        $min delete 0 end
        $min insert end $value
        $self _dispatch -mincommand
    }
    #
    ##
    # _plus
    #   Called when the + button is clicked.
    #  -  Figure out the index of the current value.  If it's not in range or
    #     does not exist, it's set to the minimum one.
    #  -  increment the index - if that's still in range use it to select the
    #     zoom value if not limit to the last one.
    #
    # Selecting the zoom value means writing a new value in the menu button and
    # setting a new -current (in a way that also pops off any command dispatch).
    #
    #  
    method _plus {} {
        set idx [lsearch -exact $options(-menulist) $options(-current) ]
        if {($idx == -1) || ($idx < [lindex $options(-zoomrange) 0] ) ||
            ($idx > [lindex $options(-zoomrange) 1])
        } {
            set newValue [lindex $options(-menulist) [lindex $options(-zoomrange) 0]]
        } else {
            if {$idx < [lindex $options(-zoomrange) 1]} {
                incr idx
            }
            set newValue [lindex $options(-menulist) $idx]
        }
        
        $self configure -current   $newValue;      # Dispatches -command  as well.
        
    }
    ##
    # _minus
    #   Called when the - button is clicked.
    #   Pretty much the same _plus above, however:
    #   -   If the current value is not in the menulist we set to the largest value.
    #   -   We limit the decrement with the lowest allowed value.
    #
    method _minus {} {
        set idx [lsearch -exact $options(-menulist) $options(-current) ]
        if {$idx == -1} {
            set newValue [lindex $options(-menulist) [lindex $options(-zoomrange) 1]]
        } else {
            if {$idx > [lindex $options(-zoomrange) 0]} {
                incr idx -1
            }
            set newValue [lindex $options(-menulist) $idx]
        }
        
        $self configure -current   $newValue;      # Dispatches -command  as well.
        
    }
    
    ##
    # _onMenuSelected
    #   Called when a menu item is selected.
    #   - configure -current to be the text of the menu item (passed as a param).
    #
    # @param value - value associated with the menu entry.
    #
    method _onMenuSelected value {
        $self configure -current $value
    }
    ##
    # _dispatch
    #   Dispatch a script option. See class header for substitutions.
    #
    #  @param optname - name of the option being dispatched.
    #
    method _dispatch optname {
        set script $options($optname)
        if {$script ne ""} {
            set script [string map \
                [list %W $self %S [list $options(-current)] %M \
                [list $options(-min)]] $script \
            ]
            uplevel #0 $script
        }
    }
    ##
    # _validateMin
    #   Ensures the min entry widget contains a valid postive number.
    #
    # @param value -  the new (string) value in the entry.
    # @return bool - True if validation succeeds.  If validation fails,
    #                $options(-min) is written in to the entry.
    # @note if the result is valid -min is configured (hencd -mincmd dispatched).
    #
    method _validateMin value {
        $min configure -validate none
        if {[string is double -strict $value] && ($value >= 0.0)} {
            $self configure -min $value
            set result 1
        } else {
            
            $min delete 0 end
            $min insert end $options(-min)
            set result 0
            tk_messageBox \
                -icon error -parent $win -title  "Invalid value" -type ok \
                -message "min value must be a positive number"
            
        }
        $min configure -validate focusout
        return $result
    }
    
}
