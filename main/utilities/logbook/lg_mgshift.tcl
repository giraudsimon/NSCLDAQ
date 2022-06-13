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
# @file lg_mgshift.tcl
# @brief Manage shifts.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
#  Manages shifts allowing you to:
#  -  Create a new shift (spawning off lg_mkshift in GUI mode).
#  -  Edit an existing shift.
#  -  A subset of Edit an existing shift is to create a new shift
#     starting with the people in an existing shift.
#
# Usage:
#   lg_mgshift  ?verb ?shift??
#
#  Valid verbs:
#      create  - creates a new shift... if the shift name is not
#                supplied lg_mkshift is called in GUI mode.
#                if supplied, lg_mkshift is called to create an empty shift
#                which we then edit.
#      edit    - Lets the user choose as shift which is then edited.  If a
#                shift name is supplied, that shift is used as the starting point
#                and there's no need to graphically select the starting shift.
#                By editing we mean that:
#                * People can be added to the shift.
#                * People can be removed from the shift.
#                * A new shift name can be specified allowing the creation
#                  of a new shift starting with an existing shift.
#
#     list    - if a shift is supplied, lists its members otherwise lists
#               all shifts and their members.
#
#  If no verb is given a simple GUI listing the shifts and provding buttons
#  labeled New... and Edit... where New... creates a brand new shift and
#  Edit...loads an existing shift into an editor.
#
#
#   This command is inherently GUI-ish. There is no version of it that
#   does not require a GUI to be popped up.
#
#


if {[array names env DAQROOT] eq "DAQROOT"} {
    set tcllibs [file join $env(DAQROOT) TclLibs]
} else {
    set tcllibs [file normalize [file join [file dirname [info script]] .. TclLibs]]
}

set bindir $env(DAQBIN)

lappend auto_path $tcllibs

package require logbookadmin
package require Tk
package require snit
package require ShiftEditor
package require dialogwrapper
package require lg_utilities

wm withdraw .

#--------------------------------------------------------------------

proc Usage { } {
    puts stderr "Usage:"
    puts stderr "   lg_mgshfit ?verb ?shift??"
    puts stderr "\nManage shifts:"
    puts stderr  "Valid verbs:"
    puts  stderr "    create  - creates a new shift... if the shift name is not"
    puts stderr "               supplied lg_mkshift is called in GUI mode."
    puts stderr "               if supplied, lg_mkshift is called to create an empty shift"
    puts stderr "               which we then edit."
    puts stderr ""
    puts stderr "      edit    - Lets the user choose as shift which is then edited.  If a"
    puts stderr "                shift name is supplied, that shift is used as the starting point"
    puts stderr "                and there's no need to graphically select the starting shift."
    puts stderr "                By editing we mean that:"
    puts stderr "                * People can be added to the shift."
    puts stderr "                * People can be removed from the shift."
    puts stderr "                * A new shift name can be specified allowing the creation"
    puts stderr "                  of a new shift starting with an existing shift."
    puts stderr ""
    puts stderr "      list   - list all shifts/members and contents or a single one"
    puts stderr ""
    puts stderr "  If no verb is given a simple GUI listing the shifts and provding buttons"
    puts stderr "  labeled New... and Edit... where New... creates a brand new shift and"
    puts stderr "  Edit...loads an existing shift into an editor."
    exit -1
    
}
#-----------------------------------------------------------------------------
#  Edit verb.

##
# getShiftNonShiftMembers
#   Determines which people are in or not in a shift:
#
# @param shift -name of a shift.
# @return list - the first list is a list of dicts of the people
#             in the shift, the second is the list of dicts of people
#             not in the shift.
#
proc getShiftNonShiftMembers {shift} {
    set members [listShiftMembers $shift]
    set people [listPeople]
    set memberIdList [peopleToIds $members]
    
    
    # Now we can separate the wheat from the chaff:
    
    
    set nonshift [list]
    set inshift    [list]
    foreach person [listPeople] {
        if {[dict get $person id] in $memberIdList} {
            lappend inshift $person
        } else {
            lappend nonshift $person
        }
    }
    
    return [list $inshift $nonshift]
}
##
# modifyShift
#   Given a shift, it's prior members and proposed updated members,
#   deletes the people no longer in the shfit and adds in the new ones.
#
# @param shift - name of the shift.
# @param initial - initial members of the shift.
# @param final   - final desired shift members.
#
proc modifyShift {shift initial final} {
    # Make lists of ids of initial and final so we can use in/ni
    # to determine membership or non membership
    
    set initialIds [peopleToIds $initial]
    set finalIds   [peopleToIds $final]
    
    
    #  Now for each initial not in final remove it.
    #  and foreach final not in initial add it:
    
    foreach id $initialIds {
        if {$id ni $finalIds} {
            removeMemberFromShift $shift $id
        }
    }
    set addIds [list]
    foreach id  $finalIds {
        if {$id ni $initialIds} {
            lappend addIds $id
        }
    }
    addMembersToShift $shift $addIds
         
}
##
# Edit the personnel on an existing shift.
#  - The shift editor is brought up as a dialog and stocked with the
#    members/non-members.
#  - On acceptance, if the shift changed, we try to create it.
#  - if the shift did not change, we try to modify it.
#
#  @param shift - name of the shift.
#
proc edit {shift} {
    if {$shift eq ""} {
        set shift [promptShift]
        if {$shift eq ""} return;          # aborted the request.
    }
    set shiftInfo [getShiftNonShiftMembers $shift]
    
    set initialMembers [lindex $shiftInfo 0]
    set initialNonMembers [lindex $shiftInfo 1]
    
    toplevel .dialog
    DialogWrapper .dialog.dialog
    set formParent [.dialog.dialog controlarea]
    set form [ShiftEditor $formParent.editor \
        -offshift $initialNonMembers -onshift $initialMembers -shiftname $shift]
    .dialog.dialog configure -form $form
    pack .dialog.dialog -fill both -expand 1
    
    set done 0
    while {!$done} {
        set result [.dialog.dialog modal]
        if {$result eq "Ok"} {
            # We need a non-empty shift name:
            set newshift [$form cget -shiftname]        
            if {$shift eq $newshift} {
                
                # Modify existing shift:
                
                modifyShift $shift $initialMembers [.dialog.dialog cget -onshift]
                set done 1
            } else {
                set done [makeNewShift .dialog  $newshift [$form cget -onshift]]
            }
            
        } else {
            set done 1
        }
    }
    destroy .dialog
    
    # Find the list of shifts and update them:
    
    if {[winfo exists .fullgui.frame.shifts]} {
        .fullgui.frame.shifts configure -selections [listShifts]
    }
}
#-------------------------------------------------------------------------------
# Create verb

