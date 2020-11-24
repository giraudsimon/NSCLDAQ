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
# @file  logbookadmin.tcl
# @brief logbook utility package.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide logbookadmin 1.0

# Add the TclLibs directory of the installation to the auto path.
# - If env(DAQROOT) is defined that's just daqroot TclLibs.
# - if not it's relative to our installation ../TclLibs:

if {[array names env DAQROOT] eq "DAQROOT"} {
    set tcllibs [file join $env(DAQROOT) TclLibs]
} else {
    set tcllibs [file normalize [file join [file dirname info script] .. TclLibs]]
}

lappend auto_path $tcllibs
package require logbook
#----------------------------------------------------------------------------
# private procs

##
# _makePersonDict
#   Given a person command, returns the dict that describes that person.
# @param person - the person to describe.
# @return dict containing:
#     - id       - the id of the person.
#     - lastName - the last name of the person.
#     - firstName - the first name of the person.
#     - salutation - the person's salutation.
#
proc _makePersonDict {person} {
    return [dict create   \
        id [$person id] lastName [$person lastName]                     \
        firstName [$person firstName]  salutation [$person salutation]   \
    ]
}
#----------------------------------------------------------------------------
# Public procs.

##
# create
#   creates a new logbook
# @param path -  filename path.
# @param experiment - the experiment number.
# @param spokesperson - the spokesperson.
# @param purpose  - Experiment purpose.
# @param select - if true, the new logbook is selected as current.
#
proc create {path experiment spokesperson purpose {select 0}} {
    logbook::logbook create $path $experiment $spokesperson $purpose
    if {$select} {
        setCurrentLogBook $path
    }
}
##
# currentLogBook
#   The current logbook path is stored in the file ~/.nscl-logbook-current
#   If that file exists, and if the contents of that file are a valid
#   file name path, we have a current logbook:
#
# @return string - empty string if there's no current logbook, the path to the
#                  logbook if there is one
#
proc currentLogBook { } {
    if {[file readable ~/.nscl-logbook-current]} {
        set fd [open ~/.nscl-logbook-current r]
        set path [gets $fd]
        close $fd
        set path [string trim $path]
        if {[file exists $path]} {
            return [file normalize $path]
        }
    }
    return "";                       # No current logbook.
}

##
# setCurrentLogBook
#   Stores the normalized version of a log book filename path
#
# @param path - path  to the logbook
#
proc setCurrentLogBook {path} {
    set path [file normalize $path]
    set fd [open ~/.nscl-logbook-current w]
    puts $fd $path
    close $fd
    
}

##
# currentLogBookOrError
#   Returns the current logbook or throws an error if none is defined.
#
# @return string -logbook path.
#
proc currentLogBookOrError {} {
    set path [currentLogBook]
    if {$path eq ""} {
        error "The current log book has not been set."
    }
    return $path
}

##
# addPerson
#    Adds a new person to the logbook:
#
# @param lastName - persons' last name.
# @param firstName - Person's first name.
# @param saultation - optional salutation.,
# @return none
#
proc addPerson {lastName firstName {salutation {}}} {
    set bookFile [currentLogBookOrError]
    set log [logbook::logbook open $bookFile]
    
    set person [$log addPerson $lastName $firstName $salutation]
    
    $person destroy
    $log destroy
}
##
# listPeople
#    Lists the people in a table:
#
# @return a list of dicts. Each dict contains:
#     - id       - the id of the person.
#     - lastName - the last name of the person.
#     - firstName - the first name of the person.
#     - salutation - the person's salutation.
#
proc listPeople { } {
    set path [currentLogBookOrError]
    set log [logbook::logbook open $path]
    set people [$log listPeople]
    set result [list]
    foreach person $people {
        lappend result [_makePersonDict $person]
        $person destroy
    }
    $log destroy
    return $result
}

##
# createShift
#    Create a shift given its name and the list of ids of people to put in it.
#    The ids can be gotten by filtering the appropriate people out of the
#    list returned from listPeople.
#
# @param name - name of the shift.
# @param members - list of person ids.
# @return none
#
proc createShift {name members} {
    set path [currentLogBookOrError]
    set log [logbook::logbook open $path]
    
    # Turn the people ids into people commands - on error
    # Cleanup and then report the error in case there's an upper level catch.
    
    set people [list]
    foreach id $members {
        set status [catch {$log getPerson $id} result]
        if {$status} {
            foreach p $people {destroy $p}
            $log destroy
            error $result
        } else {
            lappend people $result
        }
    }
    # Create the shift:
    
    set status [catch {$log createShift $name $people} result]
    foreach p $people [$p destroy]
    $log destroy
    
    if {$status} {
        error $result
    }  else {
        $result destroy;       # kill the shift command.
    }
}
##
# setCurrentShift
#   Sets a shift to be current
# @param shiftName - name of the shift to make current.
#
proc setCurrentShift {shiftName} {
    set path [currentLogBookOrError]
    set log [logbook::logbook open $path]
    set status [catch {$log setCurrentShift $shiftName} msg]
    $log destroy
    if {$status} {
        error $msg
    }
}
##
# listShiftMembers
#   List the members in the same firm as listPeople
#   given a shift name.
#
# @param shift - name of the shif to list.
#
proc listShiftMembers {shift} {
    set path [currentLogBookOrError]
    set log [logbook::logbook open $path]
    set status [catch {$log getShift $shift} result]
    set members [list]
    if {$status == 0} {
        set people [$result listMembers]
        foreach person $people {
            lappend members [_makePersonDict $person]
            $person destroy
        }
        $shift destroy
        $log destroy
        return $members
    } else {
        #Error:
        $log destroy
        error $result
    }
}

##
# listShifts
#   Lists the names of the shifts.

##
# currentShift
#   Provides the name of the current shift or "" if there isn't one.
#

##
# beginRun
#   Begin a new run

##
# endRun
#   ends the current run

##
# pauseRun
#   Pauses the current run.

##
# resumeRun
#    Resumes the current run.

##
# emergencyEndRun
#    Does an emergency end of the run.


    
##
# listRuns
#    Lists the runs.

##
# currentRun
#   Information about the current run.

##
# findRun
#   Finds a run by run number and provides its information.