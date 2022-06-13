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
# @file lg_addperson.tcl
# @brief Adds a new person to the database.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names env DAQROOT] eq "DAQROOT"} {
    set tcllibs [file join $env(DAQROOT) TclLibs]
} else {
    set tcllibs [file normalize [file join [file dirname [info script]] .. TclLibs]]
}
lappend auto_path $tcllibs

package require logbookadmin

##
# Usage:
#   lg_addperson lastName firstName ?salutation?
#  If no parameters are given, a GUI is popped up to allow people to be added
#  until the user is done.
# @note - there must be a current database or else this command fails.

proc Usage { } {
    puts stderr Usage:
    puts stderr {   lg_addperson ?lastname firstname ?salutation??}
    puts stderr {Where:}
    puts stderr {   Addding a person requires at least the name; salutation optional}
    puts stderr {   If there are no parameters, a GUI pops up to support adding}
    puts stderr {   as many people as you want.}
    puts stderr {NOTE:}
    puts stderr {   The logbook database file must have been selected to use this}
    exit -1
    
}

##
# GUIPrompt
#   Prompt to add people to the database.
#   We pop up a form that contains prompts for the lastname, firstname,
#   and optional salutation.  At the bottom of the form are the buttons
#   Add and Done which do about what you think they do.
#
set done 0
proc GUIPrompt {} {
    package require Tk
    package require snit
    package require textprompter
    wm withdraw .;                         # Only want our stuff up there.
    
    proc add {widget last first sal} {
        if {($first eq "") || ($last eq "")} {
            tk_messageBox -title "Need both last/first" -icon error -type ok \
                -message {Please fill in at least the last name and the first name}
        } else { 
            addPerson $last $first $sal
            $widget clear
        }
    }
    proc done {widget last first sal} {
        set ::done 1
    }
    snit::widgetadaptor PromptPerson {
        option -addcommand [list]
        option -donecommand [list]
        
        constructor args {
            installhull using ttk::frame
            
            #ttk::label $win.lastlabel -text {Last name:}
            #ttk::entry $win.last
            textprompt $win.last -text {Last name:}
            
            #ttk::label $win.firstlabel -text {First name:}
            #ttk::entry $win.first
            textprompt $win.first  -text {First name:}
            
            #ttk::label $win.sallabel -text {Salutation:}
            #ttk::entry $win.sal
            textprompt $win.sal -text {Salutation:}
            
            ttk::button $win.add -text Add -command [mymethod _dispatch -addcommand]
            ttk::button $win.done -text Done -command [mymethod _dispatch -donecommand]
            
            grid $win.last      -row 0  -sticky w
            grid $win.first      -row 1 -sticky w
            grid $win.sal        -row 2 -sticky w
            
            grid $win.add $win.done
            
            $self configurelist $args
        }
        # Public methods:
        
        method clear {} {
            $win.last  delete 0 end
            $win.first delete 0 end
            $win.sal   delete 0 end
        }
        # Private methods:
        
        method _dispatch optname {
            set script $options($optname)
            if {$script ne 0} {
                lappend script $self [$win.last get] [$win.first get] [$win.sal get]
                uplevel #0 $script
            }
        }
    }
    toplevel .prompt
    PromptPerson .prompt.person -addcommand add -donecommand done
    bind .prompt <Destroy> [list incr ::done]
    pack .prompt.person -expand 1 -fill both
    vwait ::done
}

###############################################################################
#  Entry point


if {[currentLogBook] eq "" } {
    puts stderr {There is on current logbook use $DAQBIN/lg_current to select one}
    exit -1
}

if {[llength $argv] == 0} {
    GUIPrompt
    exit 0
} elseif {[llength $argv] > 3} {
    Usage
} elseif {[llength $argv] < 2} {
    Usage
}

set last [lindex $argv 0]
set first [lindex $argv 1]
set sal   [lindex $argv 2]
addPerson $last $first $sal
exit 0