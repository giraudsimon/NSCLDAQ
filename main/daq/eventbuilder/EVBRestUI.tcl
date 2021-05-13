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
# @brief Provide User interface components to display EVB statistics
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
        set data [dict get $value queues]
        foreach queue $data {
            
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
##
# @class BarrierStatsView
#    Provides a view for the barrier statistics overview.  This is two lines of
#    labels.  The first label for complete barriers, the second for
#    incomplete barriers.  Each line includes barrier counts, and number of
#    homogeneous/heterogeneous barriers.
#
# OPTIONS
#    -barrierstats  -  A dict that comes from  e.g. EVBRestClient::barrierstats.
#
snit::widgetadaptor BarrierStatsView {
    option -barrierstats -configuremethod _cfgBarrierStats
    
    constructor args {
        installhull using ttk::frame
        
        ttk::label $win.htype -text Type
        ttk::label $win.hcounts -text "Counts"
        ttk::label $win.homogeneous -text "Homogeneous"
        ttk::label $win.heterogeneous -text "Heterogeneous"
        
        ttk::label $win.lcomplete -text "Complete"
        ttk::label $win.ccounts   -text 0
        ttk::label $win.cho       -text 0
        ttk::label $win.che       -text 0
        
        ttk::label $win.lincomplete -text "Incomplete"
        ttk::label $win.icounts   -text 0
        ttk::label $win.iho       -text 0
        ttk::label $win.ihe       -text 0
        
        
        grid $win.htype $win.hcounts $win.homogeneous $win.heterogeneous \
            -sticky w -padx 3
        grid $win.lcomplete $win.ccounts $win.cho $win.che \
            -sticky w -padx 3
        grid $win.lincomplete $win.icounts $win.iho $win.ihe \
            -sticky w -padx 3
        
        $self configurlist $args
    }
    
    #---------------------------------------------------------------------------
    #  Configuration  management
    
    ##
    # _cfgBarrierStats
    #    Accept new barrier statistics.
    #
    # @param optname - name of the option being configured.
    # @param value   - new value.
    #
    method _cfgBarrierStats {optname value} {
        
        # Complete
        
        set complete   [dict get $value complete]
        set incomplete [dict get $value incomplete]
        
        foreach key [list barriers homogeneous heterogeneous]  \
            c [list ccounts cho che]                          \
            i [list icounts iho ihe] {
            
            $win.$c configure -text [dict get $complete $key]
            $win.$i configure -text [dict get $incomplete $key]
        }
        
        set options($optname) $value
    }
}
##
# @class CompleteBarrierView
#    Provides a view of the complete barrier statistics.
#    This consists of a treeview with a pair of top levels:
#    Type and By Source By source has subelements for each source id.
#    The columns are:
#    -   Type - Barrier type.
#    -   Count- Number of times.
#
# OPTIONS
#   -completebarrierdetails - the dict which was gotten from
#                              EVBRestClient::completebarrierdetails.
#
snit::widgetadaptor CompleteBarrierView {
    option -completebarrierdetails -configuremethod _cfgCompleteBarrierDetails
    
    #  These variables keep track of element ids so we can update existing ones
    #  in place which can be done without collapsing the entire tree and without
    #  searching the tree.
    
    variable byType;                    # Tree item for Type top level of tree.
    variable typeEntries -array [list]; # Indexed by barrier type.
    variable bySource;                  # Tree item for by source
    variable sourceIds -array [list];   # Indexed by sid source id elements.
    variable sourceTypes -array [list]; # Indexed by sid.type details entries.
        
    ##
    # constructor -
    #   - Install the hull as a ttk::Frame
    #   - Create the treeview and a vertical scrollbar.
    #   - Create the top levels (byType and bySource) and record their ids
    #     in the appropriate variables.
    #   - process options if supplied (to e.g. create an initial view).
    #
    constructor {args} {
        installhull using ttk::frame
        
        # Create the widgets.
        
        set cols [list type count]
        set headings [list Type Count]
        ttk::treeview $win.tree -yscrollcommand [list $win.vscroll set] \
            -columns $cols -displaycolumns $cols -show [list tree headings] \
            -selectmode none
        
        ttk::scrollbar $win.vscroll -orient vertical \
            -command [list $win.tree yview]
        
        grid $win.tree $win.vscroll -sticky  nsew
        grid columnconfigure $win 0 -weight 1
        
        # Add the tree toplevel items.
        
        foreach c $cols h $headings {
            $win.tree heading  $c -text $h
        }
        set byType [$win.tree insert {} end -text {By Type}]
        set bySource [$win.tree insert {} end -text {By Source}]
        
        #  Process any initial statistics:
        
        $self configurelist $args
        
    }
    #---------------------------------------------------------------------
    #  Private utilities
    
    ##
    # _updateTypeStatistics
    #   Update the By Type statistics.
    # @param stats - bytype statistics from the -completebarrierdetails option.
    #
    method _updateTypeStatistics {stats} {
        # For each type, if a line corresponding to the type does not exist,
        # create it.  Update the counts on that line
        
        set types [list]
        foreach item $stats {
            set type [dict get $item type]
            set count [dict get $item count]
            lappend types $type
            
            if {[array names typeEntries $type] eq ""} {
                # Make one

                set  typeEntries($type) [$win.tree insert $byType end]
            }
            # Set the values:
            
            set entry $typeEntries($type)
            $win.tree item $entry -values [list $type $count]
        }
        # For each existing item if a type does not exist kill it off:
        
        foreach item [array names typeEntries] {
            if {$item ni $types} {
                $win.tree delete $typeEntries($item)
                array unset typeEntries $item
            }
        }
            
        
    }
    
    ##
    # _updateSourceStatistics
    #    Update by source id statistics.
    # @param stats -bysource statistics from the -completebarrierdetails option.
    # @note - while we eliminate sources that disappear we don't take the trouble
    #         to eliminate disappearing types within a source.
    #
    method _updateSourceStatistics {stats} {
        set idlist [list]
        foreach stat $stats {
            set id [dict get $stat id]
            set count [dict get $stat count]
            set details [dict get $stat details]
            lappend idlist $id
            
            # If there's no id element make one:
            
            if {[array names sourceIds $id] eq ""} {
                set sourceIds($id) [$win.tree insert $bySource end -text $id]
            }
            set identry $sourceIds($id)
            $win.tree item $identry -values [list "" $count]
            
            #  Now the types in each source id:
            
            foreach typestat $details {
                set type [dict get $typestat type]
                set count [dict get $typestat count]
                set index $id.$type
                
                # IF necessasry make one:
                
                if {[array names sourceTypes $index] eq ""} {
                    set sourceTypes($index) [$win.tree insert $identry end ]
                }
                set tentry $sourceTypes($index)
                $win.tree item $tentry -values [list $type $count]
            }
            # Eliminate disappearing source ids:
            

        }       
            
        puts " array : list [array names sourceIds] :  $idlist"
        foreach id [array names sourceIds] {
            if {$id ni $idlist} {
                puts "Elminating $id"
                $win.tree delete $sourceIds($id)
                array unset sourceIds $id
                array unset sourceTypes $id.*
            }
        }
    }
    
    # 
    #---------------------------------------------------------------------
    #  Configuration handling.
    
    ##
    # _cfgCompleteBarrierDetails
    #   Process a statistics update.
    #
    # @param optname - option being configured.
    # @param value   - new statistics values.
    #
    method _cfgCompleteBarrierDetails {optname value} {
        set byTypeInfo [dict get $value bytype]
        set bySourceInfo [dict get $value bysource]
        
        $self _updateTypeStatistics $byTypeInfo
        $self _updateSourceStatistics $bySourceInfo
        
        set options($optname) $value
    }
}
    
##
# @class IncompleteBarrierView
#
#  Provides a view of incomplete barrier statistics. The widget is a treeview
#  with top levels:
#  -  Number Missing - showing the number of times a specific number of sources
#                was missing from an incomplete barrier and
#  -  BySource   - Showing the sources missing and how often.
#
#  The only column is 'Count' which shows the number of counts for some event.
#
#  Under Occurences are elements showing the number of missing sources and
#  the count column in each is the number of times that number of sources was
#  missing.
#
#  Under BySource are source ids where counts is the number of times that
#  source id was missing.
#
# OPTION
#   -incompletedetails - a dict from e.g.
#        EVBRestClient::incompletebarrierdetails
#
snit::widgetadaptor IncompleteBarrierView {
    option -incompletedetails -configuremethod _cfgIncompleteDetails
    
    variable byMissing;       # holds id of "Number Missing" top level
    variable byMissingItems -array [list]; # indexed by missing count 
    variable bySourceId;     # Holds top level for by source.
    variable bySourceItems -array [list];  #Indexed by source id.
    
    ##
    # Constructor:
    #   - installs a ttk::frame as the hull.
    #   - Creates the tree view and its vertical scroll bar and lays them out.
    #   - Creates the top level items and saves their ids.
    #   - Configures in case -incompletedetails was supplied at construction time.
    #
    constructor {args} {
        installhull using ttk::frame
        
        #  Create the treeview and its scroll bar and lay them out
        
        ttk::treeview $win.tree -yscrollcommand [list $win.vscroll set] \
            -columns count -displaycolumns count -show [list tree headings] \
            -selectmode none
        $win.tree heading count -text "Count"
        
        ttk::scrollbar $win.vscroll -orient vertical \
            -command [list $win.tree yview]
        grid $win.tree $win.vscroll
        grid columnconfigure $win.tree 0 -weight 1
        
        #  Create the two top level items:
        
        set byMissing  [$win.tree insert {} end -text "By # Missing"]
        set bySourceId [$win.tree insert {} end -text "By Source id"]
        
        #  Configure if needed:
        
        $self configurelist $args
    }
    #---------------------------------------------------------------------
    # Private utilities:
    
    ##
    # _updateMissing
    #    Update the missing source data.  These are the entries subordinate
    #    to $byMissing.  There will be  one for each number of sources missing from
    #    barriers with a count of the number of times that number of sources
    #    was missing.
    # @param data list of dicts with number and count keys.
    #
    #
    method _updateMissing {data} {
        set missingList [list]
        foreach item $data {
            set num [dict get $item number]
            set count [dict get $item count]
            lappend missingList $num
            
            # Figure out the treeview element to set a count for on.
            
            if {[array names byMissingItems $num] eq ""} {
                # Make a new subordinate entry:
                
                set byMissingItems($num) \
                    [$win.tree insert $byMissing end -text $num]
            }
            $win.tree item $byMissingItems($num) -values [list $count]
        }
        # Expunge entries that disappered:
        
        foreach index [array names byMissingItems] {
            if {$index ni $missingList} {
                $win.tree delete $byMissingItems($index)
                array unset byMissingItems $index
            }
        }
    }
    ##
    # _updateBySource
    #     Update the by source id part of the tree.
    # @param data - list of dicts with keys id (sourceid) and count.
    #
    method _updateBySource {data} {
        set sourceList [list]
        foreach item $data {
            set source [dict get $item id]
            set count [dict get $item count]
            lappend sourceList $source
            
            if {[array names bySourceItems $source] eq ""} {
                set bySourceItems($source) \
                    [$win.tree insert $bySourceId end -text $source]
            }
            $win.tree item $bySourceItems($source) -values [list $count]
        }
        #  Kill sids no longer here.
        
        foreach sid [array names bySourceItems] {
            if {$sid ni $sourceList} {
                $win.tree delete $bySourceItems($sid)
                array unset bySourceItems $sid
            }
        }
    }
    
    #--------------------------------------------------------------------
    # Configuration management.
    #
    
    ##
    # _cfgIncompleteDetails
    #   Configure the -incompletedetails option.
    #
    # @param optname - Name of option being configured
    # @param value   - New proposed value.
    #
    method _cfgIncompleteDetails {optname value} {
        set missing [dict get $value histogram]
        set source  [dict get $value bysource]
        
        $self _updateMissing $missing
        $self _updateBySource $source
        
        set options($optname) $value
    }
    
}
##
# @class DataLateView
#   Provides a view of data late statistics.  This consists of a tree view
#   with the following  top levels:
#    - Totals - total count and worst case timestamp.
#    -  One item for each source id that has the source id label and
#        count/worst case columns as well.
# OPTIONS
#   -datalatestatistics - Dict from e.g. EVBRestClient::datalatestatistics.
#
snit::widgetadaptor DataLateView {
    option -datalatestatistics -configuremethod _cfgDataLateStatistics
    
    variable totalItemId;                 # Totals item id.
    variable sourceItems -array [list];   # Indexed by sourceid (tree element ids)
    
    ##
    # constructor:
    #    -  Use a ttk::frame as a hull.
    #    -  make the treeview and vertical scrollbar with count and worst
    #       columns and lay them out.
    #    - Add a Totals item to the tree.
    #    - Configure any options.
    #
    constructor {args} {
        installhull using ttk::frame
        
        #  Create the tree view, scroll bar and lay them out.
    
        set columns [list count worst]
        set titles [list "Count" "Worst dt"]
        ttk::treeview $win.tree -yscrollcommand [list $win.vscroll set] \
            -columns $columns -displaycolumns $columns -show [list tree headings] \
            -selectmode none
        foreach c $columns h $titles {
            $win.tree heading $c -text $h
        }
        ttk::scrollbar $win.vscroll -command [list $win.tree yview]   \
            -orient vertical
        
        grid $win.tree $win.vscroll
        grid columnconfigure $win 0 -weight 1
        
        #  Create the totals line.
        
        set totalItemId [$win.tree insert {} end -text Totals -values [list 0 0]]
        
        # Configure any options
        
        $self configurelist $args
    }
    #-----------------------------------------------------------------------
    #  Configuration management.
    
    ##
    # _cfgDataLateStatistics
    #   New statistics - results in a display update.
    #
    # @param optname - Name of the option being updated.
    # @param value   - New value.
    #
    method _cfgDataLateStatistics {optname value} {
        set tCount [dict get $value count]
        set tWorst [dict get $value worst]
        $win.tree item $totalItemId -values [list $tCount $tWorst]
        
        # Update/add new source id lines
        
        set sids [list]
        foreach item [dict get $value details] {
            set sid [dict get $item id]
            set count [dict get $item count]
            set worst [dict get $item worst]
            lappend sids $sid
            
            if {[array names sourceItems $sid] eq ""} {
                set sourceItems($sid) [$win.tree insert {} end -text $sid]
            }
            $win.tree item $sourceItems($sid) -values [list $count $worst]
        }
        
        # delete any removed source ids.
        
        foreach id [array names sourceItems] {
            if {$id ni $sids} {
                $win.tree delete $sourceItems($id)
                array unset sourceItems $id
            }
        }
        
        # Update the option value.
        
        set options($optname) $value
    }
}
##
# @class OutOfOrderView
#    
#   Provides a view of out of order statistics.  This is organized
#   in a manner similar to the data late stats. Top levels are summary
#   and items for each id.   The columns are count, prior and offending.
#   Note that we will format all timestamps in hexadecimal with 64 bits
#   (leading zeros).
# OPTIONS
#   -oostatistics  - statistics information from e.g. EVBRestClient::ooStatistics
snit::widgetadaptor OutOfOrderView {
    option -oostatistics -configuremethod _cfgOOStatistics
    
    variable SummaryItem;             # Summary treeview entry.
    variable SourceItems -array [list]; # Per source items.
    
    
    ##
    # constructor
    #  - install ttk::frame as hull.
    #  - Install/layout treeview and v scrollbar.
    #  - Add the summary top level.
    #  - Configure any options.
    #
    constructor {args} {
        installhull using ttk::frame
        
        #  Build the GUIs

        set columns [list count prior offending]
        set headings [list "Count" "Last Good TS" "Offending TS"]
        ttk::treeview $win.tree -yscrollcommand [list $win.vscroll set] \
            -columns $columns -displaycolumns $columns -show [list tree headings] \
            -selectmode none
        foreach c $columns h headings {
            $win.tree heading $c -text $h
        }
        
        ttk::scrollbar $win.vscroll -command [list $win.tree set] \
            -orient vertical
        
        grid $win.tree $win.vscroll -sticky nsew
        grid columnconfigure $win 0 -weight 1
        
        
        #  Add summary top level.
        
        set SummaryItem [$win.tree insert {} end -text "Summary"]
        
        # configure options

        $self configurelist $args
    }
    #------------------------------------------------------------------------
    #  Configuration management.
    
    ##
    # _cfgOOStatistics
    #     Refresht the display from new data.
    #
    # @param optname - option name
    # @param value   - option value
    #
    method _cfgOOStatistics {optname value} {
        set summary [dict get $value summary]
        set counts [dict get $summary count]
        set prior  [format %016x [dict get $summary prior]]
        set bad    [format %016x [dict get $summary offending]]
        $win.tree item $SummaryItem -value [list $counts $prior $bad]
        
        # Add update ids.
        
        set idList [list]
        foreach item [dict get $value bysource] {
            set id [dict get $item id]
            set counts [dict get $item count]
            set prior [format %016x [dict get $item prior]]
            set bad  [format %016x [dict get $item offending]]
            lappend idList $id
            
            if {[array names SourceItems $id] eq ""} {
                set SourceItems($id) [$win.tree insert {} end -text $id]
            }
            $win.tree item $SourceItems($id) -values [list $counts $prior $bad]
        }
        
        # Kill off ids that disappered - that can happen if e.g. the orderer
        # restarted between updates.
        
        foreach index [array names SourceItems] {
            if {$index ni $idList} {
                $win.tree delete $SourceItems($index)
                array unset SourceItems $index
            }
        }
        
        set options($optname) $value
    }
}
##
# @class ConnectionView
#    Provides a megawidget that displays connection status.
#    This is a treeview used as a table with columns:
#    -   host - host a connection comes from.
#    -   description - textual description of the data source.
#    -   state - Connection state string.
#    -   idle  - if connection hasn't been contributing data.
#
# OPTIONS:
#   - -connections - connection from e.g. EVBRestClient::connections.
#
snit::widgetadaptor ConnectionView {
    option -connections -configuremethod _cfgConnections
     
     # note that each time we must refresh the whole table as we don't have
     # any good thing to hang this on (e.g. sources can have several ids).
     
     constructor {args} {
        installhull using ttk::frame
        
        set columns [list host description state idle]
        set headings [list Host "Connection Description"  State "Is Idle?"]
        ttk::treeview $win.tree -yscrollcommand [list $win.vscroll set] \
            -columns $columns -displaycolumns $columns -show headings \
            -selectmode none
        foreach c $columns h $headings {
            $win.tree heading $c -text $h
        }
        
        ttk::scrollbar $win.vscroll -command [list $win.tree yview] \
            -orient vertical
        
        grid $win.tree $win.vscroll -sticky nsew
        grid columnconfigure $win 0 -weight 1
        
        $self configurelist $args
     }
    #------------------------------------------------------------------------
    # Private utilities:
    
    ##
    # _clear
    #   Clear all the lines from the display.#
    #
    method _clear {} {
        foreach item [$win.tree children {}] {
            $win.tree delete $item
        }
    }
    ##
    # _addConnection
    #    Adds a connection to the display.
    #
    # @param connection - a dict containing the connection information.
    #
    method _addConnection {connection} {
        set host [dict get $connection host]
        set desc [dict get $connection description]
        set state [dict get $connection state]
        set idle [dict get $connection idle]
        if {$idle} {
            set idle idle
        } else {
            set idle ""
        }
        
        $win.tree insert {} end -values [list $host $desc $state $idle]
        
    }
    
    #------------------------------------------------------------------------
    # Configuration management
    
    ##
    # _cfgConnections
    #    Called to refresh the display due to a new set of connections.
    #
    # @param optname - option being configured.
    # @param value   - new value - this is a list of dicts of the form
    #                  returned by EVBRestClient::connections.
    #
    method _cfgConnections {optname value} {
        $self _clear
        foreach connection $value {
            $self _addConnection $connection
        }
        
        set options($optname) $value
    }
}
    
##
# @class FlowControlView
#    Very simple view for flow control state.
#
# OPTIONS:
#   -flowcontrol -boolean that's true if flow control is asserted and false otherwise.
#
snit::widgetadaptor FlowControlView {
    option -flowcontrol -configuremethod _cfgFlowControl
    
    constructor {args} {
        installhull using ttk::frame
        ttk::label $win.label -text "Flow Control: "
        ttk::label $win.state -text "-unknown-"
        
        grid $win.label $win.state -sticky nsew
    }
    #-------------------------------------------------------------------------
    #  Configuration management.
    
    ##
    # _cfgFlowControl
    #    Update the $win.state with meaningful text given a new boolean value for
    #    the flow control
    #
    # @param optname - name of the option being configured.
    # @param value   - new value.
    #
    method _cfgFlowControl {optname value} {
        if {$value} {
            $win.state configure -text "active"
        } else {
            $win.state configure -text "  off  "
        }
        
        set options($optname) $value
    }
}



    


