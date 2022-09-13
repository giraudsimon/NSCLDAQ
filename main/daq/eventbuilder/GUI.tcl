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
#        

##
# @file GUI.tcl
# @brief Provides the top level GUI widgets and other related stuff.

#  Add here to the package include path if it's not there already:

set here [file dirname [info script]]
if {[lsearch $::auto_path $here] == -1} {
    lappend ::auto_path $here
}

# Package provides and requires:

package provide EVB::GUI 1.0
package require Tk
package require snit
package require ring


package require EVB::connectionList
#package require ring

# Establish the EVB namespace into which we're going to squeeze:

namespace eval ::EVB {
    
}
#----------------------------------------------------------------------------
#
#  Common utility procs: daqdev/NSCLDAQ#700
#



##
# statusBar
#   contains a pair of text widgets. The first one is a label
#   "Flow control"  the second widget is a label that either contains
#    "Active" or "Accepting Data" depending onthe state of the flow control.
#
snit::widgetadaptor EVB::StatusBar {
    component state
    
    variable flowstate ""
    constructor args {
        installhull using ttk::frame
        
        ttk::label $win.flowlabel -text  "Flow Control: "
        install state using ttk::label $win.flowstate \
            -textvariable [myvar flowstate]
        
        grid $win.flowlabel -sticky w
        grid $win.flowstate -row 0 -column 1 -sticky e
        
        EVB::onflow add [mymethod _xon] [mymethod _xoff]
        $self _xon junk;     # Assume we start up xon'ed.
    }
    destructor {
        EVB::onflow remove [mymethod _xon] [mymethod _xoff]
    }
    ##
    # _xon
    #   Called when the event builder XON's its clients.
    #
    method _xon {args} {
        set flowstate "Accepting Data"
    }
    ##
    # -xoff
    #   Called when the event builder XOFF"s its clients.
    #
    method _xoff {args} {
        set flowstate "Flow Control Active"
    }
}

