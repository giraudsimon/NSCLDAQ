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
# @file lg_selshift
# @brief Select the current shift.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
#  This can be used in one of two ways:
#
#  $DAQBIN/lg_selshift shiftname - selects that shfit as current.
#  $DAQBIN/lg_selfshift    - brings up a persistent GUI that lets you select
#                            the current shift, and displays the current shift
#                            and its members.
#

package require logbookadmin


##
# GUI
#   Called if the GUI form has been selected; only then are Tk and Snit
#   brought in to build megawidgets to support this.
#
#
proc GUI {} {
    package require Tk
    package require snit
    
    ##
    # @class CurrentShift
    #      Shows the current shift and its members.
    #      this widget auto updates every second or so.  This means that
    #      if the shift changes via some external action, this
    #      class will automatically reflect that change.
    #
    snit::widgetadaptor CurrentShift {

        constructor args {
            installhull using ttk::frame
            
            # Labels across the top show the current shift:
            
            ttk::label $win.shiftlabel -text "Current Shift Name: "
            ttk::label $win.shiftname  -text "                      "
            
            #  A list box containing the members of the shift:
            
            ttk::label $win.memberlabel -text {Shift Members:}
            listbox $win.members -width 32 \
                -state disabled -yscrollcommand [list $win.sb set]
            ttk::scrollbar $win.sb -orient vertical -command [$win.members yview]
            
            # Lay all this out:
            
            grid $win.shiftlabel -row 0 -column 0 -sticky e
            grid $win.shiftname  -row 0 -column 1 -sticky w
            grid $win.memberlabel -row 1 -column 0 -sticky w
            grid $win.members -row 2 -column 0 -sticky nsew
            grid $win.sb      -row 2 -column 1 -sticky nsw
            
            $self update 
        }
        ##
        # Get the current shift, and if there is one and its different
        # than what we display, update:
        
        method update {} {
            set shift [currentShift]
            if {$shift ne [$win.shiftname cget -text]} {
                $win.shiftname configure -text $shift
                $win.members configure -state normal
                $win.members delete 0 end
                if {$shift ne ""} {
                    set members [listShiftMembers $shift]
                
                    foreach member $members {
                        $win.members insert end \
                           "[dict get $member salutation] [dict get $member firstName] [dict get $member lastName]"
                    }
                }
                $win.members configure -state disabled
            }
            after 1000 [mymethod update]
        }
    }
    CurrentShift .w
    pack .w -fill both -expand 1
    
}
    

if {[llength $argv] == 1} {
    setCurrentShift [lindex $argv 0]
} elseif {[llength $argv] == 0} {
    GUI
} else {
    puts stderr "Usage:"
    puts stderr "   lg_selshift ?shift-name?"
    puts stderr "Where:"
    puts stderr "   shift-name if supplied is the name of the shift to be made current"
    puts stderr "              if the shift-name is supplied the program exits after "
    puts stderr "              setting that shift current."
    puts stderr "   If no shift is given, the program starts a GUI which is intended"
    puts stderr "   to allow the user to select the current shift interactively"
    puts stderr "   changing shifts as often as desired before exiting."
    puts stderr "   The intent of this latter program is that it be used during the"
    puts stderr "   run to indicated shift changes."
}