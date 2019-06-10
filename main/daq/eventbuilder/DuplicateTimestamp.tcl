
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   DuplicateTimestamp.tcl
# @brief  Provide UI elements for duplicate timestamp statistics.
# @author <fox@nscl.msu.edu>

package provide EVB::DuplicateTimestamp 1.0
package require Tk
package require snit
package require EVBUtilities

##
# @class DuplicateTimestampWidget
#
#   Provides a widget that displays the statistics of a single sourcde Id's
#   duplicate timestamp.  The widget is just two labels.  The left one is
#   the duplicate timestamp counter while the right one is the most recently
#   seen timestamp.
#
# OPTIONS
#    * -count    - Count of duplicates.
#    * -laststamp- Most recent duplicate timestamp value.

snit::widget EVB::DuplicateTimestampWidget {
    component count
    component lastStamp
    
    delegate option -count        to count     as -text
    delegate option -laststamp to lastStamp as -text
    
    ##
    # constructor
    #    Install a ttk::frame as the hull, install and layout the
    #    components.
    #    Process arguments so that any intial settings will get made.
    #
    constructor args {
        
        
        install count     using label $win.count
        install lastStamp using label $win.laststamp
        
        grid $count $lastStamp -sticky nsw -padx 5
        grid columnconfigure $win {0 1} -weight 1
        
        $self configurelist $args
    }
}

##
# @class EVB::DuplicateTimestampStatistics
#   This is a manager for a sorted widget that maintains statistics
#   for source ids that have observed duplicate timstamps.
#   The sorted widget will get populated with DuplicateTimestampWidget
#   The widget layout is:
#
#   +---------------------------------------------+
#   |   Duplicate Timestamps:   total-count       |
#   |                                             |
#   |  id       count/last-stamp                  |
#   |   .............................             |
#   +---------------------------------------------+
#
#  METHODS:
#     update - passes in the result from EVB::dupstats
#              to update the megawidget values.
#     clear  - Clear all counters and destroy the contents of the id table.
#
snit::widgetadaptor EVB::DuplicateTimestampStatistics {
    component totalcounts
    component sidcontainer
    
    delegate option * to sidcontainer
    

    
    ##
    # constructor
    #   *   install the hull as a ttk::Frame.
    #   *   install/create and layout the widgets:
    #   *   process the command options.
    #
    constructor args {
        installhull using ttk::frame
        ttk::label $win.tottitle -text {Duplicate Timestamps:}
        install totalcounts using ttk::label $win.totals -text 0
        install sidcontainer using EVB::utility::sortedWidget $win.detail \
            -lefttitle id -righttitle {Count  last timestamp}  \
            -create [mymethod _createWidget] -update [mymethod _updateWidget]
        
        grid $win.tottitle $totalcounts -sticky wns
        grid $sidcontainer -columnspan 2 -sticky nsew
        grid columnconfigure $win {0 1} -weight 1
        grid rowconfigure $win 1 -weight 1
        
        $self configurelist $args
        
    }
    ##
    #  Called with new information about duplicate stamps and updates the
    #  uderlying widget
    #
    # @param dupinfo - output from EVB::dupstat get
    #
    method update {dupinfo} {
        $totalcounts config -text [lindex $dupinfo 0]
        foreach info [lindex $dupinfo 1] {
            $sidcontainer update [lindex $info 0] $info
        }
    }
    ##
    # clear
    #   Clear all counters
    #
    method clear  {} {
        $totalcounts config -text {0}
        $sidcontainer reset
    }
    
    #-----------------------------------------------------------------------
    # Private methods.
    #
    
    ##
    # _createWidget
    #
    #   Required by the sortedWidget container.  This creates a new widget
    #   for the statistics for an id.
    #
    # @param path - widget path to create.
    #
    method _createWidget path {
        EVB::DuplicateTimestampWidget $path
    }
    ##
    # _updateWidget
    #   Updates the contents of an EVB::DuplicateTimestampWidget
    #  @param path - widget path.
    #  @param info - The counter information for that widget.
    #
    method _updateWidget {path info} {
        $path configure -count [lindex $info 1] -laststamp [lindex $info 2]
    }
}

##
# @class EVB::DuplicateWindow
#
#  Just a top level widget that contains an EVB::DuplicateTimestampStatistics widget.
#  all methods/options are delegated to that widget.
#
snit::widgetadaptor EVB::DuplicateWindow {
    component container
    delegate option * to container
    delegate method * to container
    
    ##
    # constructor
    #   Install the hull as a top level and inside of that, pack an
    #   EVB::DuplicateTimestampStatistics widget.
    #
    constructor args {
        
        installhull using toplevel  
        install container using EVB::DuplicateTimestampStatistics $win.container
        
        pack $container -expand 1 -fill both
        
        $self configurelist $args
        set me $::tcl_platform(user)
        wm title $win "Ts dups for $me"
    }
}
##
# @class EVB::DuplicatePopup
#
#    Manager for an EVB::DuplicateWindow, created as needed
#    We implement the following relay methods:
#
#  METHODS
#   _configureRelay  - Used to relay the -title, -lefttitle, righttitle options
#                      ensuring the widget exists before performing these actions.
#   update           - Ensures widget exists before relaying update.
#   clear            - nsures the wiget exists before relaying clear.
#
snit::type EVB::DuplicatePopup {
    option -title      -configuremethod _configureRelay  -default {Per source details}
    option -lefttitle  -configuremethod _configureRelay  -default id
    option -righttitle -configuremethod _configureRelay  -default {Count  last timestamp}
    
    ##
    # update
    #    Ensure the widget is created and then invoke its update method.
    #
    # @param info - the output from EVB::dupstats get
    #
    method update info {
        set widget [$self _getWidget]
        $widget update $info
    }
    ##
    # clear
    #  Ensure the widget is created and then clear it:
    #
    method clear {} {
        [$self _getWidget] clear
    }
    ##
    # _configureRelay
    #    Relays a config operation
    #
    # @param opt - The option to configure
    # @param val - New value.
    #
    method _configureRelay {opt val} {
        set options($opt) $val
        [$self _getWidget] configure $opt $val
    }
    #----------------------------------------------------------------------
    # Internal methods.
    
    ##
    # _toplevel
    #    chose a determinstic name for our toplevel
    #
    method _toplevel {} {
       return [string map [list :: _] ".dup$self"] 
    }
    ##
    # getWidget
    #   Returns the widget, creating it if need be./
    #
    method _getWidget {} {
        set name [$self _toplevel]
        if {$name ni [winfo children .]} {
            EVB::DuplicateWindow $name \
                -title $options(-title) -lefttitle $options(-lefttitle) \
                -righttitle $options(-righttitle)
        }
        return $name
    }
}


    
