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

package require EVB::inputStatistics
package require EVB::outputStatistics
package require EVB::barriers
package require EVB::connectionList
package require EVB::Late
package require EVB::LatePopup
package require EVB::DuplicateTimestamp
package require EVB::OutOfOrderUI
package require RingStatus
package require ring

# Establish the EVB namespace into which we're going to squeeze:

namespace eval ::EVB {
    array set lastLateStats [list]
    variable  lateDialog ""
    
    set lastDupStats [list 0 {}]
    variable  dupDialog ""
    
    
    variable lastInBytes  0
    variable lastOutBytes 0
    variable lastinputstats
    array set lastinputstats [list]
    
    variable ooWidget
    
    variable lastLateSummary [list 0  0]
    variable lastIncompleteStats [list]
    variable lastLatewidgetStats [list]
    
}

#-----------------------------------------------------------------------------
#
#  Widgets for each tab
#

##
# @class ::EVB::summary
#
#   This class is a widget that displays the top level summary counters.
#   Specifically input statistics, timestmpa information, output statistices
#   Barrier summary and the connection list are displayed as shown in LAYOUT
#
#    This is a widgetadaptor in order to allow a ttk::frame to be used as the
#    hull.
#
# LAYOUT:
#
#    Note in most cases the blocks shown below are  themselves megawidgets.
#
#
#   +------------------------------------------------------+
#   |inputstatistics::           | Output summary          |
#   |     summaryDisplay         +-------------------------+
#   |                            |  Barrier summary        |
#   +----------------------------+-------------------------+
#   |     Connection list/status                           |
#   +------------------------------------------------------+
#   | Flow control  xxxxxxxxxxxxx                          |
#   +------------------------------------------------------+
#
# @note the connection list/statis widget is fully autonomous.
# 
# OPTIONS
#
#   - -flowcontrol    - True if Xoffed False if Xoned.
#
#   Delegated to the input summary:
#
#   - -infragments    - total number of in flight fragments.
#   - -oldest       - Oldest in-flight timestamp.
#   - -newest       - Newest in-flight timestamp.
#   - -deepestid    - If of deepest input queue.
#   - -deepestdepth - Depth of deepest input queue.
#   - -queuedbytes   - Total number of queued bytes.
#
#   Delegated to the output summary:
#
#   - -outfragments    - Number of fragments that have been output.
#   - -hottestoutid    - Id from which the most fragments have been written.
#   - -hottestoutcount - Number of fragments written from hottestoutid.
#   - -coldestoutid    - Id from which the fewest fragments have been written.
#   - -coldestoutcount - Number of fragments written from coldestoutid.
#   - -outbytes        - Total number of dequeued bytes.
#   - -outrate         - output rates.
#
#   Delegated to the barrier summary:
#
#   - -completebarriers   - Number of complete barriers seen.
#   - -incompletebarriers - Number of incomplete barriers seen
#   - -mixedbarriers      - Number of barriers with heterogeneous counts.
#  
#
snit::widgetadaptor ::EVB::summary {
    component inputSummary
    component outputSummary
    component barrierSummary
    component connectionList
    
    option -flowcontrol -default 0 -configuremethod _SetFlowControl;              # Initially data can flow.
    
    # Delegate the input summary options:
    
    delegate option -infragments  to inputSummary  as -fragments
    delegate option -oldest       to inputSummary
    delegate option -newest       to inputSummary
    delegate option -deepestid    to inputSummary
    delegate option -deepestdepth to inputSummary
    delegate option -queuedbytes  to inputSummary  as -bytes
    
    # Delegate output summary options:
    
    delegate option -outfragments    to outputSummary as -fragments
    delegate option -hottestoutid    to outputSummary as -hottestid
    delegate option -hottestoutcount to outputSummary as -hottestcount
    delegate option -coldestoutid    to outputSummary as -coldestid
    delegate option -coldestoutcount to outputSummary as -coldestcount
    delegate option -outbytes        to outputSummary
    delegate option -outrate         to outputSummary
   
    # Delegate barrier summary
    
    delegate option -completebarriers   to barrierSummary as -completecount
    delegate option -incompletebarriers to barrierSummary as -incompletecount
    delegate option -mixedbarriers      to barrierSummary as -heterogenouscount
    
    
    
    ##
    # constructor
    #
    # @param args - configuration option/value pairs.
    #
    constructor args {
        
        # Install the hull and its components.
        
        installhull using ttk::frame
        install     inputSummary using ::EVB::inputStatistics::summaryDisplay \
            $win.insummary -text {Input Statistics}
        
        install     outputSummary using ::EVB::outputSummary \
            $win.outsummary -text {Output Statistics}
        
        install     barrierSummary using EVB::BarrierStats::Summary \
            $win.barriers -text {Barrier Statistics}
        
        install connectionList     using ::EVB::connectionList \
            $win.connections -text {Connections}
        
        
        label $win.flowlabel -text "Flow Control: "
        label $win.flow      -text "Accepting Data"
        
        # layout the widgets:
       
        grid $inputSummary   -row 0 -column 0 -rowspan 2 -sticky nsew -padx 5 -pady 5
        grid $outputSummary  -row 0 -column 1 -sticky nsew -padx 5 -pady 5
        grid $barrierSummary -row 1 -column 1 -sticky nsew -padx 5 -pady 5
        grid $connectionList -row 2 -column 0 -columnspan 2 -sticky nsew -padx 5 -pady 5
        grid $win.flowlabel $win.flow 
       
        grid columnconfigure $win 0 -weight 1 
        grid columnconfigure $win 1 -weight 1 
        grid rowconfigure $win 2 -weight 1 
    
        grid columnconfigure $inputSummary 0 -weight 1
        grid rowconfigure $inputSummary 0 -weight 1
        
        
        
        $self configurelist $args
    }
    ##
    # _SetFlowControl
    #    Handles changes to the -flowcontrol option value.
    #
    method _SetFlowControl {optname optval} {
        if {$optval} {
            $win.flow config -text {Flow control active}
        } else {
            $win.flow config -text {Accepting Data}
        }
        set options($optname) $optval
    }
}
##
# @class EVB::sourceStatistics
#
#  Displays detailed source and barrier statistics by source.
#  See LAYOUT below for details.  This is a snit::widgetadaptor to allow
#  ttk::frame to be the hull without any tomfoolery with the snit valid hull
#  list.
#
# LAYOUT:
#   +------------------------------------------------------------------------+
#   | Per queue source statistics      | Per queue barrier statistics        | 
#   | (EVB::inputStatstics::queueStats | EVB::BarrierStats::queueBarriers    |
#   +------------------------------------------------------------------------+
#
# OPTIONS:
#
# METHODS:
#    getQueueStatistics     - Returns the queue source statistics widget
#    getBarrierStatistics   - Returns the barrier statistics widget.
#
snit::widgetadaptor ::EVB::sourceStatistics {
    component queuestats
    component barrierstats
    component connections
    component vertPane
    component paneFrame
    delegate option * to hull
    
    ##
    # constructor
    #   Build the widgets lay them out and configure the hull
    #
    # @param args - configuration option / value pairs.
    #
    constructor args {
        
        # The hull will be a ttk::frame.
        
        installhull using ttk::frame
        
        # Create the components:
        
        install vertPane using ttk::panedwindow $win.pane -orient vertical
        install paneFrame using ttk::frame $win.paneFrame 

        install queuestats using EVB::inputStatistics::queueStats $win.paneFrame.queue \
            -width 250 -height 100 -title {Queue statistics}
        
        install barrierstats using EVB::BarrierStats::queueBarriers $win.paneFrame.barrier \
            -width 250 -height 100 -title {Barrier statistics}
        
        install connections using EVB::connectionList $win.connections \
            -text "Connected clients" 
        

        # Layout the components:
 
#        grid configure $win -padx 5 -pady 5 

        grid $queuestats -row 0 -column 0 -sticky nsew -padx 5 -pady 5
        grid $barrierstats -row 0 -column 1 -sticky nsew  -padx 5 -pady 5

#        grid $connections -column 0 -columnspan 2 -sticky nsew \
#                                           -padx 5 -pady 5
#
        grid rowconfigure $paneFrame 0 -weight 1
        grid columnconfigure $paneFrame {0 1} -weight 1

        $vertPane add $paneFrame -weight 1
        $vertPane add $connections -weight 1
        grid $vertPane -sticky nsew

        grid columnconfigure $win 0 -weight 1
        grid rowconfigure $win 0 -weight 0 

        # process the options:
        
        $self configurelist $args
    }
    
    ##
    # getQueueStatistics
    #
    # Return the queuestats component.  This allows clients to maintain
    # its appearance/value
    #
    # @return widget - the queuestats component object.
    method getQueueStatistics {} {
        return $queuestats
    }
    ##
    # getBarrierStatistics
    #
    #  Return the barrierstats component object.  This allows
    #  clients to maintain its appearance/values.
    #
    # @return widget - the barrierstats component object.
    #
    method getBarrierStatistics {} {
        return $barrierstats
    }
    
}
##
# @class EVB::barrierStatistics
#
#   Widget that displays barrier statistics.
#
#  LAYOUT:
#   +-------------------------------------------------+
#   |      EVB::BarrierStats::Summary                 |
#   +-------------------------------------------------+
#   |      EVB::BarrierStats::queueBarriers           |
#   +-------------------------------------------------+
#
# OPTIONS:
#     - -incompletecount - Sets the number of incomplete
#                        barriers in the summary widget.
#     - -completecount   - Sets the number of complete barriers in the summary
#                        widget.
#     - -heterogenouscount - Sets the number of complete barriers that were
#                        heterogenous in type.
#
# METHODS:
#    - setStatistic sourceid barriertype count -
#                         sets the barrier statistics for a source id.
#    - clear             - Clears the counters for each source.
#    - reset             - Removes all source statistics.
#
snit::widgetadaptor EVB::barrierStatistics {
    component summary
    component perQueue
    
    delegate option -text to hull
    delegate option * to summary
    delegate method * to perQueue
    
    ##
    # constructor
    #
    #   Create the hull as a lableframe to allow client titling.
    #   Then create the components and lay everything out.
    #
    constructor args {
        installhull using ttk::labelframe
        
        install  summary using EVB::BarrierStats::Summary $win.summary
        install perQueue using EVB::BarrierStats::queueBarriers  $win.perqueue
       
         
        grid $summary -sticky new
        grid $perQueue -sticky news
  
        grid configure $win -padx 5 -pady 5 
        grid columnconfigure $win 0 -weight 1           
  
        $self configurelist $args
    }
}
##
# @class EVB::errorStatistics
#
# Top level page for displaying error statistics (hopefully this page is
# very boring in actual run-time).
#
# LAYOUT:
#    +--------------------------------------------------------+
#    | EVB::lateFragments   | EVB::BarrierStats::incomplete   |
#    +--------------------------------------------------------+
#
# METHODS:
#   - getLateStatistics       - Returns the EVB::lateFragments widget.
#   - getIncompleteStatistics - Returns the EVB::BarrierStats::incomplete widget.
#
#
snit::widgetadaptor EVB::errorStatistics {
    component lateStats
    component incompleteStats
    
    delegate option * to hull
    
    ##
    # constructor
    #
    #   Install the hull, the two components and lay them out.
    #
    constructor args {
        installhull using ttk::labelframe
        
        install lateStats using       EVB::lateFragments            $win.late
        install incompleteStats using EVB::BarrierStats::incomplete $win.inc
        
        grid $win.late $win.inc -sticky nsew -padx 5 -pady 5
        grid columnconfigure $win 0 -weight 1 -uniform a 
        grid rowconfigure $win 0 -weight 1 

        grid configure $win -padx 5 -pady 5 

        $self configurelist $args
    }
    #-----------------------------------------------------------------------
    # Public methods.
    #
    
    ##
    # getLateStatistics
    #
    #  Return the late statistics widget so that it can be updated.
    #
    method getLateStatistics {} {
        return $lateStats
    }
    ##
    # getIncompleteStatistics
    #
    #  Return the incomplete barrier statistics widget.
    #
    method getIncompleteStatistics {} {
        return $incompleteStats
    }
}

