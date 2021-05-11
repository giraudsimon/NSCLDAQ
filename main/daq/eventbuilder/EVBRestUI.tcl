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
# @file EVBRestUI.tcl
# @brief Provide User interface components and integrated test of EVB statistics
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}

package provide EVBRestUI 1.0
package require Tk
package require snit


##
# @class InputStatsView
#
#  Provides a view of input statistics. This is just a strip of labels
#
# OPTIONS
#    -inputstats - input statistics dict as it comes from e.g.
#                  EVBRestClient::inputstats
# 
snit::widgetadaptor InputStatsView {
    option -inputstats -configuremethod _cfgInputStats
    
    constructor args {
        installhull using ttk::frame
        
        ttk::label $win.oldestl -text "Oldest: "
        ttk::label $win.oldest  -text 0
        
        ttk::label $win.newestl -text "Newest: "
        ttk::label $win.newest  -text "0"
        
        ttk::label $win.fragmentsl -text "Fragments: "
        ttk::label $win.fragments -text "0"
        
        grid $win.oldestl $win.oldest \
            $win.newestl $win.newest  \
            $win.fragmentsl $win.fragments \
            -sticky nsew -padx 3
        
        
        $self configurelist $args
    }
    #--------------------------------------------------------------------------
    # Configuration management
    
    ##
    # _cfgInputStats
    #    Respond to configure -inputstats data
    #     Pulls the pieces out of the dict and sets the appropriate
    #     label -text
    # @param optname - name of the option.
    # @param value   - Value.
    #
    method _cfgInputStats {optname value} {
        foreach key [list oldest newest fragments] \
            w [list $win.oldest $win.newest $win.fragments] {
            $w configure -text [dict get $value $key]
        }
        
        set options($optname) $value
    }
}
