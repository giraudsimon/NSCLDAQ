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
# @file lg_browse
# @brief Logbook browser.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
#  This program contains a logbook browser.
#  The logbook top level is a tabbed notebook.  The
#  tabs are:  Runs Notes Shifts and People
#  The contents of each of these tabs will be described
#  in the comment headers of its snit megawidget as will
#  how users can interact with that listing.
#  In general, however, each tab is a treeview.
#

if {[array names env DAQROOT] eq "DAQROOT"} {
    set tcllibs [file join $env(DAQROOT) TclLibs]
} else {
    set tcllibs [file normalize [file join [file dirname [info script]] .. TclLibs]]
}
lappend auto_path $tcllibs

set bindir $env(DAQBIN)
set addPerson [file join $bindir lg_addperson]
set manageShift [file join $bindir lg_mgshift]
set makeShift [file join $bindir lg_mkshift]
set selectShift [file join $bindir lg_selshift]

# Helper programs:

package require Tk
package require logbookadmin
package require snit
package require dialogwrapper
package require textprompter


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
# @class PeopleViewer
#    Tree view with the following columns:
#     lastName, firstName, salutation.
#   This is a flat (table) view and the contents are
#   auto maintained with a configurable update interval.  The
#   The widget is created with the assumption that people can't be
#   changed, can be added but not deleted.  This makes the update
#   check relatively quick.
#   A popup menu that's bound to right click allows you to add a new
#   person to the list.  This is done in a blocking way and forces an
#   update of the contents; regardless of the timing.
#
# OPTIONS:
#   -update - seconds between updates (defaults to 1).
#
snit::widgetadaptor PeopleViewer {
    option -update -default 1
    
    variable afterid -1
    ##
    # Constructor
    #   The widget adapts a ttk::frame and contains a treeview
    #    and a vertical scroll bar.  Most likely horizontal scrolling
    #   is not required.  The user can expand the treeview horizontally
    #   but not the scroll bar.  Both widgets epxand vertically as needed.
    #
    constructor args {
        installhull using ttk::frame
        
        ttk::treeview $win.tree -yscrollcommand [list $win.scroll set]
        ttk::scrollbar $win.scroll \
            -orient vertical -command [list $win.tree yview] 

        # Configure the treeview:
        
        $win.tree configure -columns [list lastname firstname salutation] \
            -displaycolumns [list lastname firstname salutation] \
            -selectmode none -show headings
        foreach col [list lastname firstname salutation] \
            title [list "Last Name" "First Name" "Salutation"] {
            $win.tree heading $col -text $title
        }

        #Arrange the widges in the frame:
        
        grid $win.tree $win.scroll -sticky nsew
        grid columnconfigure $win 0 -weight 1
        grid columnconfigure $win 1 -weight 0
        grid rowconfigure    $win 0 -weight 1
        
        # Make the tree view periodically update:
        
        $self _periodicupdate
        
        # Provide a context menu to post on <Button-3> that
        # allows the lg_adduser command to be spawned off to get a new
        # user
        
        menu $win.contextmenu -tearoff 0
        $win.contextmenu add command -label {Add...} -command [mymethod _adduser]
        bind $win.tree <3> [list $win.contextmenu post %X %Y]
        bind $win.tree <KeyPress> [list $win.contextmenu unpost]
        
    }
    destructor {
        after cancel $afterid
    }
    
    method _update {} {
        set currentPeople [listPeople]
        set contents      [$win.tree children {}]
        
        if {[llength $currentPeople] != [llength $contents]} {
            # Need to change contents.
            # - clear the tree contents.
            
            $win.tree delete $contents
            
            # - marshall the list of dicts into a list of -values lists.
            
            set valuesList [list]
            foreach person $currentPeople {
                set values [list \
                    [dict get $person lastName] [dict get $person firstName]\
                    [dict get $person salutation]\
                ]
                lappend valuesList $values
            }
            
            # - Sort that list by item 0 (last name)
            
            set valuesList [lsort -dictionary -index 0 $valuesList]
            
            # - insert items into the tree.
            
            foreach value $valuesList {
                $win.tree insert {} end -values $value
            }
        }
        
    }
    ##
    # _periodicupdate
    #   Calls update periodically using the -update option as the
    #   number of seconds between calls. Maintainng this schedule
    #   separate from the actual _update method ensures that we can
    #   do a manual update off schedule (e.g. when a person is added)
    #   without needing to play fancy feet with the scheduled after.
    #
    method _periodicupdate {} {
        $self _update
        set afterid \
            [after [expr {1000*$options(-update)}] [mymethod _periodicupdate]]
    }
    method _adduser {} {
        exec $::addPerson
        $self _update
    }
}

