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
# @file  mg_seqedit.tcl
# @brief  Sequence editor
# @author Ron Fox <fox@nscl.msu.edu>
#

set libdir [file normalize [file join [file dirname [info script] ] .. TclLibs ]]
lappend auto_path $libdir

package require sequence
package require programs
package require Tk
package require snit
package require selectablelist
package require dialogwrapper
package require sqlite3

#------------------------------------------------------------------------------
# Megawidgets:

##
# @class sequence::StepLister
#    Lists the steps in a sequence.
#
# OPTIONS:
#    -steps   - list of dicts that define the steps.
#    -command - Command invoked when a step is clicked.
# COMMANDS1
#   bind      - Provide an event binding for step(s).
#   tree      - Returns the tree widget.  Note that 'overuse' of this may
#               indicate a need to add options or methods to the
#               megawidget.
#   itemAt    - useful for context menu handlers.. given window coordinates,
#               Returns the dict for the step taht matches or
#   
#              
# @note
#    This is just a treeview.  The visible columns are:
#    -  Step numbver.
#    -  Program name.
#    -  Predelay
#    -  Post delay.
#   A hidden column contains the full dict that defines the step.
#
#
snit::widgetadaptor sequence::StepLister {
    component list
    option -steps -default [list] -configuremethod _cfgSteps -cgetmethod _cgetSteps
    option -command
    
    constructor args {
        installhull using ttk::frame
        
        install list using ttk::treeview $win.list
        $list configure \
            -columns [list step program pre-delay post-delay definition] \
            -displaycolumns [list step program pre-delay post-delay]     \
            -show [list headings] -yscrollcommand [list $win.vscroll set]      \
            -selectmode browse
        
        #  Add the heading text:
        
        foreach column [list step program pre-delay post-delay] \
            heading [list Step Program "Pre delay" "Post Delay"] {
            $list heading $column -text $heading -anchor w
        }
        
        #  Support the -command option by binding to the select event:
        
        bind $list <<TreeviewSelect>> [mymethod _onSelect]
        bind $list <Double-Button-1>        [mymethod _unselect]
        
        ttk::scrollbar $win.vscroll -command [list $list yview] -orient vertical
        
        grid $list $win.vscroll -sticky nsew
        
        $self configurelist $args
        
    }
    #--------------------------------------------------------------------------
    # Configuration methods.
    
    ##
    # _cfgSteps
    #   Configure the steps.  This means:
    #   - Clearing the treeview.
    #   - Adding each element of the list passed in in the order in which it
    #     is passed in. Note that each element is given a pair of tags, specifically:
    #     *   It's step number.
    #     *   The 'step' tag.  These are used to support the bind public method.
    #
    # @param optname - option name
    # @param value   - proposed option value.
    # @note - there's no need to set options(-steps) as we'll use a cget method
    #         to retrieve the steps.
    # @note - errors due to invalid $value elements can result in a badly/incompletely
    #         formatted tree.
    #
    method _cfgSteps {optname value} {
        $list delete [$list children {}];      # Clear the tree.
        
        foreach step $value {
            set stepno [dict get $step step]
            set progname [dict get $step program_name]
            set predelay [dict get $step predelay]
            set postdelay [dict get $step postdelay]
            
            $list insert {} end \
                -values [list $stepno $progname $predelay $postdelay $step] \
                -tags [list step $stepno]
        }
    }
    ##
    # _cgetSteps
    #   Return the list of steps that are in the tree.  The return value
    #   is the list of step dicts (the value in the invisible column  at the
    #   end of each entry's column dat).
    #
    # @param optname - option name - ignored.
    #
    method _cgetSteps {optname} {
        set items [$list children {} ]
        set result [list]
        foreach item $items {
            set info [$list item $item -values]
            lappend result [lindex $info end]
        }
        
        return $result
    }
    
    #---------------------------------------------------------------------------
    #  Event handling
    
    ##
    # _onSelect
    #   The selection changed.
    #   Invoke the user's -command (if there is one) with the
    #   dict of the selected item.
    #   If there is no selection an empty list is passed.
    #
    method _onSelect {} {
        set script $options(-command)
        if {$script ne ""} {
            set item [$list selection]
            set arg [list]
            if {$item ne ""} {;     # Browse select means there's only one.
                set arg [lindex [$list item $item -values] end]
            }
            lappend script $arg
            uplevel #0 $script
        }
    }
    
    ##
    # _unselect
    #   Control-mouse-1 will remove any existing selection.
    #
    method _unselect {} {
        set selection [$list selection]
        if {[llength $selection] != 0} {
            $list selection remove $selection
        }
    
    }
    #---------------------------------------------------------------------------
    # Public methods
    
    ##
    # bind
    #    Allows users to bind an event either to all elements of the tree
    #    or to a specific step.
    #
    # @param event  - specification of the event to bind.
    # @param script - the script to bind
    # @param ?step? - optional step number to which the script is bound
    #                 if omitted the event/script is bound to all steps.
    # @note The normal value rules for the script hold.
    # @note This is implemented using the $list tag bind.  Each element of the
    #       tree is given two tags:  The step number and a 'step' tag
    #       these are used in performing the actual binding operation.
    # @note  Since tag bindings are used, the only events that are supported are:
    #       -   <KeyPress>
    #       -   <KeyRelease>
    #       -   <ButtonPress>
    #       -   <ButtonRelease>
    #       -   <Motion>
    #  To use any other binding you'll need to bind to the tree widget directly
    #  and if needed convert mouse positions to item indices.
    #
    method bind {event script {step {}}} {
        set tag step;            # If none is given
        if {$step ne ""} {
            set tag $step;      # But one was given.
        }
        $list tag bind $tag $event $script
    }
    ##
    # tree
    #   Return the treeview widget.  Hopefully, this is mostly used to
    #   support event bindings that are not supported by tag bindings however:
    #   If this gets overused, it might be best to think about adding functionality
    #   to the megawidget itself (e.g. in bindings, it may be useful to have
    #   substitutions that provide step information from the pointer position
    #   rather than extracting that from megawidget itself.
    #
    # @return widget-path = the fll path to the treeview widget:
    #
    method tree {} {
        return $list
    }
    ##
    # itemAt
    #   Given a widget x/y returns the dict that describes the element at that
    #   position.
    #
    # @param x   - x position.
    # @param y   - y position.
    # @return dict - Dict of the element at x/y
    # @retval    - empty dict - if there's no element under x/y.
    #
    method itemAt {x y} {
        set element [$list identify item $x $y]
        if {$element eq ""} {
            return [dict create]
        } else {
            set data [$list item $element -values]
            return [lindex $data end]
        }
    }   
}


