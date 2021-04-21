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
# @file  ReadoutRESTUI.tcl
# @brief GUI elements to control/display a Readout program via REST.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide ReadoutRESTUI 1.0
package require Tk
package require snit


##
# @class ReadoutStatistics
#    Provide statistics widget for READOUT
#
# OPTIONS
#   -data   - a statistics dict from getStatistics.
#
snit::widgetadaptor  ReadoutStatistics {
    option -data -configuremethod _cfgData
    
    ##
    # Construct the widgets in a ttk::frame hull and lay them out.
    #
    constructor args {
        installhull using ttk::frame
        
        # Top Row:
        
        ttk::label $win.statistics -text Statistics
        ttk::label $win.triggers   -text Triggers
        ttk::label $win.accepted   -text {Accepted Triggers}
        ttk::label $win.bytes      -text {Bytes of Data}
        
        #   Per-run row:
        
        ttk::label $win.perrun    -text {Per run}
        ttk::label $win.pertrigs
        ttk::label $win.peraccepts
        ttk::label $win.perbytes
        
        #  Cumulative
        
        ttk::label $win.cumulative -text {Cumulative}
        ttk::label $win.cumtrigs
        ttk::label $win.cumaccepts
        ttk::label $win.cumbytes
        
        grid $win.statistics $win.triggers $win.accepted $win.bytes -sticky e -padx 5
        grid $win.perrun $win.pertrigs $win.peraccepts $win.perbytes -sticky e -padx 5
        grid $win.cumulative $win.cumtrigs $win.cumaccepts $win.cumbytes -sticky e -padx 5
        
        $self configurelist $args
    }
    #---------------------------------------------------------------------
    # Configuration management
    
    ##
    # _cfgData
    #    Configure the -data option.
    #
    # @param optname - option name.
    # @param value   - data dict.
    #
    method _cfgData {optname value} {
        
        $win.pertrigs configure -text [dict get $value perRun triggers]
        $win.peraccepts configure -text [dict get $value perRun acceptedTriggers]
        $win.perbytes  configure -text [dict get $value perRun bytes]
        
        $win.cumtrigs configure -text [dict get $value cumulative triggers]
        $win.cumaccepts configure -text [dict get $value cumulative acceptedTriggers]
        $win.cumbytes  configure -text [dict get $value cumulative bytes]
        
        set options($optname) $value
    }
}
    
