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
# @file lg_wrnote.tcl
# @brief  Write a note.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names env DAQROOT] eq "DAQROOT"} {
    set tcllibs [file join $env(DAQROOT) TclLibs]
} else {
    set tcllibs [file normalize [file join [file dirname [info script]] .. TclLibs]]
}

set bindir $env(DAQBIN)

lappend auto_path $tcllibs

package require logbookadmin
package require lg_noteutilities
package require Tk
package require snit
package require Iwidgets
package require dialogwrapper
package require textprompter



##
# @class ImageLinkPrompter
#
#  prompt for a markdown image.  These consist of a filename and an optional
#  caption.
#  
snit::widgetadaptor ImageLinkPrompter {
    
    constructor args {
        installhull using ttk::frame
        
        iwidgets::extfileselectionbox $win.file
        #ttk::label                    $win.caplabel -text "Caption"
        #ttk::entry                    $win.caption -width 38
        
        textprompt $win.caption -text "Caption" -width 38
        
        grid $win.file -columnspan 2 -sticky nsew
        grid $win.caption -sticky w
        
    }
    ##
    # getFile - get the selected file.
    # @return string - file choice.
    
    method getFile {} {
        return [$win.file  get]
    }
   
   
    # getCaption - get the seleted caption.
    # @return string - caption string
    
    method getCaption {} {
        return [$win.caption get] 
    }
}

