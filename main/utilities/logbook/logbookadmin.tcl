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
    set tcllibs [file normalize [file join [file dirname [info script]] .. TclLibs]]
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

##
#  _makeTransitionDict
#     Given a raw transition dictionary from the transitions subcommand
#     of a run instance,  maps that to the transitiondict that
#     _makeRunDict produces.
#
# @param d - the inbound dict.
# @return dict - the resulting dict.
#
proc _makeTransitionDict {d} {
    set result [dict create                                 \
        transitionName [dict get $d transitionName ]        \
        transitionTime [dict get $d transitionTime]         \
        transitionComment [dict get $d transitionComment]   \
    ]
    set shift [dict get $d shift]
    set shiftName [$shift name]
    $shift destroy
    dict set result shift $shiftName
    return $result
}
##
# _makeRunDict
#   Given a run command, produces the dict that describes it. See
#   listRuns for documentation of the contents of that dict.
#
# @param run - the run command ensemble
# @return dict - as described above and in the listRuns comment header.
#
proc _makeRunDict {run} {
    set result [dict create                                   \
        number [$run number] title [$run title]               \
        isActive [$run isActive]
    ]
    set transitions [$run transitions]
    foreach transition $transitions {
        set trdict [_makeTransitionDict $transition]
        set shift [dict get $transition shift]
        $shift destroy
        dict lappend result transitions $trdict
    }
    return $result
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
proc createLogBook {path experiment spokesperson purpose {select 0}} {
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
    foreach p $people {$p destroy}
    $log destroy
    
    if {$status} {
        error $result
    }  else {
        $result destroy;       # kill the shift command.
    }
}
##
# addMembersToShift
#     Add members to an existing shift.
#
#  @param shiftName - name of the shfit.
#  @param members   - Ids of the shift members.
#
proc addMembersToShift {shiftName members} {
    set path [currentLogBookOrError]
    set log  [logbook::logbook open $path]
    
    set people [list]
    set shift ""
    set status [catch {
        set shift [$log findShift $shiftName]
        if {$shift eq ""} {
            error "No such shiftname $shift"
        }
        foreach member $members {
            lappend people [$log getPerson $member]
        }
        # The new members may have an intersection with the
        # current shift members, remove those from the people list
        # destroything them as we go.  Do this by making a set (list)
        # containing the ids of all people on shift.
        
        set currentMembers [listShiftMembers $shiftName]
        set currentMids [list]
        foreach member $currentMembers {
            lappend currentMids [dict get $member id]

        }
        #  Destroy those that are in shift already:
        
        set destroyedIndices [list]
        set i 0
        foreach person $people {
            if {[$person id] in $currentMids} {
                $person destroy
                lappend destroyedIndices $i
            }
            incr i
        }
        # Remove them from people in reverse order so the ids are good:
        
        set destroyedIndices [lreverse $destroyedIndices]
        foreach i $destroyedIndices {
            set people [lreplace $people $i $i]
        }
        #  So now we add the people to the shift:
        
        foreach person $people {
            $log addShiftMember $shift $person
            $person destroy
        }
        $shift destroy
        $log destroy
        
    } msg]
    if {$status} {
        # Clean up any stuff we made before reporting the error:
        
        foreach person $people {
            $person destroy
        }
        if {$shift ne ""} {$shift destroy}
        error "Failed to add members to shift $shift : $msg : $::errorInfo"
    }
    
}
##
# removeMemberFromShift
#   Removes a member from a shift given their id.
#
# @param shiftName - name of the shift.
# @param member    - Id of the member to remove.
#
proc removeMemberFromShift {shiftName member} {
    set path [currentLogBookOrError]
    set log  [logbook::logbook open $path]
    
    set person ""
    set shift ""
    set status [catch {
        set shift [$log findShift $shiftName]
        set person [$log getPerson $member]
        
        $log removeShiftMember $shift $person
        $shift  destroy
        $person destroy
        $log    destroy
    } msg]
    if {$status} {
        if {$person ne ""} {$person destroy}
        if {$shift ne ""} {$shift destroy}
        $log destroy
        error $msg
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
# @param shift - name of the shift to list.
#
proc listShiftMembers {shift} {
    set path [currentLogBookOrError]
    set log [logbook::logbook open $path]
    set status [catch {$log findShift $shift} result]
    set members [list]
    if {$status == 0} {
        set people [$result members]
        foreach person $people {
            lappend members [_makePersonDict $person]
            $person destroy
        }
        $result destroy
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
# @return list - of shift names.
#
proc listShifts {} {
    set path [currentLogBookOrError]
    set log  [logbook::logbook open $path]
    
    set status [catch {$log listShifts} result ]
    $log destroy
    if {$status} {
        error $result
    }
    set retval [list]
    foreach shift $result {
        set shiftName [$shift name]
        $shift destroy
        lappend retval $shiftName
    }
    return $retval
}
##
# currentShift
#   Provides the name of the current shift or "" if there isn't one.
#
# @return string -shift name or empty string.
#
proc currentShift {} {
    set path [currentLogBookOrError]
    set log  [logbook::logbook open $path]
    
    set shift [$log getCurrentShift]
    if {$shift ne ""} {
        set name [$shift name]
        $shift destroy
        
    } else {
        set name ""
    }
    return $name
}

##
# beginRun
#   Begin a new run
#
# @param num - run number.
# @param title - Title of the run .
# @param remoark - Remark to be associated with the run.
# @return none.
#
proc beginRun {num title {remark {}}} {
    set path [currentLogBookOrError]
    set log  [logbook::logbook open $path]
    
    set status [catch {$log begin $num $title $remark} result]
    $log destroy
    if {$status} {
        error $result
    }
    $result destroy;               # a run instance command.
}

##
# endRun
#   ends the current run
# @param remark - remark to be associated with this transition.
# @return none
#
proc endRun {{remark {}}} {
    set path [currentLogBookOrError]
    set log  [logbook::logbook open $path]
    
    set run [$log currentRun]
    if {$run eq ""} {
        $log destroy
        error "There is no active/current run."
    }
    set status [catch {$log end $run $remark} result]
    if {$status} {
        $log destroy
        error $result
    }
    $log destroy
    $result destroy ;               # it's the run instance object.
}

##
# pauseRun
#   Pauses the current run.
# @param remark - remark to be associated with this transition.
# @return none
#
proc pauseRun {{remark {}}} {
    set path [currentLogBookOrError]
    set log  [logbook::logbook open $path]
    
    set run [$log currentRun]
    if {$run eq ""} {
        $log destroy
        error "There is no active/current run."
    }
    set status [catch {$log pause $run $remark} result]
    if {$status} {
        $log destroy
        error $result
    }
    $log destroy
    $result destroy
}

##
# resumeRun
#    Resumes the current run.
# @param remark - remark to be associated with the state transition.
# @return none
#
proc resumeRun {{remark {}}} {
    set path [currentLogBookOrError]
    set log  [logbook::logbook open $path]
    
    set run [$log currentRun]
    if {$run eq ""} {
        $log destroy
        error "There is no active/current run."
    }
    set status [catch {$log resume $run $remark} result]
    if {$status} {
        $log $destroy
        error $result
    }
    $log destroy
    $result destroy
}

##
# emergencyEndRun
#    Does an emergency end of the run.
#
# @param remark - remark associated with the change in state.
# @return none
#
proc emergencyEndRun {{remark {}}} {
    set path [currentLogBookOrError]
    set log  [logbook::logbook open $path]
    
    set run [$log currentRun]
    if {$run eq ""} {
        $log destroy
        error "There is no active/current run."
    }
    set status [catch {$log emergencyStop $run $remark} result]
    if {$status} {
        $log $destroy
        error $result
    }
    $log destroy
    $result destroy
}


    
##
# listRuns
#    Lists the runs.
# @return a list of dicts.  Each dict describes a run and has the following
#     key/value pairs:
#   -  number - the run number.
#   -  title  - the run title.
#   -  isActive - (bool) if the run is active/current.
#   -  transitions - A list of dicts logging the transitions of this run:
#                    This dict has the following key/values:
#               *   transitionName - The actual transition type.
#               *   transitionTime - [clock seconds] when the transition was logged.
#               *   transitionComment - remark associated with the transition
#               *   shift - name of the shift on duty when the transition occured.
# @note _makeRunDict is used to create each run dictionary as this avoids code
#       duplication with currentRun, findRun etc.
proc listRuns {} {
    set path [currentLogBookOrError]
    set log  [logbook::logbook open $path]
    
    set result [list]
    set runs [$log listRuns]
    foreach run $runs {
        lappend result [_makeRunDict $run]
        $run destroy
    }
    return $result
}

##
# currentRun
#   Information about the current run.
# @return a dict as described in list runs or an empty dict if there is no
#         current run.
#
proc currentRun { } {
    set path [currentLogBookOrError]
    set log  [logbook::logbook open $path]
    
    set result [dict create]
    set run [$log currentRun]
    if {$run ne ""} {
        set result [_makeRunDict $run]
        $run destroy
    }
    $log destroy
    return $result
}

##
# findRun
#   Finds a run by run number and provides its information.
#
# @param num - run number.
# @return dict as described in listRuns or an empty dict if there is no current
#              run.
#
proc findRun {num} {
    set path [currentLogBookOrError]
    set log  [logbook::logbook open $path]
    
    set result [dict create]
    set run [$log findRun $num]
    if {$run ne ""} {
        set result [_makeRunDict $run]
        $run destroy
    }
    $log destroy
    return $result
    
}