#------------------------------------------------------------------------------
#  Utility procs:

##
#  Usage
#   Error message output with usage and exit.
# @param msg - the message.
#
proc Usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "   \$DAQROOT/mg_seqedit configuration-database"
    puts stderr "  Edits the sequences in the configuration database."
    puts stderr "Where:"
    puts stderr "   configuration-database is the experiment configuration."
    exit -1
}
##
# _getTrigger
#    @param db   - database  command.
#    @param name - Sequence name.
#    @return string- Name of the sequence's trigger.
#    @note in the unlikely event there's no match, an empty string is returned.
#
proc _getTrigger {db name} {
    set sequences [::sequence::listSequences $db]
    foreach seq $sequences {
        set seqname [dict get $seq name]
        set trigger [dict get $seq transition_name]
        if {$seqname eq $name} {
            return $trigger
        }
    }
    return ""
}
##
# _populateSequenceList
#    -  Clear the list of sequences.
#    -  Loes them with the current set.
#
# @param db    - database command.
# @param widget - the widget.
#
proc _populateSequenceList {db widget} {
    set sequences [::sequence::listSequences $db]
    $widget delete 0 end
    
    foreach s $sequences {
        $widget insert end [dict get $s name]
    }
}

##
# _deleteSequence
#    Gives the user the option to completey delete the sequence:
#    If approved:
#    - The sequence is deleted from the database.
#    - The squence editor top leve lis destroyed.
#    - The .sequences list of sequences is repopulated.
#
#  @param db    - database access command.
#  @param name  - Sequence name.
#  @param seqeditor - sequenceeditor top level widget
#
proc _deleteSequence {db name seqeditor} {
    set reply [tk_messageBox -parent $seqeditor -type yesno -icon question  \
        -title "Delete $name" \
        -message "Really delete sequence '$name'?  This action cannot be undone."
    ]
    if {$reply eq "yes"} {
        ::sequence::rmvSequence $db $name
        destroy $seqeditor
        _populateSequenceList $db .sequences
    }
}
    
