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
# @file lg_mkshift
# @brief Create a new shift
# @author Ron Fox <fox@nscl.msu.edu>
#
if {[array names env DAQROOT] eq "DAQROOT"} {
    set tcllibs [file join $env(DAQROOT) TclLibs]
} else {
    set tcllibs [file normalize [file join [file dirname [info script]] .. TclLibs]]
}
lappend auto_path $tcllibs

package require logbookadmin
package require report
package require struct

##
# Usage:
#   lg_mkshift ?shiftname?
#
#  Makes a new shift. The shift name is ensured to be unique.
#  If the shift name is not provided, a GUI is popped up to allow
#  the creation and population of the shift.  If the shift name is
#  supplied, the user must add people to the shift at a later date
#  using lg_mgshifts (manage shifts).
#


proc Usage {} {
    puts stderr {Usage:}
    puts stderr {    lg_mkshift ?shiftname?}
    puts stderr {Where:}
    puts stderr {   Omitting shiftname pulls up a GUI to allow you to define}
    puts stderr {   a new shift and its members.  Supplying the optional shiftname}
    puts stderr {   creates a new shift (if the name is unique) that has no members.}
    puts stderr {   The shift management program (lg_mgshfits) can then be used to}
    puts stderr {   add members to that or any other shift.}
    exit -1
}

##
# shiftExists
#    @param shiftName.
#    @return bool - true if a shift with that name already exists.
#
#
proc shiftExists {shiftName} {
    set shifts [listShifts]
    return [expr {$shiftName in $shifts}]
}
##
#  reportShiftMembers
#  Produce a report of the shift members to stderr.
#
#   @param shiftname - Name of the shift.
#   @return report   - A pair consisting of a report and the matrix
#                      of shift name information.  When  no longer needed
#                      the destroy method must be called on each of them.
#   @note - it's the caller's responsibility to ensure the shift actually
#           exists.
proc reportShiftMembers {shiftName} {
    set p [struct::matrix]
    $p   add columns 3
    $p   add row [list "salutation " "First Name " "Last Name "]
    foreach person [listShiftMembers $shiftName] {
        $p add row [list \
            "[dict get $person salutation] "  "[dict get $person firstName] " \
            "[dict get $person lastName] "    \
        ]
    }
    set rep [report::report r[incr ::reportnum] [$p columns]]
    return [list $rep $p]
}

##
# duplicateShiftMessage
#    Error message for a shift is a duplicate
# @param shiftName - name of the shift
# @return string   - error message.
#
proc duplicateShiftMessage {shiftName} {
    # Get the member report string:
    
    set reportInfo [reportShiftMembers $shiftName]
    set report [lindex $reportInfo 0]
    set matrix [lindex $reportInfo 1]
    set members [$report printmatrix $matrix]
    $report destroy
    $matrix destroy
    
    set msg "$shiftName already exists with the following members:\n\n"
    append msg $members
    append msg "\nUse lg_mgshift to add members to the shift."
    return $msg
}

