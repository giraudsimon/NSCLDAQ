#******************************************************************************
#
# CAEN SpA - Software Division
# Via Vetraia, 11 - 55049 - Viareggio ITALY
# +390594388398 - www.caen.it
#
#***************************************************************************//**
# 

# @file xyplot.tcl
# @author: Ron Fox.
# @brief Container for plotchart xy plot.
#

# Package name is xyplotContainer just in case theres a Plotchart::xyplot
# that's part of plotchart itself.

package provide Plotchart::xyplotContainer 1.0
package require Tk        8.5-
package require snit
package require Plotchart 3.0.0
package require Plotchart::series
package require Plotchart::marker

##
# @class xyplot
#
#    This snit::type is a container for a plotchart xyplot in a canvas.
#    While plotchar tis a very convenient plotting package it suffers from the
#    problems of simplicity.  Specifically, sometimes when changing the plot
#    it's sufficient to just redraw the data (e.g. new values for a data series.
#    other times you basically have to destroy and recreate the plot.
#
#    This class attempts to encapsulate a plotchart xyplot so that the client
#    just makes changes and lets this class worry about how to get plotchart
#    to do its bidding.
#
#   Currently the the following objects can be plotted:
#   * Series (see Plotchart::series)  These are sets of xy pairs.
#   * Markers (see Plotchart::marker)  These are little colored triangles that
#     can be put anywhere on a plot in world coordinates.  These are generally used
#     to point at things.
#
# OPTIONS:
#   * -xmin    - Minimum value of the x axis.
#   * -xmax    - Maximum value of the x axis.
#   * -ymin    - Minimum value of the y axis.
#   * -ymax    - Maximum value of the y axis.
#   * -title   - Plot title (if not empty).
#   * -xtitle  - Xaxis title.
#   * -ytitle  - Yaxis title.
#   * -plotid  - Current plotchart id (readonly).
#   * -showkey - True if the key should be displayed.
#   * -canvas  - Canvas on which to draw the plot (mandatory immutable)
#   * -grid    - If true a grid is displayed.
#   * -plottype - Specifies the type of xyplot created.  Defaults to xyplot.
#
# Methods:
#   series          - Adds or modifies a data series.
#   deleteSeries    - Removes a data series.
#   getSeries       - Lists the series names.
#   getSeriesMinMax - Get information about min/max and where values for series
#                     within an X/Y range.
#   addSeriesPoint  - Append a point to a series.
#   clearSeries     - Kill off points in a series and schedule a replot.
#   marker          - Adds or modifies the position of a marker to the plot.
#   deleteMarker    - Removes a marker from the plot.
#   markers         - Returns the set of markers
#   worldToCanvas   - Convert world coordinates to a canvas pixel position.
#   canvasToWorld   - COnvert a canvas to a world coordinate position.
#   resize          - Change the size of the canvas (requires a plot redraw).
#   getPlotLimits   - Return axis limits of the plot.
#   
snit::type Plotchart::xyplotContainer {
    
    # Plot world coordinate limiits:
    
    option -xmin -configuremethod _ScheduleRecreate
    option -xmax -configuremethod _ScheduleRecreate
    
    option -ymin -configuremethod _ScheduleRecreate
    option -ymax -configuremethod _ScheduleRecreate
    
    # Govern the plotting - any XYPlot can be used as the plot type.
    
    option -plottype -default ::Plotchart::createXYPlot -configuremethod _ScheduleRecreate
    
    #  Titles:
    
    option -title  -configuremethod _SetTitle -default ""
    option -xtitle -configuremethod _SetXTitle -default ""
    option -ytitle -configuremethod _SetYTitle -default ""
    option -showkey -configuremethod _ConfigureShowKey \
        -default [expr bool(true)]

    option -plotid -readonly yes -default ""
    option -canvas -default "";#  -readonly true
    option -grid   -default 0  -configuremethod _SetGrid
    
    # Since several options may be quickly modified that would cause a plot
    # recreate (consider an expansion that modifies -xmin -xmax -ymin -ymax),
    # these are not done immediately but scheduled for later when hopefully
    # all changes have been done and configuration is quiescent.
    # recreateAfterId maintains the id of an [after] command used to schedule
    # the recreate.
    #
    
    variable recreateAfterId -1
    
    # Similarly data redraws come in bunches:
    
    variable redrawAfterId -1
    
    ##
    #   The following array contains all of the data series.
    #   The name of the series is the array index.  each element of the array
    #   is a dict containing:
    #   * name  - The name of the series again.
    #   * color - The color that should be used to draw the series.
    #   * data  - A Plotchart::series object containing the points.
    #
    variable plotSeries -array [list]
    ##
    #  This array is the last set of series names plotted.  It is used
    #  to minimize the regeneration of the key.
    #
    variable lastSeries -array [list]
    
    ##
    # The following array contains all of the markers that are plotted on
    # the canvas.  Elements are indexed by the marker name.
    # Each element is a dict that contains:
    #  * name - the name of the series again
    #  * marker - The marker again.
    #
    variable markers -array [list]
    
    
    #--------------------------------------------------------------------------
    #   Construction and any other canonicals
    #
    
    ##
    # constructor
    #
    #   Just lets the plot be configured.  That will reschedule the plot.
    #   creation.  Note that if the plot does not yet have a -canvas value
    #   when the recreation happens a plot won't get made.
    #
    # @param args - Set of -option values that configure the plot.
    #
    constructor args {
        $self configurelist $args
        
        if {$options(-canvas) eq ""} {
            error "There must be a -canvas option to create a PlotChart::xyplot."
        }
    }
    ##
    # destructor
    #
    #   * If a recreation is scheduled, cancel it.
    #   * If the plot exists destroy it as much as we can.
    #   * Destroy any data series objects
    #   * Destroy any marker objects.
    #
    #
    destructor {
        if {$recreateAfterId != -1} {
            after cancel $recreateAfterId
        }
        if {$redrawAfterId != -1} {
            after cancel $redrawAfterId
        }
        #
        #  Must be a plot and a canvas to destroy stuff.
        #
        set plotid $options(-plotid)
        if {$plotid ne "" && $options(-canvas) ne ""} {
            $plotid deletedata
            $self _ClearPlot
        }
        # Destroy data series:
        
        foreach seriesName [array names plotSeries] {
            set series [dict get $plotSeries($seriesName) data]
            $series destroy
            unset plotSeries($seriesName)
            
        }
        # Destroy markers:
        
        foreach markerName [array names markers] {
            set marker [dict get $markers($markerName) marker]
            $marker destroy
            unset markers($markerName)
        }
        # Kill the marker key:
        
        $options(-canvas) delete markerKey
    }
    #--------------------------------------------------------------------------
    # Public methods:
    
    ##
    # worldToCanvas
    #     Convert world coordinates to a canvas pixel position.
    #
    #  @param x - world coordinate x position.
    #  @param y - world coordinate y position.
    #
    # @return  2 element list of x/y pixel positions.
    #
    # @note pixels are integers but plotchart returns a pair of reals.
    #       we integerize them here for simplicity sake.
    #
    method worldToCanvas {x y} {
        set pixels [Plotchart::coordsToPixel [internalPlotId $options(-plotid)] $x $y]
        
        set xPix [expr {round([lindex $pixels 0])}]
        set yPix [expr {round([lindex $pixels 1])}]
        
        return [list $xPix $yPix]
    }
    
    
    #   canvasToWorld
    #
    # Convert a canvas to a world coordinate position.
    #
    # @param x - The x coordinate of the point to convert.
    # @param y - The y coordinate of the point to convert.
    #
    # @return list - 2 element list containing the x, y world coordinates
    #                that correspond to the pixel provided.
    #
    method canvasToWorld {x y} {
        return [Plotchart::pixelToCoords [internalPlotId $options(-plotid)] $x $y]
    }
    
    ##
    # series
    #  Adds or modifies a data series.  Data series are created if they
    #  don't exist or modified if they do.  Regardless, _RedrawData scheduled
    #  to replot the data.
    #
    # @param name  - Name of the data series.
    # @param xcoords - X coordinates of points.
    # @param ycoords - Y coordinates of points.
    # @param color - Drawing color defaults to 'black'.
    #
    method series {name xcoords ycoords {color black}} {

        # Either destroy the old series
        # and get the dict or
        # make an empty dict:
        
        
        if {[array names plotSeries $name] ne ""} {
            $self deleteSeries $name
            set isNew 0
        } else {
            set isNew 1
        }
        # Create the data series and fill in the dict.
        # then store in the plotSeries array:
        #
        
        set data [Plotchart::series %AUTO% -xdata $xcoords -ydata $ycoords]
        set dict [dict create name $name color $color data $data]
        set plotSeries($name) $dict
    
        # If this is a new series we need to schedule the redraw.
        # otherwise, deleteSeries already did:
    
        if {$isNew} {
            $self _ScheduleRedraw
        }
    }   
    ##
    #   deleteSeries
    #
    #    Destroys a series. This will force a redraw to be scheduled
    #    if the series actually exists.  Note that if the series does not
    #    exist, no redraw is forced and no error is thrown.
    #
    # @param name - name of the data series to destroy.
    #
    method deleteSeries {name} {

        if {[array names plotSeries $name] ne ""} {
            set dict $plotSeries($name)
            set series [dict get $dict data]
            $series destroy
            unset plotSeries($name)
            
            $self _ScheduleRedraw
            
        }
    }
    ##
    # getSeries
    #   Returns a list of the series names that are known:
    #
    # @return list
    #
    method getSeries {} {
        return [array names plotSeries]
    }
    ##
    # getSeriesColor
    #
    #  Figure out the color of the series.
    # @param name - name of a series.
    #
    method getSeriesColor {name} {
        if {[array names plotSeries $name] ne ""} {
            return [dict get $plotSeries($name) color]
        } else {
            error "Series $name does not exist"
        }
    }
    ##
    # setSeriesPointLimit
    #   Set the limit on the number of points a series will hold before it
    #    trims
    #
    #  @param name  - name of the series.
    #  @param n     - number of points.
    #
    method setSeriesPointLimit {name n} {
        if {[array names plotSeries $name] ne ""} {
            set series [dict get $plotSeries($name) data]
            $series configure -ptlimit $n
        } else {
            error "Series $name does not exist"
        }
    }

    ##
    # getSeriesMinMax
    #
    # Get information about min/max and where values for series
    # within an X/Y range.
    #
    # @param left - Minimum x value.
    # @param right - Maximum x value.
    #
    # @return dict The dict has the following keys:
    #   * names - List of series names.
    #   * mins  - List of corresponding minimum values.
    #   * maxes - List of corresponding maximum values.
    #   * xmin  - List of X coordinates of minima.
    #   * xmax  - List of X coordinates of minima.
    #
    method getSeriesMinMax {left right} {
        set result [dict create\
            names [list] mins [list] maxes [list] xmin [list] xmax [list]]
        foreach name [lsort -increasing [array names plotSeries]] {
            dict lappend result names $name
            
            #  Get the series that corresponds to this; clip its x points
            #  to the interval and figure out the answer... If there are
            #  no points in the clipping interval, an empty element is added
            #  to each list:
            
            set seriesDict $plotSeries($name)
            set series [dict get $seriesDict data]
            set trace [$series clipx $left $right]
            if {[llength [lindex $trace 0]] == 0} {
                set min ""
                set max ""
                set xmin ""
                set xmax ""
            } else {
                set max [::tcl::mathfunc::max {*}[lindex $trace 1]]
                set min [::tcl::mathfunc::min {*}[lindex $trace 1]]
                
                # Need to find the a coordinate at which we
                # match these values:
                
                set xmin ""
                set xmax ""
                
                foreach x [lindex $trace 0]  y [lindex $trace 1] {
                    if {$max == $y} {
                        set xmax $x
                    }
                    if {$min == $y} {
                        set xmin $x
                    }
                    # Short circuit loop exit:
                    
                    if {($xmin != "") && ($xmax != "")} {
                        break
                    }
                }
            }
            dict lappend result mins  $min
            dict lappend result maxes $max
            dict lappend result xmin  $xmin
            dict lappend result xmax  $xmax
        }
        return $result
    }
    ##
    # addSeriesPoint
    #   Adds a new point to a series.
    #
    # @param name - series name.
    # @param x    - Point x coordinate.
    # @param y    - Point y coordinate.
    #
    # @note I don't think this requires a re-plot
    method addSeriesPoint {name x y} {
        if {[array names plotSeries $name] eq ""} {
            error "No such series $name"
        }
        set seriesDict $plotSeries($name)
        set series [dict get $seriesDict data]
        $series append $x $y
        if {$options(-plotid) eq ""} {
            $self _ScheduleRedraw
        } else {
            $options(-plotid) plot $name $x $y
        }
    }
    ##
    #  clearSeries
    #
    # Clear a series and schedule a replot:
    #
    # @param name - Name of the series.
    #
    method clearSeries {name} {
        if {[array names plotSeries $name] eq ""} {
            error "Series $name does not exist"
        }
        set series [dict get $plotSeries($name) data]
        $series clear
        $self _ScheduleRedraw
    }
    
    ##
    #   marker
    #     Adds or modifies the position of a marker to the plot.
    #     At this point no ability to change marker size is provided.
    #     IF the marker already exists it is just configured/moved.
    #     If the marker does not exist, the marker is created and drawn.
    #
    # @param name  - Name of the marker.
    # @param x     - X position of the marker point.
    # @param y     - Y position of the marker point.
    # @param faces - Position the marker is facing (up|down|left|right)
    # @param color - Color in which the marker will be drawn (defaults to black)
    #
    method marker {name x y faces {color black}} {
        # What we do depends on whether the marker exists:
        if {[array names markers $name] ne ""} {
            
            # Modify existing marker.
            
            set dict $markers($name)
            set marker [dict get $dict marker]
            $marker configure -color $color -direction $faces
            $marker moveTo $x $y
        } else { 
            # Create new marker:
            set marker [Plotchart::marker \
                %AUTO% -plot $options(-plotid) -color $color -direction $faces]
            $marker drawAt $x $y
            set dict [dict create name $name marker $marker]
            set markers($name) $dict
            
            $self _RedrawMarkerKey;    # New marker/new key entry.
        }
    }
    ##
    # markerMove
    #   Change nothing but the position of a marker.  It is an error
    #   for that marker to not exist:
    #
    # @param name - Name of the marker.
    # @param x    - world coordinates of new x position of point.
    # @param y    - world coordinates of new y position of point.
    #
    method markerMove {name x y} {
        if {[array names markers $name] ne ""} {
            set dict $markers($name)
            set marker [dict get $dict marker]
            $marker moveTo $x $y
        }
    }
    ##
    # deleteMarker
    #
    #   Destroys a marker.  When the marker is destroyed it is removed
    #   from view and the marker key is refreshed.  If the marker named
    #   does not exist, this is a total no-op.
    #
    # @param name   - Name of the marker.
    #
    method deleteMarker {name} {
        if {[array names markers $name] ne ""} {
            set dict $markers($name)
            unset markers($name)
            
            set marker [dict get $dict marker]
            $marker destroy
            
            $self _RedrawMarkerKey;     # To remove the existing marker.
        }
    }
    ##
    # deleteAllMarkers
    #
    #  Destroys all known markers:
    #
    method deleteAllMarkers {} {
        foreach name [array names markers] {
            $self deleteMarker $name
        }
    }
    ##
    # markers
    #
    #   Returns a list where each element of the list is a dict that consists
    #   of the keys:
    #     * name - The name of the marker.
    #     * marker - The marker object name.
    #
    method markers {} {
        set result [list]
        foreach name [array names markers] {
            lappend result $markers($name)
        }
        return $result
        
    }
    ##
    # resize
    #   ReSize the underlying canvas.  This requires a full redraw.
    #
    #  @param width  - New canvas width.
    #  @param height - new canvas height
    #
    method resize {width height} {
        $options(-canvas) configure -width $width -height $height
        $self _ScheduleRecreate -grid $options(-grid);  # TODO: Split option from schedule
    }
    ##
    # getPlotLimits
    #   Returns the current world coordinate limits of the x/y axis.
    #   For e.g. stripchart, this may be different than the limits
    #   defined by -xmin/-xmax, -ymin/-ymax since plotting points can change
    #   the wc limits of the plot.
    #
    # @return list [list xmin xmax ymin ymax]
    #
    method getPlotLimits {} {
        return [$options(-plotid) limits]
    }
    
    #
    #-------------------------------------------------------------------------
    # Private methods
    #
    
    ##
    # _ScheduleRecreate
    #
    #  When idle recreate the plot;  This is called when an option is configured
    #  that requires a recreation of the plot.  In that case a recreate is
    #  scheduled if there is not already one pending.  The idea is to minimize
    #  the plotchart recreations.  This is both a performance thing and because
    #  I'm not 100% sure recreating the plot doesn't have memory leaks from
    #  plotchart (or 'even' in my code for that matter).
    #
    # This is a -configuremethod therefore:
    #
    # @param optionName - name of the option being configured.
    # @param value      - new value for the option.
    #
    method _ScheduleRecreate {optionName value} {
        # Update the option value:
        

        set options($optionName) $value
        #  If necessary schedule the update:
        
        if {$recreateAfterId == -1} {
            set recreateAfterId [after 10 [mymethod _Recreate]];   
        }
    }
    ##
    # _SetTitle
    #    Configuration handler for -title option.  The option value is updated
    #    and _DrawTitle is called to draw the title string.
    #
    #  @param optionName  - Name of the option being modified (-title).
    #  @param value       - new value for the title string.
    #
    method _SetTitle {optionName value} {
        set options($optionName) $value
        
        $self _DrawTitle
    }
    ##
    # _SetXTitle
    #
    #     Set X axis title.   Sets the -xtitle option and calls
    #     _DrawXTitle.
    #     This is a configuremeethod therefore:
    #
    # @param optionName - Name of the option being modified (-xtitle)
    # @param value      - New value for the option.
    #
    method _SetXTitle {optionName value} {
        set options($optionName) $value
        
        $self _DrawXTitle
    }
    ## 
    # _SetYTitle
    #
    #    Set Y axis title. Sets the -ytitle option and calls
    #    _DrawYTitle.  This is a configuremethod so:
    #
    # @param optionName  - Name of the option being set (-ytitle)
    # @param value       - new value for the option.
    #
    method _SetYTitle {optionName value} {
        set options($optionName) $value
        
        $self _DrawYTitle
    }
    ##
    # _ConfigureShowKey
    #
    #    Handle configuration requests for -showkey.  The new value is set
    #    and methods are called only if there is a change:
    #    - _DrawLegend - if the change is from false -> true.
    #    - _HideLegend - if the change is from true -> false.
    #
    #  This is a configuremethod so:
    #
    # @param optionName - The name of the option begin modified (-showkey)
    # @param value      - New value for the key.. This must validate to
    #                     a boolean, else an error is thrown.
    #
    method _ConfigureShowKey {optionName value} {
        # Validate the value:
        
        if {![string is  boolean -strict $value]} {
            error "$optionName value must be a boolean was: $value"
        }
        # Get the old and set the new values:
        
        set oldValue $options($optionName)
        set options($optionName) [expr {bool($value)}]
        
        #  If there was a change figure out what to do:
        
        if {$oldValue != $value} {
            if {$value} {
                $self _DrawLegend
            } else {
                $self _HideLegend
            }
        }       
    }
    ##
    #  _SetGrid
    #     Set the grid on or off (-grid option handler)
    #
    # @param option - option name.
    # @param value  - Value of the option.
    #
    method _SetGrid {option value} {
        if {$options(-plotid) eq ""} {
            return ;                     # The create schedule will turn on the grid
        }
        if {$value} {
            $self _ShowGrid
        } else {
            $self _HideGrid
        }
        set options($option) $value
    }
    ##
    # _ScheduleRedraw
    #
    #   When idle redraw the data series on the plot.  If redrawAfterId is not
    #   -1, an after idle is scheduled to invoke _Redraw.
    #
    method _ScheduleRedraw {} {

        if {$redrawAfterId == -1} {

            set redrawAfterId [after idle [mymethod _Redraw]]
        }
    }
    ##
    # _DrawTitle
    #
    #   Draw the -title string.  This is only done if there is a plot id
    #   The title is drawn at 'left' and is drawn even if -title is empty so that
    #   if there was previously a title, it is made blank.
    #
    method _DrawTitle {} {
        if {$options(-plotid) != ""} {
            $options(-plotid) title $options(-title) left
        }
    }
    ##
    # _DrawXTitle       
    #
    #   Draw the -xtitle string as an x axis title.
    #   This is only done if there is a plot id.
    #   The axis title is drawn even if -xtitle is an empty strig in case
    #   it used to not be
    #
    method _DrawXTitle {} {
        if {$options(-plotid) ne ""} {
            $options(-plotid) xtext $options(-xtitle)
        }
    }
    ##
    # _DrawYTitle
    #
    #     Draw the -ytitle string as the y axis title.
    #     this is only done if there is a plot id.  The axis title is drawn
    #     even if the string is blank.  This allows for the case
    #     when the title used to not be blank.
    #
    method _DrawYTitle {} {
        if {$options(-plotid) ne ""} {
            $options(-plotid) ytext $options(-ytitle)
        }
    }
    ##
    # _DrawLegend       - Draw the legend.
    #
    #  Assuming a legend containing the series in lastSeries update the
    #  legend by removing those entries that are no longer there and
    #  adding new ones.
    #  -plotid value
    #
    method _DrawLegend {} {
        set plot $options(-plotid)
        if {$plot ne ""} {
            #
            #  Remove series that are gone:
            
            set currentSeries [lsort [array names plotSeries]]
            set priorSeries  [array names lastSeries]
            foreach old $priorSeries {
                if {$old ni $currentSeries} {
                    unset lastSeries($old)
                    catch {$plot removefromlegend $old}
                }
            }
            # Add any new series:
            
            foreach s $currentSeries {
                if {$s ni $priorSeries} {
                    set lastSeries($s) $s
                    $plot legend $s $s
                }
            }
        }
    }
    ##
    # _HideLegend
    #
    #    Undraw the legend.  If there is a -plotid, all elements are removed
    #    from the legend.
    #
    method _HideLegend {} {
        set plot $options(-plotid)
        if {$plot ne ""} {
            foreach series [array names plotSeries] {
                catch {$plot removefromlegend $series}
            }
        }
    }
    ##
    # _Recreate
    #
    #    Recreate the plot from scratch.
    #    * If the plot exists we need to destroy its data and its elements.
    #    * We need to create the new plot using the parameters in
    #      -xmin,-xmax and -ymin,-ymax to determine the axis limits.
    #    * We draw the series, titles, axis labels and, if enabled the
    #      legend.
    #    * We recreate all markers on the new plot, and invoke drawAt to
    #      redraw them as well.
    #    * We set the recreateAfterId to -1 indicating there is no pending
    #      recreate request.
    #
    method _Recreate {} {
         
        # Kill off any existing plot:
        # and save the marker information so the markers can be regenerated.
        #
        
        set savedMarkers [list]
        array unset lastSeries
        
        
        set oldPlot $options(-plotid)
        if {$oldPlot ne ""} {

            foreach name [array names markers] {
                set marker [dict get $markers($name) marker]
                set where [$marker coords]
                set color [$marker cget -color]
                set direction [$marker cget -direction]
                lappend savedMarkers [dict create \
                    name $name coords $where color $color direction $direction]
                $marker destroy
                unset markers($name)
            }
            Plotchart::ClearPlot [internalPlotId $oldPlot]            
            $oldPlot deletedata
            $self _ClearPlot
            set options(-plotid) ""    ;  # There's no longer a plot.
            
        }
        
        #  Figure out the axis specs and create the plot:
        
        set xSpecs [Plotchart::determineScale $options(-xmin) $options(-xmax) 0]
        set ySpecs [Plotchart::determineScale $options(-ymin) $options(-ymax) 0]
        
        # TODO:  Make this code vs the code above for choosing axis ticks and position
        #        labels an option with a bit more choice about what can be shown in the
        #        commented code.

#        set xTickInterval [expr {max(1,int(($options(-xmax) - $options(-xmin))/5.0))}]
#        set yTickInterval [expr {max(1, int(($options(-ymax) - $options(-ymin))/5.0))}]       
#        set xSpecs [list $options(-xmin) $options(-xmax) $xTickInterval]
#        set ySpecs [list $options(-ymin) $options(-ymax) ];   #  
        
        # Note that in general the X/Y specs will have different -xmin -xmax
        # -ymin -ymax then requested so we update those values so that cget is accurate:
        
        set options(-xmin) [lindex $xSpecs 0]
        set options(-xmax) [lindex $xSpecs 1]
        set options(-ymin) [lindex $ySpecs 0]
        set options(-ymax) [lindex $ySpecs 1]
        
        set plotId [$options(-plottype) $options(-canvas) $xSpecs $ySpecs ]
        set options(-plotid) $plotId

        
        # redraw the series, titles, and conditionally the legend.
        
        $self _Redraw
        $self _DrawTitle
        $self _DrawXTitle
        $self _DrawYTitle
        if {$options(-showkey)} {
            $self _DrawLegend
        }
        if {$options(-grid)} {
            $self _ShowGrid
        }
        # Each marker must be regenerated as the -plot option is immutable.
        #
        foreach savedMarker $savedMarkers {
            set name [dict get $savedMarker name]
            set coords [dict get $savedMarker coords]
            set color [dict get $savedMarker color]
            set facing [dict get $savedMarker direction]
            
            $self marker $name {*}$coords $facing $color
        }
        #
        #  Indicate any scheduled recreate is satisfied.
        #
        set recreateAfterId -1
    }
    ##
    # _Redraw
    #
    #     Redraw the plot data series.
    #
    method _Redraw {} {
        
        # we only do anything if there is a plot to draw on:
        
        set plot $options(-plotid)
        if {$plot ne ""} {
            $plot deletedata;             # get rid of the old data...
            foreach name [array names plotSeries] {
                $self _DrawSeries $name
            }
        }
        if {$options(-showkey)} {
            $self _DrawLegend
        }
        #
        #  Indicate we've satisfied any pending redraw.
        #
        set redrawAfterId -1
    }
    ##
    # _DrawSeries
    #    Draw a single new series.  The series is alread stored in the
    #    plotSeries array.
    #
    # @param name - name of the series.
    #
    method _DrawSeries {name} {
        
        set dict $plotSeries($name)
        set color  [dict get $dict color]
        set series [dict get $dict data]
        
        # Get the X/Y data clipped to our plotting window and if necessary
        # decimated:
        
        set clippedCoords [$series clipx \
            $options(-xmin) $options(-xmax)]

        set coords [$self _Decimate $clippedCoords]
        

        # plot the series and set its color:
        # If there are no points give up:

        $options(-plotid) dataconfig $name -color $color -type line
        
        if {[llength [lindex $coords 0]] < 2} return
        
        
        $options(-plotid) plotlist $name [lindex $coords 0] [lindex $coords 1]
    }
    ##
    # _Decimate
    #
    #    Given a set of data points, if necessary reduces the plot burden
    #    to only those points that can be shown given the resolution of the
    #    plot/canvas.   In each pixel intervael, the highest and lowest
    #    values for the data that would be crammed into that pixel are
    #    retained while all others are not.  The data are only decimated
    #    if there are 3x or more data points as pixels.
    #
    # @param data  - Two element list of x and then y coordinates.
    #
    # @return 2 element list, same format as the input data.
    #
    method _Decimate {data} {

        # Figure out how many pixels are on the plot's x direction and therefore
        # if we have to decimate at all:
    
        set npts [llength [lindex $data 0]]
        
        set lowXpixel [lindex [$self worldToCanvas $options(-xmin) 0] 0]
        set hiXpixel [lindex [$self worldToCanvas $options(-xmax) 0] 0]
        set nPixels [expr {$hiXpixel - $lowXpixel + 1}]

        if {$npts < (3*$nPixels)} {
            return $data;               # Nothing to gain by decimation.
        }
        #  Ok we're going to decimate.  What we'll do is to figure out
        #  for each pixel, the range of world coordinates it represents
        #  we'll then go through each data range and reduce the result.
        #  Two assumptions will be made:
        #   - the data are already clipped to the plotting window.
        #   - the data are monotonically increasing in x.
        
        set xcoords [list]
        set ycoords [list]
        set xdata   [lindex $data 0]
        set ydata   [lindex $data 1]
        
        set index 0;         # Starting with first data point.
        
        
        # At NSCL the compiled decimation proc is not available as that's part
        # of the CAEN digitizer wrapper which is not used/supplied.

        for {set p $lowXpixel} {$p < $hiXpixel} {incr p} {
            
            # figure out the world coordinate interval represented by this pixel.
            
            set lowx [lindex [$self canvasToWorld $p 0] 0]
            set hix  [lindex [$self canvasToWorld [expr {$p+1}] 0] 0]
            
            # Get the min and max of the set of Y coordinates that are in this
            # interval and add two points, one at each end of the wc interval
            # one with max, one with min....they'll live on the same pixel
            # anyway.
            
            set yvalues [list]
            while {([lindex $xdata $index] < $hix) && ($index < $npts)} {
                lappend yvalues [lindex $ydata $index]
                incr index
            }
            # only add coordinates if we have a nonempty set of ys:
            
            if {[llength $yvalues]} {
                set miny [::tcl::mathfunc::min {*}$yvalues]
                set maxy [::tcl::mathfunc::max {*}$yvalues]
                
                lappend xcoords $lowx $lowx
                lappend ycoords $miny $maxy
            }
        }
        
        # construct and return the result:
        
        return [list $xcoords $ycoords]
        
        
    }
    ##
    # _ClearPlot
    #
    # Remove all graphical entities from the canvas that have to do with the plot.
    # These are things that have any of the following tags (from plochart's
    # manpage):
    #
    # General graphical objects:
    #
    # * mask - Used to manipulate the opaque rectangles that ensure data outside
    #   the viewport are not shown.
    # * topmask, horizmask, vertmask - specialised tags, used for scrollable plots.
    # * title - Used for title strings.
    # * BalloonText, BalloonFrame - Used to manipulate balloon text.
    # * PlainText - Used to manipulate ordinary text without any decoration.
    # * background - Tag used for gradient and image backgrounds (and for gradient-filled bars).
    # * xaxis, yaxis - Tags used for all objects related to horizontal or vertical axes. (also: both for numerical axes and axes with labels as in barcharts). Note, however, that the text along the axes has no particular tag.
    # * raxis - Tag used for all objects related to a right axis.
    # * taxis - Tag used for all objects related to a time axis.
    # * axis3d - Tag used for 3D axes
    # * xtickline, ytickline - Tags used for ticklines.
    # * legend, legengb, legendobj - Tags used for the legend. The latter is
    #   used to manipulate the legend as a whole.
    # * legend_series - Tag used to control the appearance of the legend entry
    #                   ("series" should be replaced by the series name).
    # * object - used as standard tag for all objects drawn with the ::Plotcharto::drawobject procedure. Tags given at object creation time are added to this tag.
    # *  data - The general tag to identify graphical objects associated with
    #    data.
    # * data_seriesname - The tag specific to a data series
    #    ("seriesname" should be replaced).
    # * band - The horizontal or vertical band drawn with the xband otr yband
    #   subcommands have this tag by the actual name).
    # * xtext - The text labelling the xaxis.
    # * ytext - The text labelling hte yaxis horizontically.
    # * vtext - The text labelling the yaxis vertically.
    #
    method _ClearPlot {} {
        #
        #  Invariant tags:
        #
        set tags [list mask topmask title BaloonText BalooonFrame PlainText \
            background xaxis yaxis raxis taxis axis3d xtickline ytickline \
            legend legendb legendobj object data band xtext ytext vtext MarkerKey]
        #
        # Add in the tags for the series, these are legend_$series and
        # data_$series
        #
        foreach series [array names dataSeries] {
            lappend tags legend_$series data_$series
        }
        # Get them all cleared from the canvas:
        
        $options(-canvas) delete {*}$tags
    }
    ##
    # _RedrawMarkerKey
    #
    #  Re creates the marker key;
    #  - Delete all MarkerKey tagged canvas items.
    #  - Create the alphabetical text key of the markers.
    #
    method _RedrawMarkerKey {} {
        set c $options(-canvas)
        set keySeparation 15
        
        $c delete MarkerKey

        set height 25
        $c create text 0 $height -fill black -text "Triggers: " -anchor nw \
            -tags MarkerKey
        incr height $keySeparation
        foreach name [lsort -increasing [array names markers]] {
            set dict $markers($name)
            set marker [dict get $dict marker]
            set color  [$marker cget -color]
            
            $c create text 15 $height \
                -fill $color -text $name -anchor nw -tags MarkerKey
            _DrawTriangle $c 7 $height $color MarkerKey
            
            incr height $keySeparation
        }
    }
    ##
    # _HideGrid
    #
    #   remove the gridlinees from the plot
    #
    method _HideGrid {} {
        set c $options(-canvas)
        
        $c delete xtickline
        $c delete ytickline
        
    }
    ##
    # _ShowGrid
    #
    #   Show the grid (ticklines in plotchart parlance)
    #
    method _ShowGrid {} {
        $options(-plotid) xticklines black dots1
        $options(-plotid) yticklines black dots1
    }
    #--------------------------------------------------------------------------
    #
    #   _DrawTriangle $c 7 $height $color MarkerKey
    #
    #  Draws a triangle on the canvas - this is used as part of drawing the marker key:
    #
    # @param c - Canvas to draw on
    # @param x - X position of triangle point.
    # @param y - Y height of the text of the key.
    # @param color - Color of triangle (filled).
    # @param tag   - Tag to associate with the triangle.
    #
    proc _DrawTriangle {c x y color tag} {
        incr y 7
        set left [expr {$x - 5}]
        set top  [expr {$y - 5}]
        set bot  [expr {$y + 5}]
        
        $c create polygon $x $y $left $top $left $bot \
            -outline $color -fill $color -tags $tag
    }
    ##
    # internalPlotId
    #
    # Dirty kludge:
    #
    #  Convert a plotchart plot id into the internal name used to index
    #  variables.
    #
    # @param plot - Plot id.
    # @return string Internal plot id.
    #
    proc internalPlotId plot {
        set plotNameList [split $plot _]
        return [lindex $plotNameList 1]
    }
    
}
