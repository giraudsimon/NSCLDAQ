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
# @file mg_kvclientui.tc;
# @brief  User interface that can be used by a key value server client.
# @author Ron Fox <fox@nscl.msu.edu>
#
# Note that the test proc both exercises the UI and shows how to use it.

if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}

package provide kvclientui 1.0
package require kvwidgets

##
# @class KVEdit
#    Widget to support editing a variable.
#
# OPTIONS:
#   -name   - name of the variable (key).
#   -value  - Value of the variable (entry).
#
snit::widgetadaptor KVEdit {
    option -name "      "
    option -value
    
    constructor {args} {
        installhull using ttk::frame
        ttk::label $win.name -textvariable [myvar options(-name)]
        ttk::entry $win.value -textvariable [myvar options(-value)]
        
        grid $win.name $win.value -sticky w -padx 5
        
        $self configurelist $args
    }
}

##
# @class KvUI
#   User interface to interact with the key value store.
#
# OPTIONS:
#    -data    - Data from e.g. KvClient::listAll on an object.
#    -command - Callback script invoked for value modification
#
# METHODS:
#    load     - load the editor part of the widget with a specific variable/value
#
snit::widgetadaptor KvUI {
    component list
    component entry
    
    option -command -default [list]
    delegate option -data to list as -kvpairs
    
    constructor {args} {
        installhull using ttk::frame
        install list using KvList $win.list -selectcommand [mymethod _loadEntry]
        install entry using KVEdit $win.entry
        ttk::button $win.update -text Update -command [mymethod _dispatchCommand]
        
        grid $list -sticky nsew
        grid $entry -sticky nsw
        grid $win.update -sticky w
        
        $self configurelist $args
    }
    #--------------------------------------------------------------------------
    # Event handlers.
    
    ##
    # _loadEntry
    #    Called when an item is selected from the list.  It's loaded into the
    #    editor.
    #
    # @param var - key valud dict.
    #
    method _loadEntry {info} {
        $entry configure -name [lindex $info 0]
        $entry configure -value [lindex $info 1]
    }
    ##
    # _dispatchCommand
    #    Dispatches a command to the user's script.
    #    This is called when the Update button is clicked.
    #    the user's script gets the name and new value.
    #
    method _dispatchCommand {} {
        set cmd $options(-command)
        if {$cmd ne ""} {
            set name [$entry cget -name]
            set value [$entry cget -value]
            lappend cmd $name $value
            uplevel #0 $cmd
        }
    }
}

#------------------------------------------------------------------------------
#  Test/demo code.

##
# @param user - user running the manager server.
# @param host - the host that server is running under.
#
proc test {user host} {
    package require kvclient
    KvClient c -user $user -host $host;  # Normally use %AUTO% and assign the result
    
    proc updateUI {w client ms} {
        $w configure -data [$client list]
        
        after $ms updateUI $w $client $ms
    }
    proc setValue {client name value} {
        
        # Technically we should catch errors to deal with the possibility
        # some external force is removing this variable from the database
        # if that happens, this will throw an error.  Note that we can't
        # check/set as the deletion can happen in the time window between
        # check and set.  So [catch] is the only real alternative.
        #
        $client setValue $name $value
    }
    
    KvUI .ui -command [list setValue c]
    pack .ui
    updateUI .ui c 1000
}