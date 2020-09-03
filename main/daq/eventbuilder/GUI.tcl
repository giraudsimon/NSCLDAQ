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

#package require EVB::inputStatistics
#package require EVB::outputStatistics
#package require EVB::barriers
package require EVB::connectionList
#package require EVB::Late
#package require EVB::LatePopup
#package require EVB::DuplicateTimestamp
#package require EVB::OutOfOrderUI
#package require RingStatus
#package require ring

# Establish the EVB namespace into which we're going to squeeze:

namespace eval ::EVB {
    
}
#----------------------------------------------------------------------------
#
#  Common utility procs: daqdev/NSCLDAQ#700
#


if 0 {
##
# _processFlowControl
#    - Validates that the flow control option is boolean.
#    - computes the correct string to put in the flow control widget.
#    Factoring this out of the various widgets that display flow control
#    supports a common set of strings.
# @note further factoring would involve a re-arrangement of the GUI such that
#       there's a status tab below the tabbed notebook.
# @param optval - value of the configuration option.
#
proc processFlowControl {optval} {
    snit::boolean validate $optval
    set text "Flow control: "
    if {$optval} {
        append text "Flow Control Active"
    } else {
        append text "Accepting Data"
    }
    return $text
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
    delegate option -inflight     to inputSummary
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
    delegate option -window          to outputSummary
   
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
        
        
        ttk::label $win.flowlabel -text "Flow Control: "
        ttk::label $win.flow      -text "Accepting Data"
        
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
        set text [_processFlowControl $optval]
        set options($optname) $optval
        $win.flow config -text $text
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
#   |                Flow control label                                      |
#   +------------------------------------------------------------------------+
#
# OPTIONS:
#  -flowcontrol             - Set the status of the flow control label.
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
    component flowLabel
    
    option -flowcontrol -default 0 -configuremethod _ConfigFlowControl
    
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
        
        install flowLabel using ttk::label $win.flow -text "Flow Control: Accepting Data"
        

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
        
        grid $flowLabel -sticky w

        grid columnconfigure $win 0 -weight 1
        grid rowconfigure $win 0 -weight 0 

        # process the options:
        
        $self configurelist $args
    }
    #------------------------------------------------------------------------
    # Private methods:
    
    
    ##
    # _ConfigFlowControl
    #     Configure the flow control state.  This affects the flowLabel
    #     widget:
    #
    # @param optname - the option being configured (-flowcontrol I guess).
    # @param optval  - the boolean  value:
    #                - False- Flow Control:  Accepting Data
    #                - True - FLow Control:  Flow control Active.
    #
    method _ConfigFlowControl {optname optval} {
        set text [_processFlowControl $optval]   
        set options($optname) $optval        
        $flowLabel configure -text $text
    }
    #------------------------------------------------------------------------
    # public methods:
    #    
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
#   |     flow on/off label                           |
#   +-------------------------------------------------+
#
# OPTIONS:
#     - -incompletecount - Sets the number of incomplete
#                        barriers in the summary widget.
#     - -completecount   - Sets the number of complete barriers in the summary
#                        widget.
#     - -heterogenouscount - Sets the number of complete barriers that were
#                        heterogenous in type.
#     -flowcontrol - Sets the flow control label.
# METHODS:
#    - setStatistic sourceid barriertype count -
#                         sets the barrier statistics for a source id.
#    - clear             - Clears the counters for each source.
#    - reset             - Removes all source statistics.
#
snit::widgetadaptor EVB::barrierStatistics {
    component summary
    component perQueue
    component flowLabel
    
    option -flowcontrol -default 0 -configuremethod _CfgFlow
    
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
        install flowLabel using ttk::label $win.flow -text "Flow Control: Accepting Data"
       
         
        grid $summary -sticky new
        grid $perQueue -sticky news
        grid $flowLabel -sticky e
  
        grid configure $win -padx 5 -pady 5 
        grid columnconfigure $win 0 -weight 1           
  
        $self configurelist $args
    }
    #-----------------------------------------------------------------------------
    # Private methods
    ##
    # _CfgFlow - configure flow control label.
    #
    # @param optname - name of the option.
    # @param optval  - boolean option value.
    method _CfgFlow {optname optval} {
        set text [_processFlowControl $optval]
        set options($optname) $optval
        $flowLabel configure -text $text
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
#    |                        Flow control                    |
#    +--------------------------------------------------------+
#
# OPTIONS:
#   - -flowcontrol - boolean controlling the flow control label
# METHODS:
#   - getLateStatistics       - Returns the EVB::lateFragments widget.
#   - getIncompleteStatistics - Returns the EVB::BarrierStats::incomplete widget.
#
#
snit::widgetadaptor EVB::errorStatistics {
    component lateStats
    component incompleteStats
    component flowLabel
    
    option -flowcontrol -default 0 -configuremethod _CfgFlow
    delegate option * to hull
    delegate method * to hull
    ##
    # constructor
    #
    #   Install the hull, the two components and lay them out.
    #
    constructor args {
        installhull using ttk::labelframe
        
        install lateStats using       EVB::lateFragments            $win.late
        install incompleteStats using EVB::BarrierStats::incomplete $win.inc
        install flowLabel       using ttk::label $win.flow -text {Flow Control: Accepting Data}
        
        grid $win.late $win.inc -sticky nsew -padx 5 -pady 5
        grid $flowLabel -columnspan 2 -sticky w
        
        grid columnconfigure $win 0 -weight 1 -uniform a 
        grid rowconfigure $win 0 -weight 1 

        grid configure $win -padx 5 -pady 5 

        $self configurelist $args
    }
    #-----------------------------------------------------------------------
    # Private methods.
    
    ##
    # _CfgFlow
    #    Configure the flow control label.
    #
    # @param optname  - name of the config option.
    # @param optval   - option value (bool).
    #
    method _CfgFlow {optname optval} {
        set text [_processFlowControl $optval]
        set options($optname) $optval
        $flowLabel configure -text $text
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
# @class EVB::RingStatus
#    This is a frame that consists of a RingStatus widget and a label
#    The label reflects the flow off/on status of the event builder while
#     RingStatus show sthe backlog by pid of rings involved in the Event builder.
#
# OPTION
#    -flowcontrol - boolean true means flow off.
#
snit::widgetadaptor EVB::RingStatus {
    component status
    component flowLabel
    option    -flowcontrol -default 0 -configuremethod _ConfigFlow
    delegate method * to status
    delegate option * to status
    constructor args {
        installhull using ttk::frame
        install status using ::RingStatus $win.status
        install flowLabel using ttk::label $win.flow -text "Flow Control: Accepting Data"
        
        grid $status;              # row 0
        grid $flowLabel;           # row 1
        
        # Handle expansion correctly.
        
        grid rowconfigure $win 0  -weight 1
        grid rowconfigure $win 1 -weight 1
        grid columnconfigure $win 0 -weight 1
        
        #
        
        $self configurelist $args
    }
    ##
    # _ConfigFlow
    #   Set the flow control label appropriately:
    #
    # @param optname    -name of the configuration option (-flowcontrol)
    # @param optval     -proposed new value.  Must be a boolean:
    #                   - False - Text is "Flow control: Accepting Data"
    #                   - True  - Text is "Flow control:  Flow Control Active"
    #
    method _ConfigFlow {optname optval} {
        set text [_processFlowControl
        set options($optname) $optval
        $flowLabel configure -text $text
    }
}
#-----------------------------------------------------------------------------
# @class EVB::OutOfOrderWindow
#    This widget just consists of a frame containing and out of order
#    window and a label widget to show the flow control state.
#
#  OPTIONS:
#    -flowcontrol - boolean indicating if flow control is active (true) or not
#                  (false).  Controls the text in the label field.
#
snit::widgetadaptor EVB::OutOfOrderWindow {
    component ooWindow
    component flowLabel
    option -flowcontrol -default 0 -configuremethod _CfgFlowControl
    
    delegate option * to ooWindow
    delegate method * to ooWindow
    
    constructor args {
        installhull using ttk::frame
        install ooWindow using ::OutOfOrderWindow $win.oo
        install flowLabel using ttk::label $win.flow -text "Flow Control: Accepting Data"
        
        grid $ooWindow
        grid $flowLabel
        
        grid columnconfigure $win 0 -weight 1
        grid rowconfigure    $win 0 -weight 1
        grid rowconfigure    $win 1 -weight 1
        
        $self configurelist $args
    }
    ##
    # _CfgFlowControl
    #     Configure the flow control widget.
    #
    # @param optname   - name of the option configured (-flowcontrol e.g.)
    # @param optval    - new boolean value for the option:
    #                  - True - flowLabel will display "Flow Control: Flow Control Active"
    #                  - False - flowlabel will display "Flow COntrol: Accepting Data"
    #
    method _CfgFlowControl {optname optval} {
        
        set text [_processFlowControl $optval]
        set options($optname) $optval
        $flowLabel configure -text $text
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
    
    option -flowcontrol -default 0 -configuremethod _CfgFlow
    
    delegate option * to hull
    delegate method * to hull
    
    variable outOfOrderTabno
    variable errorTabno
    
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
        
        install ringStats using EVB::RingStatus $win.ringstats
        $hull add $ringStats -text {Output Ring Stats}
        
        install ooStats using EVB::OutOfOrderWindow $win.oo
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
        
        image create photo redbackground -file [file join $::here red.gif]
        
        #  If the tab changes to one of the red-able tabs, turn it back:
        
        bind $win <<NotebookTabChanged>> [mymethod _normalizeTabColor]
    }
    #------------------------------------------------------------------------
    # Public methods:
    
    
    ##$
    # Set the out of order stats tab red.   This only will be done if that's not the
    # current tab:
    #
    method setLateRed {} {
        if {[$hull index current] != $outOfOrderTabno} {
            $hull tab $outOfOrderTabno -image redbackground -compound center    
        }
        
    }
    ##
    # Set the out of order stats tab to normal background.
    #
    method setLateNormal {} {
        $hull tab $outOfOrderTabno -compound text
        
    }
    ##
    #  Set the error counters tab to red background.
    #  This is only done if the tab is not currently displayed:
    #
    method setErrorsRed {} {
        if {[$hull index current] != $errorTabno} {
            $hull tab $errorTabno -image redbackground -compound center
        }
        
    }
    ##
    # Set the error counters tab background to normal.
    #
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
    #-----------------------------------------------------------------------------
    #  Private methods
    #
    
    ##
    # _normalizeTabColor
    #    Called when tab changes.  Get the current tab.  If it's either of
    #    outofOrderTabno or errorTabno, set the tab color to normal.
    #
    method _normalizeTabColor {} {
        set current [$win index current]
        
        if {$current == $outOfOrderTabno} {
            $self setLateNormal
        }
        if {$current == $errorTabno} {
            $self setErrorsNormal
        }
    }
    ##
    # _CfgFLow
    #   Configure flow control - configure flow control of the components.
    #
    method _CfgFlow {optname optval} {
        snit::boolean validate $optval
        set options($optname) $optval
        
        foreach comp [list $summaryStats $sourceStats $ooStats $barrierStats \
                        $errorStats $ringStats] {
            $comp configure -flowcontrol $optval
        }
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
    EVB::onflow add \
        [list flowOnOff $summary  0]  \
        [list flowOnOff $summary 1]
    
    set EVB::ooWidget [$widget getOutOfOrderStats]

    EVB::ootrace add addOutOfOrder

    return $widget
}
proc flowOnOff {widget state args} {
    $widget configure -flowcontrol $state
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
proc EVB::maintainGUI {widget {ms 2000} } {
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
    set inflightCount [lindex $inputStats 4]
    
   # puts stderr "$inflightCount fragments in flight"
    
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
    -inflight $inflightCount                                        \
	-oldest [lindex $inputStats 0] -newest [lindex $inputStats 1] \
	-deepestid $deepest -deepestdepth $deepestCount               \
        -hottestoutid $hottestSrc -hottestoutcount $hottestCount      \
        -coldestoutid $coldestSrc -coldestoutcount  $coldestCount     \
	-completebarriers [lindex $completeBarriers 0]                \
	-incompletebarriers [lindex $incompleteBarriers 0]            \
	-mixedbarriers      [lindex $completeBarriers 2]              \
	-outfragments $totalFrags                                     \
        -queuedbytes $totalin -outbytes $totalout -outrate $outputRate  \
        -window [EVB::config get window] 

    $barriers configure -incompletecount [lindex $incompleteBarriers 0] \
	-completecount [lindex $completeBarriers 0] \
	-heterogenouscount [lindex $completeBarriers 2]

    $lateWidget configure -count [lindex $lateStats 0] -worst [lindex $lateStats 1]
    if {($lateStats ne $::EVB::lastLatewidgetStats) && ($::EVB::lastLatewidgetStats ne "")} {
        $widget setErrorsRed
    }

    set ::EVB::lastLatewidgetStats $lateStats


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
            if {$stats ne $::EVB::lastIncompleteStats} {
                $widget setErrorsRed
            }
            set ::EVB::lastIncompleteStats $stats
        }
        # Data late information:
    
        if {[dict exists $sourceStatistics($source) late]} {
            set stats [dict get $sourceStatistics($source) late]
            if {$stats ne $::EVB::lastLateSummary}    {
                $widget setLateRed
            }
            $lateWidget source $source [lindex $stats 1] [lindex $stats 2]
            set ::EVB::lastLateSummary $stats
        }
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

proc onDestroy {widget} {
    if {$widget eq "."} {
        exit
    }
    
}

bind . <Destroy> [list onDestroy %W]
}

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
        $self _xon;     # Assume we start up xon'ed.
    }
    destructor {
        EVB::onflow remove [mymethod _xon] [mymethod _xoff]
    }
    ##
    # _xon
    #   Called when the event builder XON's its clients.
    #
    method _xon {} {
        set flowstate "Accepting Data"
    }
    ##
    # -xoff
    #   Called when the event builder XOFF"s its clients.
    #
    method _xoff {} {
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
    
    variable afterId -1
    constructor args {
        installhull using ttk::frame
        install table using ttk::treeview $win.table \
            -show tree -columns {col1 col2 col3 col4} \
            -displaycolumns #all -yscrollcommand [list $win.vscroll set]
        scrollbar $win.vscroll -orient vertical -command [list $table yview]
        grid $table $win.vscroll -sticky nsew
        $self _addInputStats
        $self _addOutputStats
        $self _addQueueStats
        $self _addBarrierStats
        
        $self _refresh
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
            [lindex $stats 4] [lindex $stats 0] [lindex $stats 1] \
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
            set oldest [lindex $stat 2]
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
            set oldest [lindex $stat 2]
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
        
        set missingHisto [lindex $stats 3]
        $self _updateMissingHisto $missingHisto
        
        set missingSources [lindex $stats 4]
        $self _updateMissingSources $missingSources
    }
    ##
    #_updateMissingHisto
    #    Updates the incomplete barrier type missing histogram.
    # @param stats - Statistics consisting of a list of pairs
    #                containing source ids and the number of times
    #                that source id was missing.
    #
    method _updateMissingHisto stats {
        # Make the usual dict of existing sources.
        # The lines look like:
        #   "Missed type n"  "m times" where the first part of
        # that line is the text and the second the first value.
        #
        
        set histoDict [dict create]
        foreach child [$table children $incompleteBarriersId] {
            set text [$table item $child -text]
            if {[lindex $text 0] eq "Missed"} {
                set type [lindex $text 2]
                dict append $histoDict $type $child
            }
        }
        # Now process the statistics updateing existing and
        # creating new items.
        
        foreach stat $stats {
            set type [lindex $stat 0]
            set count [lindex $stat 1]
            
            if {[dict exists $histoDict $type]} {
                set id [dict get $histoDict $type]
            } else {
                set id [$table insert $incompleteBarriersId 0 \
                        -text "MIssing type $type"            \
                ]
            }
            $table item $id -values [list   \
                "$count Times"
            ]
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
            if {[lindex $text 0] eq "source"} {
                dict append $missingDict $[lindex $text 1] $child
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
                    -text [list "Source $sid"]                \
                ]
            }
            $table item $id -values [list "$count times"]
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
        
        grid $statistics  -sticky nsew
        grid $connections -sticky nsew
        grid $statusbar   -sticky nsew
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