##
# Create a new shift.
#   - if the shift is not given just call lg_mkshift to create/stock the shift.
#   - if the shift is given, create it using lg_mkshift and then
#     edit it.
proc create {{shift {}}} {
    if {$shift eq ""} {
        exec [file join $::bindir lg_mkshift]
    } else {
        if {[catch {exec [file join $::bindir lg_mkshift] $shift} msg]} {
            puts stderr $msg
            exit -1
        }
        edit $shift
    }
}
#-----------------------------------------------------------------------------
# list verb

##
# printShift
#    Prints a shift and its members.
# @param shift - name of the shift to print.
#
proc printShift {shift} {
    set members [listShiftMembers $shift];     # So if bad shift we fail early
    puts "------------------- Shift: '$shift' ---------------------------"
    puts [reportPeople $members]
}

##
# listShifts
#   If a shift is supplied, that shift and that shift alone is listed to stdout
#   if no shift is supplied, all shifts are listed:
#
# @param shift - shift to list or empty if none
#
proc printShifts {shift} {
    if {$shift eq ""} {
        foreach s [lsort [listShifts]] {
            printShift $s
        }
    } else {
        printShift $shift
    }
}
#-----------------------------------------------------------------------------
#  No verb selected:


##
# fullGuiEdit
 #    Called in response to the full Gui's Edit button:
 #    - If a shift has been selected just edit it.  If not
 #      pop up a message box indicating a shift must be selected.
 # @param w ItemSelector widget holding the choice.
 #
 proc fullGuiEdit  {w} {
    set shift [$w get]
    if {$shift eq ""} {
        tk_messageBox -parent $w -type ok -icon error -title "Shift needed" \
            -message {You must select a shift to edit from the list of shifts}
    } else {
        edit $shift
    }
 }

##
# fullPrompter
#   This is the GUI that is used if the user does not specify a verb  on the
#   command line.
#   We pop up a list of shifts and three buttons:  New... Edit.. and Done
#  *   New invokes create
#  *   Edit, requires the selection of a shift from the
#      list of shifts and invokes edit with that shift.
#  *   Done returns.
#  We handle stalling in this proc via a vwait on fullprompter  which gets
#  incremented by the done handler so we don't need to use modality for our chunk
#  of the GUI.
#
set fullprompter 0
proc fullPrompter { } {
    
    #  Create the widgets:
    
    toplevel .fullgui
    set f [ttk::frame .fullgui.frame]
    ItemSelector $f.shifts -selections [listShifts]
    ttk::button $f.new -text New... -command [list create]
    ttk::button $f.edit -text Edit... -command [list fullGuiEdit $f.shifts]
    ttk::button $f.done -text Done -command [list incr fullprompter]
    
    #Lay them out:
    
    grid $f.shifts -columnspan 3 -sticky nsew
    grid $f.new $f.edit $f.done
    pack $f -fill both -expand 1
    
    # In case the user destroys the top level rather than clicking done:
    
    bind .fullgui <Destroy> [list incr fullprompter]
    
    # Wait for Done:
        
    vwait ::fullprompter
}

#---------------------------------------------------------------
# Entry point

set verb ""
set shift ""

# Check for legality:

if {[llength $argv] > 2} {
    Usage
} else {
    if {[llength $argv] <= 2} {
        set verb [lindex $argv 0]
    }
    if {[llength $argv] == 2} {
        set shift [lindex $argv 1]
    }
}

#  Do what we need to do based on the verb (or lack of it):

if {$verb eq ""} {
    fullPrompter
} elseif {$verb eq "create"} {
    create $shift
    exit 0
} elseif {$verb eq "edit"} {
    edit $shift
    exit 0
} elseif {$verb eq "list"} {
    printShifts $shift;     # listShifts is a admin api call.
    exit 0
} else {
    puts stderr "Invalid verb '$verb'"
    Usage
}

exit -1
