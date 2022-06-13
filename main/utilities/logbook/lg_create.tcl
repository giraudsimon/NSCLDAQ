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
# @file   lg_create.tcl
# @brief  Logbook creation 
# @author Ron Fox <fox@nscl.msu.edu>
#

##
# Create a new logbook, optionally making it the current log book or not.
#
#  Usage:  lg_create -filename file? ?-current 0|1? ?-experiment exp-id? ?-spokesperson person-name? ?-purpose experiment-purpose
#
#  If one of the elements described below is not supplied, A form will be
#  presented for all items that were not supplied that are needed.
#  *   -filename is required and is the filename into which the logbook is created.
#  *   -current  is optional; If boolean true (default), the logbook becomes
#                the current one.  If not the logbook is created but does not
#                become current.
#  *   -experiment - the FRIB experiment id - must be provided one way or another.
#  *   -spokesperson - The experiment's spokesperson.
#  *    -purpose  - The purpose of the experiment.
#
if {[array names env DAQROOT] eq "DAQROOT"} {
    set tcllibs [file join $env(DAQROOT) TclLibs]
} else {
    set tcllibs [file normalize [file join [file dirname [info script]] .. TclLibs]]
}
lappend auto_path $tcllibs

package require logbookadmin

#------------------------------------------------------------------------------
# Local procs:



##
# Usage
#   Output usage of the program to stderr and then exit.
#
proc Usage {} {
    puts stderr {Usage:}
    puts stderr {lg_create -filename file ?-current 0|1? ?-experiment expid? \ }
    puts stderr {   ?-spokesperson experimentalist? ?-purpose exp-purpose?}
    puts stderr
    puts stderr {Note that all missing information about the experiment will be}
    puts stderr {prompted graphically}
    exit -1
}
##
# dictGetWithDefault
#   Return the value of a key from a dict.  If the key is missing return
#   a default value instead:
#
# @param d   - dict.
# @param key - key to try for.
# @param default - default value if key does not exist.
# @return string - either the key value of $default if it does not exist.
#
proc dictGetWithDefault {d key default} {
    if {[dict exists $d $key]} {
        return [dict get $d $key]
    } else {
        return $default
    }
}
##
# GuiPrompt
#   Given what we have, prompt for experiment information.  Note that
#   This is done via a dialog like thing in Tk and we don't allow ourselves
#   to return until all items have been entered or Cancel is clicked or the dialog
#   Destroyed - in those latter cases we exit without doing anything.
#
# @param expid   - experiment id.
# @param spokes  - experiment spokesperson.
# @param purpose - Experiment person.
# @return dict containing:
#      *   -experiment - experiment id.
#      *   -spokesperson - Experiment spokesperson.
#      *   -purpose   - experiment purpose.
#
proc GuiPrompt {expid spokes purpose} {
    package require Tk
    package require dialogwrapper
    package require snit
    package require textprompter
    
    wm withdraw .;            # We only want the dialog to be visible.
    
    ##
    #  Here's the snit megawidget that will be used in the form part of the dialog
     
    snit::widgetadaptor ExpPrompt {
        option -experiment
        option -spokesperson
        option -purpose -configuremethod _setPurpose -cgetmethod _getPurpose
        
        constructor args {
            installhull using ttk::frame
            #ttk::label $win.explabel -text {Experiment: }
            #ttk::entry $win.exp      \
                -textvariable [myvar options(-experiment)]  -width 8
            textprompt $win.exp -text {Experiment: } \
                -textvariable [myvar options(-experiment)]  -width 8
            
            #ttk::label $win.spokeslabel -text {Spokesperson: }
            #ttk::entry $win.spokes      \
            #    -textvariable [myvar options(-spokesperson)] -width 32
            textprompt $win.spokes -text {Spokesperson: } \
                -textvariable [myvar options(-spokesperson)] -width 32
            
            ttk::label $win.purposelabel -text {Purpose:}
            text      $win.purpose  \
                -yscrollcommand [list $win.scroll set] -wrap word
            ttk::scrollbar $win.scroll -orient vertical -command [$win.purpose yview]
            
            # Layout the widgets:
            
            
            grid $win.exp -sticky w
            grid $win.spokes -sticky w
            
            grid $win.purposelabel -row 2 -column 0 -sticky w
            grid $win.purpose      -row 3 -column 0 -columnspan 2 -sticky nsew
            grid $win.scroll       -row 3 -column 2 -sticky nsew
            
            $self configurelist $args
            
        }
        #  Configuration handling for the text widget:
        
        ##
        # _setPurpose
        #   Set the text widget from the proposed value of the -purpose opt.
        #
        method _setPurpose {optname optval} {
            $win.purpose delete 0.0 end
            $win.purpose insert 0.0 $optval
        }
        ##
        # _getPurpose
        #    Get and return the value of the text widget.
        #
        method _getPurpose {optname} {
            return [$win.purpose get 0.0 end]
        }
        
        
    }
    toplevel .dialog
    DialogWrapper .dialog.dialog
    set formParent [.dialog.dialog controlarea]
    set form       [ExpPrompt $formParent.form -experiment $expid -spokesperson $spokes -purpose $purpose]
    .dialog.dialog configure -form $form
    pack .dialog.dialog -expand 1 -fill both
    
    set done 0
    while {!$done} {
        set result [.dialog.dialog modal];          # Wait for the user.
        if {$result eq "Ok"} {
            set result [dict create                                           \
                -experiment [string trim [$form cget -experiment]]             \
                -spokesperson [string trim [$form cget -spokesperson]]        \
                -purpose      [string trim [$form cget -purpose]]             \
            ]

            # If any are empty we prompt the user to fill them all and continue
            # Otherwise we're done
            
            if {([dict get $result -experiment] eq "")       ||
                ([dict get $result -spokesperson] eq "")     ||
                ([dict get $result -purpose] eq "")} {
                    tk_messageBox -title {Missing info} -icon info -type ok \
                        -message {Please fill in *all* fields -- don't leave anything blank}
                } else {
                    set done 1;                      # Got what we  needed.
                }
        }  else {
            exit -1;                              # User abort.
        }
    }
    destroy .dialog
    return $result
   
}
    


#------------------------------------------------------------------------------
#  Entry:
#   Process args like a dict:

set params [dict create {*}$argv]

if {![dict exists $params -filename]} {
    Usage
}
# Set all the bits and pieces we have to their values from dict or otherwise:

set filename [dict get $params -filename]
set makeCurrent [dictGetWithDefault $params -current 1]
set experiment  [dictGetWithDefault $params -experiment ""]
set spokes      [dictGetWithDefault $params -spokesperson ""]
set purpose     [dictGetWithDefault $params -purpose ""]

#  We need to prompt if any of experiment, spokes, or purpose are empty:

if {($experiment eq "") || ($spokes eq "") || ($purpose eq "") } {
    set expInfo [GuiPrompt $experiment $spokes $purpose]
    set experiment [dict get $expInfo -experiment]
    set spokes     [dict get $expInfo -spokesperson]
    set purpose    [dict get $expInfo -purpose]
}
#  Create the database:
    
createLogBook $filename $experiment $spokes $purpose $makeCurrent
exit 0
