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
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file header.tcl
# @brief Megawidget containing the scaler display header.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide header 1.0
package require Tk
package require snit
package require scalerUtil


##
# runInfo megawidget:
#    +--------------------------------------------------+
#    | Title:   <current title>   Run:  <run num>       |
#    | State:   <current state>                         |
#    +--------------------------------------------------+
#
# OPTIONS:
#  *  -title    - value of the title
#  *  -run      - Run number value.
#  *  -state    - Run state (defaults to -Unknown-)
#
snit::widgetadaptor runInfo {
    option -title
    option -run -default 0
    option -state -default -Unknown-
    
    constructor args {
        installhull using ttk::frame
        
        $self configurelist $args
        
        ttk::label $win.tlabel -text {Title: }
        ttk::label $win.title -textvariable ${selfns}::options(-title)
        
        ttk::label $win.rlabel -text {Run: }
        ttk::label $win.run    -textvariable ${selfns}::options(-run)
        
        ttk::label $win.slabel -text {State: }
        ttk::label $win.state  -textvariable ${selfns}::options(-state)
        
        grid $win.tlabel $win.title -sticky e
        grid $win.rlabel $win.run   -sticky e
        grid $win.slabel $win.state -sticky e
        
        
    }
}

##
# runTime
#    Megawidget that has information about timing specifically:
#    * The elapsed run time.
#    * The most recent update interval for each data source.
#
# OPTIONS
#   -elapsed    - Number of seconds of elapsed run time.
#
# Methods:
#   update      - Update/add the dt for a data source.
#   clear       - Clear the dts for all sources.
#
#
snit::widgetadaptor runTime {
    option -elapsed -default 0 -configuremethod _formatElapsed
    
    variable formattedElapsed "0 0:0:0.0"
    
    #  This array is indexed by source id and has values that are the widgets
    #  that contain the update interval for that source id.
    #
    
    variable sources -array [list]
    
    constructor args {
        installhull using ttk::frame
        ttk::label $win.lelapsed -text "Elapsed Run Time: "
        ttk::label $win.elapsed  -textvariable [myvar formattedElapsed]
        
        ttk::separator $win.labelsep -orient vertical
        ttk::separator $win.headersep -orient horizontal
        
        ttk::label $win.lsource  -text "  Source ID  "
        ttk::label $win.ldt      -text "Update Interval"
        
        
        $self configurelist $args
        
        grid $win.lelapsed $win.elapsed $win.labelsep $win.lsource $win.ldt -sticky nsw
        grid $win.headersep -             -              -             -    -sticky ew
    }
    ##
    # update
    #   Update (if needed adding) the update interval for a data source.
    #
    # @param sid - The source id.
    # @param dt  - The time in seconds of the update.
    #
    method update {sid dt} {
        if {[array names sources $sid] eq ""} {
            $self _createSource $sid
        }
        set srcWidget $sources($sid)
        $srcWidget configure -text $dt
    }
    ##
    # clear all update intervals for all sources.
    #
    method clear {} {
        foreach source [array names sources] {
            $sources($source) configure -text 0
        }
    }

    
    #----------------------------------------------------------------------
    #
    #   private methods.
    #
    
    ##
    # _createSource
    #    Create a new source widget set and grid it in.  The widget set consists
    #    of a vertical separator, and two label widgets, for the source id and its
    #    dt.  The dt label widget path is saved and put in the soures array indexe
    #    by sourceid.  The source id is used to generate unique widget names
    #
    # @param sid - the new source id.
    #
    method _createSource sid {
        ttk::separator $win.sep_$sid -orient vertical
        ttk::label $win.sid_$sid     -text $sid
        ttk::label $win.dt_$sid      -text { }
        
        grid x x $win.sep_$sid $win.sid_$sid $win.dt_$sid -sticky nsw
        set sources($sid) $win.dt_$sid
    }
    
    ##
    # _formatElapsed
    #    Take the elapsed time and format it into d hh:mm:ss
    #     as it is being configured into -elapsed.
    #
    # @param opt   - Name of the option being configured.
    # @param value - New value.
    #
    method _formatElapsed {opt value} {

        set formattedElapsed "[formatElapsedTime $value]  "
        set options($opt) $value
    }
}

##
# header
#   Heading of the scaler display.  Contains side by side
#   runInfo and runTime widgtets.
#
#  All options and methods from each are exposed/delegated.
#
snit::widgetadaptor header {
    component info
    component timing
    
    delegate option -title to info
    delegate option -run   to info
    delegate option -state to info
    
    delegate option -elapsed to timing
    
    delegate method update to timing
    delegate method clear  to timing
    
    ##
    # constructor
    #
    constructor args {
        installhull using ttk::frame
        
        install info   using runInfo $win.info
        install timing using runTime $win.time
        pack $info -fill y -expand 1 -side left
        pack $timing -fill y -expand 1 -side right
        
        $self configurelist $args
    }
}