#-------------------------------------------------------------------
# GUI:
#   We bring up a shift creation dialog box; allow people to be added
#   to the shift and then on OK:
#   - If the shift is already in existence retry.
#   - If not create the shift.
#
proc GUI {} {
    package require Tk
    package require snit
    package require DataSourceUI
    wm withdraw .
    
    snit::widgetadaptor buildShift {
        #TODO : implement column click sorting.
        
        ## note in the configuraiton handling methods below offshift
        # and onshift are compnents of widget names create in the
        # constructor (e.g. $win.offshift is a treeview as is $win.onshift)
        # Change those at youre peril.
        option -offshift \
            -configuremethod [list setMembers offshift] \
            -cgetmethod [list getMembers offshift]
        option -onshift \
            -configuremethod [list setMembers onshift] \
            -cgetmethod [list getMembers onshift]
        option -shiftname
        constructor args {
            installhull using ttk::frame
            
            ttk::treeview $win.offshift \
                -yscrollcommand [list $win.offscroll set] \
                -columns [list id {salutation} {firstName} {lastName}] \
                -displaycolumns [list 1 2 3]    \
                -show headings
            ttk::scrollbar $win.offscroll \
                -orient vertical -command [list $win.offshift yview]]
            ttk::treeview $win.onshift \
                -yscrollcommand [list $win.onscroll set] \
                -columns [list id {salutation} {firstName} {lastName}] \
                -displaycolumns [list 1 2 3]    \
                -show headings
            ttk::scrollbar $win.onscroll \
                -orient vertical -command [$win.onshift yview]
            
            ttk::button $win.toshift -text {->} \
                -command [mymethod movePeople $win.offshift $win.onshift]
            ttk::button $win.fromshift -text {<-} \
                -command [mymethod movePeople $win.onshift $win.offshift]
            
            ttk::frame $win.nameframe
            ttk::label $win.nameframe.namelabel -text {Shift Name: }
            ttk::entry $win.nameframe.name -textvariable [myvar options(-shiftname)]
     
            setHeadings $win.offshift
            setHeadings $win.onshift
     
            grid $win.offshift -column 0 -row 0 -rowspan 2 -sticky nsew
            grid $win.offscroll -column 1 -row 0 -rowspan 2 -sticky nsw
            
            grid $win.toshift -column 2 -row 0
            grid $win.fromshift -column 2 -row 1
            
            grid $win.onshift  -column 3 -row 0 -rowspan 2 -sticky nsew
            grid $win.onscroll -column 4 -row 0 -rowspan 2 -sticky nsw
            
            grid $win.nameframe -row 2 -column  0 -sticky nwew
            grid $win.nameframe.namelabel -row 0 -column 0 -sticky e
            grid $win.nameframe.name -row 0 -column 1 -sticky w
            $self configurelist $args
        }
        #----------------------- configuration operations ------------
        
        ##
        # setMembers
        #   Sets the  contents of a tree view from the list of dicts
        #   that describe the people to appear in it.
        #
        #  @param wid - the tail name widget to modify.
        #  @param optname - Option name - ignored.
        #  @param optval - list of person dicts.
        #
        method setMembers {wid optname optval} {
            set wid $win.$wid
            clearTree $wid
            foreach person $optval {
                $wid insert {} end -values [list          \
                    [dict get $person id] [dict get $person salutation] \
                    [dict get $person firstName] [dict get $person lastName] \
                ]
            }
        }
        ##
        # getMembers
        #   Reconstructs the the list of dicts of the people in the
        #   specified treeview:
        #
        # @param wid - tail of widget path that specifies the
        # @param optname - the name of the option that's being queried
        #                 which we ignore.
        #
        #
        method getMembers {wid optname} {
            set wid $win.$wid
            set people [$wid children {}]
            set result [list]
            foreach person $people {
                set info [$wid item $person -values]
                lappend result [dict create \
                    id [lindex $info 0] lastName [lindex $info 3] \
                    firstName [lindex $info 2] salutation [lindex $info 1] \
                ]   
            }
            return $result
        }
        #-------------- event handling ----------------------------
        
        ##
        # movePeople
        #   Moves selected people from one tree view to another:
        #
        # @param from - from widget
        # @param to   - to widget
        method  movePeople {from to } {
            set moveItems [$from selection]
            foreach item $moveItems {
                set values [$from item $item -values]
                $from delete $item
                $to insert {} end -values $values
            }
        }
        
        #------------------------ procs ---------------------------
        
        ##
        # clearsw the tree specified:
        proc clearTree {wid} {
            
            set items [$wid children {}]
            foreach item $items {
                $wid delete $item
            }   
        }
        ##
        # Set treeview headings for wid parameter.
        #
        proc setHeadings {wid} {
            $wid heading salutation -text Salutation
            $wid heading firstName  -text {First Name}
            $wid heading lastName   -text {last Name}
        }
    }
    toplevel .dialog
    DialogWrapper .dialog.dialog
    set formParent [.dialog.dialog controlarea]
    set form [buildShift $formParent.form -offshift [listPeople]]
    .dialog.dialog configure -form $form
    pack .dialog.dialog -expand 1 -fill both
    
    set done 0
    while {!$done} {
        set result [.dialog.dialog modal]
        if {$result eq "Ok"} {
            ##
            #  Get shift and validate it:
            #   must not be an empty string and must not yet exist:
            #
            set shiftName [string trim [$form cget -shiftname]]
            
            if {$shiftName eq ""} {
                tk_messageBox -parent .dialog -title {Specify shift} -type ok -icon error \
                    -message {Please fill in the name of the new shift}
            } elseif {[shiftExists $shiftName]} {
                tk_messageBox -parent .dialog -title {Duplicate shift} -type ok -icon error \
                    -message [duplicateShiftMessage $shiftName]
            } else {
                # Shift is ok...get the members and try to create
                # the shift.  If successful we're done. If not
                # report the error and try again (should succeed unless
                # Some other user yanked a person out from us.
                # So we'll reconfigure the off/onshift values:
                
                set members [$form cget -onshift]
                set shiftIds [list]
                foreach member $members {
                    lappend shiftIds [dict get $member id]
                }
                set status [catch {
                    createShift $shiftName $shiftIds
                } msg]
                if {$status} {
                    tk_messageBox -parent .dialog -title {Shift creation failed} \
                        -type ok -icon error \
                        -message "Could not make shift $shiftName: $msg"
                    $form configure -offshift [listPeople]
                    $form configure -onshift [list]
                } else {
                    set done 1
                }
            }
        
        } else {
            set done 1
        }
    }
    destroy .dialog 
}

#--------------------------------------------------------------------
#  Entry point
#


##
#  There must be 0 or 1 parameters:
#

if {[llength $argv] == 0} {
    GUI
} elseif {[llength $argv] == 1} {
    set shift [lindex $argv 0]
    if {[shiftExists $shift]} {
        puts stderr [duplicateShiftMessage $shift]
        exit -1
    }
    createShift $shift [list]
} else {
    Usage
}
exit 0;                    # in case we're graphic. 
