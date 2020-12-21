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
# @file  ShiftEditor.tcl
# @brief Snit megawidget to edit shifts.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide ShiftEditor 1.0
package require Tk
package require snit
package require textprompter

##
# @class ShiftEditor
#
#   Megawidget to allow shifts to be edited.
#
#  OPTIONS
#     -offshift  - suppy/retrieve the list of people dicts for people not on shift.
#     -onshift   - same  as above but for people that are _on_ shift.
#     -shiftname - set/retrieve the name of the shift.
#
snit::widgetadaptor ShiftEditor {
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
        
        #ttk::frame $win.nameframe
        #ttk::label $win.nameframe.namelabel -text {Shift Name: }
        #ttk::entry $win.nameframe.name -textvariable [myvar options(-shiftname)]
        textprompt $win.nameframe -text {Shift Name: } -textvariable [myvar options(-shiftname)]
 
        setHeadings $win.offshift
        setHeadings $win.onshift
 
        grid $win.offshift -column 0 -row 0 -rowspan 2 -sticky nsew
        grid $win.offscroll -column 1 -row 0 -rowspan 2 -sticky nsw
        
        grid $win.toshift -column 2 -row 0
        grid $win.fromshift -column 2 -row 1
        
        grid $win.onshift  -column 3 -row 0 -rowspan 2 -sticky nsew
        grid $win.onscroll -column 4 -row 0 -rowspan 2 -sticky nsw
        
        grid $win.nameframe -row 2 -column  0 -sticky w
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
    # clears the tree specified:
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