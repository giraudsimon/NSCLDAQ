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

# Helper programs:

set bindir $env(DAQBIN)
set addPerson [file join $bindir lg_addperson]
set manageShift [file join $bindir lg_mgshift]
set makeShift [file join $bindir lg_mkshift]
set selectShift [file join $bindir lg_selshift]
set editNote [file  join $bindir lg_wrnote]



package require Tk
package require logbookadmin
package require logbook
package require snit
package require dialogwrapper
package require textprompter
package require Markdown
package require lg_noteutilities
package require lg_utilities



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
#-------------------------------------------------------------------------------
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
            -columns [list id lastname firstname salutation] \
            -displaycolumns [list lastname firstname salutation] -selectmode browse -show [list tree headings]
        foreach col [list  #0 lastname firstname salutation] \
                title [list  Shift "Last Name" "First Name" "Salutation"] {
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
        set shift [lindex [$win.tree item $item -text ] 0];  #shift name.
        
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
            set values ""
            if {$shift eq $current} {
                set values  "(current)"      
            }
            $win.tree item $element -text "$shift $values" \
                  -tags [list shift]
            incr idx
        }
        # There must be additional elements not done:
        
        for {set i $idx} {$idx < [llength $shifts]} {incr idx} {
            set shift [lindex $shifts $idx]
            set values ""
            set current [currentShift]
            if {$shift eq $current} {
                set values (current)
            }
            lappend shiftElements [$win.tree insert {} end -text "$shift $values" \
                -values $values -tags [list shift]   \
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
            set shiftName [$win.tree item $shiftItem -text]
            return [lindex $shiftName 0]
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
#------------------------------------------------------------------------------
##
# @class BookView
#    Provides a view of the notes and runs that are in the logbook.  Notes are
#    associated with an author (a person) and optionally with a run.
#    The browser contains a treeview that displays folders for each run
#    and a final folder with the notes that don't belong to any run.
#    line under the runs contains one of the following:
#    *  Note - a note
#    *  Transition - documenting a transition.
#    These are sorted chronologically
#    * Run provide
#        Run number
#        Title
#        Current state.
#    Transitions provide:
#    *   The transition time
#    *   On-duty shift at the time the transition occured.
#    *   Type of transition.
#    *   any remark provided for the transition.
#    *   Below it the names of the people on that shift.
#    Notes provide:
#    *  The author
#    *  The time at which the note was written.
#    *  The note title (Part of the first lin eo fthe note.)
#
#   Double clicking in a note views its html rendition in a browser.
#   Right-clicking provides the ability to compose a new note.
#   because of the potential for this megawidget to display a large amount of
#   information, update rate defaults to every 2 seconds.
#
# OPTIONS
#    -update  - seconds between updates.
#
snit::widgetadaptor BookView {
    component tree
    variable afterid -1
    variable tmpfileIndex 0
    
    option -update -default 2
    
    constructor args {
        installhull using ttk::frame
        
        install tree using ttk::treeview $win.tree \
            -yscrollcommand [list $win.scroll set] \
            -columns [list id title author state-time remark] \
            -displaycolumns [list title author state-time remark] \
            -selectmode browse -show [list tree headings]
        
        ttk::scrollbar $win.scroll -orient vertical -command [list $tree yview]
        
        foreach col [list #0 title author state-time  remark] \
            head [list run Title Person/Shift State/Time Remark] {
            $tree heading $col -text $head
        }
        #  The tree always has an element for notes not associated with
        #  any run.  This will always be first:
        
        $tree insert {} end -text None
            
        
        # Layout
        
        grid $tree $win.scroll -sticky nsew
        grid columnconfigure $win 0 -weight 1
        grid columnconfigure $win 1 -weight 0
        grid rowconfigure    $win 0 -weight 1
        
        $self configurelist $args
        
        #
        #  Bindings:
        #    Wish I could bind <Double-1> to the note tag but that's not
        #    an event sent to bindings so we'll bind it to the whole shebang
        #    and just ignore if the item it's on isn't a note.
        #
        bind $tree <Double-1> [mymethod _onDoubleLeft]
        
        menu $tree.context -tearoff 0
        $tree.context add command -label "Compose Note..." -command [mymethod _composeNote]
        $tree.context add command -label "Make PDF from selected..."     -command [mymethod _selectedToPdf]
        $tree.context add command -label "Make PDF from whole book..."   -command [mymethod _allToPdf]
        bind $tree <3> [list $tree.context post %X %Y]
        bind $tree <KeyPress> [list $tree.context unpost]
        
        # _autoUpdate  will fill the tree with runs and notes and
        # reschedule.
        
        $self _autoUpdate
    }
    destructor {
        after cancel $afterid;       # Don't  leave any hanging afters.
    }
    #--------------------------------------------------------------------
    # Update handling
    
    ##
    # _addNote
    #   Given a  note and the container it goes in adds it to the end.
    #
    # @param parent - parent of the note.
    # @param note - note dict.
    #
    method _addNote {parent note} {
        set values [_makeNoteValueList $note]
        $tree insert $parent end -text note -values $values -tags note
    }
    
    ##
    # _updateNonRunNotes
    #    Updates the list of notes that are in the non run container.
    #    The assumption is that notes don't change nor do they get deleted.
    #    added notes, are also, therefore, in chronological order.
    #    it's also the case that id order is the same as chronological order.
    # @param parent - parent of the non-run notes.
    # @param notes  - List of note dicts not associated with any runs.
    #
    method _updateNonRunNotes {parent notes} {
        set existingNotes [$tree children $parent]
        set newNotes [lrange $notes [llength $existingNotes] end]
        foreach note $newNotes {
            $self _addNote $parent $note
        }
    }
    ##
    # _addTransition
    #    Add a transition item to a container
    # @param container - parent
    # @param transition - Transition item.
    #
    method _addTransition {container transition} {
        set values [_transitionValues $transition]
        set text [dict get $transition transitionName]
        set tid [ \
            $tree insert $container end -text $text -values $values \
            -tags transition                                        \
        ]
        # Fill in the shift members:
        
        set shiftMembers [listShiftMembers [dict get $transition shift]]
        foreach member $shiftMembers {
            set values [list                                \
                [dict get $member id] "" \
                "[dict get $member firstName] [dict get $member lastName]"
            ]
            $tree insert $tid end -values $values -tags person
        }
    }
    ##
    # _updateExistingRun
    #    Given a run that already has a container item, updates its contents.
    # @param container - the container for the run (folder if you will).
    # @param run       - Run dict.
    # @param notes    - (possibly empty) list of notes associated with the run
    #
    method _updateExistingRun {container run notes} {
        set contents [_mergeRunContents [dict get $run transitions] $notes]
        set existingItems [$tree children $container]
        set newContents [lrange $contents [llength $existingItems] end]
        
        foreach item $newContents {
            if {[dict get $item type] eq "transition"} {
                $self _addTransition $container $item
            } elseif {[dict get $item type] eq "note"} {
                $self _addNote $container $item
            } else {
                puts stderr "BookView::_updateExistingNote unrecognized item type: $type"
            }
        }
    }
    ##
    # _updateRuns
    #    Update the runs in the treeview.  The runs are in chronological
    #    order rather than run number order.  This ensures that
    #    as far as adding runs goes, we just need to add them to the end,
    #    however, each existing run might have new transitions and new
    #    notes to add.  One of the things we do first is make a chronological
    #    list of _all_ items that are associated with a run.  This, once more
    #    allows us to merely add items rather than edit them.
    #    Each run will have its state updated based on the last transition.
    #    This is a detailed state (e.g. PAUSED if the run is paused).
    #
    # @param runContainers - item ids of runs displayed in the tree.
    # @param runs - list of run dicts these should correspond to the
    #               run containers in order.
    # @param runNotes - dict indexed by run number which contains the
    #                note dicts for thtat run.
    #
    method _updateRuns {runContainers runs runNotes} {
        
        set existingRuns [lrange $runs 0 [llength $runContainers]-1]
        set newRuns      [lrange $runs [llength $runContainers] end]
        
        # For the existing notes we don't need to make a new folder.
        
        foreach runFolder $runContainers run $existingRuns {
            set number [dict get $run number]
            set notelist [dict get $runNotes $number]
            set values [_runValues $run]
            $tree item $runFolder -values $values;    # in case state changed.
            $self _updateExistingRun $runFolder $run $notelist
        }
        # For new notes we do:
        
        foreach run $newRuns {
            set number [dict get $run number]
            set values [_runValues $run]
            set container [$tree insert {} end -text $number -value $values -tags run]
            set notelist [dict get $runNotes $number]
            $self _updateExistingRun $container $run $notelist
        }
        
    }
    
    ##
    # _update
    #    Does the actual mechanics of updating the tree. This is separate
    #    from the scheduling to allow us to potentially schedule early.
    #
    method _update {} {
        
        #  Get the top level things the first one is the non run container.
        #  the remainder are runs.
        
        set toplevels [$tree children {}]
        set nonRunNoteContainer [lindex $toplevels 0]
        set runContainers       [lrange $toplevels 1 end]
        
        #  Update the non run notes.
        
        set notes [listNonRunNotes]
        $self _updateNonRunNotes $nonRunNoteContainer $notes
        
        #  Update the contents of the run containers.
        #  We need to get the run information and the notes for each run.
            
        set runs [listRuns]
        set runNotes [dict create]
        foreach run $runs {
            set runNum [dict get $run number]
            set noteList [listNotesForRun $runNum]
            dict set runNotes $runNum $noteList
        }
        $self _updateRuns $runContainers $runs $runNotes

    }
    ##
    # _autoUpdate
    #   Do and update and reschedule.
    #
    method _autoUpdate {} {
        set afterid [after [expr {1000*$options(-update)}] [mymethod _autoUpdate]]
        $self _update
        
    }
    
    ##
    # _getSelectedNote
    #   If the selected entity is a note (has the note tag) returns its id
    #   otherwise, returns an empty string.
    #
    # @return string (hopefully a note id)
    #
    method _getSelectedNote {} {
        set selected [$tree selection]
        if {[llength $selected] == 0} {
            return ""
        }
        set selected [lindex $selected 0]
        set tags [$tree item $selected -tags]
        if {"note" in $tags} {
            set values [$tree item $selected -values]
            return [lindex $values 0]
        } else {
            return ""
        }
        # Should never get here but:
        
        return ""
    }
    ##
    # _getSelectedRun
    #    Given a selected item, returns the number of the run
    #    encompassing it if it's inside a run or else returns ""
    #    if the selection isn't in a run.
    #    THis is done by looking for the tag "run" in the item and parents.
    #
    method _getSelectedRun {} {
        set selected [$tree selection]
        if {[llength $selected] == 0} {
            return ""
        }
        set selected [lindex $selected 0]
        while {$selected ne {}} {
            if {"run" in [$tree item $selected -tags]} {
                return [$tree item $selected -text]
            } else {
                set selected [$tree parent $selected]
            }
        }
        return ""
    }
    #-------------------------------------------------------------------
    # Event handling

    
    ##
    # _onDoubleLeft
    #   Handle double left click on a note:
    #   If the double click was not over a note, just give up.
    #   If the double click format the markdown for it into a file.
    #   Point the user's default webbrowser at that file.
    #
    method _onDoubleLeft {} {
        set noteId [$self _getSelectedNote]
        if {$noteId != ""} {
            set note [getNote $noteId]
            set fulltext [makeNoteMarkdown $note]
            
            set html [Markdown::convert $fulltext]
            
            set  filename [_makeNoteFilename $noteId]
            
            set fd [open $filename w]
            puts $fd $html
            close $fd
            exec xdg-open $filename &
            
        }
    }
    ##
    # _composeNote
    #     Bring up the note editor.  If the selection is inside a
    #     run when this is invoked, the note is initially associated with that run.
    #     The author can chose to associate the note with no run or another run
    #     in the composition window.
    #
    method _composeNote {} {
        set run [$self _getSelectedRun]
        if {$run ne ""} {
            exec $::editNote $run &
        } else {
            exec $::editNote &
        }
    }
    ##
    # _selectedToPdf
    #    Converts the selected thing to PDF:
    #    - If the selection is a note, it and it alone is converted to PDF.
    #    - If the selection is not a note but in a run, or a run, the entire run
    #      is converted to PDF.
    #    - If the selection is the 'None' top level item, all notes not associated
    #      with any run are converted to PDF.
    #
    #  PDF conversion is done via pandoc.  We run the pandoc command as a filter
    #  and incrementally feed it markdown.  This keeps us from bloating if
    #  asked to format something really big.
    #  If nothing is selected, then  nothing is done.
    #
    method _selectedToPdf { } {
        set run [$self _getSelectedRun];    # Run number if so.
        set note [$self _getSelectedNote];  # Note if so.
        set none 0
        set selected [$tree selection]
        if {[llength $selected] != 0} {
            if {[$tree item  [lindex $selected 0] -text] eq "None"} {
                set none 1
            }
        }
        # If we have either we prompt the output filename:
        #
        if {($run ne "") || ($note ne"") || $none} {
            set filename [_promptPDFfile]
            
            if {$filename ne ""} {
                #  Ok we got this far, there will actually be output:
                #  open the pandoc filter:
                
                set pdf [open "|pandoc - -o $filename" w]
                if {$note ne ""} {
                    _noteToFd $note $pdf
                } elseif {$run ne ""} {
                    _runToFd $run $pdf
                } elseif {$none} {
                    set notes [listNonRunNotes]
                    puts $pdf "\nNotes not associated with any run"
                    puts $pdf "==================================\n"
                    foreach note $notes {
                        set noteid [dict get $note id]
                        puts $pdf [_noteToFd $noteid $pdf]
                        puts $pdf "\n\n***\n\n"
                    }
                }
                close $pdf
            }

        }
    }
    ##
    #  _allToPdf
    #     Converts the entire logbook to pdf:
    #     All runs are formatted
    #     All notes not associated with any run are then formatted.
    #
    # PDF conversion is done via pandoc.  We run the pandoc command as a filter
    # and incrementally feed it markdown.  THis keeps us from bloating
    # if asked to format something really big.
    #
    method _allToPdf {} {
        set filename [_promptPDFfile]
        if {$filename ne ""} {
            set pdf [open "|pandoc - -o$filename" w]
            
            printall $pdf
            
            close $pdf
        }
    }
    
    #-----------------------------------------------------------------------
    # procs
    
    ##
    # _makeNoteValueList
    #   Given a note dict create the tree entry value list for that note
    # @param note
    # @return list - suitable for -values option in the notes item.
    #
    proc _makeNoteValueList {note} {
        set author [dict get $note author]
        set authorText "[dict get $author firstName]  [dict get $author lastName]"
        set time [clock format  [dict get $note timestamp] -timezone $::timezone]
        set title [getNoteTitle $note]
        
        set values [list                                                    \
            [dict get $note id] $title $authorText $time                    \
        ]
        return $values
    }
    ##
    # _getRunState
    #   Given a list of run transitions, returns the run state.
    #   (Defensive programming) if there are no transitions, the run
    #   state is "Not started'
    # @param transitions - list of transition dicts.
    # @return string - run state.
    #
    proc _getRunState {transitions} {
        if {[llength $transitions] == 0} {
            return "Not started"
        } else {
            set lastTransition [lindex $transitions end]
            set type [dict get $lastTransition transitionName]
            if {$type in [list BEGIN RESUME]} {
                return Active
            } elseif {$type in [list END "EMERGENCY_END"]} {
                return "Ended"
            } elseif {$type == "PAUSE"} {
                return "Paused"
            } else {
                return "Unknown: $type"
            }
        }
    }
    ##
    # _runValues
    #   Given a run dict, creates the run values list.  Depends as well on
    #   _getRunState to get the run state from the transition list.
    # @param run - dict for the run.
    # We produce a list that consist of:
    #     "title" "" "state" 
    #
    proc _runValues {run} {
        set title [dict get $run title]
        set state [_getRunState [dict get $run transitions]]
        return [list "" $title "" $state]
    }
    
    ##
    # _transitionValues
    #    Turn a transition dict into the things needed for its entry's -value
    #    element.  This includes:
    #
    # @param t - the transition dict.
    # @return list containing:
    #    "" Shift-name timestamp remark
    #
    proc _transitionValues {t} {
        set shift [dict get $t shift]
        set time [clock format  [dict get $t transitionTime] -timezone $::timezone]
        set remark [dict get $t transitionComment]
        
        return [list "" "" $shift $time $remark]
    }
}


################################################################################
## Entry point, make a tabbed notebook with one browser per page:

ttk::notebook .nb
PeopleViewer .nb.people
ShiftView    .nb.shifts
BookView     .nb.book

.nb add .nb.book -text LogBook
.nb add .nb.shifts -text Shifts
.nb add .nb.people -text People

pack .nb -fill both -expand 1
set exp [kvGet experiment]
set sp  [kvGet spokesperson]
set purpose [kvGet purpose]
wm title . "Logbook database: [currentLogBookOrError] ($exp spokesperson: $sp  purpose: $purpose)"

