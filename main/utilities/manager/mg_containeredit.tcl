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
# @file  mg_containeredit.tcl
# @brief provide the ability to edit container definitions.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide containeredit 1.0
package require Tk
package require snit
package require sqlite3
package require containers

namespace eval ::container {}
##
# @class container::Editor
#    Provides an editor for the container part of the database.
#    The top part contains a list of the containers - on the left side,
#    clicking a container will populate it's definition on the right side.
#    Double clicking a container will allow you to to edit it.
#    Clicking New... will allow you to edit a new container.
#    Note that when editing an existing container; if you change its name,
#    You can use it as a starting point for a new container.
#
# OPTIONS
#    -containers   - list of container definitions from, e.g. container::listDefinitions
#    -newcommand   - Command to execute if a new container is being created
#    -replacecommand - command to execute if a container is being modified.
#
# Normally -newcommand will invoke container::add and -replacecommand will
# first container::remove then container::add
# Both of those commands have appended to them the full container description.
#
snit::widgetadaptor container::Editor {
    option -containers -default [list] -configuremethod _update
    option -newcommand -default [list]
    option -replacecommand -default [list]
    
    component containerNames;     # list box with container names.
    component currentimage;       # Label with current image name.
    component currentbindings ;   # listbox with current bindings list.
    component initscript;         # Button to popup current init script if one.
    
    constructor args {
        installhull using ttk::frame
        
        # Container list.
        
        install containerNames using listbox $win.names -selectmode single \
            -yscrollcommand [list $win.cscroll set]
        bind $containerNames <<ListboxSelect>> [mymethod _onSelect] 
        ttk::scrollbar $win.cscroll -orient vertical \
            -command [list $containerNames yview]
        
        grid $containerNames $win.cscroll -sticky nsew
        
        # Frame and contents for the selected image.
        #  - component currentimage    - currently selected image.
        #  - Component currentbindings - List of current bindings.
        #  - Component initscript      - Shows the current initialization script
        
        ttk::frame $win.current
        install currentimage using ttk::label $win.current.image \
            -text "Current image:       "
        
        install currentbindings using listbox $win.current.bindings \
            -yscrollcommand [list $win.current.bscroll set] \
            -xscrollcommand [list $win.current.bxscroll set]
        ttk::scrollbar $win.current.bscroll -orient vertical \
            -command [list $currentbindings yview]
        ttk::scrollbar $win.current.bxscroll -orient horizontal \
            -command [list $currentbindings xview]
        
        install initscript using ttk::button $win.current.script \
            -text "Init Script.." -command [mymethod _showCurrentScript] \
            -state disabled
        
        
        grid $currentimage -sticky nsew
        grid $currentbindings $win.current.bscroll -sticky nsew
        grid $win.current.bxscroll -sticky new
        grid $initscript
        
        grid $win.current -row 0 -column 2 -sticky nsew
    
        
        # Process options
        
        $self configurelist $args
    }
    
    #--------------------------------------------------------------------------
    # Configuration:
    #
    
    method _update {optname optval} {
        set options($optname) $optval;   # now cget works.
        
        $containerNames delete 0 end
        foreach container $optval {
            $containerNames insert end [dict get $container name]
        }
        #  Select the first item in the list:
        
        $containerNames selection set 0;   # Forces _onSelect call.
        event generate $containerNames <<ListboxSelect>>
    }
    #---------------------------------------------------------------------------
    # Event handling
    #
    
    ##
    # _onSelect
    #   Called when a container name is selected from th elist box.
    #   we populate the rest of the UI based on which container was
    #   selected.
    #
    method _onSelect {} {
        set selected [$self _getCurrent]
        $self _emptySelected
        if {$selected ne ""} {
            $currentimage configure -text "Current Image: [dict get $selected image]"
            
            # List the mount points if there are any:
            
            if {[dict exists $selected bindings]} {
                foreach mountpoint [dict get $selected bindings] {
                    set text $mountpoint
                    if {[llength $mountpoint] == 2} {
                        set text "[lindex $mountpoint 0] -> [lindex $mountpoint 1]"
                    }
                    $currentbindings insert end $text
                }
            }
            
            #  set the stae of the initscript button accordingly:
            
            if {[dict exists $selected init]} {
                $initscript configure -state normal
            } else {
                $initscript configure -state disabled
            }
        }
    }
    ##
    # _showCurrentScript
    #    Called when the button that shows the current image's initscript
    #    is clicked.  We create a new toplevel named
    #    win.initscript - Destroying it if it exists already.
    #    The top level looks like this:
    #
    #    +---------------------------------------------+
    #    |   scrolling text widget with script         |
    #    +---------------------------------------------+
    #    |"Init script for container: name" [dismiss]  |
    #    +---------------------------------------------+
    #
    #  The top level title will also contain the same text as the label
    #  at the bottom.
    #  Dismiss's command will be destroy $win.initscript.
    #
    #
    method _showCurrentScript {} {
        set info [$self _getCurrent]
        if {$info ne ""} {
            if {![winfo exists $win.initscript]} {
                toplevel $win.initscript
                text $win.initscript.script -yscrollcommand [list $win.initscript.sb set]
                ttk::scrollbar $win.initscript.sb -command [list $win.initscript.script yview] \
                    -orient vertical
                ttk::label $win.initscript.name
                ttk::button $win.initscript.dismiss -text Dismiss -command [list destroy $win.initscript]
                
                grid $win.initscript.script -row 0 -column 0 -sticky nsew
                grid $win.initscript.sb     -row 0 -column 1 -sticky nsw
                grid $win.initscript.name   -row 1 -column 0 -sticky w
                grid $win.initscript.dismiss -row 1 -column 1 -sticky e
            }
            # We know the top level exists fill it in:
            
            set name [dict get $info name]
            set text [dict get $info init]
            set title "Init script for container: $name"
            wm title $win.initscript  $title
            $win.initscript.script delete 1.0 end
            $win.initscript.script insert end $text
            $win.initscript.name configure -text $title
        }
        
        
        
        
    }
    #------------------------------------------------------------------
    #  Utilities
    #
    ##
    # _getCurrent
    # @return dict - return the currently selected container definition dict.
    #                Empty string if there is no current selection.
    #
    method _getCurrent {} {
        set selection [$containerNames curselection]
        set result [dict create]
        if {$selection ne ""} {
            set result [lindex $options(-containers) $selection]
        }
        
        return $result
    }
    ##
    # _emptySelected
    #    Clear out the selection.
    #
    method _emptySelected {} {
        $currentimage configure -text "Current Image: "
        $currentbindings delete  0 end
        $initscript configure -state disabled
    }
}


proc container::editorTest {} {
    catch [file delete containereditortest.db]
    exec [file join $::env(DAQBIN) mg_mkconfig] containereditortest.db
    sqlite3 db containereditortest.db
    container::add db a thing1 ~/.bashrc a
    container::add db b thing2 "" [list [list a] [list b /usr/opt/daq/12.0-pre3] [list d]]
    container::add db test minimal.img "~/.profile" ""
    
    container::Editor .e -containers [container::listDefinitions db]
    file delete containereditortest.db      
    
    pack .e -fill both -expand 1
    
}


