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
# @file mg_kvwidgets.tcl
# @brief Widgets to display/enter key value pairs.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide kvwidgets 1.0
package require Tk
package require snit


##
# @class KvList
#    Provides a list object that displays key value pairs.  The GUi is a
#    ttk::treeview with key and value columns.  A vertical scrollbar facilitates
#    vertical scrolling.  We also provide means to process selections of rows
#    editing items, deleting items and adding items.
#
# OPTIONS
#    -kvpairs   - Dict of key value pairs: Note the UI is loaded sorted by key.
#    -selectcommand - Script invoked by a selection.
# METHODS
#   add    - adds a new key/value pair.
#   delete - Removes a key
#   modify - Modifies an existing key/value pair.
#
#
snit::widgetadaptor KvList {
    option -kvpairs -default [dict create]  \
        -configuremethod _cfgContents -cgetmethod _cgetContents
    option -selectcommand -default [list]
    
    component tree
    delegate option * to tree
    delegate method * to tree
    
    constructor args {
        installhull using ttk::frame
        
        install tree using ttk::treeview $win.tree -columns [list key value] \
            -displaycolumns [list key value] -show [list headings] \
            -yscrollcommand [list $win.sb set] -selectmode browse
        foreach col [list key value] title [list Key Value] {
            $tree heading $col -text $title
        }
        bind $tree <<TreeviewSelect>> [mymethod _dispatchSelection]
        
        ttk::scrollbar $win.sb -orient vertical -command [list $tree yview]
        
        grid $tree $win.sb -sticky nsew
        
        $self configurelist $args
        
    }
    #-------------------------------------------------------------------------
    # Public methods
    
    ##
    # add
    #    Add a new entry.  Note that the key must not yet be in the tree or
    #    an error will be thrown.
    #
    # @param key   - new key.
    # @param value - New value
    #
    method add {key value} {
        set existing [$self _findKey $key]
        if {$existing eq ""} {
            $tree insert {} end -values [list $key $value]
        } else {
            error "There's already a keyword $key in editor."
        }
    }
    
    ##
    # delete
    #   Remove an existing entry
    #
    # @param key - the key to remove.
    #
    method delete {key} {
        set existing [$self _findKey $key]
        if {$existing ne ""} {
            $tree delete $existing
        } else {
            error "There is no key '$key' to delete from the list."
        }
    }
    
    ##
    # modify
    #   Change the value of an existing key.
    #
    # @param key  - existing key.
    # @param newValue - the new value it should have.
    #
    method modify {key newValue} {
        set existing [$self _findKey $key]
        if {$existing ne ""} {
            $tree item $existing -values [list $key $newValue]
        } else {
            error "There is no key '$key' whose value can be modified to '$newValue'"
        }
    }
    ##
    # exists
    #  Determine if a key exists in the list:
    #
    # @param key - the key to check for
    # @return bool - true if it does.
    #
    method exists {key} {
        if {[$self _findKey $key] ne ""} {
            return 1
        } else {
            return 0
        }
    }
    
    
    #--------------------------------------------------------------------------
    # Configuation management
    
    ##
    # _cfgContents
    #     Configures the contents of the tree view.
    #     Note that the keys are sorted alphabetically.  Thus a simple resort is
    #     .list configure -kvpairs [.list cget -kvpairs]
    #
    # @param optname  - name of the option.
    # @param value    - This is a dict whose keys are the keys and values the values.
    #
    method _cfgContents {optname value} {
        $tree delete [$tree children {}]
        
        set keys [dict keys $value]
        set keys [lsort -increasing -dictionary $keys]
        
        foreach key $keys {
            set v [dict get $value $key]
            $tree insert  {} end -values [list $key $v]
        }
    }
    ##
    # _cgetContents
    #    Returns the key value pairs the widget has in its list.
    # @param optname - option name.
    # @return dict   - with key/value pairs that match the dict's.
    method _cgetContents {optname} {
        set items [$tree children {}]
        set result [dict create]
        foreach item $items {
            set kvpair [$tree item $item -values]
            dict set result [lindex $kvpair 0] [lindex $kvpair 1]
        }
        return $result
    }
    #--------------------------------------------------------------------------
    # Event handling
    
    ##
    # _dispatchSelection
    #
    #     Called on a <<TreeviewSelect>> virtual event.
    #     Get the contents of the selected element, if there is one and
    #     call the user's handler if there is one.
    #
    method _dispatchSelection {} {
        set script $options(-selectcommand)
        if {$script ne ""} {
            set selection [$tree selection]
            if {$selection ne ""} {
                set v [$tree item $selection -values]
                set param [dict create [lindex $v 0] [lindex $v 1]]
            } else {
                set param [dict create]
            }
            lappend script $param
            uplevel #0 $script
        }
    }
    #-------------------------------------------------------------------------
    # Private utilities.
    
    ##
    # _findKey
    #    Find the entry with the specified key:
    #
    # @param key
    # @return entry id.
    # @retval "" If there's no match.
    #
    method _findKey {key} {
        set children [$tree children {} ]
        foreach item $children {
            set values [$tree item $item -values]
            if {$key eq [lindex $values 0]} {
                return $item
            }
        }
        # No match
        
        return ""
    }
}

##
# @class KvEntry
#    Provides a widget that can be used to enter a keyword/value pair:
#
# OPTIONS:
#   -key  - the keyword.
#   -value - THe value.
#
snit::widgetadaptor KvEntry {
    option -key
    option -value
    
    constructor args {
        installhull using ttk::frame
        
        ttk::label $win.lkey -text "Key:"
        ttk::entry $win.key -textvariable [myvar options(-key)]
        ttk::label $win.lvalue -text "Value:"
        ttk::entry $win.value -textvariable [myvar options(-value)]
        
        grid $win.lkey $win.key $win.lvalue $win.value -sticky nsew
        
        $self configurelist $args
    }
}