#_newSequence
#   Adds a new sequence (named by the contents of .new) to the list of
#   sequences.
#  @param db      - the database command.
#  @param newwid  - the entry widget that has the name of the new sequence.
#  @param trigwid - The combobox widget that has the triggering state.
#  @param seqwid  - The list box like object that has the list of sequences.
#
proc _newSequence {db newwid trigwid seqwid} {
    set newName [$newwid get]
    set trigName [$trigwid get]
    
    # Prevent duplicate:
    
    if {$newName in [$seqwid get 0 end]} {
        tk_messageBox -parent . -title {Duplicate} -icon error -type ok \
            -message "There is already a sequence named $newName  " 
        
        return
    }
    # Require valid trigger state:
    
    if {$trigName ni [::sequence::listStates $db]} {
        tk_messageBox -parent . -title {Invalid trigger state} -icon error \
            -type ok -message "$trigName is not a valid state name."
        return
    }
    # Stuff should work now:
    
    ::sequence::add $db $newName $trigName
    $seqwid insert end $newName
}
#-------------------------------------------------------------------------------
# Seauence editing.

variable currentSequence ""
variable selectedStep "";               # Dict of selected seq or empty.
variable seqEditor "";                  # Sequence listbox widget.
variable menuX     0;                   # Where the context menu posted in 
variable menuY     0;                   # listbox window coordinates.

##
# _selectStep
#    Called when a step is selected.  We just save that in the selectedStep
#    variable.
#
# @param step - the dict that decribes the selected step.
#
proc _selectStep {step} {
    variable selectedStep
    set selectedStep $step
}
##
# _locateStep
#    Returns the index of the step in a list of steps with the requested
#    step number.
#
# @param step  - Number of the step to locate.
# @param steps - List of step indices.
# @return integer - list index or -1 if not found.
#
proc _locateStep {step steps} {
    set index 0
    foreach s $steps {
        if {[dict get $s step] == $step} {
            return $index
        }
        incr index
    }
    return -1
}