##
# EVB::statistics
#   Displays the event builder statistics in a relatively
#   Compact tabular form.  The table is actually a treeview
#   and some of the elements can be expanded.
#   Here are the elements and what expansion gives:
#
#   Input statistics  Below this (note children but next items) are:
#   -  A line of column headers queued fragments, oldest timestamp, newest timestamp, deepest queue, inflight frags
#   -  Hierarchically below this are the same information for each data source
#     
snit::widgetadaptor EVB::statistics {
    component table
    
    variable inputStatsId
    variable outputStatsId
    variable queueStatsId
    variable completeBarriersId
    variable incompleteBarriersId
    variable errorsId
    variable dataLateId
    variable outOfOrderId
    variable ringStats
    
    variable afterId -1
    constructor args {
        installhull using ttk::frame
        install table using ttk::treeview $win.table \
            -show tree -columns {col1 col2 col3 col4} \
            -displaycolumns #all -yscrollcommand [list $win.vscroll set]
        
        $table column #1 -stretch off -width [expr 8*15]
        $table column #2 -stretch off -width [expr 10*15]
        $table column #3 -stretch on -minwidth [expr 10*15]
        $table column #4 -stretch off   -width [expr 10*15]
        scrollbar $win.vscroll -orient vertical -command [list $table yview]
        grid $table $win.vscroll -sticky nsw
        
        grid rowconfigure $win 0 -weight 1
        grid columnconfigure $win 0 -weight 1
        grid columnconfigure $win 1 -weight 0
        
        $self _addInputStats
        $self _addOutputStats
        $self _addRingStats
        $self _addQueueStats
        $self _addBarrierStats
        $self _addErrorStats
        
        $self _refresh
        
        #  Create and configure tags that will be used to highlight
        #  Errors e.g.
        
        $table tag configure RED -foreground red
    }
    destructor {
        after cancel $afterId 
    }
    #------------------------------------------------------------
    # Private methods.
    ##
    # _refresh
    #   Refresh the display elements.
    #
    method _refresh {} {
        $self _refreshInputStats
        $self _refreshOutputStats
        $self _refreshQueueStats
        $self _refreshBarrierStats
        $self _refreshRingStats
        
        # Initialize the tags on the top level stuff so that
        # they're not red. If there are any counter changes,
        # the tag RED will be added to highligh the errors.
        # So intermittent errors will give a 2 second red flash
        # while persistent errors will keep the items red.
        
        foreach item [list $errorsId $dataLateId $outOfOrderId] {
            $table item $item -tags [list]
        }
        
        $self _refreshDataLateStats
        $self _refreshOutOfOrderStats
        
        set afterId [after 2000 $self _refresh]
        
    }
    #-----------------------------------------------------
    #  input statistics methods.
    
    ##
    # _addInputStats
    #   *  Add a line labeled input statistics and column labels
    #      for the input statistics data
    #   *  Add the initial line of statistics
    #   *  Save the id of the statistics in inputStatsId
    #      as _refresh will create entries for each source id
    #      hierarchically below it.
    #
    method _addInputStats {} {
        $table insert {} end -text {Input Stats.} \
            -values [list {qd frags} {oldest ts} {newest ts} {deepest/total}]
        set inputStatsId [$table insert {} end -text {All} -values [lrepeat 4 0]]
    }
    ##
    # _refreshInputStats
    #   Get the input stats from the event builder and
    #   update:
    #   *  The statistics line in $inputStatsId
    #   *  Any existing children.
    #   *  Create any children not yet in existence.
    method _refreshInputStats {} {
        set stats [EVB::inputStats]
        set questats [lindex $stats 3]
        set deepest [_findDeepestInputQ $questats]
        $table item $inputStatsId -values [list      \
            [lindex $stats 4] [format 0x%08x [lindex $stats 0]]   \
            [format 0x%08x [lindex $stats 1]]                    \
            [lindex $stats 2] $deepest                            \
        ]
        # Make a dict with keys source ids and values the
        # ids of elements that represent the source statistics value
        # for the that sid.
        set sourceDict [dict create]
        set sourceItems [$table children $inputStatsId]
        foreach item $sourceItems {
            set sid [$table item $item -text]
            dict append sourceDict $sid $item
        }
        $self _refreshPerQInputStats $sourceDict $questats
    }
    ##
    # _refreshPerQInputStats
    #    Refresh the per queue input statistics.  If  a q
    #    already has an entry it will be in the dict. if not
    #    a new entry is cretaed below inputStatsId.
    # @param sdict - Dict with source id keys and values the
    #              id of an element with input stats for that sid.
    # @param stats - Per queue statistics id, depth bytes, oldest, total.
    #
    method _refreshPerQInputStats {sdict stats} {
        foreach stat $stats {
            set sid [lindex $stat 0]
            if {[dict exists $sdict $sid]} {
                set id [dict get $sdict $sid]
            } else {
                set id [$table insert $inputStatsId end -text $sid]
            }
            set depth [lindex $stat 1]
            set oldest [format 0x%8x [lindex $stat 2]]
            set bytes [expr {[lindex $stat 3]/1024}]kB
            set dequed [expr {[lindex $stat 4]/1024}]kB
            set total [expr {[lindex $stat 5]/1024}]kB
            $table item $id -values [list                   \
                $depth:$bytes $oldest "" $total            \
            ]
            
        }
    }
    ##
    # _findDeepestInputQ
    #   Given per queue statistics find the deepest queue.
    # @param info - list of queue information.
    # @return string of the form queue : depth
    # @retval -1:-1 if the info contains no elements.
    #
    proc _findDeepestInputQ {info} {
        set qid -1
        set max -1
        foreach q $info {
            set id [lindex $info 0]
            set depth [lindex $info 1]
            if {$depth > $max} {
                set qid $id
                set max $depth
            }
        }
        return "$qid:$max"
    }
    #-----------------------------------------------------
    #  Output statistics
    
    ##
    # _addOutputStats
    #   Adds the output statistics top level entry to the
    #   statistics page.  The output statistics consist of
    #   label line that has column headings and
    #   a total stat line with children for each known source id.
    #   the only statistic (header) is the total fragments output.
    #
    method _addOutputStats {} {
        $table insert {} end -text {Output Statistics} -values {Fragments}
        set outputStatsId [$table insert {} end -text {Total frags} -values 0]
    }
    ##
    # _refreshOutputStats
    #   Called to refresh the values on the output statistics
    #   from the event builder.
    #
    method _refreshOutputStats {} {
        set stats [EVB::outputStats get]
        $table item $outputStatsId -values [lindex $stats 0]
        set perQ [lindex $stats 1]
        set sdict [dict create]
        foreach child [$table children $outputStatsId] {
            set sid [$table item $child -text]
            dict append sdict $sid $child
        }
        foreach info $perQ {
            set sid [lindex $info 0]
            set stat [lindex $info 1]
            if {[dict exists $sdict $sid]} {
                set id [dict get $sdict $sid]
            } else {
                set id [$table insert $outputStatsId end -text $sid]
            }
            $table item $id -values $stat
        }
    }
    #-------------------------------------------------------
    # queue statistics
    
    # _addQueueStats
    #   Queue statistics are a combination of input and output
    #   statistics for a single queue
    #   
    #   The headings are
    #     depth - Depth of input queue.
    #     oldest ts 
    #     newest ts 
    #     outfrag:bytes
    method _addQueueStats {} {
        set queueStatsId [$table insert {} end -text {Q stats} \
            -values [list depth oldest qdBytes outbytes]]
    }
    ##
    # _refreshQueueStats
    #  - Combine the per queue input/output statistics.
    #  - Make a dict of sources for which we already have entries
    #  - Foreach statistics entry, if we don't yet have an entry make one
    #  - Update the values.
    #
    method _refreshQueueStats {} {
        
        #  Make a dict of sid -> table entries (known now)
        
        set children [$table children $queueStatsId]
        set existsDict [dict create]
        foreach child $children {
            set sid [$table item $child -text]
            dict append existsDict $sid $child
        }
        # Get and combine the input/output statistics and combine
        
        set inputStats [lindex [EVB::inputStats] 3];   # just the queue
        
        set statistics [_makeQstats $inputStats]
        
        #  Now we have a dict that's keyed by source id and
        #  has combined statistics (see _makeQstats below).
        
        dict for {sid value} $statistics {
            if {[dict exists $existsDict $sid]} {
                set id [dict get $existsDict $sid]
            } else {
                # Need to make an  new one:
                
                set id [$table insert $queueStatsId end -text $sid]
            }
            # Now set the values for entry $id:
            # value is a dict with the values for the statistics.
        
            $table item $id -values [list \
               [dict get $value depth] [dict get $value oldest] \
               [dict get $value received]K [dict get $value sent]K \
            ]
        }
    }
    ##
    # _makeQstats
    #   Given per-queue input  stats,
    #   creates and returns a dict of combined stats.
    #
    # @param inputStats - list of per-queue input statistics.
    # @return dict - keys are source ids, values are dicts with stats
    #               that have the following keys:
    #               - depth - current queue depth.
    #               - oldest - oldest timestamp in the queue.
    #               - received - # Kbytes queued.
    #               - sent - #Kbytes sent.
    #
    # 
    proc _makeQstats {inputStats} {
        set result [dict create]
        
        #  Loop over input stats...
        #  We assume, with good reason, sid's are unique.
        #
        foreach stat $inputStats {
            set sid [lindex $stat 0]
            set depth [lindex $stat 1]
            set oldest [format 0x%08x [lindex $stat 2]]
            set recvd  [expr {[lindex $stat 3]/1024}]
            set sent   [expr {[lindex $stat 4]/1024}]
            
            dict append result $sid [dict create               \
                depth $depth oldest $oldest received $recvd sent $sent \
            ]
        }
        
        return $result
    }
    #-----------------------------------------------------------
    #   Barrier statistics
    #
    
    ##
    # _addBarrierStats
    #   Put int top level elements for incomplete and complete
    #   barriers.
    #
    method _addBarrierStats {} {
        set completeBarriersId [$table insert {} end -text {Complete Barriers} \
            -values [list {Number: 0} {Homogeneous: 0} {Heterogeneous: 0} ] \
        ]
        set incompleteBarriersId [$table insert {} end -text {Incomplete Barriers} \
            -values [list {Number: 0} {Homogeneous: 0} {Heterogeneous: 0}]
        ]
    }
    ##
    # _refreshBarrierStats
    #   Refreshes the barrier stats -- this can be quite
    #   complex.  More in the comments below.
    #
    method _refreshBarrierStats {} {
        set stats [EVB::barrierstats]
        set complete [lindex $stats 0]
        set incomplete [lindex $stats 1]
        
        $self _refreshCompleteBarriers $complete
        $self _refreshIncompleteBarriers $incomplete
    }
    ##
    # _refreshCompleteBarriers
    #   Refreshes the UI for complete barriers:
    #   -  Updates the statistics in the  completeBarriersId entry
    #   -  Updates or add "Type n" entries with the number of barriers
    #      of that type seen.
    #   -  For each source Id we have a line "Source sid" with
    #      sub lines for fragment type:  and Number:
    # @param stats - The comlete part of the barrier statistics.
    #
    method _refreshCompleteBarriers {stats} {
        set completecount [lindex $stats 0]
        set homogeneous  [lindex $stats 1]
        set heterog      [lindex $stats 2]
        
        $table item $completeBarriersId \
            -values [list "Number: $completecount"            \
                "Homogeneous: $homogeneous"                   \
                "Heterogeneous: $heterog"
            ]
        
        set typeinfo    [lindex $stats 3]
        $self _refreshCompleteBarrierTypes $typeinfo
        
        set sourceinfo  [lindex $stats 4]
        $self _refreshCompleteBarrierSources $sourceinfo
    }
    ##
    # _refreshCompleteBarrierTypes
    #   Given  a list of pairs containing the barrier type
    #   and number of times that barrier has been seen, updates
    #   that part of the complete barrier statistics,
    #   $comleteBarriers has children.  Ones with names
    #   like "Type: $n"  represent statistics for barrier type n.
    #
    # @param typeinfo - the list of type/count pairs.
    #
    method _refreshCompleteBarrierTypes {typeinfo} {
        
        #  Make dict of existing barrier type childeren:
        
        set typeChildren [dict create]
        foreach child [$table children $completeBarriersId] {
            set text [$table item $child -text]
            if {[lindex $text 0] eq "Type:"} {
                set id [lindex $text 1]
                dict append typeChildren $id $child
            }
        }
        #  Now loop over the typeinfo list, if necessary we make
        #  a new child to $completeBarriersId  These are made at the
        #  top of the children (sources at the bottom).
        #
        foreach info $typeinfo {
            set type [lindex $info 0]
            set count [lindex $info 1]
            if {[dict exists $typeChildren $type]} {
                set id [dict get $typeChildren $type]
            } else {
                set id [$table insert $completeBarriersId 0 \
                    -text "Type: $type"
                ]
            }
            $table item $id -values "Occured: $count"
        }
    }
    ##
    # _refreshCompleteBarrierSources
    #   Refreshes the statistics for per source barrier information.
    #   Each source will have a single item below completeBarriersId
    #   Under that source will be children containing the
    #   barrier type and count of number of times that barrier type
    #   has been seen by that source.
    #
    # @param stats - Statistics consisting of a list of pairs.
    #                The first element of each item is a source id.
    #                The second is a pair containing the barrier
    #                type and number of times that type has been
    #                received by that source.
    #
    method _refreshCompleteBarrierSources {stats} {
        
        #  First make a dict of source children to the complete barrier
        #  top level  The dict has source id keys and
        #  item id values.
        
        set sourceChildren [dict create]
        foreach child [$table children $completeBarriersId] {
            set text [$table item $child -text]
            if {[lindex $text 0] eq "Sid:"} {
                set sid [lindex $text 1]
                dict append sourceChildren $sid $child
            }
        }
        #  Now loop over the source statistics.
        #  ANd update the stats for each single source
        
        foreach info $stats {
            set sid [lindex $info 0]
            if {[dict exists $sourceChildren $sid]} {
                set id [dict get $sourceChildren $sid]
            } else {
                set id [$table insert $completeBarriersId end \
                        -text "Sid: $sid"                     \
                ]
            }
            $self _refreshBarrierSource $id [lindex $info 1]
        }
    }
    ##
    # _refreshBarrierSource
    #   Refresh the barrier statistics for one source.
    # @param id - item id of the source.
    # @param stats - list of pairs of barrier type/count
    #
    method _refreshBarrierSource {id stats} {
        
        # Make the usual dict of children.  In this
        # case the children represent barrier types:
        
        set typeChildren [dict create]
        foreach child [$table children $id] {
            set type [lindex [$table item $child -text] 1]
            dict append typeChildren $type $child
        }
        #  Now update the statistics creating new entries
        #  as needed.
        
        foreach stat $stats {
            set type [lindex $stat 0]
            set count [lindex $stat 1]
            if {[dict exists $typeChildren $type]} {
                set cid [dict get $typeChildren $type]
            } else {
                set cid [$table insert $id end -text "Type: $type"]
            }
            $table item $cid -values $count
        }
        
    }
    ##
    # _refreshIncompleteBarriers
    #   Refreshes the incomplete barrier subtree.
    #
    # @param stats - the incomplete barrier statistics. A list
    #                consisting of:
    #               -  number of incomplete barriers.
    #               -  Number of homogeneous incompletes.
    #               -  Number of heterogeneous incompletes.
    #               -  Missing source count histogram.
    #               -  Times each source was missed.
    #
    method  _refreshIncompleteBarriers stats {
        
        #  Update the top level stats then invoke updates for the histogram
        #  and the source counts:
        
        $table item $incompleteBarriersId -values [list \
            "Number: [lindex $stats 0]"                 \
            "Homogeneous: [lindex $stats 1]"            \
            "Heterogeneous: [lindex $stats 2]"          \
        ]
        
        set numMissingHisto [lindex $stats 3]
        $self _updateMissingHistogram $numMissingHisto
        set missingSources [lindex $stats 4]
        $self _updateMissingSources $missingSources
    }
    ##
    # _updateMissingHistogram
    #    Updates the statistics of the frequency missing sources
    #
    # @param stats - list of pairs of # frags missing , number of times.
    #
    method _updateMissingHistogram {stats} {
        
        set histoDict [dict create]
        foreach child [$table children $incompleteBarriersId] {
            set text [$table item $child -text]
            if {[lindex $text 0] eq "Missed"} {
                dict append histoDict [lindex $text 1] $child
            }
        }
        foreach stat $stats {
            set missing [lindex $stat 0]
            set times   [lindex $stat 1]
            
            if {[dict exist $histoDict $missing]} {
                set id [dict get $histoDict $missing]
            } else {
                set id [$table insert $incompleteBarriersId end \
                    -text "Missed $missing sources"]
            }
            $table item $id -values [list "$times times"]
        }
            
        
    }
    ##
    # _updateMissingSources
    #   Updates the missing sources part of the incomplete barriers
    #   display.  This is the same as the histo but
    #   for sources not present in incomplete barriers.
    #
    # @param stats - list of pairs, first element of a pair is the
    #                source id, second number of times the source
    #                was missing at barrier timeout.
    method _updateMissingSources stats {
        #
        #  Once more make the dict of existing lines in this
        #  case lines look like:
        #    "source id" "missing n times"
        #
        
        set missingDict [dict create]
        foreach child [$table children $incompleteBarriersId] {
            set text [$table item $child -text]
            if {[lindex $text 0] eq "Source"} {
                dict append missingDict [lindex $text 1] $child
            }
        }
        #  Now iterate through the stats updating and creating
        # as needed.
        
        foreach stat $stats {
            set sid [lindex $stat 0]
            set count [lindex $stat 1]
            if {[dict exists $missingDict $sid]} {
                set id [dict get $missingDict $sid]
            } else {
                set id [$table insert $incompleteBarriersId end \
                    -text "Source $sid"                \
                ]
            }
            $table item $id -values [list "$count times"]
        }
    }
    #-----------------------------------------------------
    #  Error counters.
    
    ##
    # _addErrorStats
    #   Adds a line "Errors" with the various error statistics
    #   children below it.
    #
    method _addErrorStats {} {
        set errorsId [$table insert {} end -text {Errors}]
        set dataLateId [$table insert $errorsId end -text {Data Late}]
        set outOfOrderId [$table insert $errorsId end -text {Out of order}]
    }
    ##
    #  _refreshDataLateStats
    #
    #  Refresh the data late statistics from the EVB::dlatestats
    #
    method  _refreshDataLateStats {} {
        
        set stats [EVB::dlatestats]
        
        # Update the top line with data late count and worst case
        # timestamp diff.
        
        set lates [lindex $stats  0]
        set worst [format 0x%08x [lindex $stats 1]]
        
        set values [list    \
            "Count: $lates" "Worst dt: $worst"
        ]
        #
        #  If there were changes in the statistics,
        #  set the appropriate items red.
        #  Note we don't need to do this in the per source
        #  statistics because totals can only change if a source
        #  changes.
        #
        if {$values ne [$table item $dataLateId -values]} {
            $table item $dataLateId -tags RED
            $table item $errorsId   -tags RED
        }
        $table item $dataLateId -values $values
        
        set perSource [lindex $stats 2]
        $self _refreshPerSourceLates $perSource
    }
    ##
    # _refreshPerSourceLates
    #   Refresh the data lates per source. These will be lines
    #   subordinate to  $dataLateId containing
    #   "Source: n"  "Count: m"  "Worst dt: dt"
    #
    # @param stats - per source statistics consisting of a list of
    #                triples where each triple contains
    #                - source id.
    #                - Number of times that source was late.
    #                - Worst case time difference between
    #                  late fragment and last one emitted.
    #
    method _refreshPerSourceLates stats {
        
        #  Make the usual sid keyed dict of existing line ids:
        
        set sidDict [dict create]
        foreach child [$table children $dataLateId] {
            set text [$table item $child -text]
            set sid [lindex $text 1]
            dict append sidDict $sid $child
        }
        # Iterate over the statistics:
        
        foreach stat $stats {
            set sid     [lindex $stat 0]
            set count   [lindex $stat 1]
            set worstdt [format 0x%08x [lindex $stat 2]]
            if {[dict exists $sidDict $sid]} {
                set id [dict get $sidDict $sid]
            } else {
                set id [$table insert $dataLateId end -text "Source: $sid"]
            }
            $table item $id -values [list                 \
                "Count: $count" "Worst dt: $worstdt"      \
            ]
        }
    }
    ##
    # _refreshOutOfOrderStats
    #   Refresh the data out of order stats.  The top line has
    #   values like:
    #      "Count: n"  "prior TS  x"  "Bad TS y"
    #
    #  Note that the timestamps are in hex.
    #
    method _refreshOutOfOrderStats {} {
        set stats [EVB::getoostats]
        set totals [lindex $stats 0]
        set persrc [lindex $stats 1]
        #
        # Once more since these values are totals we don't
        # need to look at the per source statistics since
        # they create the totals.
        #
        set values [makeOOValueList $totals]
        if {$values ne [$table item $outOfOrderId -values]} {
            $table item $outOfOrderId -tags RED
            $table item $errorsId -tags RED
        }
        $table item $outOfOrderId -values $values
        
        #  Now the per source items done in the usual way:
        #  The text fields for children have the form "Source: id"
       
        set srcDict [dict create]
        foreach child [$table children $outOfOrderId] {
            set text [$table item $child -text]
            set srcid [lindex $text 1]
            dict append srcDict $srcid $child
        }
        
        foreach stat $persrc {
            set srcid [lindex $stat 0]
            set values [makeOOValueList [lrange $stat 1 end]]
            
            if {[dict exists $srcDict $srcid]} {
                set id [dict get $srcDict $srcid]
            } else {
                set id [$table insert $outOfOrderId end -text "Source: $srcid"]
            }
            $table item $id -value $values
        }
    }
    proc makeOOValueList {stats} {
        set count [lindex $stats 0]
        set lastTs [format 0x%08x [lindex $stats 1]]
        set badTs  [format 0x%08x [lindex $stats 2]]
        
        set result [list "Count: $count" "Prior TS $lastTs" "Bad TS $badTs"]
        return $result
    }
    #-------------------------------------------------------
    # Output Ring buffer statistics.
    # NOTE:  ::OutputRing contains the name of the ringbuffer.
    
    ##
    # _addRingStats
    #
    #  The top line looks like:
    #    "Ring: ringname" "Size: kbytes" "Backlog kbytes" "Free: kbytes"
    #  Below this will be lines for each consumer of the form:
    #    "PID: pid"                      "Backlog kbytes"
    #
    method _addRingStats {} {
        set ringStats [$table insert {} end -text "Ring: $::OutputRing"]
        
    }
    ##
    # _refreshRingStats
    #   Called to refresh the ring statistics.  See _addRingStats
    #   for what this looks like.  We use the ringbuffer command
    #   from the ring package to get the information about the
    #   usage of the ::OutputRing
    #
    method _refreshRingStats {} {
        set statistics [ringbuffer usage $::OutputRing]
        
        # Set the top level statistics:
        
        set size    [expr {[lindex $statistics 0]/1024}]KB
        set backlog [expr {[lindex $statistics 4]/1024}]KB
        set free    [expr {[lindex $statistics 1]/1024}]KB
        
        $table item $ringStats -values [list  \
            "Sizes: $size" "Backlog: $backlog" "Free: $free" \
        ]
        
        # Build the usual dict of, in this case consumer PID->element ids:
        
        set consumerDict [dict create]
        foreach child [$table children $ringStats] {
            set pid [lindex [$table item $child -text] 1]
            dict append consumerDict $pid $child
        }
        #  Now update or add lines as needed.
        #  We're also going to make a list of consumer PIDS to prune lines
            
        set consumerList [list]
        foreach item [lindex $statistics 6] {
            set pid [lindex $item  0]
            set backlog [expr {[lindex $item 1]/1024}]KB
            lappend consumerList $pid
            if {[dict exists $consumerDict $pid]} {
                set id [dict get $consumerDict $pid]
            } else {
                set id [$table insert $ringStats end -text "PID: $pid"]
            }
            $table item $id -values [list {} "Backlog: $backlog"]
        }
        
        # Consumers come and go so if we have lines in the consumer dict
        # that don't appear in the pid list, delete those elements.
        #
        dict for {pid id} $consumerDict {
            if {$pid ni $consumerList} {
                $table delete $id
            }
        }
            
        
    }
        
    
        
    
}

