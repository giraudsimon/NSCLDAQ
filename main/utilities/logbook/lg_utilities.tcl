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
package require lg_noteutilities
package require logbookadmin
package require report
package require struct

# We don't require Tk  so that using this in a non-gui
# program won't pull in Tk.  Users of the megawidgets are responsible
# for pulling in Tk themselves

package require snit


#  Modify if your logbook didn't come from NSCL/FRIB

set timezone :America/Detroit

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
# Procs that need Tk:

##
# makeNewShift
#   Given a new shift and its proposed members checks that all is good
#   to create that shift and, if possible attempts to create it:
#
# @param parent   - parent over which error messages should be displayed
# @param shiftName - new shift name
# @param members   - member dict for the new shift.
# @return int        - 1 success. 0 failure.
#
proc makeNewShift {parent shiftName members} {
    set shiftName [string trim $shiftName]
            
    if {$shiftName eq ""} {
        tk_messageBox -parent $parent  -title {Specify shift} -type ok -icon error \
            -message {Please fill in the name of the new shift}
        return 0
    } elseif {[shiftExists $shiftName]} {
        tk_messageBox -parent $parent -title {Duplicate shift} -type ok -icon error \
            -message [duplicateShiftMessage $shiftName]
        return 0
    } else {
        # Shift is ok...get the members and try to create
        # the shift.  If successful we're done. If not
        # report the error and try again (should succeed unless
        # Some other user yanked a person out from us.
        # So we'll reconfigure the off/onshift values:
        
        set shiftIds [peopleToIds $members]
        
        set status [catch {
            createShift $shiftName $shiftIds
        } msg]
        if {$status} {
            tk_messageBox -parent $parent -title {Shift creation failed} \
                -type ok -icon error \
                -message "Could not make shift $shiftName: $msg"
            return 0
        } else {
            set done 1
        }
    }
}
    
##
# _promptPDFfile
#    Prompt for a new PDF filename.  We require that the filename
#    end in .pdf as pandoc does.
# @return string - the file name selected.
# @retval ""     - No legal filename was selected.
#
proc _promptPDFfile { } {
    set filename [tk_getSaveFile  -title {PDF file} \
        -defaultextension .pdf \
        -filetypes {{{PDF files} .pdf }}
    ]
    if {($filename ne "") && ([file extension $filename] ne ".pdf")} {
        tk_messageBox -title {Not PDF file} -type ok -icon error \
            -message "$filename does not end in .pdf as required by pandoc"
        set filename ""
    }
    return $filename
}

#----------------------------------------------------------
#  Procs that don't need Tk

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
    

##
# shiftExists
#    @param shiftName.
#    @return bool - true if a shift with that name already exists.
#
#
proc shiftExists {shiftName} {
    set shifts [listShifts]
    return [expr {$shiftName in $shifts}]
}

##
# duplicateShiftMessage
#    Error message for a shift is a duplicate
# @param shiftName - name of the shift
# @return string   - error message.
#
proc duplicateShiftMessage {shiftName} {
    # Get the member report string:

    
    set msg "$shiftName already exists with the following members:\n\n"
    append msg [reportPeople [listShiftMembers $shiftName]]
    append msg "\nUse lg_mgshift to add members to the shift."
    return $msg
}
##
# peopleToIds
#   Turns a list of people dicts into a list of people ids.
#
proc peopleToIds {people} {
    set result [list]
    foreach person $people {
        lappend result [dict get $person id]
    }
    return $result
}
##
# lpop
#   Given a list, remove and return the first element of that list.
# @param l - list to pop from, treated by reference.
# @return first element of the list ("" if the list is empty).
#         l is modified to remove that element.
# Sample usage:
#
#  \verbatim
#      set l [list a b c d e]
#      set first [lpop l]
#
#      # first is a, and l is [list b c d e].
#
proc lpop {l} {
    upvar $l ls
    set result [lindex $ls 0]
    set ls [lrange $ls 1 end]
    
    return $result
}