#-----------------------------------------------------------------------------
#
# @class EVB::statusNotebook
#
#  The overall GUI widget. This is just a ttk::notebook with
#  tabs for each of the top level widgets.'
#
#
# METHODS:
#   - getSummaryStats - Returns the summary widget so that it can be updated.
#   - getSourceStats  - Returns the source statistics widget so that it can be
#                     updated.
#   - getOutOfOrderStats - Get tab for per source out of order information.
#   - getBarrierStats - Returns the barrier statistics widget so that it can be
#                     updated.
#   - getErrorStats   - Get the error statistics widget
#   - getRingStats    - Get ring status widget.
#  ..
snit::widgetadaptor EVB::statusNotebook {
    component summaryStats
    component sourceStats
    component ooStats
    component barrierStats
    component errorStats
    component ringStats
    
    delegate option * to hull
    delegate method * to hull
    
    variable outOfOrderTabno
    variable errorTabNo
    
    ##
    # constructor
    #
    #  Install the hull which gets all of the options (to configure
    #  child widgets get the widget identifier of the child [see METHODS] ).
    #
    #  Install the componen widgets as pages in the notebook.
    #  configure.
    #
    constructor args {
        # Install the hull as a tabbed notebook.
        
        installhull using ttk::notebook 
        
        # Install the components as pages:
        
        install summaryStats using EVB::summary $win.summary
        $hull add $summaryStats -text Summary
        
        install sourceStats using EVB::sourceStatistics $win.sources
        $hull add $sourceStats -text {Source Statistics}
        
        install ringStats using RingStatus $win.ringstats
        $hull add $ringStats -text {Output Ring Stats}
        
        install ooStats using OutOfOrderWindow $win.oo
        $hull add $ooStats -text {Out of order}
        set outOfOrderTabno 3
        
        install barrierStats using EVB::barrierStatistics $win.barrier
        $hull add $barrierStats -text {Barriers}
        
        install errorStats using EVB::errorStatistics  $win.errors
        $hull add $errorStats -text {Errors}
        set errorTabno 5
        
        $self configurelist $args

        grid rowconfigure $win 0 -weight 1
        grid columnconfigure $win 0 -weight 1
        
        # Load the red.gif image so that we can set the color of error-ing
        # tabs to red:
        
        image create photo redbackground -file [file join $here red.gif]
        
        #  If the tab changes to one of the red-able tabs, turn it back:
        
        bind $win <<NotebookTabChanged>> [mymethod _normalizeTabColor]
    }
    #------------------------------------------------------------------------
    # Public methods:
    
    method setLateRed {} {
        $win tab $outOfOrderTabno -image redbackground -compound center
    }
    method setLateNormal {} {
        $win tab $outOfOrderTabno -compound text
        
    }
    method setErrorsRed {} {
        $win tab $errorTabno -image redbackground -compound center
        
    }
    method setErrorsNormal {} {
        $win tab $errorTabno -compound text
    }
    
    ##
    # getRingStats
    #   Return the status widhget for the ring.
    #
    method getRingStats {} {
        return $ringStats
    }
    
    ##
    # getSummaryStats
    #   Return the widget that manages the summary statistics.
    #
    method getSummaryStats {} {
        return $summaryStats
    }
    ##
    # getSourceStats
    #  Return the widget that manages the source statistics.
    #
    method getSourceStats {} {
        return $sourceStats
    }
    ##
    # getOutOfOrderStats
    #
    #   Returns the out of order statistics window.
    method getOutOfOrderStats {} {
        return $ooStats
    }
    ##
    # getBarrierStats
    #   Return the widget that manages the barrier statistics.
    #
    method getBarrierStats {} {
        return $barrierStats
    }
    ##
    # getErrorStats
    #
    #   Get the error Statistics widget.
    #
    method getErrorStats {} {
        return $errorStats
    }
}
#-----------------------------------------------------------------------------
#  Private methods
#

