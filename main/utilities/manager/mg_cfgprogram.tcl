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
# @file   mg_cfprogram.tcl
# @brief  Program to configure programs.
# @author Ron Fox <fox@nscl.msu.edu>
#

set libdir [file normalize [file join [file dirname [info script]] .. TclLibs]]
lappend auto_path $libdir

package require sqlite3
package require programedit
package require programs
package require dialogwrapper

#-----------------------------------------------------------------------------
# Global variables


#-----------------------------------------------------------------------------
# Utility procs.

##
# Usage
#   Print an error message and program usage:
#      mg_cfgprogram database-path
#
# @param msg - Error message.
#
proc Usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "   mg_cfgprogram  database-path"
    puts stderr "Where:"
    puts stderr "  database-path is the path to an already created configuration "
    puts stderr "  database path"
    exit -1
}

##
# _makeDialog
#
#    Create the dialog toplevel that contains a program::View as the form.
# @return - 3 element list containing in order:
#           *  The top level we created.
#           *  The dialogwrapper megawidget.
#           *  The program::View established as the dialog's form.
# @note - the dialog is not yet modal so the caller will need to set it to
#         be modal to accept user input.
#
proc _makeDialog { } {
    toplevel .t
    set dlg [DialogWrapper .t.wrapper]
    set c [$dlg controlarea]
    set form [program::View $c.form]
    $dlg configure -form $form
    pack $dlg -fill both -expand 1
    
    return [list .t $dlg $form]
}

##
# _loadForm
#   Given a ::program::View widget and a program definition,
#   loads the form with the definition of the program.  This supports both
#   program editing and creation of new programs based on existing program
#   definitions.
#
# @param form   - the ::program::View widget to stock.
# @param def    - The program definition to load into the form.
#
proc _loadForm {form def} {
    $form configure -name [dict get $def name]
    $form configure -image [dict get $def path]
    $form configure -type  [dict get $def type]
    $form configure -host [dict get $def host]
    
    if {[dict exists $def directory]} {
        $form configure -directory [dict get $def directory]
    }
    if {[dict exists $def container_name]} {
        $form configure -container [dict get $def container_name]
    }
    if {[dict exists $def options]}  {
        $form configure -options [dict get $def options]
    }
    if {[dict exists $def parameters]} {
        $form configure -parameters [dict get $def parameters]
    }
    if {[dict exists $def environment]} {
        $form configure -environment [dict get $def environment]
    }
    
}
##
# Creates a new program from the information in the view form.
#
# @param db  - database command.
# @param form - the form widget.
#
proc _makeNewProgram {db form} {
    set host [$form cget -host]
    set image [$form cget -image]
    set name [$form cget -name]
    
    # Check that we have the minimal set of parameters:
    
    if {($name eq "") || ($host eq "") || ($image eq "")} {
        tk_messageBox -parent $toplevel -title "Missing required" \
        -icon error -type ok  \
        -message "The program name, executable file and host are required parameters"
    } else {
        set container [$form cget -container]
        set dir       [$form cget -directory]
        set type      [$form cget -type]
        set options   [$form cget -options]
        set params    [$form cget -parameters]
        set env       [$form cget -environment]
        
        program::add $db $name $image $type $host [dict create \
            options $options parameters $params environment $env \
            directory $dir container $container
        ]
    }
}
##
# _newDefinition
#    Creates a new program definition: Brings up a program::View with no
#    pre-loaded definition in a dialogwrapper with Ok and Cancel buttons.
#    On OK, we read the definition out of the dialog and try to create a new definition.
#
# @param db - database command.
#
proc _newDefinition {db} {
    set widgets [_makeDialog]
    set toplevel [lindex $widgets 0]
    set dialog   [lindex $widgets 1]
    set form     [lindex $widgets 2]
    
    set answer [$dialog modal]
    if {$answer eq "Ok"} {
        # Pull out the stuff from the dialog:
        # If the name is a duplicate then message that out.
        
        set name [$form cget -name]
        if {[program::exists $db $name]} {
            tk_messageBox -parent $topelvel -title "Duplicate" -icon error \
                -type ok -message "A program '$name' is already defined"
        } else {
            _makeNewProgram $db $form
        }
        
    }
    
    destroy $toplevel;            # Kill off everything.
    .selection configure -programs [::program::listDefinitions $db]
    
}
##
# _editDefinition
#   Edits an existing program definition:
#    - Create the dialog.
#    - Load the form with the program defined.
#    - Start the dialog...
#    - If ok, delete the old program and create the new one.
# @param db  - the database command.
# @param def - The definition dict to load into the dialog
#
proc _editDefinition {db def} {
    set widgets [_makeDialog]
    set toplevel [lindex $widgets 0]
    set dialog   [lindex $widgets 1]
    set form     [lindex $widgets 2]
    _loadForm $form $def
    
    set answer [$dialog modal]
    if {$answer eq "Ok"} {
        set oldName [dict get $def name]
        set newName [$form cget -name]
        if {$oldName eq $newName} {
            program::remove $db $oldName
        }
        _makeNewProgram $db $form
        .selection configure -programs [::program::listDefinitions $db]
    }
    
    destroy $toplevel
}
#------------------------------------------------------------------------------
# Entry:




if {[llength $argv] != 1} {
    Usage "Incorrect number of command line parameters"
}

set file [lindex $argv 0]

sqlite3 db $file

# Let's get the list of program definitions and put up a SelectProgram
# widget.
#
#  At the bottom of that we'll put
#   A "New..." Button that allows the creation of a new program.
#   We'll also establish a -command on the program definition list that
#   allows us to edit an existing definition.
#
set definitions [program::listDefinitions db]
program::SelectProgram .selection -command [list _editDefinition db] \
    -programs $definitions
ttk::button .new -text New... -command [list _newDefinition db]

grid .selection -sticky nsew
grid .new       -sticky w