##
# _compareStamps
#   lsort compare operations for dicts that contain timestamp keys.
#
# @param item1 - first item.
# @param item2 - second item.#
# @return comparison indicator:
# @retval -1   - first item is strictly less than second.
# @retval  0   - first item is equal to second.
# @retval  1   - first item is greater than second.
#
proc _compareStamps {item1 item2} {
    set t1 [dict get $item1 timestamp]
    set t2 [dict get $item2 timestamp]
    
    if {$t1 < $t2} {return -1}
    if {$t1 == $t2} {return 0}
    if {$t1 > $t2} {return 1}
}
##
# _mergeRunContents
#    Creates a time ordered merged list of transition and note dicts.
#    Each dict also gets a new key: type which is either transition or
#    note depending on what it is.  Futhermore, transitions get a key
#    timestamp which has the same value as transition time to facilitate
#    time sorting.
#
# @param transitions - likely not empty set of run transition dicts.
# @param notes       - possibly empty set of note dicts associated with the run.
#
proc _mergeRunContents {transitions notes} {
    set result [list]
    
    # Massage the transition dicts and add them to the result list.
    
    foreach transition $transitions {
        set time [dict get $transition transitionTime]
        dict set transition type transition
        dict set transition timestamp $time
        lappend result $transition
    }
    
    # Massage the note dicts and add _them to the result list.
    
    foreach note $notes {
        dict set note type note
        lappend result $note
    }
    # Sort the result by timestamp and returnL
    
    set result [lsort -increasing -command _compareStamps $result]
    return $result
}
##
# _runHeaderToFd
#   Write a run header to a file descriptor:
#
# @param num  - run number:
# @param title - Run Title.
# @param fd   - file descriptor to which to write the header
#
proc _runHeaderToFd {num title fd} {
    puts $fd "Run number: $num : $title"
    puts $fd "================="
}
##
# _transitionToFd
#    Writes information about a transition to the fd.
#    This consist of header 2 text of the form:
#    Run Transition:  transition-type at time-stamp : comment
#   Followed by a table of on shift people.
#
# @param transition   - Transition dict.
# @param fd           - Target file descriptor.
#
proc _transitionToFd {transition fd} {
    set what [dict get $transition transitionName]
    set when [clock format \
        [dict get $transition transitionTime] -timezone $::timezone]
    set why [dict get $transition transitionComment]
    puts $fd "Run Transition: $what at $when : $why"
    puts $fd "------------------------------"
    puts $fd ""
    set shiftName [dict get $transition shift]
    set members [listShiftMembers $shiftName]
    puts $fd "|  On Shift    |"
    puts $fd "|---------------|"
    foreach member $members {
        set sal [dict get $member salutation]
        set ln  [dict get $member lastName]
        set fn  [dict get $member firstName]
        puts $fd "| $sal $fn $ln |"
    }
    puts $fd ""
}
##
# _runToFd
#    Converts a run to markdown suitable to be piped to
#    pandoc.  The run consists of a header describing the run.
#    It then consists of a series of time orderd events in the run
#    these events can be transitions to run state,
#    or notes associated with the run.
#
#  @param num   - the run numbmer to output - we are assured this exists.
#  @param  fd    - the file descriptor to which to write the note.
#  @note This proc requires the lg_noteutilities package to provide
#         _notToFd.
#
proc _runToFd  {num fd} {
    set run [findRun $num]
    set title [dict get $run title]
    set transitions [dict get $run transitions]
    set notes       [listNotesForRun $num]
    set contents    [_mergeRunContents $transitions $notes]
    
    _runHeaderToFd $num $title $fd
    foreach item $contents {
        set type [dict get $item type]
        if {$type eq "transition"} {
            _transitionToFd $item $fd
        } elseif {$type eq "note"} {
            puts $fd "\nNote:"
            puts $fd "------\n"
            _noteToFd [dict get $item id] $fd
        }
    }
}

proc printall {pdf} {
    puts $pdf "Run Logs:"
    puts $pdf "==========================="
    foreach run [listRuns] {
        _runToFd [dict get $run number] $pdf
        puts $pdf "\n***\n";      # HR between runs.
    }
    puts $pdf "Notes not associated with any run:"
    puts $pdf "=================================="
    
    set notes [listNonRunNotes]
    foreach note $notes {
        _noteToFd [dict get $note id] $pdf
        puts $pdf "\n***\n"
    }
}