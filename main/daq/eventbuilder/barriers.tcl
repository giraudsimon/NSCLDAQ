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
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file barriers.tcl
# @brief Widgets to display barrier statistics.

#-------------------------------------------------------------------------------
#
#  Package stuff

# If this location is not in the path, add it.

set here [file dirname [info script]]
if {[lsearch -exact $auto_path $here] == -1} {
    lappend auto_path $here
}

package provide EVB::barriers 1.0

package require Tk
package require snit
package require Iwidgets
package require EVBUtilities

#------------------------------------------------------------------------------
#  Establish namespaces.

namespace eval EVB {
    namespace eval BarrierStats {}
}

#-------------------------------------------------------------------------------
#  The widgets
#


##
#  @class Summary
#
#   Widget that summarizes barrier statistics.
#
# OPTIONS:
#   - -completecount     - Number of complete barriers received.
#   - -incompletecount   - Number of incomplete barriers received.
#   - -heterogenouscount - Number of barriers that have mixed types.
#
# LAYOUT:
#   +--------------------------------------------+
#   |                Barriers                    |
#   |  Complete      Incomplete   Heterogenous   |
#   |  <complete>    <incomplete> <heterogenous> |
#   +--------------------------------------------+
#
snit::widgetadaptor EVB::BarrierStats::Summary {
    option -completecount     -default 0
    option -incompletecount   -default 0
    option -heterogenouscount -default 0
    
    delegate option -text to hull
    constructor args {
        installhull using ttk::labelframe
        #
        #  Create the widgets
        
        ttk::label $win.title         -text {Barriers}
        ttk::label $win.completel     -text {Complete}
        ttk::label $win.incompletel   -text {Incomplete}
        ttk::label $win.heterogenousl -text {Heterogenous}
        
        ttk::label $win.complete     -textvariable ${selfns}::options(-completecount)
        ttk::label $win.incomplete   -textvariable ${selfns}::options(-incompletecount)
        ttk::label $win.heterogenous -textvariable ${selfns}::options(-heterogenouscount)
        
        # Lay them out.
        
        grid x                $win.title        x -padx 5
        grid $win.completel   $win.incompletel  $win.heterogenousl -padx 5
        grid $win.complete    $win.incomplete   $win.heterogenous  -padx 5
        
        grid columnconfigure $win {0 1 2} -weight 1 -uniform a
        
        grid configure $win -padx 5 -pady 5 

        $self configurelist $args
    }
}
##
# @class BarrierTypes
#
#  Displays a set of barrier type counts for a data source.
#
# OPTIONS
#    none
# METHODS
#    setCount barrierId count - Sets the number of barrier fragments
#                               gotten for barrierId to count.
#    clear                    - Sets all barrier counts to zero.
#    reinit                   - Gets rid of all 'table' elements.
#
#
# LAYOUT
#  +---------------------------------------+
#  |   Barrier Type          Count         |
#  |         id                count      ^|
#  |      ...                   ...       V|
#  +---------------------------------------+
#
#  Barrier ids are always maintained in sorted order (ascending).
#
snit::widget EVB::BarrierStats::BarrierTypes {
    component container
    
    delegate method setCount to container as setItem
    delegate method clear    to container
    delegate method reinit   to container
    
    delegate option * to container
    
    ##
    # Constructor
    #
    # @param args -option value pairs
    #
    constructor args {
        
        install container using EVB::utility::sortedPair $win.sp  \
            -title {Source Stats} -lefttitle {id} -righttitle {Bar. frags.}
        grid $win.sp  -sticky nsews
        
        
        
    }

}
##
# @class EVB::BarrierStats::queueBarriers
#
#   provide a per queue barrier statistics list.  This is a really
#   spiffy keen nested use of EVB::SortedWidget  where the managed widgets
#   are EVB::BarrierStats::BarrierTypes (a sorted pair in disguise).
#
# OPTIONS
#  - -text - The entire widget (well widgetadaptor really) is wrapped in a
#            ttk::labelframe.  This option is delegated to that frame so that
#            the frame can be titled.
# - -lefttitle - Title for the source column.
# - -righttitle - Title for the per source statistics widget column.
#
# METHODS
#   setStatistic sourceid barriertype count
#       - If necessary creates a new widget containing a BarrierTypes widget.
#         if necessary creates a new barrier type label/value pair within that
#         widget and sets the count of that widget to $count.
#
#   clear - Clears all values in all widgets to zero.
#   reset - Destroys all of the existing widgets
#
snit::widgetadaptor EVB::BarrierStats::queueBarriers {
    delegate option * to hull
    
    delegate method reset to hull 
    ##
    #  The constructor installs the hull with:
    #  - the overall title as the -text option if supplied.
    #  - the left title as Srcid
    #  - the right title as Statsitics
    #  - The -create script as a script that will crate a BarrierTypes widget
    #  - the -update script as a script that will set the BarrierTypew widget
    #    to the specified single value.
    #
    constructor args {
        installhull using ::EVB::utility::sortedWidget \
            -lefttitle "Source ID"  \
            -create [mymethod _CreateSource]           \
            -update [mymethod _UpdateSourceElement]   \
            -width 250 -height 100
      
        grid configure $win -padx 5 -pady 5  
        $self configurelist $args
    }
    
    #--------------------------------------------------------------------------
    # Public methods.
    #
    
    ##
    # setStatistic sourceid barriertype count
    #
    #  If necessary this creates a new source widget and even the
    #  barrier counter item within it.  Callbacks work with the
    #  sorted widget to do the widget specific work and the sorted widget
    #  really does the heavy lifting.
    #
    #  @param sourceid - The id of the source.
    #  @param barriertype - A barrier type.
    #  @param count       - Number of barrier packets seen by sourceid for the
    #                       barriertype barrier.
    #
    method setStatistic {sourceid barriertype count} {
        $hull update $sourceid [list $barriertype $count]
    }
    ##
    # clear
    #
    #   Clears the counts of all widgets to zero.
    #   The source  widgets one by one are cleared.
    #
    method clear {} {
        set idList [$hull listids]
        foreach id $idList {
            set widgdet [$hull getWidget $id]
            $widget clear
        }
    }
    
    
    #--------------------------------------------------------------------------
    # Private methods.
    #
    
    ##
    # _CreateSource
    #
    #   Create a new BarrierTypes widget that is empty.
    #
    # @param path  - Required widget path name
    #
    method _CreateSource path {
        EVB::BarrierStats::BarrierTypes $path -width 380
    }
    ##
    # _UpdateSourceElement widget clientData
    #
    #   Called back when we're asked to update one of our source element widgets.
    #   Just pull the barrier type and counts from the clientData and
    #   pass them on down to the setCount method of the widget.
    #
    # @param widget - the widget to update (this is a BarrierTypes widget)
    # @param clientData - Data passed along to us by the callback. This is a 2
    #                     element list containing in order:
    #                     - The barrier type for which we have a counter.
    #                     - The value of the counter.
    #
    method _UpdateSourceElement {widget clientData} {
        $widget setCount [lindex $clientData 0] [lindex $clientData 1]
    }
}
##
# @class EVB::BarrierStats::incomplete
#
#  This widget provides a mechanism to display the number of times each
#  source has failed to contribute to a barrier.
#
#  This is just a sortedPair widget where the key is the source id and
#  the right hand side is the incomplete count.
#
#
snit::widgetadaptor EVB::BarrierStats::incomplete {
    delegate option * to hull
    delegate method * to hull
    
    ##
    #  constrcuctor
    #
    # Just specifies some reasonable default parameters for the  column titles.
    #
    constructor args {
        installhull using ::EVB::utility::sortedPair \
	    -title {Missing Barrier Fragments} \
            -lefttitle "Source ID" -righttitle Missing
       
        grid configure $win -sticky nsew 
        grid columnconfigure $win 0 -weight 1 
        $self configurelist $args
    }
}

