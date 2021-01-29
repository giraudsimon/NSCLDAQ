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
# @class container::BindingsList
#    List of bindings that can scroll both ways.
#    This can be used inboth container::Creator and container::Editor
#    below
#  The visual presentation is a listox with v and h scrollbars.
# OPTIONS:
#   -bindings - Bindings with which to populate the listbox.
#               This will have the form of the bindings dict key in
#               a container definition from container::listDefinitions
#   -selectscript - Script called if the selection changes.  The value of the
#               current selection is returned.
#
#
snit::widgetadaptor container::BindingsList {
    component bindings
    
    option -bindings -default [list] -configuremethod _update
    option -selectscript [list]
    
    delegate option * to bindings
    
    
    constructor args {
        installhull using ttk::frame
        install bindings using listbox $win.list \
            -yscrollcommand [list $win.vscroll set]   \
            -xscrollcommand [list $win.hscroll set] -selectmode single
        ttk::scrollbar $win.vscroll -orient vertical \
            -command [list $bindings yview]
        ttk::scrollbar $win.hscroll -orient horizontal \
            -command [list $bindings xview]
        
        bind $bindings <<ListboxSelect>> [mymethod _onSelect]
        
        grid $bindings $win.vscroll -sticky news
        grid $win.hscroll           -sticky ewn
        
        $self configurelist $args
        
        
    }
    #---------------------------------------------------------------------------
    #  Configuration methods.
    
    ##
    # _update
    #    Update the list box from a new set of binding definitions.
    # @param name  - option name (-bindings)
    # @param value - option values
    #
    method _update {name value} {
        set options($name) $value
        
        $bindings delete 0 end
        
        foreach binding $value {
            set text $binding
            if {[llength $binding] == 2} {
                set text "[lindex $binding 0] -> [lindex $binding 1]"
            }
            $bindings insert end $text
            
        }
        
    }
    #--------------------------------------------------------------------------
    #  Event handling
    #
    
    ##
    # _onSelect
    #   A selection was performed. If there is a -selectscript then
    #   invoke it with the selected binding passed as a parameter.
    #
    method _onSelect {} {
        set index [$bindings curselection]
        set script $options(-selectscript)
        if {($index ne "")  && ($script ne "")} {
            puts "$options(-bindings)"
            puts [lindex $options(-bindings) $index]
            set binding [lindex $options(-bindings) $index]
            puts "uplevel #0 $script [list $binding]"
            uplevel #0 $script [list $binding]
        }
    }
}