##
# _addStep
#   Adds a new step to the sequence being edited.  If there is a selected step,
#   the new step is added above it.  If not, the new step is added at the end
#   of all steps.  We do ensure that the step is fully specified.
#
# @param frame
#
proc _addStep {frame} {
    variable selectedStep
    variable seqEditor
    
    # Let's pull all the stuff from the frame and ensure we have what we need:
    
    set program [$frame.programs get]
    set pre     [$frame.pre get]
    set post    [$frame.post get]
    
    if {$program eq ""} {
        tk_messageBox -parent $frame -title "Missing program" -icon error \
         -type ok    -message "You must select a program for this step"
        return
    }
    if {($pre < 0) || ($post < 0)} {
        tk_messageBox -parent $frame -title "bad delays" -icon error -type ok \
            -message "The delays must be at least 0 seconds."
        return
    }
    
    #  There's a bit of a cheat here.  the only stuff we really care about in
    #  the dict.  The only things needed to define steps to the sequence
    #  package and widget are the step, program_name predelay and postdelay
    
    set existingSteps [$seqEditor cget -steps]

    if {$selectedStep eq ""} {
    
        if {[llength $existingSteps] == 0} {
            # New only step
            
            set newStepno $::sequence::step_interval
        } else {
            
            # New last step amongst others.
            set newStepno [dict get [lindex $existingSteps end] step ]
            set newStepno [expr {$newStepno + $::sequence::step_interval}]
        }
        
        lappend existingSteps [dict create \
                step         $newStepno    \
                program_name $program      \
                predelay     $pre          \
                postdelay    $post         \
        ]
        
    } else {
        # We are going to put the new step prior to the selected step.
        # That means we need to find the selected step...we'll do that by
        # finding the step number.
        
        set selStepno [dict get $selectedStep step]
        set afterIndex [_locateStep $selStepno $existingSteps]
        
        #  If the index is 0, special case where we prepend the whole list:
        
        if {$afterIndex == 0} {
            set newStepno [expr {$selStepno/2}];    # Step number of the new step.
            
        } else {
            # It's betweens two steps.
            
            set priorStepno [dict get                               \
                [lindex $existingSteps [expr {$afterIndex-1}]] step \
            ]
            set newStepno [expr {($priorStepno+$selStepno)/2.0}]
            
        }
        # Put the new list element prior to the afterIndex:
        
        set existingSteps [linsert $existingSteps $afterIndex [dict create              \
                step         $newStepno                                     \
                program_name $program                                       \
                predelay     $pre                                           \
                postdelay    $post                                          \
            ]                                                               \
        ]
        
    }
    $seqEditor configure -steps $existingSteps
}
##
# _createNewStepUI
#    Create the chunk of the user interface that adds a new step.
#
# @param frame - frame in which to build it.
# @param db    - database command.
#
proc _createNewStepUI {frame db} {
    set programDefs [::program::listDefinitions $db]
    set namelist [list]
    foreach def $programDefs {
        lappend namelist [dict get $def name]
    }
    ttk::label     $frame.programlbl -text {Program name}
    ttk::combobox $frame.programs   -values $namelist
    
    ttk::label     $frame.prelbl     -text {Pre delay}
    ttk::spinbox   $frame.pre        -from 0 -to 60 -increment 1
    $frame.pre set 0
    
    ttk::label $frame.postlbl        -text {Post delay}
    ttk::spinbox $frame.post         -from 0 -to 60 -increment 1
    $frame.post set 0
    
    ttk::button $frame.add -text Add -command [list _addStep $frame]
    
    grid $frame.add $frame.programlbl $frame.programs \
     $frame.prelbl $frame.pre $frame.postlbl $frame.post -sticky w
    
    
    
}
##
# _deleteStep
#    Delete the step on which the context menu posted.
#
# @note uses menuX, menuY to figure out which that was:
#
proc _deleteStep {} {
    variable seqEditor
    variable menuX
    variable menuY
    variable selectedStep
    
    set step [$seqEditor itemAt $menuX $menuY]
    if {$step ne ""} {
        set stepNo [dict get $step step]
        set stepList [$seqEditor cget -steps]
        set stepIndex [_locateStep $stepNo $stepList]
        set stepList [lreplace $stepList $stepIndex $stepIndex]
        $seqEditor configure -steps $stepList
    }
    set selectedStep ""
    
}
##
# _moveUp
#   If the step on which the context menu posted was not the first,
#   move it up one position in the step list.  This requires recomputing the
#   step number, of course.  We're going to reduce the step definition dict to
#   the set of things actually needed to define the step to the sequence::addStep#
#   proc.
#
# @note we use menuX, menuY and seqEditor variables.
#
proc _moveUp {} {
    variable menuX
    variable menuY
    variable seqEditor
    
    #  Which one was picked:
    
    set selectedStep [$seqEditor itemAt $menuX $menuY]
    if {$selectedStep ne ""} {
        set selectedNum [dict get $selectedStep step]
        set existingSteps [$seqEditor cget -steps]
        
        set stepIndex [_locateStep $selectedNum $existingSteps]
        if {$stepIndex > 0} {
            #  Otherwise can't move up.
            
            # Get information about the step - we need to recompute the step number
            # and put it where it belongs.
            
            set program [dict get $selectedStep program_name]
            set pre     [dict get $selectedStep predelay]
            set post    [dict get $selectedStep postdelay]
            
            # Two cases - this becomes the new first step or not.
            
            if {$stepIndex == 1} {
                # Will be new first.
                
                set existingSteps [lreplace $existingSteps $stepIndex $stepIndex]
                set firstNum  [dict get [lindex $existingSteps 0] step]
                set stepnum   [expr {$firstNum/2.0}]
                set existingSteps [linsert $existingSteps 0 [dict create     \
                        step         $stepnum                                \
                        program_name $program                                \
                        predelay     $pre                                    \
                        postdelay    $post                                   \
                    ] \
                ]
                
                
            } else {
                # Won't be new first.
                
                set existingSteps [lreplace $existingSteps $stepIndex $stepIndex]
                
                # We're between stepIndex-1 and stepIndex -2
                
                set subsequent [dict get [lindex $existingSteps $stepIndex-1] step]
                set prior      [dict get [lindex $existingSteps $stepIndex-2] step]
                set newStepno [expr {($prior + $subsequent)/2.0}]
                
                set existingSteps [linsert $existingSteps $stepIndex-1 \
                    [dict create                                      \
                        step          $newStepno                      \
                        program_name  $program                        \
                        predelay      $pre                            \
                        postdelay     $post                           \
                    ]                                                 \
                ]
                
            }
        }
        $seqEditor configure -steps $existingSteps
    }
}
##
# _moveDown
#    Moves the item the menu posted on down one slot.
# @note we use menuX, menuY and seqEditor variables.
#
proc _moveDown { } {
    variable menuX
    variable menuY
    variable seqEditor
    
    # which one was picked:
    
    set selectedStep [$seqEditor itemAt $menuX $menuY]
    if {$selectedStep ne ""} {
        set selectedNum [dict get $selectedStep step]
        set existingSteps [$seqEditor cget -steps]
        
        # We can't move it down if its already last:
        
        set selectedIndex [_locateStep $selectedNum $existingSteps]
        set lastIndex [expr {[llength $existingSteps] -1}]
        if {$selectedIndex <  $lastIndex} {
            
            #  Pick out the information about the step we're moving
            
            set program [dict get $selectedStep program_name]
            set pre     [dict get $selectedStep predelay]
            set post    [dict get $selectedStep postdelay]
            
            if {($selectedIndex + 1) == $lastIndex} {
                # We're going to become the last step.
                
                set lastStep [dict get [lindex $existingSteps end] step]
                set newStep  [expr {$lastStep + $::sequence::step_interval}]
                
                # Remove the step and put it last:
                
                set existingSteps [lreplace \
                    $existingSteps $selectedIndex $selectedIndex \
                ]
                
                lappend existingSteps  [dict create \
                    step         $newStep           \
                    program_name $program           \
                    predelay     $pre               \
                    postdelay    $post              \
                ]
                
                
            } else {
                # We won't become the last step.  The new step will be between
                # once we remove the step from the existing steps, the
                # step will get dropped between the selectedIndex and selectedIndex+1
                #
                
                set existingSteps [lreplace \
                    $existingSteps $selectedIndex $selectedIndex \
                ]
                set priorStepno [dict get \
                    [lindex $existingSteps $selectedIndex] step \
                ]
                set nextStepno [dict get \
                    [lindex $existingSteps $selectedIndex+1] step \
                ]
                set newStep [expr {($priorStepno + $nextStepno)/2.0}]
                set existingSteps [linsert $existingSteps $selectedIndex+1 \
                    [dict create                     \
                        step         $newStep        \
                        program_name $program        \
                        predelay     $pre            \
                        postdelay    $post           \
                    ]                                \
                ]
                
            }
        
            
            $seqEditor configure -steps $existingSteps
        }
    }
}

