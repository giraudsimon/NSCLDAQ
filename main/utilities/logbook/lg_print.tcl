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
# @file lg_print.tcl
# @brief Command line logbook PDF maker 
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names env DAQROOT] eq "DAQROOT"} {
    set tcllibs [file join $env(DAQROOT) TclLibs]
} else {
    set tcllibs [file normalize [file join [file dirname [info script]] .. TclLibs]]
}
lappend auto_path $tcllibs

package require lg_noteutilities
package require lg_utilities

##
# Usage:
#    lg_print filename ?what...?
#
# Where:
#    filename  - is the name of the file into which to create the PDF.
#                this must have the file extension .pdf to be valid due to
#                pandoc restrictions.
#    what      - specifies what to print.  If omitted, the entire logbook is
#                printed.  Otherwise, each what element can be either a run number
#                or 'none'  Each run specified is then rendered in PDF in the order
#                specified.  'none' indicates that in that position, the
#                notes that are not associated with any run will be printed.
#
proc Usage {{msg {}} } {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "    lg_print filename ?what..."
    puts stderr "Where:"
    puts stderr "   filename - is the name of the PDF file to generate.  This must"
    puts stderr "              have the extension '.pdf'"
    puts stderr "   what     - These optional parameters determine what is converted."
    puts stderr "              If omitted, the entire logbook is printed."
    puts stderr "              Each what can be either an integer, in which that"
    puts stderr "              run number is printed or 'none' in which case the"
    puts stderr "              the notes not associated with any run are printed."
    puts stderr "              The order of each 'what' item determines the order"
    puts stderr "              in which items are printed."
    puts stderr "Examples:"
    puts stderr "   lg_print all.pdf;                 # Prints everything to all.pdf"
    puts stderr "   lg_print runs1-4.pdf 1 2 3 4;     # prints runs 1-4 to runs1-4.pdf"
    puts stderr "   lg_print stuff.pdf  none 1 2 3 4; # prints un-associated notes then runs 1-4."
    exit -1
}

#------------------------------------------------------------------------------
# Entry point

#  Need at least one parameter - the filename:

if {[llength $argv] < 1} {
    Usage "lg_print must be given at least an output filename"
}
set filename [lpop argv]
if {[file extension $filename] ne ".pdf"} {
    Usage "The output filename must have a '.pdf' extension"
}

set pdf [open "|pandoc - -o$filename" w]
if {[llength $argv] == 0} {
    printall $pdf
} else {
    foreach item $argv {
        if {[string is integer -strict $item]} {
            _runToFd $item $pdf
            puts $pdf "\n***\n"
        } elseif {$item eq "none"} {
            set notes [listNonRunNotes]
            puts $pdf "Notes not associated with any run"
            puts $pdf "=================================="
            foreach note $notes {
                _noteToFd [dict get $note id] $pdf
                puts $pdf "\n***\n"
            }
        } else {
            close $pdf;    # They get what we did so far.
            Usage "Invalid 'what' item: '$item'"
            
        }
    }
}
close $pdf