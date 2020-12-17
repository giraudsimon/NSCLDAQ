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
#  A default logbook must have been set using e.g. lg_current
#  The logbook contents are presented in a treeview widget
#  Folders are in order:
#     *  runs
#            This contains the logged runs.  Underneath this is a folder for
#            each run that specifies the run number, title, the time the run started
#               and shift.
#            and the start run remark.  Below the folder in time order are the
#            following:
#            -   Run transitions (transition type time and remark name of shift)
#            -   Notes associated with the run (author, time, and first non-empty line of text).
#            -   Notes can be double clicked to view in a browser (markdown
#                processed to html.
#     *  notes
#            Contains two folders. The top one is notes not associated with runs.
#            the bottom contains a set of folders one for each run that has
#            associated notes.  Each run folder is labeled with run number,
#            title, start time, end time.
#            notes themselves can be double clicked to view as HTML in a browser.
#     *  shifts
#          Each shift is a folder with people inside.
#     *  people
#         Just a list of people
#

if {[array names env DAQROOT] eq "DAQROOT"} {
    set tcllibs [file join $env(DAQROOT) TclLibs]
} else {
    set tcllibs [file normalize [file join [file dirname [info script]] .. TclLibs]]
}
lappend auto_path $tcllibs

package require logbookadmin