##
# _postContextMenu
#   Posts a context menu:
#
# @param menu - the menu to post.
# @param rootX - The root window X position at which to post.
# @param rootY - The root window Y position at which to post.
# @param widX  - The widget X position at which to post.
# @param widY  - The widget Y position at which to post.
#
# @note widX, widY are just saved in menuX and menuY to establish which
#                  or what this context is about.
#
proc _postContextMenu {menu rootX rootY widX widY} {
    variable menuX
    variable menuY
    
    set menuX $widX
    set menuY $widY
    
    $menu post $rootX $rootY

}
##
# _saveSequence
#   Save the sequence in the sequence editor.
#   -  Remove all current steps from the sequence.
#   -  Add in the steps in the widget.
#   -  Destroy the widget's top level.
#
# @param db      - database command.
# @param seqName - name of the sequence we're editing
# @param steps   - Widget containing the current steps.
# @note The step numbers will be re-assigned by the sequence package when we
#       re-build the sequence.
#
proc _saveSequence {db seqName steps} {

    # Kill off the old steps.

    set oldSteps [::sequence::listSteps $db $seqName]
    foreach s $oldSteps {
        set stepno [dict get $s step]
        ::sequence::rmvStep $db $seqName $stepno
    }
    #  Generate the steps in the widget:
    
    set newSteps [$steps cget -steps]
    foreach s $newSteps {
        set program [dict get $s program_name]
        set pre     [dict get $s predelay]
        set post    [dict get $s postdelay]
        
        ::sequence::addStep $db $seqName $program $pre $post
    }
    #  Now destroy the editor window.
    
    destroy [winfo toplevel $steps]
}
##
# _sequenceSelected
#   Reponds to a double click on a sequence.  We will bring up a sequence
#   editing dialog and, once that's filled in, update the sequence appropriately.
#
# @param -  db - the database command.
# @param - seqName -name of the sequence that must be edited.
#
proc _sequenceSelected {db seqName} {
    variable currentSequence
    variable seqEditor
    
    #  Open a new topelevel widget and put in it:
    #   - A StepLister with the steps from the selected sequence
    #   - A step addition frame.
    #   - A save button.
    #   - A cancel button.
    #
    
    if {![winfo exists .seqeditor]} {
        toplevel .seqeditor
        set seqEditor [sequence::StepLister .seqeditor.list \
            -steps [::sequence::listSteps $db $seqName]  \
            -command [list _selectStep] \
        ]
        set triggerName [_getTrigger $db $seqName]
        ttk::frame .seqeditor.info
        ttk::label .seqeditor.info.info -text "Trigger: $triggerName"
        grid .seqeditor.info.info -sticky w
        
        set newstep [ttk::labelframe .seqeditor.newstep -text {Define step}]
        _createNewStepUI $newstep $db
        
        
        set actions [ttk::frame .seqeditor.actions]
        ttk::button $actions.delete -text Delete...  \
            -command [list _deleteSequence $db $seqName .seqeditor]
        ttk::button $actions.save -text Save \
            -command [list _saveSequence $db $seqName .seqeditor.list]
        ttk::button $actions.cancel -text Cancel -command [list destroy .seqeditor]
        
        grid $actions.save $actions.delete $actions.cancel -sticky w
        grid .seqeditor.list -sticky nsew
        grid $newstep        -sticky nsew
        grid $actions        -sticky nsew
        grid .seqeditor.info -sticky nsew
        
        set currentSequence $seqName
        
        #  We want to post a context menu that allows steps to be
        #  deleted, moved up or down... if the right button is clicked on
        #  any item.
        
        menu .seqeditor.context -tearoff 0 
        .seqeditor.context add command -label Delete  \
            -command [list _deleteStep]
        .seqeditor.context add command -label {Move Up} -command [list _moveUp]
        .seqeditor.context add command -label {Move Down} -command [list _moveDown]
        
        $seqEditor bind <Button-3> [list _postContextMenu .seqeditor.context %X %Y %x %y]
    }
    
}

#------------------------------------------------------------------
# Entry point.

if {[llength $argv] != 1} {
    Usage {Invalid number of command line arguments}
}

sqlite3 db $argv

SelectorList .sequences -selectmode single -command [list _sequenceSelected db]
_populateSequenceList db .sequences

ttk::frame .actions

ttk::label .actions.newlabel -text "New sequence:"
ttk::entry .actions.new
ttk::label .actions.trglabel -text {Trigger state: }
ttk::combobox .actions.trigger -values [::sequence::listStates db]
ttk::button .actions.addnew -text {Add} \
    -command [list _newSequence db .actions.new .actions.trigger .sequences]

grid .sequences -sticky nsew 
grid .actions.newlabel .actions.new .actions.trglabel .actions.trigger -sticky w -padx 3
grid .actions.addnew -sticky w
grid .actions -sticky nswe




