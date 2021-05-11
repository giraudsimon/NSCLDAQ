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
##
# @class QueueStatsView
#     Provides a queue statistics view.  This is primarily a treeview
#     with one line per source id that has a heading/column for each
#     dict key in the statistics.
#
# OPTIONS
#   -queuestats - Queue statistics that comes from the e.g.
#                 EVBRestClient::queuestats
#
# @note the assumption is that source ids, never vanish.
#
snit::widgetadaptor QueueStatsView {
    option -queuestats -configuremethod _cfgQueueStats
    
    #  Array indexed by source id that contains the treeview
    #  source item ids.
    #
    variable SourceItems -array [list]
    
    #
    
    constructor {args} {
        installhull using ttk::frame
        
        set columns [list id depth oldest bytes dequeued queued]
        ttk::treeview $win.tree -yscrollcommand [list $win.vscroll set] \
            -columns $columns -show headings -selectmode none
        ttk::scrollbar $win.vscroll -command [list $win.tree yview] \
            -orient vertical
        
        foreach h [list Id Depth Oldest Bytes Dequeued Queued] c $columns {
            $win.tree heading $c -text $h
        }
        
        grid $win.tree $win.vscroll -sticky nsew
        grid columnconfigure $win 0 -weight 1
        
        
        $self configurelist $args
    }
    #--------------------------------------------------------------------------
    #   _cfgQueueStats
    #   Called when -queuestats is configured to update the view.
    #
    # @param optname - name of the option being configured.
    # @param value   - value which is a list of dicts as might come from
    #                  EVBRestClient::inputstats
    #
    method _cfgQueueStats {optname value} {
        foreach queue $value {
            
            # Extract the stuff from the dict:
            
            set id [dict get $queue id]
            set depth [dict get $queue depth]
            set oldest [dict get $queue oldest]
            set bytes [dict get $queue bytes]
            set dq    [dict get $queue dequeued]
            set tq    [dict get $queue totalqueued]
            
            # Get the entry id (if necessary, making one):
            
            if {[array names SourceItems $id] ne ""} {
                set entry $SourceItems($id)
            } else {
                set entry [$win.tree insert {} end]
                set SourceItems($id) $entry
            }
            
            # Set the values:
            
            $win.tree item $entry -values [list $id $depth $oldest $bytes $dq $tq]
        }
        
        set options($optname) $value
    }
}
    

