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

# Helper programs:

package require Tk
package require logbookadmin
package require snit


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