#------------------------------------------------------------------------------
#   Testing stuff.

##
# Testing utility to update a widget option from a trace on a variable
# that has a name matching the option:
#
# @param widget - the widget to modify.
# @param name1  - name of the variable (and option)
# @param name2  - index of the variable if an array.
# @param op     - operation performed.
#

namespace eval EVB {
    namespace eval test {}
}

proc EVB::test::BarrierSummary {} {
    
    # Wiget under test.
    
    ::EVB::BarrierStats::Summary .target
    pack .target
    
    # Control panel for test:
    
    toplevel .panel
    label .panel.completel -text {Complete}
    entry .panel.complete  -textvariable -completecount
    grid .panel.completel .panel.complete
    
    label .panel.incompletel -text {Incomplete}
    entry .panel.incomplete  -textvariable -incompletecount
    grid  .panel.incompletel .panel.incomplete
    
    label .panel.heterogenousl -text {Heterogenous}
    entry .panel.heterogenous -textvariable -heterogenouscount
    grid  .panel.heterogenousl .panel.heterogenous
    
    
    foreach var [list -completecount -incompletecount -heterogenouscount] {
        trace add variable ::$var write [list EVB::test::updateWidgetOption .target]
    }
    
}

proc EVB::test::BarrierTypes {} {
    EVB::BarrierStats::BarrierTypes .target
    pack .target
    
    # Control panel widgets and actions
    
    toplevel .panel
    ttk::button .panel.clear -text Clear -command [list .target clear]
    ttk::button .panel.reinit -text Empty -command [list .target reinit]
    
    ttk::label .panel.idl -text Id
    ttk::label .panel.cl  -text Counts
    ttk::entry .panel.id
    ttk::entry .panel.c
    ttk::button .panel.set -text set -command {.target setCount [.panel.id get] [.panel.c get]}
    
    grid .panel.idl .panel.cl
    grid .panel.id  .panel.c .panel.set
    grid .panel.clear .panel.reinit
    
    

}



