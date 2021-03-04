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
# @file  mg_seqedit.tcl
# @brief  Sequence editor
# @author Ron Fox <fox@nscl.msu.edu>
#

set libdir [file normalize [file join [file dirname [info script] ] .. TclLibs ]]
lappend auto_path $libdir

package require sequence
package require programs
package require Tk
package require snit
package require selectablelist
package require dialogwrapper
package require sqlite3

#------------------------------------------------------------------------------
# Megawidgets:

#------------------------------------------------------------------------------
#  Utility procs:

##
#  Usage
#   Error message output with usage and exit.
# @param msg - the message.
#
proc Usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "   \$DAQROOT/mg_seqedit configuration-database"
    puts stderr "  Edits the sequences in the configuration database."
    puts stderr "Where:"
    puts stderr "   configuration-database is the experiment configuration."
    exit -1
}


#_newSequence
#   Adds a new sequence (named by the contents of .new) to the list of
#   sequences.
#  @param db      - the database command.
#  @param newwid  - the entry widget that has the name of the new sequence.
#  @param trigwid - The combobox widget that has the triggering state.
#  @param seqwid  - The list box like object that has the list of sequences.
#
proc _newSequence {db newwid trigwid seqwid} {
    set newName [$newwid get]
    set trigName [$trigwid get]
    
    # Prevent duplicate:
    
    if {$newName in [$seqwid get 0 end]} {
        tk_messageBox -parent . -title {Duplicate} -icon error -type ok \
            -message "There is already a sequence named $newName  " 
        
        return
    }
    # Require valid trigger state:
    
    if {$trigName ni [::sequence::listStates $db]} {
        tk_messageBox -parent . -title {Invalid trigger state} -icon error \
            -type ok -message "$trigName is not a valid state name."
        return
    }
    # Stuff should work now:
    
    ::sequence::add $db $newName $trigName
    $seqwid insert end $newName
}
##
# _sequenceSelected
#   Reponds to a double click on a sequence.  We will bring up a sequence
#   editing dialog and, once that's filled in, update the sequence appropriately.
#
# @param - seqName -name of the sequence that must be edited.
#
proc _sequenceSelected {seqName} {
    puts "Would edit $seqName"
}

#------------------------------------------------------------------
# Entry point.

if {[llength $argv] != 1} {
    Usage {Invalid number of command line arguments}
}

sqlite3 db $argv
set sequences [::sequence::listSequences db]

SelectorList .sequences -selectmode single -command _sequenceSelected

foreach s $sequences {
    .sequences insert end [dict get $s name]
}

ttk::frame .actions

ttk::label .actions.newlabel -text "New sequence:"
ttk::entry .actions.new
ttk::label .actions.trglabel -text {Trigger state: }
ttk::combobox .actions.trigger -values [::sequence::listStates db]
ttk::button .actions.addnew -text {Add} \
    -command [list _newSequence db .actions.new .actions.trigger .sequences]

grid .sequences -sticky nsew 
grid .actions.newlabel .actions.new .actions.trglabel .actions.trigger -sticky w -padx 3
grid .actions.addnew -sticky w
grid .actions -sticky nswe



