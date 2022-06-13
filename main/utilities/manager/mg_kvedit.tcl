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
# @file mg_kvedit.tcl
# @brief  Key value editor.
# @author Ron Fox <fox@nscl.msu.edu>
#
set libdir [file normalize [file join [file dirname [info script]] .. TclLibs]]
lappend auto_path $libdir

package require kvstore
package require sqlite3
package require Tk
package require snit
package require kvwidgets
#-------------------------------------------------------------------------------
# Global storage:
variable list
variable entry

#------------------------------------------------------------------------------
#  Utility procs.

##
# Usage
#  Output an error message, program usage and exit with an error.
#
# @param msg - the error messgae.
#
proc Usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "   \$DAQBIN/mg_kvedit configuration-database"
    puts stderr "Edits the key value store in an experiment configuration"
    puts stderr "Where:"
    puts stderr "   configuration-database - is the experiment database configuration."
    puts stderr "                            to use."
    exit -1
}
##
# _Update
#    Updates the list of key value pairs with what's in the key value entry.
#    This means that if the key already exists it is modified otherwise it's
#    inserted.
#
proc _Update { } {
    variable list;           # the list widget.
    variable entry;          # the entry widget.
    
    # Unpack the stuff in the entry:
    
    set key   [$entry cget -key]
    set value [$entry cget -value]
    
    if {[$list exists $key]} {
        set method modify
    } else {
        set method add
    }
    $list $method $key $value
}
##
# _Delete
#   Delete the item that's in the entry from the list -- if it exists.
#
proc _Delete {} {
    variable list
    variable entry
    
    set key [$entry cget -key]
    if {[$list exists $key]} {
        $list delete $key
    } else {
        tk_messageBox -title "No such key" -icon error -type ok \
            -message "There is no key named '$key'  to delete."
    }
}
##
# Save
#   Saves all of the items in the list.  This is a bit complex as we have
#   to determine which keys were deleted, which modified and which are new.
#
# @param db - the database command.
#
proc _Save {db } {
    variable list
    
    set kvpairs [$list cget -kvpairs];    # items in the list.
    set existingKeys [kvstore::listKeys $db]
    set listKeys [dict keys $kvpairs]
    
    #  Delete existingKeys that are not in listKeys:
    
    foreach key $existingKeys {
        if {$key ni $listKeys} {
            kvstore::remove $db $key
        }
    }
    #  Now go through kvpairs... if the key exists do a modify otherwise
    #  add:
    
    dict for {key value} $kvpairs {
        if {$key in $existingKeys} {
            set verb kvstore::modify
        } else {
            set verb kvstore::create
        }
        $verb $db $key $value
    }
}
##
# _loadEntry
#    Load the selection into the entry box.
#
# @param item - dict that reflects the selected item.
#
proc _loadEntry {item} {
    variable entry
    if {$item eq ""} {
        return
    }
    set key   [dict keys $item]
    set value [dict get $item $key]
    
    $entry configure -key $key
    $entry configure -value $value
}

#------------------------------------------------------------------------------
# Entry
#


if {[llength $argv] != 1} {
    Usage {Incorrect command line parameter count}
}
sqlite3 db [lindex $argv 0]

##
# Build the user interface.
#
# +--------------------------------+
# |  KV list                       |
# +--------------------------------+
# | Kv entry       | [Update]      |
# +--------------------------------+
# |  [Save]                        |
# +--------------------------------+

ttk::frame .list
set list [KvList .list.list -kvpairs [kvstore::listAll db] \
    -selectcommand [list _loadEntry]]
grid $list -sticky nsew
grid .list -sticky nsew

ttk::frame .input -relief groove -borderwidth 3
set entry [KvEntry .input.entry]
ttk::button .input.update -text Update -command [list _Update]
ttk::button .input.delete -text Delete -command [list _Delete]
grid $entry -columnspan 2 -sticky nsew
grid .input.update .input.delete -sticky w
grid .input -sticky nsew

ttk::frame .action
ttk::button .action.save -text Save -command [list _Save db]
grid .action.save -sticky w
grid .action -sticky nsew


          
    