##
# @class NoteEditor
#   The note editor has the following elements:
#   Pulldown menu for authors.
#   Listbox to choose related run if any.
#   text widget in which to edit the note.
#
# OPTIONS:
#    -runs     - list of run dictionaries to load into the run selection box.
#    -people   - List of person dicts to load into the people selection pulldown.
#    -assocrun - Associated run (could be empty).
#    -author   - Author - user error if this is empty.
#    -text     - Note text.
#
snit::widgetadaptor NoteEditor {
    option -runs -configuremethod _loadRuns
    option -people -configuremethod _loadPeople
    option -assocrun -configuremethod _loadRun -cgetmethod _getRun
    option -author   -configuremethod _loadAuthor -cgetmethod _getAuthor
    option -text -readonly 1 -cgetmethod _getText
    
    component author
    component associatedRun
    
    component runlist
    component notetext
    
    variable authorIds;             # List of author ids in combo box.
    
    constructor args {
        installhull using ttk::frame
        set header [ttk::frame $win.header];       # Top part.
        set body   [ttk::frame $win.body];         # below the header.
        
        #  The header has the people pulldown, the author and associated run.
        
        ttk::label $header.authorlabel -text {Author:}
        install author using ttk::combobox $header.author
        ttk::label $header.runlabel -text {Associated run:}
        install associatedRun using ttk::label $header.run
        
        grid $header.authorlabel $author $header.runlabel $associatedRun -sticky nsew
        
        # The body has the run selection scrolling list box and the text edit.
        
        install runlist using \
            listbox $body.runlist -yscrollcommand [list $body.runscroll set] \
            -selectmode single
        ttk::scrollbar $body.runscroll -orient vertical \
            -command [list $runlist yview]
        
        install notetext using  \
            text $body.notetext -yscrollcommand  [list $body.textscroll set]
        ttk::scrollbar $body.textscroll -orient vertical -command [$notetext yview]
        
        grid $runlist $body.runscroll $notetext $body.textscroll -sticky nsew
        
        # Stack the header over the body
        
        grid $header  -sticky nsew
        grid $body    -sticky nsew
        
        # Now some event bindings:: Double clicking a run should load it
        # into the associated run while chosing a combobox element loads the
        # author:
        
        bind $runlist <Double-1> [mymethod _selectRun]
        
        #  Now the context menu which is posted when the user right clicks
        #  in the text widget:
        
        menu $win.context -tearoff 0
        $win.context add command -label {Image...} -command [mymethod _insertImage]
        bind $notetext <3> [list $win.context post %X %Y]
        bind $notetext <KeyPress> [list $win.context unpost]
        
        # Do any configuration required:
        
        $self configurelist $args
    }
    ###
    # Configuration handling:
    
    ##
    # _loadRuns
    #   Take a runs dictionary and load it into the listbox.
    #   -  The current run is unset if that happens.
    #   -  The elements of the list box will only contain
    #      - the run number the title and (active) if the run is
    #        active/current.
    # @param optname - name of option being configured (-runs)
    # @param optval  - List of runs.
    # @note  We'll insert an empty line as well to represent a note that's
    #        not associated with any run.
    #
    method _loadRuns {optname optval} {
    
         #  Clear the listbox and current run:
         
         $associatedRun configure -text ""
         $runlist delete 0 end
         $runlist insert end "";               # no run associated.
         foreach run $optval {
            set listval [dict get $run number]
            if {[dict get $run isActive]} {
                append listval " (active)"
            }
            append listval " " [string range [dict get $run title] 0 10]...
            $runlist insert end $listval
         }
         set options($optname) $optval
         
    }
    ##
    # _loadPeople
    #    Load the people into the combobox that could be an author for the note
    # @param optname - name of the option being configured (-people)
    # @param optval  - list of people dicts.
    # @note an empty string is loaded into the combobox text indicating no selection
    # @note what's loaded into the combobox is the salutation, first, last name
    #       separated by spaces...well actually we use a Tcl list in case there
    #       are spaces in any of the fields.
    method _loadPeople {optname optval} {
        set combolist [list]
        set authorsIds [list]
        foreach person [listPeople] {
            set text [list \
                [dict get $person salutation] [dict get $person firstName] \
                [dict get $person lastName]   \
            ]
            lappend combolist $text
            lappend authorIds [dict get $person id]
        }
        $author configure -values $combolist
        $author set ""
        
        set options($optname) $optval
    }
    ##
    # _loadRun
    #   Loads a specific run in to the run dictionary. The string loaded
    #   will be the run number if the run is active (active) will be appeneded
    #   to that string.
    #
    # @param optname - name of the option being configured.
    # @param optval  - dict descrbing a run to load.
    #
    method _loadRun {optname optval} {
        set text [dict get $optval number]
        if {[dict get $optval isActive]} {
            append text " (active)"
        }
        $associatedRun configure -text $text
    }
    ##
    # _getRun
    #   Return the number of the run that's associated with this note.
    #   If there's no associated run, "" is returned.
    #
    # @optname -name of the option.
    # @return run number of run associated with this note.
    method _getRun {optname} {
        set result [lindex [$associatedRun cget -text] 0]
        return $result
        
    }
    ##
    # _loadAuthor
    #   Load the combo box with an author given the dict of a person.
    #   This results in an error and emptying the author field
    #   if the dict is not a valid author name (not in the combobox).
    # @param optname - name of the option (-author).
    # @param optval  - dict of person.
    #
    method _loadAuthor {optname optval} {
        set text [list \
            [dict get $optval salutation ] [ dict get $optval firstName] \
            [dict get $optval lastName] \
        ]
        if {$text ni [$author cget -values]} {
            $author set ""
            error "The author $text is not one of the people in the database"
        }
    }
    ##
    # _getAuthor
    #   Return the dict associated with the author currently selected.
    #   This is empty if:
    #    -    The combobox text area is empty.
    #    -    Some how the combobox text area is not in the valid choices.
    # @param optname option name - author
    #
    method _getAuthor {optname} {
        set index [$author current]
        if {$index == -1} {return ""}
        
        set id [lindex $authorIds $index]
        set salAndName [lindex [$author cget -values] $index]
        
        return [dict create \
            id $id lastName [lindex $salAndName 2]  \
            firstName [lindex $salAndName 1]    \
            salutation [lindex $salAndName 0]   \
        ]
    }
    ##
    # _getText
    #    Return the text of the note.
    # @param optname
    # @return note text.
    #
    method _getText {optname} {
        return [$notetext get 0.0 end]
    }
      
    #-------------------------------------------------------------------
    # Event handlers.
    
    ##
    # _selectRun
    #     Respond to a double click on the run list box and
    #     set that as the associated run for this note.  One possible
    #     content is an empty line which means no run is associated.
    #     
    method _selectRun {} {
        set index [$runlist curselection]
        if {[llength $index] == 0} {
            # No run:
            $associatedRun configure -text ""
        } else {
            set index [lindex $index 0]
            set text [$runlist get $index];   # Get the text.
            $associatedRun configure -text $text
        }
    }
    ##
    # _insertImage
    #   Called in response to selecting the context menu Image...
    #   item.  We pop up a dialog with an ImageSelector box.
    #   We have to get at least a filename or we don't do anything.
    #   The caption is optional.  IF all is good, an image link is inserted
    #   into the document at the insertion point (cursor).
    #
    #
    method _insertImage {} {
        $win.context unpost ;     # Should keep us from double posting.
        toplevel $win.prompt
        set d [DialogWrapper $win.prompt.dialog]
        set formContainer [$d controlarea]
        set form [ImageLinkPrompter $formContainer.form]
        $d configure -form $form
        
        pack $d -fill both -expand 1
        set result [$d modal]
        if {$result eq "Ok"} {
            set caption [$form getCaption]
            set image   [$form getFile]
            if {$image ne ""} {
                $notetext insert insert "!\[$caption\]($image)"
            } 
        }
        
        
        destroy $win.prompt
    }
    
}
#------------------------------------------------------------------------------
#  utility procs:

