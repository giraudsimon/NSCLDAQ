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
# @file lg_selshift
# @brief Select the current shift.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
#  This can be used in one of two ways:
#
#  $DAQBIN/lg_selshift shiftname - selects that shfit as current.
#  $DAQBIN/lg_selfshift    - brings up a persistent GUI that lets you select
#                            the current shift, and displays the current shift
#                            and its members.
#


if {[array names env DAQROOT] eq "DAQROOT"} {
    set tcllibs [file join $env(DAQROOT) TclLibs]
} else {
    set tcllibs [file normalize [file join [file dirname [info script]] .. TclLibs]]
}

lappend auto_path $tcllibs

package require logbookadmin



##
# GUI
#   Called if the GUI form has been selected; only then are Tk and Snit
#   brought in to build megawidgets to support this.
#
#
proc GUI {} {
    package require Tk
    package require snit
    package require DataSourceUI
    
    ##
    # @class CurrentShift
    #      Shows the current shift and its members.
    #      this widget auto updates every second or so.  This means that
    #      if the shift changes via some external action, this
    #      class will automatically reflect that change.
    #
    snit::widgetadaptor CurrentShift {
        variable afterid -1
        constructor args {
            installhull using ttk::frame
            
            # Labels across the top show the current shift:
            
            ttk::label $win.shiftlabel -text "Current Shift Name: "
            ttk::label $win.shiftname  -text "                      "
            
            #  A list box containing the members of the shift:
            
            ttk::label $win.memberlabel -text {Shift Members:}
            listbox $win.members -width 32 \
                -state disabled -yscrollcommand [list $win.sb set]
            ttk::scrollbar $win.sb -orient vertical -command [$win.members yview]
            
            # Lay all this out:
            
            grid $win.shiftlabel -row 0 -column 0 -sticky e
            grid $win.shiftname  -row 0 -column 1 -sticky w
            grid $win.memberlabel -row 1 -column 0 -sticky w
            grid $win.members -row 2 -column 0 -sticky nsew
            grid $win.sb      -row 2 -column 1 -sticky nsw
            
            $self update 
        }
        destructor {
            after cancel $afterid
        }
        ##
        # Get the current shift, and if there is one and its different
        # than what we display, update:
        
        method update {} {
            set shift [currentShift]
            if {$shift ne [$win.shiftname cget -text]} {
                $win.shiftname configure -text $shift
                $win.members configure -state normal
                $win.members delete 0 end
                if {$shift ne ""} {
                    set members [listShiftMembers $shift]
                
                    foreach member $members {
                        $win.members insert end \
                           "[dict get $member salutation] [dict get $member firstName] [dict get $member lastName]"
                    }
                }
                $win.members configure -state disabled
            }
            set afterid [after 1000 [mymethod update]]
        }
    }
    ##
    # @class ShiftSelector
    #    Lists the available shifts.  If the user clicks a shift, that
    #    selects the shift and displays its members in the right hand
    #    side list box.  Note that every second:
    #    -  The set of shifts is listed and, if it's changed,
    #       the selected shift is de-selected and the shifts updated.
    #    -  If the selected shift has member changes, those will be reflected
    #       as well by the update operation.
    #
    # OPTION
    #   -command - if defined, this script is called on a double left button
    # Methods:
    #    getSelected - Returns the selected shift.  This is "" if no shift is
    #              current selected from the shifts listbox.
    #    
    snit::widgetadaptor ShiftSelector {
        option -command [list]
        variable afterid -1
        
        # For reasons that are too hard for me to understand,
        # curselection gives an empty string if the
        # window doesn't have focus and yet, the selection change
        # event does not fire.  Therefore we let the selection
        # change event maintain the current selection in this variable
                    
        variable currentSelection ""
        
        constructor args {
            installhull using ttk::frame
            
            ttk::label $win.shiftlabel -text Shifts:
            ttk::label $win.memberlabel -text {Shift members:}
            
            listbox $win.shifts -selectmode single \
                -yscrollcommand [list $win.shiftscroll set] \
                -exportselection false
            ttk::scrollbar $win.shiftscroll -command [list $win.shifts yview] \
                -orient vertical
            
            listbox $win.members -state disabled \
                -yscrollcommand [list $win.memberscroll set]
            ttk::scrollbar $win.memberscroll -command [list $win.members yview]
            
            grid $win.shiftlabel -row 0 -column 0 -columnspan 2 -sticky w
            grid $win.memberlabel -row 0 -column 2 -columnspan 2 -sticky w
            
            grid $win.shifts $win.shiftscroll $win.members $win.memberscroll -sticky nsew
            
            ## Event handling
            
            bind $win.shifts <<ListboxSelect>> [mymethod ChangeSelection]
            bind $win.shifts <Double-1>   [mymethod SelectShift]
            
            $self update
            $self configurelist $args
        }
        #--------------------------------------------------------------------
        # Private methods
        
        ##
        # MembersChanged
        #   Returns boolean true if the set of people passed in is not a match
        #   to the set of people in the members list box.  This is determined
        #   by computing what the member dicts would look like in the list box
        #   and item by item comparing.
        #
        # @param people - list of people dicts.
        # @return bool  - 1 if that list would populate the members list box
        #                 differently than it is now
        #
        method MembersChanged {people} {
            set contents [$win.members get 0 end]
    
            if {[llength $contents] != [llength $people]} {
                return 1;  # Obviously since different number.
            }
            foreach person $people member $contents {
                if {[personToListBoxLine $person] != $member} {
                    return 1
                }
            }
            return 0
        }
        ##
        # fullyRepopulate
        #   Repopulate the entire form.
        #   - Get the name of the selected shift if there is one.
        #   - clear and repopulate the shift listbox.
        #   - If there's a selected shift and, if necessary, repopulate the
        #     members box too.
        #   - If there's a current shift, select it in the list box
        #
        # @param shifts - the shifts known to the database.
        #
        method fullyRepopulate {shifts} {
            
            if {$currentSelection ne ""} {
                set currentShift [$win.shifts get $currentSelection]
            } else {
                set currentShift ""
            }
            $win.shifts delete 0 end
            
            # Repopulate the shifts list box and reset the selection if there
            # is one:
            
            set selectionIndex -1;             # If no selection
            set index 0
            foreach shift $shifts {
                $win.shifts insert end $shift
                if {$shift eq $currentShift} {
                    set selectionIndex $index
                }
                incr index
            }

            
            ##
            # Clear the members box and, if there's a current shift get
            # its members and populate:
            
            
            if {$selectionIndex != -1} {
                $win.shifts selection set $selectionIndex
                set members [listShiftMembers $currentShift]
                $self repopulateMembers $members
            } else {
                $self clearMembers 
            }
            
        }
        ##
        # repopulateMembers
        #   Re-load the members list box with the indiccated people.
        #
        # @param shiftMembers - dicts specifying people to load into the box.
        #
        method repopulateMembers {shiftMembers} {
            $self clearMembers
            $win.members configure -state normal;    # Else we can't modify.
            foreach member $shiftMembers {
                set line [personToListBoxLine $member]
                $win.members insert end $line
            }
            $win.members configure -state disabled
        }
        ##
        # clearMembers
        #    Clear the members list box.
        # 
        method clearMembers {} {
            $win.members configure -state normal
            $win.members delete 0 end
            $win.members configure -state disabled
        }
        ##
        # update
        #    Self scheduled to run :
        #    If the set of shifts changed;
        #     - Clear the members listbox.
        #     - Clear and re-populate the shifts listbox (this should drop the)
        #       selection.
        #    If there's a selected shift and the members in it don't match
        #    what's in the members list box the members list box is repopulated.
        #    Regardless reschedule ourselves to run in another second.
        # All of this allows us to keep up-to-date with changes from other
        # programs (e.g. someone runs the shift editor while we're displaying.
        #
        method update {} {
            ##
            # Shifts can only be added, not name changed or deleted so:
            
            set priorCount [$win.shifts index end];    #existing shift count.
            set currentShifts [listShifts]
            if {$priorCount != [llength $currentShifts]} {
                $self fullyRepopulate $currentShifts
            } else {
                # If there's a selected shift, did the members in it change
                # (shift members can be fully edited - though that's discouraged
                # once the experiment starts).
                # Change in selection is handled by ChangeSelection below
                
                
                set selectedShift $currentSelection
                if {$selectedShift ne ""} {
                    #  A shift is selected.

                    set shift [$win.shifts get $selectedShift]
                    set members [listShiftMembers $shift]
                    if {[$self MembersChanged $members]} {
                        $self repopulateMembers $members
                    }
                }
            }
            set afterid [after 1000 [mymethod update]]
        }
        ##
        # ChangeSelection
        #   Called when the user selects a new shift from the shift box.
        #   the shift member box gets repopulated.
        #
        method ChangeSelection {} {
            set selection [$win.shifts curselection]
            set currentSelection $selection
            if {$selection ne ""} {
                set members [listShiftMembers [$win.shifts get $selection]]
                $self repopulateMembers $members
            } else {
                $self clearMembers
            }
        }
        ##
        # SelectShift
        #    Invoked on a double-click on the shift list box.
        #    the -command script, if defined, is called at the global level.
        #
        # @return string - the string to put in the listbox to represent the person.
        method SelectShift {} {
            set script $options(-command)
            if {$script ne ""} {
                uplevel #0 $script
            }
        }
        #-------------------------------------------------------------------
        #  Procs:
        
        ##
        #  personToListBoxLine
        #    Given a person dict constructs the line to stuff into the
        #    members list box.
        # @param member - a shift member  person dict.
        # @return string - what to put in the list box for that person.
        #
        proc personToListBoxLine {member} {
            return "[dict get $member salutation] [dict get $member firstName] [dict get $member lastName]"
        }
        #-------------------------------------------------------------------
        # public methods
        #
        
        ## 
        # getSelected
        #   Returns the shift that's currently selected or "" if none is.
        method getSelected {} {
            set idx [$win.shifts curselection]
            if {$idx ne ""} {
                return [$win.shifts get $idx]
            } else {
                return ""
            }
        }
    }
    ##
    # @class ShiftSelectionForm
    #
    #   This is a megawidget that consists of:
    #   -  A CurrentShift widget that shows the current shift and its members.
    #   -  A ShiftSelector widget that allows shifts to be browsed/selected.
    #   -  A button labeled "Accept" that selects the currently selected shift
    #      and applies it.
    #  @note double clicking on a shift in the ShiftSelector widget selects it.
    #  @todo - might be kind of cool to have a button to hide the
    #          shift selection form.
    #
    snit::widgetadaptor ShiftSelectionForm {
        component current
        component selector
        
        constructor args {
            installhull using ttk::frame
            
            install current using CurrentShift $win.current
            install selector using ShiftSelector $win.selector \
                -command [mymethod chooseShift]
            ttk::button $win.apply -text Apply -command [mymethod chooseShift]
            
            # Layout the widgets:
            
            grid $win.current -sticky wns
            grid $win.selector -sticky nsew
            grid $win.apply
            
        }
        ##
        # chooseShift
        #   Get the current shift from the selector.  If it's not ""
        #   set it as the current shift which, in turn, will eventually
        #   update the current shift component.
        #
        method chooseShift {} {
            set shift [$selector getSelected]
            if {$shift ne ""} {
                setCurrentShift $shift
            }
        }
    }
    ShiftSelectionForm .selectshift
    pack .selectshift -fill both -expand 1
    
}
    

if {[llength $argv] == 1} {
    setCurrentShift [lindex $argv 0]
} elseif {[llength $argv] == 0} {
    GUI
} else {
    puts stderr "Usage:"
    puts stderr "   lg_selshift ?shift-name?"
    puts stderr "Where:"
    puts stderr "   shift-name if supplied is the name of the shift to be made current"
    puts stderr "              if the shift-name is supplied the program exits after "
    puts stderr "              setting that shift current."
    puts stderr "   If no shift is given, the program starts a GUI which is intended"
    puts stderr "   to allow the user to select the current shift interactively"
    puts stderr "   changing shifts as often as desired before exiting."
    puts stderr "   The intent of this latter program is that it be used during the"
    puts stderr "   run to indicated shift changes."
}