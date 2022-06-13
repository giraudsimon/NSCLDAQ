#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
package require Tk
package require snit
package provide EVB::connectionList 1.0

namespace eval EVB {}

##
# This file defines a widget that can be used to display connection lists
# In order to retain the user callbacks for true user code, this operates
# via polling.
#
# The widget consists of a ttk::treeview without the tree.
# each element has host, description and State colums:
#
#
snit::widgetadaptor EVB::connectionList {
    option -updaterate -default 1; # Update rate in seconds.
    variable lastConnections [list]

    delegate option -text to hull
    
    variable afterId -1

    constructor args {
        installhull using ttk::labelframe
        
	
        ttk::treeview $win.table -columns [list Host Description State Stalled?] -displaycolumns #all \
            -show [list headings] -height 5 \
            -yscrollcommand [list $win.vsb set]
        $win.table heading #1 -text Host
        $win.table heading #2 -text Description
        $win.table heading #3 -text State
        $win.table heading #4 -text Stalled
        
        
        ttk::scrollbar $win.vsb -orient vertical \
            -command  [list $win.table yview]
      
        grid $win.table $win.vsb -sticky nsew
        grid columnconfigure $win 0 -weight 1
        grid rowconfigure $win 0 -weight 1
      
        $win.table column #1 -stretch off -width [expr 8*15]
        $win.table column #2 -stretch on  -minwidth [expr 16*15]
        $win.table column #3 -stretch off -width [expr 6*15]
        $win.table column #4 -stretch off -width [expr 8*15]
        
        grid  columnconfigure $win  0 -weight 1
        grid  columnconfigure $win 1 -weight 0
        
        $self configurelist $args
        $self _update;			# Stock the table and reschedule the update periodicity.
    }
    
    destructor {
      # Kill off any pending after:
      
      after cancel $afterId
    }
    #-----------------------------------------------------------------------
    # Internal (private) methods.

    ##
    # Update the table...relies on the eventbuilder having been started.
    #
    method _update {} {
      set connections [::EVB::getConnections]
      if {$lastConnections ne $connections} {
	    # Delete all the existing connections from the tree.
	    # Write the new ones.
	    
        $self _Clear
        $self _Stock $connections
  
        set lastConnections $connections
      }
      set afterId [after [expr $options(-updaterate)*1000] [mymethod _update]]
    }
    ##
    # Clear the widget
    #
    method _Clear {} {
      set items [$win.table children {} ]
      $win.table delete $items
    }
    ##
    # Stock the widget from the connection list.
    #
    # @param connections list of connections from the 
    # the event builder.
    #
    method _Stock connections {
      foreach connection $connections {
          $win.table insert {} end -values $connection
      }
    }
}