##
# Top levelGUI
snit::widgetadaptor EVB::GUI {
    component connections
    component statusbar
    component statistics    
    constructor args {
        installhull using ttk::frame
        install statistics using EVB::statistics $win.stats
        install connections using connectionList $win.connections
        install statusbar using EVB::StatusBar $win.statusbar
        
        ttk::frame $win.buttons
        ttk::button $win.buttons.barabort -text {Abort Barrier} \
            -command [list EVB::abortbarrier]
        
        grid $win.buttons.barabort -sticky w
        
        grid $statistics  -sticky nsew
        grid $connections -sticky nsew
        grid $win.buttons -sticky nsew
        grid $statusbar   -sticky nsew
        
        grid rowconfigure $win 0 -weight 1
        grid rowconfigure $win 1 -weight 0
        grid rowconfigure $win 2 -weight 0
        grid rowconfigure $win 3 -weight 0
        
        grid columnconfigure $win 0 -weight 1

    }
    
}

#---------------------------------------------------------------------------
#  Entry point.  Construct the GUI.  That starts it updating.


##
# EVB::createGui
#    Called by the event orderer when the GUI is being created.
#
proc EVB::createGui {win} {
    EVB::GUI $win
    pack $win -fill both -expand 1
}

##
#  The following styling magic works around a defect in Tk 8.6.9
# with regards to tree view tag handling:
#  See https://core.tcl-lang.org/tk/tktview?name=509cafafae
#  for details.
    


apply {name {
    set newmap {}
    foreach {opt lst} [ttk::style map $name] {
        if {($opt eq "-foreground") || ($opt eq "-background")} {
            set newlst {}
            foreach {st val} $lst {
                if {($st eq "disabled") || ($st eq "selected")} {
                    lappend newlst $st $val
                }
            }
            if {$newlst ne {}} {
                lappend newmap $opt $newlst
            }
        } else {
            lappend newmap $opt $lst
        }
    }
    ttk::style map $name {*}$newmap
}} Treeview

