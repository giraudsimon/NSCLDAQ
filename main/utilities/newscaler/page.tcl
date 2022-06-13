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
# @file page.tcl
# @brief A page of a scaler display
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide pagedisplay 1.0
package require Tk
package require snit

##
#  The following styling magic works around a defect in Tk 8.6.9
# with regards to tree view tag handling:
#  See https://core.tcl-lang.org/tk/tktview?name=509cafafae
#  for details.
    

# Note as KDC points out in helpme ticket 	BPL-225-12969
# If this is run inside of SpecTcl, apply has been overridden by the SpecTcl
# apply command so we only do this fix if SpecTclHome is not defined
# ..and put up with the tag failure if it is.

if {[info globals SpecTclHome] eq ""} {

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
}

##
# @class page
#
#   Encapsulates the display of a single page.
#
# OPTIONS:
#
#   -title   - title that appears at the top of the page.
#   -tabname - Short name of the page suitable for use in the tab of e.g. a ttk::notebook.
#
# METHODS
#   add     - adds a new model object.
#   update  - updates all lines on the page.
#   alarms  - Enable/disable alarms.
#
# ASSUMPTIONS:
#   The images: GreenBrick RedBrick and AmberBrick are defined and
#   are images that are the tab background images for tabs with low, high and both
#   alarm types respectively.
#
snit::widgetadaptor pageDisplay {
    option -title
    option -tab
    option -normalbackground     white
    option -lowalarmbackground   green
    option -highalarmbackground  red
    
    #  Each element of the models array is a model that manages the
    #  data for a line of the page (e.g. a singleModel).
    #  lineIndex is the line number used to index the models array.
    #  The model array contains dicts withthe following keys:
    #  *  -model - the actual model
    #  *  -id    - the id of the line in the treeview that displays the model.
    #
    variable models -array [list]
    variable lineIndex -1
    variable alarmState 1
    
    #  The variable below contains the treeview widget name:
    
    variable table
    
    ##
    # constructor:
    #   add a ttk::frame as the hull.
    #   Layout the contained widgets.
    #
    constructor args {
        installhull using ttk::frame
        
        $self configurelist $args
        
        ttk::label $win.title -textvariable ${selfns}::options(-title)
        grid $win.title -sticky nsew
        
        $self _createContentsFrame
    }

    ##
    # add
    #   Adds a model to the page display.
    #   Each model represents the data in a line of the display.
    #
    # @param model  - the model to add.
    #
    method add {model} {
        set id [$self _addLine $model];   # Add to display.
        set models([incr lineIndex]) [dict create -model $model -id $id]
    }
    ##
    # update
    #    Updates the display lines from the models.  
    # @return - directive about what to do with the page tab:
    #          * ok - No background.
    #          * low - Display low alarm tab color
    #          * high - Display high alarm tab color
    #          * both - Display both alarm tab color.
    #
    method update {} {
        array set pageTags [list]
        foreach line [lsort -increasing -integer [array names models]] {
            set id    [dict get $models($line) -id]
            set model [dict get $models($line) -model]
            
            
            # Figure out new -values list and -tags
            
            set values [list]
            lappend values [$model getNumeratorName]
            lappend values [$model getDenominatorName]
            lappend values "[$model getNumeratorRate] [$model getDenominatorRate]"
            lappend values "[$model getNumeratorTotal] [$model getDenominatorTotal]"
            lappend values "[$model getRateRatio] [$model getTotalRatio]"
            
            
            set tag [$model alarmState]
            set pageTags($tag) 1
            
            # Configure the item:
            
            $table item $id -values $values -tags $tag
        }
        # Figure out how/if to color the tab:
        
        if {$alarmState} {
            array unset pageTags ok
            if {[llength [array names pageTags *]] == 2} {
                return both
            } elseif {[array names pageTags high] eq "high"} {
                return high
            } elseif {[array names pageTags low] eq "low"} {
                return low
            }
            return ok
            
        } else {
            return ok
        }
    }
    ##
    # alarms
    #   Enable or disable the alarms
    #
    # @param state - boolean that is true if the alarms should be enabled.
    #
    method alarms state {
        
        if {$state} {
            set colors  [list $options(-normalbackground) \
                 $options(-highalarmbackground) $options(-lowalarmbackground) \
            ]
        } else {
            set colors [lrepeat 3 $options(-normalbackground)]
        }
        # Add the highlighting tags.
        
        foreach tag {ok high low} color $colors {
            $table tag configure $tag -background $color
        }  
	set alarmState $state
    }
    #------------------------------------------------------------------------
    # _createContentsFrame
    #    Creates the contents part of the page. This is a frame that contains
    #    a scrolling treeview.  The columns on the treeview are:
    #    * Numerator - The name of the numerator scaler.
    #    * Denominator - The name of the denominator scaler.
    #    * Rate        - Scaler rate(s).
    #    * Totals      - Scaler total(s).
    #    * Ratio       - Rate/total ratios.
    #
    #  The following tags are defined with differing backgrounc colors:
    #    *  ok  - white
    #    * high - Red
    #    * low  - Green
    #
    method _createContentsFrame {} {
        
        # Make the widgets:
        
        frame $win.contents
        set table [ttk::treeview $win.contents.table            \
            -columns {Numerator Denominator Rate Totals Ratio}  \
            -displaycolumns #all                                \
            -height 16 -show headings -selectmode none          \
            -yscrollcommand [list $win.contents.yscroll set]    \
            -xscrollcommand [list $win.contents.xscroll set]    \
        ]
        $table heading 0 -text Numerator
        $table heading 1 -text Denominator
        $table heading 2 -text Rate(s)
        $table heading 3 -text Total(s)
        $table heading 4 -text {Ratio [rates totals]}
        
        # 1/2 the sizer of the rates and totals columns:
        
        #  This code assumes their current sizes are the same.
        
        set wid [$table column 2 -width]
        $table column 2 -width [expr {$wid/2}]
        $table column 3 -width [expr {$wid/2}]
        
        ttk::scrollbar $win.contents.yscroll -orient vertical \
            -command [list $table yview]
        ttk::scrollbar $win.contents.xscroll -orient horizontal \
            -command [list $table xview]
        
        # Lay them out.
        
        # Weight the table and its containing frame so it grows:
        
        grid $table $win.contents.yscroll -sticky nsew
        grid $table $win.contents.xscroll -sticky nsew
        
        grid rowconfigure    $win.contents 0    -weight 1
        grid columnconfigure $win.contents 0    -weight 1
        
        # We believe the frame is row 1:
        
        grid $win.contents -sticky nsew
        grid rowconfigure    $win 1 -weight 1
        grid columnconfigure $win 0 -weight 1
        

    }
    ##
    # _addLine
    #    Add a new line to the treeview that contains the scaler
    #    display.
    #
    # @param model - The model object that provides data for that line.
    # @return id   - The id of the 'line' added to the treeview.
    #
    method _addLine {model} {
        set numTitle   [$model getNumeratorName]
        set denomTitle [$model getDenominatorName]
        set id [$table insert {} end -values [list $numTitle $denomTitle] -tags ok]
        
        return $id
    }
    
    
}
