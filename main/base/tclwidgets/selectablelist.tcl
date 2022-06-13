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
# @file selectablelist.tcl
# @brief Provides a scrollable list box that can have a selected item fire off a command.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide selectablelist 1.0
package require Tk
package require snit

##
# @class ScrollableList
#    Provides a scrollable list box.  For now the list box only scrolls in the
#    y direction which is the most common need.
#
#  - all options and methods are delegated to the listbox.
#
# METHOD listbox - Returns the path to the listbox to provide support for
#                  the extended bindings that are needed by SelectableList.
#
snit::widgetadaptor ScrollableList {
    component list
    delegate option * to list
    delegate method * to list
    
    constructor args {
        installhull using ttk::frame
        install list using listbox $win.list -yscrollcommand [list $win.scroll set]
        ttk::scrollbar $win.scroll -orient vertical -command [list $list yview]
        
        grid $list $win.scroll -sticky nsew
        
        $self configurelist $args
    }
    #--------------------------------------------------------------------------
    # Public methods:
    
    ##
    # listbox  - return the list box widget name.
    #
    method listbox {} {
        return $list
    }
}

##
# @class SelectableList
#     This is a ScrollableList that supports the -command option
#     -command is a script that is called when an entry is double clicked.
#  Substitutions:
#    - %W  - The widget name.
#    - %S  - The indices of the current selection.
#
snit::widgetadaptor SelectableList {
    
    option -command [list]
    delegate option * to hull
    delegate method * to hull
    
    constructor args {
        installhull using ScrollableList
        
        $self configurelist $args
        
        set l [$hull listbox]
        bind $l <Double-1> [mymethod _dispatch]
    }
    #-------------------------------------------------------------------------
    #  Event handling.
    
    ##
    # _dispatcxh
    #    Dispatch the -command option.
    #
    method _dispatch {} {
        set script $options(-command)
        if {$script ne ""} {
            set script [string map [list %W $win %S [list [$win curselection]]] $script]
            uplevel #0 $script
        }
    }
}

##
# @class SelectorList
#
#  Provides a selectable list encpasulated so the commands just get
#  The list of the selected items rather than their indices.
#
# OPTIONS
#   -command - script called on double click with list of items selected.
#
snit::widgetadaptor SelectorList {
    option -command [list]
    delegate option * to hull
    delegate method * to hull
    
    constructor args {
        installhull using SelectableList
        $hull configure -command [mymethod _dispatcher %W %S]
        $self configurelist $args
    }
    ###
    # _dispatcher
    #    @param widget - the widget
    #    @param sel    - selection index list.
    #
    method _dispatcher {widget sel} {
        set script $options(-command)
        if {$script ne ""} {
            set selections [list]
            foreach index $sel {
                lappend selections [$widget get $index]
            }
            lappend script $selections
            uplevel #0 $script
        }
    }
}

    