##
# @class container::Creator
#    Create a container from an existing definition or from whole cloth.
#
#    +--------------------------------------------------+
#    |  Name: [      ]   Image [         ] [Browse...]  |
#    |                                                  |
#    | Bindings                                         |
#    |   From: [    ] To: [         ][Browse...]   [Add]|
#    |   +---------------------------------+            |
#    |   |   Bindings list (scrollable)    |  [Remove]  |
#    |   +---------------------------------+            |
#    | InitScript [ filename ]   [Browse...]            |
#    +--------------------------------------------------+
#    |  [ Ok ]    [ Cancel ]                            |
#    +--------------------------------------------------+
#
#  @note - If creating from an existing container definition,
#          if there is an init script it is written to a file in /tmp and
#          that file is initially populated in the initscript entry.
#          This is necessary because the contents of the file are pulled in
#          not a reference to the file itself, in case it lives in a filesystem
#          that will not be visible to the container.
#  @note - users must take care, if they run this inside a container
#          that the From path of a binding is a host binding not
#          a mapped binding in the container (e.g.
#         /usr/opt/opt-buster not /usr/opt/).
#
#  OPTIONS
#    -name   - Container name.
#    -image  - image path.
#    -bindings - List of bindings (same form as container::listDefinitions gives).
#    -initscript - contents - current contents of an initscript.
#    -initfile   - Initialization file.
#    -okscript   - Script to execute on ok.
#    -cancelscript - Script to execute on cancel.
#
#          
snit::widgetadaptor container::Creator {
    option -name -default  [list]
    option -image -default [list]
    option -initscript -default [list] -configuremethod _configInitScript
    option -initfile -default [list] -readonly 1 -configuremethod _configDisallow
    option -okscript -default [list]
    option -cancelscript -default [list]
    
    component bindings
    delegate option -bindings to bindings
    
    constructor args {
        installhull using ttk::frame
        
        ttk::label $win.namelabel -text Name:
        ttk::entry $win.name -textvariable [myvar options(-name)]
        ttk::label $win.imagelabel -text Image:
        ttk::entry $win.image -textvariable [myvar options(-image)]
        ttk::button $win.imagebrowse -text Browse... -command [mymethod _browseImage]
        
        grid $win.namelabel $win.name $win.imagelabel $win.image $win.imagebrowse \
            -sticky news -padx 3
        
        ttk::label $win.bindingslabel -text Bindings
        grid $win.bindingslabel -sticky nsw
        
        ttk::label $win.fromlabel -text From:
        ttk::entry $win.from
        ttk::label $win.tolabel  -text To:
        ttk::entry $win.to
        ttk::button $win.tobrowser -text Browse... -command [mymethod _browseTo]
        ttk::button $win.addbinding -text Add -command [mymethod _addBinding]
        
        grid $win.fromlabel $win.from \
            $win.tolabel $win.to $win.tobrowser \
            $win.addbinding -sticky nsew -padx 3
        
        install bindings using container::BindingsList $win.bindings \
            -selectscript _onBindingsSelect -width 50
        ttk::button $win.removebinding -text Remove -state disabled
        grid $bindings  -row 3 -column 0 -columnspan 5 -sticky nsew
        grid $win.removebinding -row 3 -column 5 -sticky e
        
        ttk::label $win.initscriptlabel -text {Init Script:}
        ttk::entry $win.initscript
        ttk::button $win.browseinit -text Browse... -command [mymethod _browseInit]
        
        grid $win.initscriptlabel $win.initscript $win.browseinit -padx 3
        
        set actions [ttk::frame $win.actionframe -relief groove -borderwidth 4]
        ttk::button $actions.ok -text Ok -command [mymethod _dispatch -okscript]
        ttk::button $actions.cancel -text Cancel -command [mymethod _dispatch -cancelscript]
        
        grid $actions.ok $actions.cancel -padx 3
        grid $actions -sticky nsew -columnspan 6
        
    }
    
    
    
}

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
        
        install currentbindings using container::BindingsList $win.current.bindings 
        
        install initscript using ttk::button $win.current.script \
            -text "Init Script.." -command [mymethod _showCurrentScript] \
            -state disabled
        
        
        grid $currentimage -sticky nsew
        grid $currentbindings  -sticky nsew
        grid $initscript
        
        grid $win.current -row 0 -column 2 -sticky nsew
    
        #  Now the action buttons:
        
        set actions [ttk::frame $win.actionarea -relief groove -borderwidth 4]
        ttk::button $actions.new -text New... -command [mymethod _onNew]
        ttk::button $actions.edit -text Edit... -command [mymethod _onEdit]
        grid $actions.new $actions.edit
        grid $actions -sticky ew -columnspan 3
        
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
    #   we populate the rest of the UI based on which  was
    #   selected.
    #
    method _onSelect {} {
        set selected [$self _getCurrent]
        $self _emptySelected
        if {$selected ne ""} {
            $currentimage configure -text "Current Image: [dict get $selected image]"
            
            # Update the bindings list component:
            
            set bindingsList [list]
            if {[dict exists $selected bindings]} {
                set bindingsList [dict get $selected bindings]
            }
            $currentbindings configure -bindings $bindingsList
            
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
        $initscript configure -state disabled
    }
}


proc container::editorTest {} {
    catch [file delete containereditortest.db]
    exec [file join $::env(DAQBIN) mg_mkconfig] containereditortest.db
    sqlite3 db containereditortest.db
    container::add db a thing1 ~/.bashrc a
    container::add db b thing2 "" \
        [list [list a] [list b /usr/opt/daq/12.0-pre3] [list d]]
    container::add db test minimal.img "~/.profile" ""
    
    container::Editor .e -containers [container::listDefinitions db]
    file delete containereditortest.db      
    
    pack .e -fill both -expand 1
    
}


