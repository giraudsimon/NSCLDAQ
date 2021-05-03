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
# @file   rdo_RunControl.tcl
# @brief  Full RunControl UI with manager.
# @author Ron Fox <fox@nscl.msu.edu>
#

package require ReadoutRESTUI;           # Widgets we'll use.
package require programstatusclient;     # To query programs from manager.
package require stateclient;             # To query/control manager state.
package require kvclient;                # Title and run# are in kv store.
package require Tk


##
# @class ReadoutStateTable
#    Provides a table of Readouts and their run/manager states
#
#  This is a ttk::treeview (with vertical scrollbar)
#  run as a table with the following
#  columns:
#     *   name   - program name.
#     *   host   - where it runs.
#     *   state  - internal state.
#     *   running- X if the program is actually running.
# This is a  view.  The model comes both from the manager and,  for
# running programs, the programs themselves
#
# OPTIONS
#    none
# METHODS
#    add    - Adds a name/host
#    delete - Remove a name/host\
#    exists - Query the existence of a program.
#    list   - Return the contents of the table as  a list of dicts.
#    setActive - Set active flag of a name/host.
#    setState  - Set the runstate of a name/host.
#
snit::widgetadaptor ReadoutStateTable {
    variable ids -array [list]
    constructor {args} {
        installhull using ttk::frame
        
        set columns [list name host state running]
        ttk::treeview $win.table -yscrollcommand [list $win.vscroll set] \
            -columns $columns -show headings \
            -displaycolumns $columns         \
            -selectmode none
        
        foreach c $columns h [list Name Host State Running] {
            $win.table heading $c -text $h
        }
        
        ttk::scrollbar $win.vscroll -command [list $win.table yview] \
            -orient vertical
        
        grid $win.table $win.vscroll -sticky nsew
        grid columnconfigure $win 0 -weight 1;          # Only tree expands.
          
        #  There are no options and configureList gets unhappy if
        #  we call it an none are defined.  The code below ensure we
        #  don't forget to configure if options are later added:
        
        if {[array names options] ne [list]} {
            $self configurelist $args
        }
    }
    #--------------------------------------------------------------------------
    # Public methods:
    #
    
    ##
    # add - adds a new item to the end of the table.
    #
    # @param name - program name (contents of name column).
    # @param host - contents of host column.
    # @param state - (optional) value to put in state column.
    # @param active - (optional) value to put in the  Running columns.
    #
    method add {name host {state -} {active -}} {
        set index $name@$host
        if {[array names ids $index] eq ""} {
            set element \
                [$win.table insert {} end -values [list $name $host $state $active]]
            set ids($index) $element
        } else {
            error "This program ($name@$host) already exists."
        }
        
    }
    ##
    # delete
    #    Removes a name/host combination from the table.
    #
    # @param name - name of the program.
    # @param host - host in which it runs.
    #
    method delete {name host} {
        set index $name@$host
        if {[array names ids $index] ne ""} {
            $win.table delete $ids($index)
            array unset ids $index
        } else {
            error "$index is not in the table."
        }
    }
    ##
    # exists
    #   Determines if a name/host combination is in the table.
    #   The assumption is that the ids array is properly maintained.
    #
    # @param name - name of the program.
    # @param host - host it runs in.
    # @return boolean - true if name@host is in the table.
    #
    method exists {name host} {
        set index $name@$host
        return [expr {[array names ids $index] ne ""}]
    }
    
        
    
}
    
