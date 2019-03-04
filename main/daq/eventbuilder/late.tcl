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


# add this directory to auto_path if it's not there yet.

set here [file dirname [info script]]
if {[lsearch $::auto_path $here] == -1} {
    lappend ::auto_path $here
}

##
# @file late.tcl
# @brief Provide data late GUI information.
#


package provide EVB::Late 1.0
package require Tk
package require snit
package require EVBUtilities

namespace eval EVB {}


##
# @class EVB::lateSummary
#
# Widget that summarizes the data late packets.
# Specific information includes the total number of late
# fragments and the worst case time difference.
#
# OPTIONS
#
#   - -count - total number of late fragments.
#   - -worst - Worst case lateness.
#
# LAYOUT
#
#   +-----------------------------------------------+
#   | Late:  <late frags>  Worst: <worst time diff> |
#   +-----------------------------------------------+
#
snit::widget EVB::lateSummary {
    component innerHull
    
    option -count
    option -worst

    delegate option * to innerHull

    ##
    # constructor
    #  - Create/layout widgets.
    #  - process options.
    #
    constructor args {
	
	install innerHull using ttk::frame $win.innerHull

	ttk::label $win.innerHull.latel -text "Late: "
	ttk::label $win.innerHull.late  \
	    -textvariable ${selfns}::options(-count) \
	    -width 6

	ttk::label $win.innerHull.worstl -text "Worst: "
	ttk::label $win.innerHull.worst \
	    -textvariable ${selfns}::options(-worst) \
	    -width 11

	# layout widgets in the innerHull

	grid $win.innerHull.latel -column 0 -row 0 -sticky w
	grid $win.innerHull.late  -column 1 -row 0 -sticky e
	grid $win.innerHull.worstl -column 2 -row 0 -sticky w
	grid $win.innerHull.worst -column 3 -row 0 -sticky e

	# paste the innerHull into the hull 

	grid $win.innerHull -sticky nsew

	# process construction line options:

	$self configurelist $args
    }
}
##
# EVB::labelpair
#
#   A pair of labels glued into a single widget (see lateFragments below
#   for one use case).
#
#   +----------------------------+
#   | leftlabel      | rightlabel|
#   +----------------------------+
#
# OPTIONS
#   -lefttext   - text in left label.
#   -righttext  - text in right label.
#
snit::widgetadaptor ::EVB::labelpair {
    component left
    component right
    
    delegate option -lefttext to left   as -text
    delegate option -righttext to right as -text
    
    ##
    # Construct this all inside a ttk::frame hull:
    #
    constructor args {
        installhull using ttk::frame
        
        install left  using ttk::label $win.left -width 10
        install right using ttk::label $win.right -width 10
        
        grid $left $right
        
        $self configurelist $args
    }
}

##
# EVB::lateFragments
#
#  Displays the late fragment statistics.
#  this includes a summary widget and a 
#  set of widgets that show the detailed per source
#  statistics for all sources that have been late
#  since the last clear.
#
#
# OPTIONS
#   - -count -- Total number of late fragments.
#   - -worst -- Worst case late time difference.
#
# METHODS
#   - source id count late - provide per source statistics.
#   - clear -- clear statistics and destroy the set of
#              per source statistics.
#
# LAYOUT
#
#   +--------------------------------------+
#   | Late: -count   Worst: -worst         |
#   +--------------------------------------+
#   | Source    Lates    Worst             |
#   | id1       lates1   worst1            |
#   \                                     /
#   /                                     \
#   +--------------------------------------+
#
# NOTES:
#   # The individual source data includes a 
#   scroll bar if needed.
#
snit::widgetadaptor EVB::lateFragments {
    component summary
    component container
    
    delegate option -count to summary
    delegate option -worst to summary
    
    delegate method clear to container as reset
    
    ##
    # constructor
    #    install the ttk::frame as a hull.
    #    install the containers as a sortedWidget
    #    Lay all this out appropriately.
    #
    # @param args - configuration option values.
    #
    constructor args {
        installhull using ttk::frame
        
        install summary using ::EVB::lateSummary $win.summary
        install container using ::EVB::utility::sortedWidget $win.container \
            -title {Late Fragments By Source} -lefttitle {Id count} -righttitle {worst} \
            -create [mymethod _CreateSource] -update [mymethod _UpdateSource]
        
        grid $summary -sticky new
        grid $container -sticky sew
         
        grid columnconfigure $win 0 -weight 1
        grid rowconfigure    $win 1 -weight 1

        $self configurelist $args
    }
    ##
    # source id count late
    #
    #   Sets the count and worst case time difference for a source.
    #   If needed this makes a new widget.
    #   This will always generate a callback to _UpdateSource and, if the
    #   source does not exist will generate a callback to _CreateSource
    #
    # @param id    - Source id.
    # @param count - Number of late packets
    # @param worst - time difference of worst case late packet.
    #
    method source {id count worst} {
        $container update $id [list $count $worst]
    }
    #-----------------------------------------------------------------------
    #
    #  Private methods.
    
    ##
    # _CreateSource widget
    #
    #  Creates a widget for the statistics for an event source.  This is
    #  an ::EVB::labelpair
    #
    # @param path - the widget name path desired.
    #E
    method _CreateSource widget {
        ::EVB::labelpair $widget
    }
    ##
    #  UpdateSource widget data
    #    Updates the widget corresponding to a data source.
    #
    # @param widget - name path of the widget to update.
    # @param data   - Pair containing in turn the late count and worst case time
    #                 difference.
    #
    method _UpdateSource {widget data} {
        set count [lindex $data 0]
        set worst [lindex $data 1]
        $widget configure -lefttext $count -righttext $worst
    }
}