##
# _normalizeTabColor
#    Called when tab changes.  Get the current tab.  If it's either of
#    outofOrderTabno or errorTabno, set the tab color to normal.
#
method _normalizeTabColor {} {
    set current [$win tab index current]
    
    if {$current == $outOfOrderTabno} {
        $self setLateNormal
    }
    if {$current == $errorTabno} {
        setErrorsNormal
    }
}

#-----------------------------------------------------------------------------
#
# Stuff to maintain the status of the UI.
#

proc addOutOfOrder {id last bad} {
    $EVB::ooWidget add $id [clock seconds] $last $bad
}


##
# EVB::createGui
#
# Creates the event builder GUI.  Note this does nothing
# to maintain the GUI to do that you must call EVB::maintainGui
#
# @param widget - This is the name of the window to create.
#                 (e.g in the main toplevel use e.g. .evbstatus)
#                 This widget will be created.  It is up to the caller to lay
#                 out the window in its parent.
# @example:
# \beginverbatim
#    EVB::createGUI .evb
#    pack .evb
#    EVB::maintainGUI .evb 1000
# \endverbatim
#
# @return name of widget created.
#
proc EVB::createGui widget {


    package require EventBuilder
    EVB::statusNotebook $widget
    set EVB::lateDialog [EVB::LatePopup %AUTO%]
    
    set summary [$widget getSummaryStats]
    EVB::onflow add [list $summary configure -flowcontrol 0] [list $summary configure -flowcontrol 1]
    
    set EVB::ooWidget [$widget getOutOfOrderStats]

    EVB::ootrace add addOutOfOrder

    return $widget
}