##
# @class ShiftView
#   This megawidget provides a self maintaining view of the
#   shifts.  A  shift is shown as a folder containig the people
#   on the shift.  A context menu  allows you to
#   - Create a new shift.
#   - Edit the shift you are on/in.
#   - Make the shift you are on/in current.
#
#  The current shift is also pointed out on the tree.
#
#  OPTIONS:
#    -update - seconds between automatic updates.
#              automatic updates support seeing changes to the view
#              in the event some external force changes the database.
#  @note lg_mgshift is used to actually do shift edits.
#  @note lg_selsfhit is used to select a current shift.
snit::widgetadaptor ShiftView {
    option -update -default 1;                # Default up date rate.
    
    variable afterid -1
    variable lastCurrentShift ""
    
    constructor args {
        installhull using ttk::frame
        
        ttk::treeview $win.tree -yscrollcommand [list $win.scroll set]
        ttk::scrollbar $win.scroll -orient vertical \
            -command [list $win.tree yview]
        
        # Configure the tree:
        
        $win.tree configure \
            -columns [list shift id lastname firstname salutation] \
            -displaycolumns [list shift lastname firstname salutation] -selectmode browse -show [list tree headings]
        foreach col [list shift lastname firstname salutation] \
                title [list Shift "Last Name" "First Name" "Salutation"] {
            $win.tree heading $col -text $title
        }
            
        
        # Layout the widgets:
        
        grid $win.tree $win.scroll -sticky nsew
        grid columnconfigure $win 0 -weight 1
        grid columnconfigure $win 1 -weight 0
        grid rowconfigure    $win 0 -weight 1
        
        # Start auto update - that'll do the initial population.
        
        $self _automaticUpdate
        
        # Set bindings:
        
        menu $win.contextmenu -tearoff 0
        $win.contextmenu add command \
            -label {Add Shift...} -command [mymethod _newShift]
        $win.contextmenu add command \
            -label {Edit...} -command [mymethod _editShift]
        $win.contextmenu add command \
            -label {Set Current} -command [mymethod _makeShiftCurrent]
        
        bind $win.tree <3> [list $win.contextmenu post %X %Y]
        bind $win.tree <KeyPress> [list $win.contextmenu unpost]
        
        
    }
    destructor {
        after cancel $afterid
    }
    ##
    # _update1ShiftMembers
    #    Update the members of a single shift:
    # @param item - the item id of the shfit to update.
    #
    method _update1ShiftMembers {item} {

        set children [$win.tree children $item]
        set currentValueList [list]
        foreach child $children {
            set values [$win.tree item $child -values]
            set values [lrange $values 1 end]
            lappend currentValueList $values
        }
        set shift [lindex [$win.tree item $item -values] 0];  #shift name.
        set members [listShiftMembers $shift]
        
        # turn the current set of members into a list of values that
        # belong in the elements.  This will then be sorted by lastname
        # and we'll update the entire set if the length is different.
        # or any ids differ.  We need to worry about the latter in case this
        # item has been assigned a different shift.
        
        set valueList [list]
        foreach member $members {
            lappend valueList [list \
                [dict get $member id] [dict get $member lastName] \
                [dict get $member firstName] [dict get $member salutation] \
            ]
        }
        # Sort the resulting list of values by last name:
        
        set valueList [lsort -dictionary -index 1 $valueList]
        
        # If the lists are not the same, totally restock:
        
        if {$valueList ne $currentValueList} {
            $win.tree delete $children
            foreach values $valueList {
                set values [list "" {*}$values];   # Prepend empty shift.
                $win.tree insert $item end -values $values -tags [list person]
            }
        }
    }
    
    ##
    # _updateShiftMembers
    #   For each shift, if necessary update the members in that shift.
    #
    # @param elements -- the item identifiers for each shift.
    #
    method _updateShiftMembers {elements} {
        foreach item $elements {
            $self _update1ShiftMembers $item
        }
    }
    # _updateShifts
    #
    #    _update the set of shifts in the tree view
    #
    # @param shifts - the current set of shifts dictionary sort order.
    # @param shiftElements - the tree elements containing the shifts.
    # @return updated shift elements
    #
    method _updateShifts {shifts shiftElements} {
        set idx 0
        set current [currentShift]
        ##
        #  Set the existing elements
        #
        foreach element $shiftElements {
            set shift [lindex $shifts $idx]
            set values $shift
            if {$shift eq $current} {
                lappend values "" "(current)"      
            }
            $win.tree item $element -values $values -tags [list shift]
            incr idx
        }
        # There must be additional elements not done:
        
        for {set i $idx} {$idx < [llength $shifts]} {incr idx} {
            set shift [lindex $shifts $idx]
            set values $shift
            set current [currentShift]
            if {$shift eq $current} {
                lappend values "" (current)
            }
            lappend shiftElements [$win.tree insert {} end -values $values \
                -tags [list shift]
            ]
                
        }
        return $shiftElements
        
    }
    ##
    # _update
    #   Perform a full tree update:
    #   - If new shifts have been added put them in sorted place.
    #   - Update the current shift.
    #   - For each shift, if it's members changed, update the member list.
    #
    method _update {} {
        
        #  The shift folder list only needs to be updated
        #  if the number of shifts changed
        
        set shifts [lsort -dictionary [listShifts]]
        set shiftElements [$win.tree children {}];  # They're the top level.
        set current [currentShift]
        if {([llength $shifts] != [llength $shiftElements]) ||
            ($current ne $lastCurrentShift)} {
            set lastCurrentShift $current
            $self _updateShifts $shifts $shiftElements
        }
        $self _updateShiftMembers $shiftElements
        
        
    }
    method _automaticUpdate {} {
        $self _update
        set afterid [after [expr {$options(-update)*1000}] [mymethod _automaticUpdate]]
    }
    ##
    # _newShift
    #    Get a new shift name from the user via a modal dialog
    #    Try to create a new shift with that name and
    #    force an update.
    #
    method _newShift {} {
        set name [textprompt_dialog $win.newshift "Shift" -text "Shift Name: "]
        if {$name ne ""} {
            exec $::makeShift $name
        }
    }
    ##
    # _selectedShift
    #    @return string - the selected shift name or "" if none is selected
    #
    method _selectedShift {} {
        set selected [$win.tree selection]
        if {[llength $selected] > 0} {
            set selection [lindex $selected 0] ;           # should be one.
            set tags [$win.tree item $selection -tags]
            set shiftItem ""
            if {"person" in $tags} {
                set shiftItem [$win.tree parent $selection]
            } elseif {"shift" in $tags} {
                set shiftItem $selection
            } else {
                return "";               # Neither a shift nor a person.
            }
            set shiftName [lindex [$win.tree item $shiftItem -values]  0]
            return $shiftName
        } else {
            return ""
        }
    }
    ##
    # _editShift
    #   Figure out the shift that's currently selected and bring up
    #   $DAQBIN/lg_mgshift to edit that shift.
    #
    #   - If there's no selected item just bag it.
    #   - If the selected item has the tag 'person' we're in a person and have
    #     to get our parent.
    #   - If the selected item has the tag 'shift' we're in a shift and can just
    #     get its name.
    #   - If the selected item has neither tag again just bag it.
    #
    method _editShift {} {
        set shiftName [$self _selectedShift]
        if {$shiftName ne ""} {
            exec $::manageShift edit $shiftName
        }
    }
    method _makeShiftCurrent {} {
        set shiftName [$self _selectedShift]
        if {$shiftName ne ""} {
            exec $::selectShift $shiftName
        } 
    }
}
