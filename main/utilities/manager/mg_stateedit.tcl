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
# @file   mg_stateedit.tcl
# @brief  Editor for states and state machines.
# @author Ron Fox <fox@nscl.msu.edu>
#


set libdir [file normalize [file join [file dirname [info script]] ..  TclLibs]]

lappend auto_path $libdir


package require sequence
package require Tk
package require snit
package require sqlite3

##
# This program provides a state/state machine editor.
#   Usage:
#      mg_stateedit filename
#
#  The user interface supports adding states and editing the precursor/successor
#   states that are legal.
#

##
# @class StateEditor
#    The state editor widget.  This widget consists of  three listboxes
#
#   +-------------------------------------------------------------+
#   |  Precursors            States            Successors         |
#   |  +---------------+ +---------------+  +----------------+    |
#   |  | scrolling lb  | | Scrolling lb  |  | Scrolling lb   |    |
#   |  +---------------+ +---------------+  +----------------+    |
#   |  [         ] +     [            ] +   [              ] +    |
#   +-------------------------------------------------------------+
#
#   Clicking on a state in th middle list box requests the states
#   to be loaded into the left and right list boxes.  Clicking the +
#   button on any of the entries validates and loads the entry into the
#   list box above it.
#
# OPTIONS
#    -states          - The states in the middle list box.
#    -predecessors    - readonly content of the precursor states.
#    -successors      - readonly content of the successor states.
#    -selectcommand   - Script to run when a state is selected.
#                       Called with newstate as the value.
#
#    -precursorvalidate - Script to validate a new precursor state.
#                         Called with propsed new precursor state as a
#                         parameter, return true if it's ok.
#    -successorvalidate - Script to validate a new successor state.
#                         Called with proposed new successor state as a
#                         parameter, return true if it's ok.
#    -statevalidate  - script to validate
#
# METHODS
#    getSelection - returns the selected statename in the middle list box.
#
snit::widgetadaptor StateEditor {
    component precursors
    component states
    component successors
    
    option -states  -cgetmethod _cgetStates -configuremethod _configStates
    option -precursors -readonly 1 -cgetmethod _cgetPrecursors
    option -successors   -readonly 1 -cgetmethod _cgetSuccessors
    option -selectcommand -default [list]
    option -precursorvalidate -default [list]
    option -successorvalidate -default [list]
    option -statevalidate -default [list]
    
    constructor args {
        installhull using ttk::frame
        
        # Precursor stuff
        
        ttk::label $win.ptitle -text "Precursor states"
        install precursors using listbox $win.precursors \
            -selectmode none -yscrollcommand [list $win.pscroll set]
        ttk::scrollbar $win.pscroll -command [list $precursors yview] \
            -orient vertical
        ttk::entry $win.pentry
        ttk::button $win.padd -text + -command [mymethod _addPrecursor]  -width 1
        
        #  State stuff:
        
        ttk::label $win.stitle -text "States"
        install states using listbox $win.states \
            -selectmode single -yscrollcommand [list $win.sscroll set]
        ttk::scrollbar $win.sscroll -command [list $states yview] \
            -orient vertical
        ttk::entry $win.sentry
        ttk::button $win.sadd -text + -command [mymethod _addState]  -width 1
        
        #  Successor stuff.
        
        ttk::label $win.suctitle -text "Successor states"
        install successors using listbox $win.successors \
            -selectmode none -yscrollcommand [list $win.sucscroll set]
        ttk::scrollbar $win.sucscroll -orient vertical \
            -command [list $successors  yview]
        ttk::entry $win.sucentry
        ttk::button $win.sucadd -text + -command [mymethod _addSuccessor] -width 1
        
        #  Layout the widgets:
        
        grid $win.ptitle  -row 0 -column 0 -columnspan 2  -sticky w
        grid $win.stitle  -row 0 -column 2 -columnspan 2   -sticky w
        grid $win.suctitle -row 0 -column 4 -columnspan 2 -sticky w
        
        grid $precursors $win.pscroll $states $win.sscroll $successors $win.sucscroll -sticky nsw
        
        grid $win.pentry $win.padd $win.sentry $win.sadd $win.sucentry $win.sucadd
        
        #  Configure the selection on the state widget:
        
        bind $states <<ListboxSelect>> [mymethod _onStateSelect]
        
        $self configurelist $args
        
    }
    #--------------------------------------------------------------------------
    # Configuration methods.
    
    ##
    # _cgetStates
    #    Returns the list of states that are in the states listbox.
    #  @param option - the option that's being gotten, this is -states.
    #
    method _cgetStates {option} {
        return [$states get 0 end]
    }
    ##
    # _configStates
    #    Sets the state listbox to a new set of states:
    #    - All list boxes are cleared.
    #    - The states listbox is set with the contents of the list
    #
    # @param optname - option name (always -states).
    # @param value   - new proposed value
    # @note since we have a cget method; we don't need to set options($optname)
    #
    method _configStates {optname value} {
        $precursors delete 0 end
        $states     delete 0 end
        $successors delete 0 end
        
        $states insert end {*}$value   
    }
    ##
    # _cgetPrecursors
    #    Processes cget -predecessors
    # @param optname - option name (-precursors).
    # @return - returns the list of states in the predecessor listbox.
    #
    method _cgetPrecursors {optname} {
        return [$precursors get 0 end]
    }
    ##
    # _cgetSuccessors
    #    Process cget -successors
    # @param optname - option name (-successors)
    # @return list of states in the successors list box.
    #
    method _cgetSuccessors {optname} {
        return [$successors get 0 end]
    }
    #-------------------------------------------------------------------------
    # Event processing.

    ##
    # _onStateSelect
    #   Called when a new state is selected in the states listbox.
    #   - The precursor and successor list boxes are cleared.
    #   - If there is a -selectcommand script, it is called with the
    #     new state as a parameter and must return 2 lists:
    #    * First list is the list of precursor states that will be loaded
    #      into the precursor listbox.
    #    *  Second list is the list of successor states that will be loaded
    #       into that list box.
    #
    method _onStateSelect {} {
        #
        #  Empty the predecessor/successor listboxes:
        #
        $precursors delete 0 end
        $successors delete 0 end
        
        #  Figure out the new state -- if there is one
        
        set selected [$states curselection]
        if {[llength $selected] > 0} {
            set newState [$states get [lindex $selected 0]]
            set script $options(-selectcommand)
            if {$script ne ""} {
                lappend script $newState
                set prepost [uplevel #0 $script]
                set pre [lindex $prepost 0]
                set post [lindex $prepost 1]
                
                $precursors insert end {*}$pre
                $successors insert end {*}$post
            }
        }
    }
    ##
    # _addPrecursor
    #    If ther's a user validation script calls it to validate4 the
    #    new precursor for the current state.  If the new precusor
    #    is validated, it is added to the precursors list box.
    #    The validation script is passed three parameters:
    #     *   The current state.
    #     *   The proposed new precursor state
    #     *   The current set of precursor states
    #    If the validation script allows the new precursor it must return
    #    a boolean true otherwise a boolean false will not allow the
    #    new state to be inserted.  Any errors/reasons must be emitted by
    #    the validator.
    #
    method _addPrecursor {} {
        set valid 1;                    # Assume there's no validator.
        set proposed [$win.pentry get];   # Proposed new precursor.
        set selstate [$states curselection]
        if {[llength $selstate] > 0} {
            set script $options(-precursorvalidate)
            if {$script ne ""} {
                set state [$states get [lindex $selstate 0]]
                set existingPrior [$precursors get 0 end]
                lappend script $state $proposed $existingPrior
                set valid [uplevel #0 $script]
            }
            
        } else {
            tk_messageBox -parent $win -title "Select" -icon error -type ok \
                -message {A state must be selected to provide a new precursor}    
            set valid 0;                # A state must be selected.
        }
        
        # If still valid we can add:
        
        if {$valid} {
            $precursors insert end $proposed
        }
    }
    ##
    # _addSuccessor
    #   Essentially the same as _addPrecursor but for adding successor
    #   states.
    #
    method _addSuccessor {} {
        set valid 1;                    # Assume there's no validator.
        set proposed [$win.sucentry get];   # Proposed new precursor.
        set selstate [$states curselection]
        if {[llength $selstate] > 0} {
            set script $options(-successorvalidate)
            if {$script ne ""} {
                set state [$states get [lindex $selstate 0]]
                set successors [$successors get 0 end]
                lappend script $state $proposed $successors
                set valid [uplevel #0 $script]
            }
            
        } else {
            tk_messageBox -parent $win -title "Select" -icon error -type ok \
                -message {A state must be selected to provide a new successor}    
            set valid 0;                # A state must be selected.
        }
        
        # If still valid we can add:
        
        if {$valid} {
            $successors insert end $proposed
        }
    }
    ##
    # _addState
    #    If there's a -statevalidate script that's called to
    #    ensure the new state is ok.  The validation script is passed
    #    the proposed state and the list of existing states.
    #
    method _addState {} {
        set valid 1
        set proposed [$win.sentry get]
        set existing [$states get 0 end]
        set script $options(-statevalidate)
        
        if {$script ne ""} {
            lappend script $proposed $existing
            set valid [uplevel #0 $script]
        }
        if {$valid} {
            $states insert end $proposed
        }
    } 
}
#-------------------------------------------------------------------------------
# Utility procs



##
# Usage
#   Describes the program usage on stderr and exits.
# @param msg - error message to precede the usage info
#
proc Usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr {   $DAQBIN/mg_statedit configuration-file}
    puts stderr " Edits the run state machine to support special needs"
    puts stderr "Where:"
    puts stderr "   configuration-file - is an experiment configuration file database"
    exit -1
}
##
# _exit
#   Prompt for confirmation and if so, exit.
#
proc _exit { } {
    set reply [tk_messageBox -parent . -title {Really exit?} -icon question \
        -type yesno -message {Are you sure you want to exit?} ]
    if {$reply eq "yes"} {
        exit 0
    }
}
##
# _selectState
#    Called when a state is selected.  We return a two element list.
#    The first element of the list is the set of valid precursor states.
#    the second element of the list is the set of valid successor states.
# @param state - new selected state.
# @return list of lists  - see above.
#
proc _selectState {state} {
   set successors [::sequence::listReachableStates db $state]
   set precursors [::sequence::listLegalFromStates db $state]
   
   return [list $precursors $successors]
}

##
#  addPrecursor
#    Called when a new precursor is recommended.
#    - If the precursor is not an existing state yell -> false
#    - If the precursor is already a a precursor yell -> false.
#    - Add the valid precursor to the database.
#    - Return true allowing the precursor to be added to the database.
#
# @param db        - database command.
# @param current   - Current state.
# @param proposed  - Proposed, new precursor.
# @param existing  - Existing precursor states.
# @return bool     - true if the state is acceptable.
proc addPrecursor {db current proposed existing} {
    set allowedStates [::sequence::listStates $db]
    if {$proposed ni $allowedStates} {
        tk_messageBox -parent . -title "Invalid state" -type ok -icon error \
            -message "$proposed is not yet a defined state"
        return 0
    }
    if {$proposed in $existing} {
        tk_messageBox -parent . -title "Existing state" -type ok -icon info \
            -message "$proposed is already a precursor for $current"
        return 0
    }
    
    ## The state is allowed add it
    
    ::sequence::newTransition $db $proposed $current
    return 1
}

#-------------------------------------------------------------------------------
# Entry point.

if {[llength $argv] != 1} {
    Usage "Incorrect number of command line parameters"
}


set status [catch {sqlite3 db [lindex $argv 0]} msg]
if {$status} {
    Usage $msg
}

#  The database command is now db and we can continue.

#  In order to build our state editor we need to know the current set of states.
#
set currentStates [::sequence::listStates db]

#  Now setup the user interface.
#  This is a StateEditor widget with an Exit button at the bottom:

ttk::frame .workarea
StateEditor .workarea.editor -states $currentStates \
    -selectcommand _selectState -precursorvalidate [list addPrecursor db]
grid .workarea.editor -sticky nsew
ttk::frame .actions
ttk::button .actions.exit -text Exit -command _exit
grid  .actions.exit -sticky w
grid .workarea -sticky nsew
grid .actions  -sticky nsew

