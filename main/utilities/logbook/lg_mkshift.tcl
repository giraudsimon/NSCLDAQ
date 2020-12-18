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
# @file lg_mkshift
# @brief Create a new shift
# @author Ron Fox <fox@nscl.msu.edu>
#
if {[array names env DAQROOT] eq "DAQROOT"} {
    set tcllibs [file join $env(DAQROOT) TclLibs]
} else {
    set tcllibs [file normalize [file join [file dirname [info script]] .. TclLibs]]
}
lappend auto_path $tcllibs

package require logbookadmin
package require lg_utilities


##
# Usage:
#   lg_mkshift ?shiftname?
#
#  Makes a new shift. The shift name is ensured to be unique.
#  If the shift name is not provided, a GUI is popped up to allow
#  the creation and population of the shift.  If the shift name is
#  supplied, the user must add people to the shift at a later date
#  using lg_mgshifts (manage shifts).
#


proc Usage {} {
    puts stderr {Usage:}
    puts stderr {    lg_mkshift ?shiftname?}
    puts stderr {Where:}
    puts stderr {   Omitting shiftname pulls up a GUI to allow you to define}
    puts stderr {   a new shift and its members.  Supplying the optional shiftname}
    puts stderr {   creates a new shift (if the name is unique) that has no members.}
    puts stderr {   The shift management program (lg_mgshfits) can then be used to}
    puts stderr {   add members to that or any other shift.}
    exit -1
}


#-------------------------------------------------------------------
# GUI:
#   We bring up a shift creation dialog box; allow people to be added
#   to the shift and then on OK:
#   - If the shift is already in existence retry.
#   - If not create the shift.
#
proc GUI {} {
    package require Tk
    package require snit
    package require ShiftEditor
    package require dialogwrapper
    wm withdraw .
    
    
    toplevel .dialog
    DialogWrapper .dialog.dialog
    set formParent [.dialog.dialog controlarea]
    set form [ShiftEditor $formParent.form -offshift [listPeople]]
    .dialog.dialog configure -form $form
    pack .dialog.dialog -expand 1 -fill both
    
    set done 0
    while {!$done} {
        set result [.dialog.dialog modal]
        if {$result eq "Ok"} {
            ##
            #  Get shift and validate it:
            #   must not be an empty string and must not yet exist:
            #
            
            set shiftName [$form cget -shiftname]
            set members [$form cget -onshift]
            set done [makeNewShift .dialog $shiftName $members]
            
        
        } else {
            set done 1
        }
    }
    destroy .dialog 
}

#--------------------------------------------------------------------
#  Entry point
#


##
#  There must be 0 or 1 parameters:
#

if {[llength $argv] == 0} {
    GUI
} elseif {[llength $argv] == 1} {
    set shift [lindex $argv 0]
    if {[shiftExists $shift]} {
        puts stderr [duplicateShiftMessage $shift]
        exit -1
    }
    createShift $shift [list]
} else {
    Usage
}
exit 0;                    # in case we're graphic. 