##
# EVB::maintainGUI widget ms
#
#   Self rescheduling proc that maintains an event builder stander UI.
#
# @param widget - Widget containing the event builder standard UI.
# @param ms     - Number of ms between refresh requests.
#                 This defaults to 2000.
#
proc EVB::maintainGUI {widget {ms 2000}} {
    global OutputRing

    # Get the UI pieces:

    set summary          [$widget getSummaryStats]
    set source           [$widget getSourceStats]
    set barriers         [$widget getBarrierStats]
    set errors           [$widget getErrorStats]
    set incompleteWidget [$errors getIncompleteStatistics]
    set lateWidget       [$errors getLateStatistics]

    
    # Update the output ring status:
    
    set rstats [$widget getRingStats]
    $rstats configure -name $OutputRing
    $rstats update [ringbuffer usage $OutputRing]
 
    
    # Get the various statistics:



    set inputStats   [EVB::inputStats]
    set outputStats  [EVB::outputStats get]
    set barrierStats [EVB::barrierstats]
    set completeBarriers    [lindex $barrierStats 0]
    set incompleteBarriers  [lindex $barrierStats 1]
    set lateStats    [EVB::dlatestats]
    set dupStats     [EVB::dupstat get]
    
    EVB::updateDupStatsDialog $dupStats



    # Organize the input/output statitics by source
    # Each sourceid will have a dict that contains the following keys
    # (not all dicts will have all fields!!!)
    # inputstats   - Input statistics for that source.
    # outputstats  - Output statistics for that source
    # barrierstats - Barrier statistics for that source.
    # inompletebarriers - Incomplete barrier statistics.
    # late         - Late statistics for that source.

    # Add input statistics in:

    array set sourceStatistics [list]


    set deepest      -1;                  # Deepest queue id.
    set deepestCount -1;                  # Number in deepest queue.
    set totalin       0;                  # total bytes in.
    set totalout      0;                  # total bytes out.
    
    foreach queue [lindex $inputStats 3] {
	set quid [lindex $queue 0]
        incr totalin  [lindex $queue 3]
        incr totalout [lindex $queue 4]

	# Create the empty dict if it does not exist yet.

	if {[array names sourceStatistics $quid] eq ""} {
	    set sourceStatistics($quid) [dict create]
	}
        if {[array names EVB::lastinputstats $quid] eq ""} {
            set EVB::lastinputstats($quid) [list 0 0]
        }
	dict append sourceStatistics($quid) inputstats $queue

	# Figure out the deepest queue and its depth:

	set depth [lindex $queue 1]
	if {$depth > $deepestCount} {
	    set deepest $quid
	    set deepestCount $depth
	}
    }
    #  Byte rate in/out the multiplication by 1000.0 in the numerator
    #  serves the dual function of turning bytes/ms into bytes/sec and
    #  forcing the computation to be floating.
    set inputRate  [expr {(1000.0*($totalin - $::EVB::lastInBytes))/$ms}]
    set outputRate [expr {(1000.0*($totalout - $::EVB::lastOutBytes))/$ms}]
    
    set ::EVB::lastInBytes $totalin
    set ::EVB::lastOutBytes $totalout


    # Add output statistics in and in the meantime figure out the hottest/coldest source information

    set hottestSrc   -1
    set hottestCount -1
    set coldestSrc   -1
    set coldestCount 0xffffffff

    foreach queue [lindex $outputStats 1] {
	set quid [lindex $queue 0]

	# Create an empty dict if it does not yet exist:
	
	if {[array names sourceStatistics $quid] eq ""} {
	    set sourceStatistics($quid) [dict create]
	}
	dict append sourceStatistics($quid) outputstats $queue


	

	# Update the hottest/codest if appropriate.

	set counts [lindex $queue 1]
	if {$counts > $hottestCount} {
	    set hottestSrc $quid
	    set hottestCount $counts
	}
	if {$counts < $coldestCount} {
	    set coldestSrc $quid
	    set coldestCount $counts
	}
    }
    set totalFrags [lindex $outputStats 0]

    # Add Barrier statistics to the dict:

    #     Complete


    foreach queue [lindex $completeBarriers 4] {
	set srcId [lindex $queue 0]
	
	# If needed create an empty dict:

	if {[array names sourceStatistics $srcId] eq ""} {
	    set sourceStatistics($srcId) [dict create]
	}
	
	dict append sourceStatistics($srcId) barrierstats $queue
    }
 

    #     Incomplete

    foreach queue [lindex $incompleteBarriers 4] {
	set src [lindex $queue 0]

	# If needed make a cleared dict:

	if {[array names sourceStatistics $src] eq ""} {
	    set sourceStatistics($src) [dict create]
	}
	dict append sourceStatistics($src) incompletebarriers $queue
    }
    # Late:
    
    set lateDetails [lindex $lateStats 2]
    foreach item $lateDetails {
	set src [lindex $item 0]
	set count [lindex $item 1]
	set worst [lindex $item 2]

	if {[array names sourceStatistics $src] eq ""} {
	    set sourceStatistics($src) [dict create]
	}
	dict append sourceStatistics($src) late $item
        
	
    }
    EVB::updateLatePopup $lateDetails
        
    
    # Fill in the summary page statistics: 

    $summary configure -infragments [lindex $inputStats 2]            \
	-oldest [lindex $inputStats 0] -newest [lindex $inputStats 1] \
	-deepestid $deepest -deepestdepth $deepestCount               \
        -hottestoutid $hottestSrc -hottestoutcount $hottestCount      \
        -coldestoutid $coldestSrc -coldestoutcount  $coldestCount     \
	-completebarriers [lindex $completeBarriers 0]                \
	-incompletebarriers [lindex $incompleteBarriers 0]            \
	-mixedbarriers      [lindex $completeBarriers 2]              \
	-outfragments $totalFrags                                     \
        -queuedbytes $totalin -outbytes $totalout -outrate $outputRate

    $barriers configure -incompletecount [lindex $incompleteBarriers 0] \
	-completecount [lindex $completeBarriers 0] \
	-heterogenouscount [lindex $completeBarriers 2]

    $lateWidget configure -count [lindex $lateStats 0] -worst [lindex $lateStats 1]
    if {$lateStats ne $lastLatewidgetStats} {
        $widget setErrorsRed
    }
    set lastLatewidgetStats $lateStats


    # Fill in the source  statistics page:

    set iQStats [$source getQueueStatistics]
    set iBStats [$source getBarrierStatistics]
    foreach source [array names sourceStatistics] {
	set depth "" 
	set oldest ""
	set outfrags ""


	# Source statistics

	if {[dict exists $sourceStatistics($source) inputstats]} {
	    set inputStats [dict get $sourceStatistics($source) inputstats]
	    set depth [lindex $inputStats 1]
	    set oldest [lindex $inputStats 2]
            
            #input/output bytes/rates.
            
            set inb     [lindex $inputStats 3]
            set inbt    [lindex $inputStats 5]
            set outb   [lindex $inputStats 4]
            set laststats  $::EVB::lastinputstats($source)
            set lastinb  [lindex $laststats 0]
            set lastoutb [lindex $laststats 1]
            set ::EVB::lastinputstats($source) [list $inbt $outb]
            
            set qrate  [expr {1000.0*($inbt -  $lastinb)/$ms}]
            set dqrate [expr {1000.0*($outb - $lastoutb)/$ms}]
            
            # IF the rates are negative we've had a counter reset so the prior is
            # assumed to be zero.  Next time we'll have a good prior so we
            # don't have to worry about this.
            
            if {$qrate < 0.0} {
                set qrate [expr {1000.0*$inbt/$ms}]
            }
            if {$dqrate < 0.0} {
                set dqrate [expr {1000.0*$outb/$ms}]
            }
	}
	if {[dict exists $sourceStatistics($source) outputstats]} {
	    set outputStats [dict get $sourceStatistics($source) outputstats]
	    set outfrags [lindex $outputStats 1]
	}
	$iQStats updateQueue $source $depth $oldest $outfrags $inb $qrate $outb $dqrate

	# Barrier statistics.

	if {[dict exists $sourceStatistics($source) barrierstats] } {
	    set stats [dict get $sourceStatistics($source) barrierstats]
	    foreach type [lindex $stats 1] {
		$barriers setStatistic $source [lindex $type 0] [lindex $type 1]
		$iBStats  setStatistic $source [lindex $type 0] [lindex $type 1]
	    }
	}
	# Incomplete barriers

	if {[dict exists $sourceStatistics($source)  incompletebarriers]} {
	    set stats [dict get $sourceStatistics($source) incompletebarriers]
	    $incompleteWidget setItem $source [lindex $stats 1]
        if {$stats ne $lastIncompleteStats} {
            $widget setErrorsRed
        }
        set lastIncompleteStats $stats
	}
	# Data late information:

	if {[dict exists $sourceStatistics($source) late]} {
	    set stats [dict get $sourceStatistics($source) late]
        if {$stats != $lastLateSummary}    {
            $widget setLateRed
        }
	    $lateWidget source $source [lindex $stats 1] [lindex $stats 2]
        set lastLateSummary $stats
	}

    


    after $ms [list EVB::maintainGUI $widget $ms]
}

##
# EVB::updateLatePopup $lateStats
#   Update the popup that shows the late statistics.
#  
# @param lateStats late statistics list of source, count,worst for each source with
#                                    data late.
#
# @note EVB::lastLateStats is an array indexed by source id of the late statistics
#                           from last time around. Nothing is done if nothing changed.
# @note EVB::lateDialog is the object that displays the late stats as a popup.
#
proc EVB::updateLatePopup lateStats {
    
    foreach stat $lateStats {
        set source [lindex $stat 0]
        if {([array names EVB::lastLateStats $source] eq "") || ($EVB::lastLateStats($source) ne $stat)} {
            set EVB::lastLateStats($source) $stat
            $EVB::lateDialog source $source [lindex $stat 1] [lindex $stat 2]
        }
    }
}
##
# EVB::updateDupStatsDialog
#
#   Update the popup that shows the duplicate timstamp statistics.
#
# @param dupStats the statistics from the EVB::dupstats get command.
#
proc EVB::updateDupStatsDialog dupStats {
    if {$dupStats ne $EVB::lastDupStats} {
        set EVB::lastDupStats $dupStats
        if {$EVB::dupDialog eq ""} {
            set EVB::dupDialog [EVB::DuplicatePopup %AUTO%]
           
        }
        $EVB::dupDialog update $dupStats
    }
}
