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
#             Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file lg_utilities.tcl
# @brief Utilities for logbook commands.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide lg_utilities 1.0
package require report
package require struct

# We don't require Tk  so that using this in a non-gui
# program won't pull in Tk.  Users of the megawidgets are responsible
# for pulling in Tk themselves

package require snit

#--------------------------------------------------------------------
# Megawidgets:
#

##
# @class ItemSelector
#   Select an item from a list of items.
# OPTIONS:
#   -selections  - list of possible selections the user can choose
# METHODS
#    get         - get the text of the current selection.
#
# @note - this is just a listbox with a scrollbar and single selectmode.
#         it can be wrapped in a dialogwrapper to turn it into a
#         prompter.
#
snit::widgetadaptor ItemSelector {
    option -selections -configuremethod populate -cgetmethod getContents
    
    constructor args {
        installhull using ttk::frame
        
        listbox $win.list -yscrollcommand [list $win.scroll set] \
            -selectmode single
        ttk::scrollbar $win.scroll -orient vertical -command [list $win.list yview]
        grid $win.list $win.scroll -sticky nsew
        grid columnconfigure $win 0 -weight 1
        
        $self configurelist $args
    }
    ##  configuration management:
    
    method populate {optname optval} {
        $win.list delete 0 end
        foreach value $optval {
            $win.list insert end $value
        }
    }
    method getContents {optname} {
        return [$win.list get 0 end]
    }
    ##
    # Return the selected item contents... if nothing is selected ""
    # is returned.
    #
    method get {} {
        set result ""
        set selection [$win.list curselection]
        if {[llength $selection] > 0} {
            set index [lindex $selection 0]
            set result [$win.list get $index]
        }
        return $result
    }
    
}
##
# promptShift
#  Prompt for a shift using an ItemSelector dialog populated with
#  the shifts.  Cancel results in program exit at this stage.
#  Ok with no selector is a message for a do-over.
#
proc promptShift { } {
    set shifts [listShifts]
    toplevel .shiftprompter
    DialogWrapper .shiftprompter.dialog
    set formParent [.shiftprompter.dialog controlarea]
    set form [ItemSelector $formParent.selector -selections $shifts]
    .shiftprompter.dialog configure -form $form
    pack .shiftprompter.dialog -fill both -expand 1
    
    set done 0
    set shift ""
    while {!$done} {
        set reply [.shiftprompter.dialog modal]
        if {$reply eq "Ok"} {
            set shift [$form get]
            if {$shift eq ""} {
                tk_messageBox -parent .shiftprompter -title "Choose a shift" \
                    -type ok -icon error \
                    -message {You must choose a shift.  If there are none or you don't want to; click cancel instead}
            } else {
                set done 1
            }
        } else {
            set done 1
        }
    }
    destroy .shiftprompter
    return $shift
}
#----------------------------------------------------------
#

##
# reportPeople
#   Given a list of people as a dict, create a report string
#   of them.
#
# @param people -people dicts from e.g. listPeople or listShiftMembers
# @return string - string containing a report

proc reportPeople {people} {
    struct::matrix p
    p add columns 3
    p add row [list "Salutation " "First Name " "Last Name "]
    foreach person $people {
         p add row [list                                       \
            [dict get $person salutation] [dict get $person firstName] \
            [dict get $person lastName]                                \
        ]
    }
    report::report r [p columns]
    set result [r printmatrix p]
    r destroy
    p destroy
    return $result
}
    