##
# lSelect
#   Given a list of equi-sized objects, returns a specific member of each element.
# @param list - the list to select from
# @param el   - the offset in the list to return elements from.
# @return list of values
#
proc lSelect {list el} {
    set result [list]
    foreach item $list {
        lappend result [lindex $item $el]
    }
    return $result
}

##
# imageOffsets
#   Extract the image file offsets from offset/name pairs.
#
# @param images - list of offset/image-files from the note.
# @return list of integers - the image offsets.
#
proc imageOffsets {images} {
    return [lSelect $images 0]
}

##
# imageFilenames
#   Given a list of image offset/image-file pairs, returns a list of image files.
#
# @param images list of offsets, filenames. in that order.
# @return list of strings - list consisting of the second element of each sublist.
#
proc imageFilenames {images} {
    return [lSelect $images 1]
}
    
#---------------------------------------------------------------------------
#
# Entry point

#
#  Get a list of the runs.
#  Get a list of the people

# Build the notebook and a 'save' and 'cancel' button below it.

set runs [listRuns]
set people [listPeople]

set action 0

NoteEditor .n -people $people -runs $runs
frame      .n.action
button     .n.action.save -text Save -command [list set action ok]
button      .n.action.cancel -text Cancel -command [list set action cancel]

bind . <Destroy> [list set action cancel]

grid .n -sticky nsew
grid .n.action.save .n.action.cancel
grid .n.action -sticky nswe

# One more thing:
#   - If a  run is provided on the command line, it' set as current.
#   - If not and a run is current use that as the associated run.
#   otherwise, let the user choose if they want to:

set runinfo ""
if {[llength $argv] > 0} {
    set run [lindex $argv 0]
    set runinfo [findRun $run]
    
} else {
    set runinfo [currentRun]
}
if {$runinfo ne ""} {
    .n configure -assocrun $runinfo
}

set done 0
while {!$done} {
    vwait action
    if {$action eq "ok"} {
        set action "";              # Not sure I need to do this.
        
        set author [.n cget -author]
        set run    [.n cget -assocrun]
        set text   [.n cget -text]
        
        if {$author eq ""} {
            tk_messageBox -parent . -title "Need author" -type ok -icon error \
            -message {Notes must have an author. Please select an author}
        } else {
            set imageInfo [makeNoteImageList $text]
            set badImages [validateImageFiles $imageInfo]         
            if {[llength $badImages] > 0 } {
                set badImages [join $badImages ", "]
                tk_messageBox -parent . -title "Bad Images" -type ok -icon error \
                    -message "Some image references are broken : $badImages"
            } else {
                #  Everything is ok now  We need to use the low level logbook
                # bindings because we don't have administrative APIs for notes:
                #
                set logbook [logbook::logbook open [currentLogBook] ]
                set authorcmd [$logbook getPerson [dict get $author id]]
                set imageFiles [imageFilenames $imageInfo]
                set imageOffsets [imageOffsets $imageInfo]
                
                if {$run eq ""} {
                    # No associated run.
                    
                    $logbook createNote $authorcmd $text $imageFiles $imageOffsets
                    
                } else {
                    # Associated run.
                    
                    set runcmd [$logbook findRun $run]
                    
                    $logbook createNote \
                        $authorcmd $text $imageFiles $imageOffsets $runcmd
                    
                    $runcmd destroy
                }
                $authorcmd destroy
                $logbook destroy
                
                set done 1
            }
            
        }
    } else {
        set done 1;                  # Cancelling
    }
}



catch {destroy .}
