#******************************************************************************
#
# CAEN SpA - Software Division
# Via Vetraia, 11 - 55049 - Viareggio ITALY
# +390594388398 - www.caen.it
#
#***************************************************************************//**
# 

# @file   series.tcl
# @brief  snit object that encapsulates a series for plotchart.
#

package provide Plotchart::series 1.0
package require Plotchart 3.0.0;               # Also establishes the Plotchart namespace
package require snit
##
# @class
#   Plotchart::series
#
#  Encapsulates a series for a plotchart container object.  This works around
#  the fact that plotchart plots don't retain any knowledge about their
#  data by retaining plotted data on behalf of a plotchart conatiner.
#
#  While this class is intended to be used by Plotchart, the interaction with
#  the outside world is quite a bit more general.
#
# OPTIONS
#    * -xdata  - Provides a list of data points that define the data x coordinates
#    * -ydata  - Provides a list of data points that define the data y coordinates
#    * -command - Script called when either x or y data are modified.
#    * -ptlimit - Maximum number of points that are allowed in the
#                 series.  If adding a point exceeds this,
#                 every other point in the first 1/4 of the series
#                 are removed. The assumption is that early points
#                 are older points and that therefore they can be
#                 show in lower resolution.  This works under the
#                 assumption the points are a time series for a
#                 strip chart.
#
# METHODS
#    * append  - Appends an x/y point to the coordinates.
#    * clipx   - Returns a subset of the data that lives within the specified
#                xcoordinate boundaries.
#    * clipy   - Returns a subset of the data that lives within the specified
#                y coordinate boundaries.
#    * clip    - Returns a subset of the data that lives within the specified
#                x/y boundaries.
#
#    * limits  - Returns the X/Y limits of the data.
#    * clear   - Convenience method that is essentially:
#                object configure -xdata [list] -ydata [list]
#
# SUBSTITUTIONS
#    The -command script can provide the following script substitutions:
#
#    * %O  - The handle of the object making the callback.
#    * %X  - -xdata values.
#    * %Y  - -ydata values.
#    
snit::type Plotchart::series {
    option -xdata -default [list] -configuremethod _OnXyConfig
    option -ydata -default [list] -configuremethod _OnXyConfig
    option -command [list]
    option -ptlimit 2000000; # Points we keep before decimating the first 1/4  0 means never throw.
    
    #------------------------------------------------------------------------
    # Public methods:
    #
    
    ##
    # append
    #
    #   Appends a pont to the series.
    #
    # @param x - X coordinate of the point.
    # @param y - Y coordinate of the point.
    #
    # @note this will trigger the -command script.
    #
    method append {x y} {
        lappend options(-xdata) $x
        lappend options(-ydata) $y

        set ptLimit $options(-ptlimit)
            if {($ptLimit > 0) && ([llength $options(-xdata)] > $ptLimit)} {
            $self _ReducePts
	} 

        $self _OnChanged
    }
    
    ##
    # clipx
    #
    #  Return a subset of the data whose x-coordinates live within a window
    #
    # @param xlow - low coordinate of the clipping window.
    # @param xhigh - High coordinate of the clipping window.
    #
    # @return 2 element list containing in order remaining X and Y coords.
    #
    #  @note the clipping window is inclusive.
    #  @note at this time no assumption is made that the Xdata are sorted
    #        though what actually happens if they are not may not meet
    #        everybody's needs... as some will want the clipped points
    #        replaced with a point at the x boundaries rather than
    #        deleted.
    # @note  Any y points that don't have a corresponding X point are deleted
    #        Same applies to any x points that don't have corresponding y's.
    #
    method clipx {xlow xhigh} {
        return [$self _Clip $xlow $xhigh $options(-xdata) $options(-ydata)]
    }
    ##
    # clipy
    #   Clips the data to a window in y.  Points with y data outside the clip
    #   window are deleted.  See clipx as this is exactly the same, just
    #   clipping in y instead of x.
    #
    # @param ylow  - low limit of y data retained.
    # @param yhigh - high limit of y data retained.
    #
    # @return 2 element list of x and y coordinates of the points retained.
    #
    method clipy {ylow yhigh} {
        set clipped [$self _Clip $ylow $yhigh $options(-ydata) $options(-xdata)]
        return [list [lindex $clipped 1] [lindex $clipped 0]]
    }
    ##
    # clip
    #    clip data to within a clipping window.  This is basically
    #    a clipx followed by a clipy.
    #
    # @param xlow  - Low limit of x clipping window.
    # @param xhi   - High limit of x clipping window.
    # @param ylow  - Low limit of y cliping window
    # @param yhi   - High limit of y clipping window
    #
    # @return 2 element list of x/y coordinates that are in the clipping window.
    #
    #
    method clip {xlow xhi ylow yhi} {
        set yclip [$self _Clip $ylow $yhi $options(-ydata) $options(-xdata)]
        return [$self _Clip $xlow $xhi [lindex $yclip 1] [lindex $yclip 0]]
    }
    ##
    # limits
    #
    # Provides the limits on the data.  These are the narrowest clip window
    # that would include all of the data.
    #
    # @return 4 element list of the xlimits followed by the y limits.
    #
    # @note if a list of coordinates is empty, its limits are empty strings.
    #
    method limits {} {
       
        if {[llength $options(-xdata)] > 0} {
            set xlow [::tcl::mathfunc::min {*}$options(-xdata)]
            set xhi  [::tcl::mathfunc::max {*}$options(-xdata)]
        } else {
            set xlow ""
            set xhi ""
        }
        
        if {[llength $options(-ydata)] > 0} {
            set ylow [::tcl::mathfunc::min {*}$options(-ydata)]
            set yhi  [::tcl::mathfunc::max {*}$options(-ydata)]
        } else {
            set ylow ""
            set yhi ""
        }
        return [list $xlow $xhi $ylow $yhi]
    }
    ##
    # clear
    #
    #   Convenience method that clears the data from the series.
    #
    method clear {} {
        set options(-xdata) [list]
        set options(-ydata) [list]
        
        $self _OnChanged
    }
    #--------------------------------------------------------------------------
    #  Private methods.
    #
    
    ##
    # _OnXyConfig
    #    Invoked when -xdata or -ydata have been configured.
    #    * Save the option data
    #    * Invoke _OnChanged to dispatch the script.
    #
    # @param option - name of the option in which the data get stored.
    # @param value  - New coordinate list.
    #
    method _OnXyConfig {option value} {
        set options($option) $value
        $self _OnChanged
    }
    
    ##
    # _OnChanged
    #    Invoked when one or the other of the coordinate sets have changed.
    #    See the class comments for a list of the substitutions that are
    #    supported.
    #
    method _OnChanged {} {
        set baseCommand $options(-command)
        if {$baseCommand ne ""} {
            set command [string map \
                [list %X [list $options(-xdata)] %Y [list $options(-ydata)] \
                    %O $self] \
                $baseCommand                                            \
            ]
            uplevel #0 $command
        }
    }
    
    ##
    # _Clip
    #
    #   Clips data within limits.  Provided a set of inclusive limits,
    #   and a pair of lists, data are returned whose first list are within the
    #   limits.
    #
    # @param low - Low limit
    # @param hi  - High limit of clipping window.
    # @param clipped - List of items to be tested against the clipping windows.
    # @param depends - List of dependent variable values for clipped.
    #
    # @return 2 element list of the reduced clipped list and the corresponding
    #         dependent variable values.
    #
    # @note the description of clipx provides more information about the handling
    #       of some edge cases.  This code arose from a refactoring of
    #       clipx/clipy.
    #
    method _Clip {low hi clipped depends} {
        set c [list]
        set d [list]
        
        # Note: For large data sets, this can be time consuming.
        #       The raw CAEN did this in compiled code.  If performance
        #       is an issue, we'll need to steal that code and put it in
        #       a compiled chunk.
        #
        foreach cc $clipped dc $depends {
            if {($cc ne "") && ($dc ne "")} {
                if {($cc >= $low) && ($cc <= $hi)} {
                    lappend c $cc
                    lappend d $dc
                }
            }
        }
        
        return [list $c $d]
    }
    ##
    # _ReducePts
    #   Reduce the number of data points in the lists.  This is done by 
    #   removing every other point in the first 1/4 of the points.
    #
    method _ReducePts {} {
	set pts [expr $options(-ptlimit)/4]
	
	for {set i 0} {$i < $pts} {incr i} {
	    set options(-xdata) [lreplace $options(-xdata) $i $i]
	    set options(-ydata) [lreplace $options(-ydata) $i $i]
	}
	
    }
}