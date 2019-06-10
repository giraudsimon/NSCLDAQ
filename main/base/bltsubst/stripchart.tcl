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
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

##
# @file stripchart.tcl
# @brief subset of BLT strip chart.
#
package provide blt::stripchart 1.0
package require Tk
package require snit
package require Plotchart 2.0

# Ensure the blt namespace exists:

namespace eval ::blt {
    namespace eval component {}
}

##
#  @class blt::option::validators
#
#  This class factors out the various validation methods.
#  To use a class will install an instance as a component and delegate
#  the validators required to that component.
#
snit::type blt::option::validators {
    ##
    # isBoolean
    #
    #  Validator for boolean options.
    #
    # @param option - Option being modified.
    # @param value  - proposed value.
    #
    method isBoolean {option value} {
        if {![string is boolean -strict $value] } {
            error "$option value is not a valid boolean: '$value'"
        }
    }
    
    ##
    # isInteger
    #
    # validator for inteter option types.
    #
    # @param option - option being modified.
    # @param value  - proposed new value.
    #
    method isInteger {option value} {
        if {![string is integer -strict $value]} {
            error "$options must have an integer value got '$value'"
        }
    }
    
    ##
    # isFloatNonstrict
    #
    #  Validator for floating point values that could be ""
    #
    # @param option - option being modified.
    # @param value  - proposed new value.
    #
    method isFloatNonstrict {option value} {
        if {![string is double $value]} {
            error "$option expects a real value or an empty string got: '$value'"
        }
    }
    ##
    # isFloat
    #
    #  Same as above but strict validation
    #
    # @param option - option being modified.
    # @param value  - proposed new value.
    #
    method isFloat {option value} {
        if {![string is double -strict $value]} {
            error "$option value is not a valid real: '$value'"
        }
    }
    ##
    # isCoordinates
    #
    #  Validate that coordinate values are of the form @x,y where x and y
    #  are both integers.
    #
    # @param option - option being tested (probably -position)
    # @param value  - proposed new value.
    #
    method isCoordinates {option value} {
        set items [scan $value @%d,%d x y]
        if {$items != 2} {
            error "$option requires coordinates of the form @x,y got: '$value'"
        }
    }
    ##
    # isColor
    #   validate that an option is being given a valid color.
    # @param option - option being tested (probably -position)
    # @param value  - proposed new value.
    #
    method isColor {option value} {
        if {$value eq ""} {
            return ;      #transparent.
        }
        if {[catch {winfo rgb . $value} msg] } {
            error "$option requires a valid color : $msg"
        }
    }

        
}

##
# @class blt::component::axis
#
#  Internal type that encapsulates axis components and their options.
#
snit::type blt::component::axis {
    component validations
    
    option -plot;       # plot object
    option -which;      # Which axis (xaxis yaxis).
    
    # BLT axis options:
    
    option -autorange -validatemethod _Float -default 0.0
    option -color     -configuremethod _SetColor -default black
    option -command;            # unimplemented.
    option -descending;         # Unimplemented just use -max < -min.
    option -hide      \
        -configuremethod _SetHide -default false -validatemethod _Boolean
    option -justify;            # unimplemented
    option -limits;             # unimplemented.
    option -linewidth -configuremethod _SetWidth
    option -logscale;           # Not initially supported.
    option -loose;              # Not initially supported.
    option -majorticks;         # not supported.
    option -max   -validatemethod _FloatNonstrict -configuremethod _SetOpt \
                    -default ""
    option -min   -validatemethod _FloatNonstrict -configuremethod _SetOpt \
                    -default ""
    option -minorticks;          # Not implemented.
    option -rotate;              # Not implemented.
    option -shiftby  -validatemethod _Float
    option -showticks -default true -validatemethod _Boolean -configuremethod _SetOpt
    option -stepsize  -validatemethod _Float -default 0.0 -configuremethod _SetOpt
    option -subdivisions;        # unimplemented
    option -tickfont;            # Unimplemented.
    option -ticklength;          # unimplemented.
    option -title    -configuremethod _SetTitle
    option -titlecolor;          # unimplemented
    option -titlefont;           # unimplemented
    

    #  Validation delegations:
    
    delegate method _Float to validations   as isFloat
    delegate method _Boolean to validations as isBoolean
    delegate method _FloatNonstrict to validations as isFloatNonstrict
    
    #-------------------------------------------------------------------------
    #
    #  Construction
    #
    constructor args {
        install validations using blt::option::validators %AUTO%
        $self configurelist $args
    }
    #--------------------------------------------------------------------------
    # Public methods
    #
    
    ##
    # transform
    #   Transform real coordinates top pixels along the axis.
    #
    # @param coord - Real coordinate value.
    #
    # @return integer - corresponding pixel value
    #
    method transform coord {
        set canvas [$options(-plot) canvas]
        if {$options(-which) eq "xaxis"} {
            set coords [list $coord 0]
        } else {
            set coords [list 0 $coord]
        }
        set pixels [::Plotchart::coordsToPixel $canvas {*}$coords]

        if {$options(-which) eq "xaxis"} {
            set idx 0
        } else {
            set idx 1
        }
        return [lindex $pixels $idx]
    }
    
    ##
    # invtransform
    #
    #   Transform a pixel position along the axis into a
    #   world coordinate value.
    #
    # @param value - pixel along the axis.
    #
    # @return float real coordinates.
    method invtransform value {
        set canvas [$options(-plot) canvas]
        if {$options(-which) eq "xaxis"} {
            set pixels [list $value 0]
        } else {
            set pixels [list 0 $value]
        }
        set coords [::Plotchart::pixelToCoords $canvas {*}$pixels]
        
        if {$options(-which) eq "xaxis"} {
            set idx 0
        } else {
            set idx 1
        }
        return [lindex $coords $idx]
    }
    
    ##
    # limits
    #
    #  For now just return -min/-max.
    #
    #  @return two element list of min max
    #
    method limits {} {
       
        # Figure out the limits on the data:
        
        set range [$options(-plot) dataRange]
        set low [lindex $range 0]
        set hi  [lindex $range 1]
        if {$options(-which) eq "xaxis"} {
            set low [lindex $low 0]
            set hi [lindex $hi 0]
        } else {
            set low [lindex $low 1]
            set hi  [lindex $hi 1]
        }

        #
        #  Now set min from auto... if it's 0.0 it becomes low.  regardless,
        #  max comes from hi:
        
        set min $options(-autorange)
        set max $hi
        if {$min == 0.0} {
            set min $low
        }
        # Override if -min/-max are set:

        if {$options(-min) ne ""} {
            set min $options(-min)      
        }
        if {$options(-max) ne ""} {
            set max $options(-max)
        }
        return [list $min $max]
    }
    
    
    
    ##
    # rescale
    #
    #   Given limits of data and the axis specification options, this
    #   method re-specifies the axis.  Note that this will clear the plot
    #   of data.  Typically this is therefore called just prior to a re-plot
    #   of the data.
    #
    # @param low  Data low limit.
    # @param hi   Data Hi limit.
    #
    method rescale {low hi} {
        
        # First look at -autorange.   If 0.0 the data determine the
        # range.. otherwise the axis limits initialize to $low -autoscale.
        #
        set auto $options(-autorange)
        if {$auto == 0.0} {
            set axisLow $low
            set axisHigh $hi
        } else {
            set axisLow $low
            set axisHigh $auto
        }
        
        #  -minand -max override all of this if not "".
        
        if {$options(-min) ne ""} {
            set axisLow $options(-min)
        }
        if {$options(-max) ne ""} {
            set axisHigh $options(-max)
        }
        # Stepsize comes from stepsize..if zero; we'll just put in a tick at the
        # end of the axis
        
        set stepsize $options(-stepsize)
        if {($stepsize == 0.0) || ($stepsize > ($axisHigh - $axisLow))} {
            set stepsize [expr ($axisHigh - $axisLow)]
        }
        
        # ticklines comes from -showticks
        
  
        set showticks [expr {$options(-showticks) ? 1 : 0}]
        
        $options(-plot) dataChanged ""
    }
    
    
    #--------------------------------------------------------------------------
    #
    #  Option configuration methods.
    #
    ##
    # _SetOpt
    #
    #   Seems like -validatemethods inhibit setting the options element so
    #   We also requires those to use a configuremethod:
    #
    # @param option - option name.
    # @param value  - New value.
    #
    method _SetOpt {option value} {
        set options($option) $value
        $options(-plot) dataChanged ""
    }
    ##
    # _SetColor
    #
    #   Called when the -color attribute is set.
    #
    # @param option - should be -color
    # @param value  - New value for the color.
    #
    #
    method _SetColor {option value} {
        set options($option) $value;      # Now cget works.
        
        set canvas [$options(-plot) canvas]
        
        # Only change the color if the axis is not hidden:
        
        if {!$options(-hide)} {
            $canvas itemconfigure $options(-which)  -fill $value
        }
    }
    
    ##
    # _SetHide
    #
    #   Process the configuration of -hide.
    #   if this value is set to a boolean true, the axes are hidden
    #   by setting their color to the canvas background color, otherwise
    #   the axis color is set to the -color option value.
    #
    #  @param option - name of option (-hide).
    #  @param value  - New value.
    #
    method _SetHide {option value} {
        set options($option) $value;    # so cget works.
        
        set canvas [$options(-plot) canvas]
        
        # Figure out what the  new color should be:
        
        if {$value} {
            set newColor [$canvas cget -background]
        } else {
            set newColor $options(-color)
        }
        #  Set the new color
        
        $canvas itemconfigure $options(-which)  -fill $newColor
    }
    ##
    # _SetTitle
    #
    #   Process the configuration of the -title option.
    #
    # @param option - The option (-title).
    # @param value  - Text of the new title.
    #
    method _SetTitle {option value} {
        set options($option) $value
        set plot [$options(-plot) getPlotId]
        
        # Compute the method to use based on the -which option:
        
        if {$options(-which) eq "xaxis"} {
            set method xtext
        } else {
            set method ytext
        }
        $plot $method $value
    }
    
    ##
    # _SetWidth
    #
    #   Set the width of the axis.
    #
    #  @param option - name of option (-hide).
    #  @param value  - New value.
    #
    method _SetWidth {option value} {
        set options($option) $value
        
        set canvas [$options(-plot) canvas]
        $canvas itemconfigure $options(-which) -width $value
    }
    
}

##
# @class blt::component::crosshairs
#
#  Draws and maintains a cross hair cursor across a canvas.
#
snit::type blt::component::crosshairs {
    component validators
    
    variable xHairId -1;               # vertical crosshair id.
    variable yHairId -1;               # horizontal Cross hairs item id.
    variable script;                   # Motion script
    
    option -canvas;                    # Canvas on which the cross hairs are displayed.
    
    # BLT Options for a cross hair:
    
    option -color -default black -configuremethod _SetColor
    option -dashes -configuremethod _SetDash
    option -hide   -default true -validatemethod _Boolean -configuremethod _Hide
    option -linewidth -default 1 -configuremethod _SetWidth
    option -position \
        -default @0,0 -configuremethod _SetPosition -validatemethod _Coords
    
    # method delegations:
    
    delegate method _Boolean to validators as isBoolean
    delegate method _Coords  to validators as isCoordinates
    
    #-------------------------------------------------------------------------
    # canonicals
    
    constructor args {
        install validators using blt::option::validators %AUTO%
        $self configurelist $args
    }
    #-----------------------------------------------------------------------
    # public entries:
    #
    
    ##
    # off
    #
    #    Disables the display of the cross hairs.  This is the same as
    #    configure -hide true
    #
    method off {} {
        $self configure -hide true
    }
    ##
    # on
    #
    #  Enables the display of the crosshairs.  This is identical to
    #  configure -hide false.
    #
    method on {} {
        $self configure -hide false
    }
    ##
    # toggle
    #
    #   Changes the state of the cross hairs.
    #
    method toggle {} {
        set state $options(-hide)
        set state [expr {!$state}]
        
        $self configure -hide $state
    }
    #-------------------------------------------------------------------------
    #  configure methods.
    #
    
    ##
    # _SetColor
    #
    #  Changes the color of the cross hairs component.
    #
    # @param option - option name (-color)
    # @param color - a valid color selector.
    #
    method _SetColor {option color} {
        set options($option) $color
        
        # only set the color if the cross hair line exist
        
        if {$xHairId != -1} {
            set canvas $options(-canvas)
            $canvas itemconfigure $xHairId -fill $color
            $canvas itemconfigure $yHairId -fill $color
        }
    }
    ##
    # _SetDash
    #
    # Set the crosshairs dahsed line rendition.
    #
    # @param option - option name.
    # @param value  - Dash pattern.
    #
    method _SetDash {option value} {
        set options($option) $value
        
        # Only set dashes if the lines are drawn/visible.
    
    
        if {$xHairId != -1} {
            set canvas $options(-canvas)
            $canvas itemconfigure $xHairId -dash $value
            $canvas itemconfigure $yHairId -dash $value
        }
    }
    
    ##
    # _Hide
    #   Turn on/off the cross hair. This involves drawing the crosshairs
    #   a the the current position but using a <Motion> binding to make the
    #   crosshairs  intersection follow the mouse.
    #
    # @param option - The option
    # @param avalue - The value.
    #
    method _Hide {option value} {
        set prior $options($option)
        set value [expr {$value ? true : false}]
        set options($option) $value
        

        
        #  Only do stuff if the states don't match:
        
        if {$prior != $value} {
            
            set c $options(-canvas)
            if {!$value} {
                #
                #  Draw the lines as indicated by -position
                #  then add the motion handler and figure that there'll be some
                #  motion if the  mouse is over the line.
                
                set coords [$self _GetPosition]
                set extent [$self _GetExtent]
                set x0 [lindex $coords 0]
                set y0 [lindex $coords 1]
                set x1 [lindex $extent 0]
                set y1 [lindex $extent 1]
                
                set xHairId \
                    [$c create line $x0 0 $x0 $y1 \
                     -fill $options(-color) -dash $options(-dashes) -width $options(-linewidth)]
                set yHairId \
                    [$c create line 0 $y0 $x1 $y0 \
                     -fill $options(-color) -dash $options(-dashes) -width $options(-linewidth)]
                
                $self _AddMotionHandler
                
            } else {
                $c delete $xHairId
                $c delete $yHairId
                set xHairId -1
                set yHairid -1
                
                $self _RemoveMotionHandler
            }
        }
    }
    
    ##
    # _SetWidth
    #
    #   Process the configure -linewidth operation.  If the crosshairs
    #   are drawn, this itemconfigures them.
    #
    # @param option - -linewidth
    # @param value  - new line width value
    #
    method _SetWidth  {option value} {
        set options($option) $value
        
        if {$xHairId != -1} {
            set c $options(-canvas)
            
            $c itemconfigure $xHairId -width $value
            $c itemconfigure $yHairId -width $value
        }
        
    }

    ##
    # _SetPosition
    #
    #  Modify the position of the crosshairs.
    #
    # @param option - -position
    # @param value  - New coordinates in @x,y format.
    #
    method _SetPosition {option value} {
        set options($option) $value

        if {$xHairId != -1} {
           set c $options(-canvas)
            
            set coords [$self _GetPosition]
            set extent [$self _GetExtent]
            set x0 [lindex $coords 0]
            set y0 [lindex $coords 1]
            set x1 [lindex $extent 0]
            set y1 [lindex $extent 1]
            
            $c coords $xHairId $x0 0 $x0 $y1
            $c coords $yHairId 0 $y0 $x1 $y0
            
        }
        
    }

    #-------------------------------------------------------------------------
    # Private utilities
    #
    
    
    ##
    #  _AddMotionHandler
    #
    #  Adds a mouse move handler to the set of canvas bindings.
    #  The handler is addtitive so that existing motion handlers are not
    #  disturbed
    #
    method _AddMotionHandler {} {
        set script [mymethod _Moved %x %y]
        bind $options(-canvas) <Motion> +$script
        
    }
    ##
    # _RemoveMotionHandler
    #
    #    Removes only our motion binding from the canvas.  This is done by
    #    listing the bindings, removing the one that matches us and
    #    restablishing the remaining bindings.
    method _RemoveMotionHandler {} {
        set bindings [bind $options(-canvas) <Motion>]
        
        set ourBinding [lsearch -exact $bindings $script]
        
        # Defensive programming: remove our binding if found.
        
        if {$ourBinding != -1} {
            set bindings [lreplace $bindings $ourBinding $ourBinding]
        }
        # Now re-establish the other bindings:
        
        foreach binding $bindings {
            bind $options(-canvas) <Motion> +$binding
        }
    }
    
    ##
    # _GetPosition
    #
    #  Return the value of -position as an x/y coordinat list.
    #
    # @return 2 element list of x y
    #
    # @note we assume the validation has done its work and the format of the
    #       position is correct.
    method _GetPosition {} {
        scan $options(-position) @%d,%d x y
        return [list $x $y]
    }
    
    ##
    # _GetExtent
    #
    # Return the x/y extent of the canvas in pixels.
    # Makes the assumption that the canvas cget for -height and -width returns
    # pixels (seems to be true as of 8.6)
    #
    # @return 2 element list of x/y extent.
    #
    method _GetExtent {} {
        set c $options(-canvas)
        
        set xExtent [$c cget -width]
        set yExtent [$c cget -height]
        
        return [list $xExtent $yExtent]
    }
    
    ##
    # _Moved
    #
    #   Mouse motion handler, just configure -position
    #
    #  @param x - new x position in the widget.
    #  @param y - new y position in the widget.
    method _Moved {x y} {
        $self configure -position @$x,$y
    }
}
##
# @class blt::component::element
#
#  Data series actually.
#
snit::type blt::component::element {
    component validators
    
    option -chart;                   # Plotchart chart.
    option -name;                    # Series name.
    
    option -activepen;               # Not supported yet.
    option -color     -default black  -configuremethod _Reconfigure
    option -dashes;                  # Not supported.
    option -data      -default [list] -configuremethod _Changed
    option -fill;                   # not supported.
    option -hide      -default no -validatemethod _Boolean -configuremethod _Changed
    option -label;                 # Not supported.
    option -linewidth -default 0 -validatemethod _Integer -configuremethod _Reconfigure
    option -mapx      -default x;   # Not supported.
    option -mapy      -default y;   # Not suported.
    option -offdash;               # Not supported
    option -outline;               # not supported
    option -outlinewidth;          # not supported.
    option -pixels;                # not supported
    option -scalesymbols;          # Not supported
    option -smooth;                # Not supported.
    option -styles;                # Not supported.
    option -symbol;                # Not supported (yet)
    option -weights;               # Not supported (yet)
    option -xdata     -configuremethod _WatchVector -default ""
    option -ydata     -configuremethod  _WatchVector -default ""
    
    variable lastXvectorValues [list]
    variable lastYvectorValues [list]
    
    # Maps option -> canvas item map.
    
    variable optionCanvasMap -array {
        -color     -fill
        -linewidth -width
    }
    
    # Maps option -> Plotchart series options:
    
    variable optionPlotMap -array {
        -color -color
        -linewidth -width
    }
    
    
    delegate method _Boolean to validators as isBoolean
    delegate method _Integer to validators as isInteger
    
    #--------------------------------------------------------------------------
    # Canonicals.
    #
    
    ##
    # Constructor
    #
    # @param args - option value pairs.
    #
    constructor args {
        install validators using blt::option::validators %AUTO%
        $self configurelist $args
    }

    
    #-------------------------------------------------------------------------
    #
    #    Public methods:
    
    ##
    # activate - not supported.
    #
    method activate args {} 
    
    ##
    # closest - not supported:
    #
    method closest args {}
    
    ##
    # deactivate - not supported.
    #
    method deactivate args {}
    
    ##
    # show not supported
    #
    method show args {} 
    
    ##
    # type
    #   Returns line
    #
    method type args {
        return "line"
    }

    ##
    # getData
    #
    #   Return the current data as a pair of lists, x data and ydata.
    #   If both x and y vectors were established we return those otherwise,
    #   the -data are unentangled into the correct format.
    #
    # @return 2 element list containing in the first element the set of x points
    #         and the set of y points in the second element.
    #
    # @note the result could be a pair of empty lists.
    method getData {} {
        if {!$options(-hide)} {
            if {($options(-xdata) ne "") && ($options(-ydata) ne "")} {
                if {([llength $lastXvectorValues ] > 0) && ([llength $lastYvectorValues] >0)} {
                    return [list $lastXvectorValues $lastYvectorValues]
                } else {
                    return [list [list] [list]]
                }
            } else {
                set xcoords [list]
                set ycoords [list]
                foreach {x y} $options(-data) {
                    lappend xcoords $x
                    lappend ycoords $y
                }
                return [list $xcoords $ycoords]
            }
        } else {
            # Hidden:
            return [list [list] [list]];   # Return no points.
        }
    }
    ##
    # configureSeries
    #
    #  Called once our element has been plotted to configure the visual
    #  appearance of the series represented by this element.
    #
    #  @param plotId - Plotchart id... saves a call back to get it.
    #  @param name   - The plot's idea of our name.
    #
    # @note - This is a bit dirty because plotchart does not let us
    #         configure prior bits of a series.  We need to
    #         * Configure the series so future points are right
    #         * Configure items with the tag data_$name so past
    #           items are right.
    #
    method configureSeries {plotId name} {
        if {$options(-hide)} return
        
        # Configure the series:
        
        foreach option [array names optionPlotMap] {
           
            $plotId dataconfig $name $optionPlotMap($option) $options($option)
        }
        #  Configure the existing items:
        
        set tag data_$name
        set c [$plotId canvas]
        foreach option [array names optionCanvasMap] {
           
            $c itemconfigure $tag $optionCanvasMap($option) $options($option)  
        }
    }
    #-------------------------------------------------------------------------
    #  Configuration methods
    #
    
    ##
    # _Changed
    #
    #   Handler for any option that requires  a redraw of the data.
    #
    # @param option - the option that changed.
    # @param value  - New value of the option.
    #
    method _Changed {option value} {
        
        set options($option) $value
        $options(-chart) dataChanged $options(-name);    # Notify the plot.
    }
    ##
    # _Reconfigure
    #
    #  Handler for any option that requires a change of the series configuration.
    #  just fake up a call to configureSeries:
    #
    # @param option - the option being configured.
    # @param value  - New value.
    #
    method _Reconfigure {option value} {
        set options($option) $value
        
        set id [$options(-chart) getPlotId]
        
        $self configureSeries $id $options(-name)
    }
    ##
    # _WatchVector
    #
    #   Called when an xdata or ydata vector have been changed.
    #   *  If there's a prior vector, get rid of the watch on it.
    #   *  Get the data for the new vector.
    #   *  Set a watch on its changes.
    #   *  Force a redraw.
    #
    # @param option - the option being configured (-xdata or -ydata).
    # @param value  - new vector name.
    #
    # @note - this is one of the fiew cases where options($option) is not
    #         immediately set.
    #
    method _WatchVector {option value} {
        #
        #  Get the name of the variable that will hold the vector data:
        
        if {$option eq "-xdata"} {
            set varname lastXvectorValues 
            
        } elseif {$option eq "-ydata"} {
            set varname lastYvectorValues 
            
        } else {
            error "Internal - element::_WatchVector called with $option"
        }
        #
        # get rid of the watch on the old vector
        #
        if {$options($option) ne ""} {
            $options($option) removetrace [mymethod _VectorChanged $option $varname]
        }
        #  Save the new vector in the option, set up a new trace and get the
        #  current set of values:
        
        set options($option) $value
        $value addtrace [mymethod _VectorChanged $option $varname]
        set $varname [$value range]
        
        #  Force the redraw
        
        $options(-chart) dataChanged $options(-name)
    }
    #--------------------------------------------------------------------------
    #  Private utilities:
    #
    
    ##
    # _VectorChanged
    #
    #   Called when one of the data vectors (-xdata or -ydata) changed.
    #   this is called by the vector itself via the -tracewrite option.
    #
    # @param whichopt - Either -xdata or -ydata lets us pick up the vector name.
    # @param whichvar - one of the last*vectorValues .. where we put the data
    #                   from the vector.
    # @param vectorname - Passed in by caller, name of the vector object
    method _VectorChanged {whichopt whichvar vectorname} {
        set $whichvar [$options($whichopt) range]
        $options(-chart) dataChanged $options(-name)
    }
    
}

##
# @class blt::component::grid
#
#  The stuff we can do in the grid component is done here.
snit::type blt::component::grid {
    option -plot;                       # Plotchart id.
    
    # BLT Options:
    
    option -color black
    option -dashes
    option -hide
    option -linewidth
    option -mapx
    option -mapy
    option -minor
}

##
# @class blt::component::legend
#
#   Legend component mappings.
#
snit::type blt::component::legend {
    component validations
    
    option -plot;                    # Graph object
    
    option -activebackground;                      # Unimplemented.
    option -activeborderwidth;                     # Unimplemented.
    option -activeforeground;                      # Unimplemented.
    option -activerelief;                          # Unimplemented.
    option -anchor;                                # Unimplemented see -position
    option -background -default "" -validatemethod _Color -configuremethod _SetBackground
    option -borderwidth;                           # Unimplemented
    option -font       -default " *-Helvetica-Bold-R-Normal-*-10-100-*" -configuremethod _SetFont
    option -foreground;                            # Unimplemented
    option -hide  -default no -validatemethod _Boolean -configuremethod _SetHide    
    option -ipadx;                                 # Unimplemented
    option -ipady;                                 # Unimplemented.
    option -padx;                                  # Unimplemented
    option -pady;                                  # Unimplemented
    option -position -default top-right -validatemethod _Position -configuremethod _SetPosition
    option -raised;                                # Unimplemented
    option -relief;                                # Unimplemented
    
    #  Delegate option check methods:
    
    delegate method _Color to validations as isColor
    delegate method _Boolean to validations as isBoolean

    
    # Active entry items is an array to allow [array names] glob matching
    # and O(1) manipulation of the contents.
    
    variable legendItems -array [list];            # Currently active legend items.
    
    #---------------------------------------------------------------------------
    # Canonicals
    
    ##
    # constructor - just set the options:
    #
    constructor args {
        install validations using blt::option::validators %AUTO%
        $self configurelist $args
    }
    
    
    #--------------------------------------------------------------------------
    # Public methods:
    #
    
    ##
    # activate
    #
    #   Add items to the legend.
    #
    # @param pattern - glob pattern of the elements to add.
    #
    method activate pattern {

        set entries [$options(-plot) matchingElements $pattern]
        set plotId  [$options(-plot) getPlotId]
        
        foreach entry $entries {
            if {!$options(-hide)} {
                $plotId legend $entry $entry
            }
            set legendItems($entry) $entry  
        }
    }
    ##
    # deactivate
    #
    #  Remove a set of items from the legend.
    #
    method deactivate pattern {
        set entries [$options(-plot) matchingElements $pattern]
        set plotId  [$options(-plot) getPlotId]
        
        foreach entry $entries {
            if {[array names legendItems $entry] ne ""} {
                if {!$options(-hide)} {
                    catch {$plotId removefromlegend $entry}; # Plotchart bug on last delete.
                }
                unset legendItems($entry)
            }
        }
    }
    ##
    # get - unimplemented.
    #
    method get args {
        error "legend::get is unimplemented"
    }
    
    ##
    # recreate
    #   Called by the plot to recreated the legend on a total recreation:
    #
    method recreate {} {
        if {!$options(-hide) && ([llength [array names legendItems]] > 0)} {
            
            set plotId [$options(-plot) getPlotId]
            # Process the options:
            
            $plotId legendconfig \
                -background $options(-background) -font $options(-font) \
                -position $options(-position)
           
            # Put items in the legend
            
            foreach name [lsort [array names legendItems]] {
                $plotId legend $name $name
            }
 
        }
    }
    #-------------------------------------------------------------------------
    # Private methods:
    #
    
    ##
    # Kill off all elements in the legend.  This followed by recreate seems the
    # only way to get options paid attention to.
    #
    method _kill {} {
        if {!$options(-hide)} {
            set plotId [$options(-plot) getPlotId]
            foreach name [array names legendItems] {
                catch  {$plotId removefromlegend $name}; # Plotchart bug on last remove
            }
        }
    }
    #--------------------------------------------------------------------------
    #  Option configuration methods.
    
    ##
    # _SetBackground
    #
    #  Process changes to the -background option.  If not hidden,
    #  the legend is reconfigured.
    #
    # @param option -name of the option (-background)
    # @param value  -new color (validated already).
    #
    method _SetBackground {option value} {
        set options($option) $value
        
        if {!$options(-hide)} {
            set plotId [$options(-plot) getPlotId]
            $plotId legendconfig -background $value
            
            $self _kill
            $self recreate
        }
    }
    ##
    # _SetFont
    #
    #   Process a change in the requested legend font.
    #
    # @param option -name of the option (-font)
    # @param value  -new font.
    #
    method _SetFont {option value} {
        set options($option) $value
        
        if {!$options(-hide)} {
            set plotId [$options(-plot getPlotId]
            $plotId legendconfig -font $value
            $self _kill
            $self recreate

        }
    }
    ##
    # _SetHide
    #
    #  Only takes action if something changed.
    #  If -hide -> true, then remvoe all legend elements and hope that makes
    #  the legend disappear.
    #  If -hide -> false, then call recreate to recreate the legend.
    #
    # @param option -the option name (-hide)
    # @param value  -The new option value, which has been validated as a boolean.
    #
    method _SetHide {option value} {
        set redraw [expr bool($options($option)) != bool($value)]
        
        if {$redraw} {
            if {!$value} {
                set options($option) [expr bool($value)]
                $self recreate;           #Unhiding
            } else {
              
              # hiding
              
                $self _kill
                 set options($option) [expr bool($value)]
            }
        }
        
    }
    ##
    # _SetPosition
    #
    #  Set the legend position on the plot.
    #
    # @param option - the option name (-position)
    # @param value  - validated option value.
    method _SetPosition {option value} {
        set options($option) $value
        
        if {!$options(-hide)} {
            set plotId [$options(-plot) getPlotId]
            $plotId legendconfig -position $value
            $self _kill
            $self recreate

        }
    }
    #-------------------------------------------------------------------------
    # Option validators
    #
    
    ##
    # _Position
    #   Ensures a value is a correct plotchart position... note that we don't
    #   attempt to use blt legend positions...so this is an incompatiblity.
    #
    # @param option - option being configured (don't care).
    # @param position - legend position.
    #
    method _Position {option position} {
        
        set validPositions [list top-left top-right bottom-left bottom-right]
        if {$position ni $validPositions} {
            error "$option requires a valid position ($validPositions) got '$position'"
        }
    }
}
##
# @class blt::component::pen
#
#  Pens are bundles of stlyes that can be attached to data series (elements).
#
snit::type  blt::component::pen {
    option -color        black
    option -dashes       ""
    option -fill         defcolor
    option -linewidth    0
    option -offdash      defcolor
    option -outline      defcolor
    option -outlinewidth 1
    option -pixels       0.125i
    option -symbol       circle
    option -type         line
    
}

##
# @class blt::stripchart
#
#  Subset implementation of the blt stripchart widget.   This is a graphical
#  widget that provides a rolling strip chart.
#
#
snit::widgetadaptor ::blt::stripchart {
    component stripchart;        # Really the canvas containing the plotchart item
    
    # Axis components.
    
    component x
    component y
    component crosshairs
    component grid
    component legend
    
    
    ##
    #  These are the options of supported by BLT
    #  We'll attach configuremethods to those we can support.
    #  The remainder are all going to remain here so that scripts won't
    #  outright crash.
    #
    option -bottommargin
    option -bufferelements
    option -buffergraph
    option -font
    option -halo
    option -invertxy
    option -justify
    option -leftmargin
    option -plotbackground
    option -plotborderwidth
    option -plotpadx
    option -plotpady
    option -plotrelief
    option -rightmargin
    option -tile
    option -title
    option -topmargin
    
   # Option delegations
   
   delegate option -background  to stripchart
   delegate option -borderwidth to stripchart
   delegate option -cursor      to stripchart
   delegate option -height      to hull
   delegate option -relief      to stripchart
   delegate option -takefocus   to stripchart
   delegate option -width       to hull
   
   

   variable axisComponents -array {
    
   }
   variable elements -array {
    
   }
   
   variable plotId  [list]


    ##
    # constructor
    #   - Create a canvas,
    #   - Create a stripchart in the canvas.
    #   - Create/install all of the components we need to get started.
    #   - Process the option database.
    #
    # @param args - the options and their values for the initial configuration.
    #
    constructor args {
        installhull using ttk::frame
        install stripchart using canvas $win.plot
        
        # Create the plotchart stripchart with provisional values for the
        # axis:
        
        set plotId [::Plotchart::createStripchart $stripchart {0 1 .5} {0 1 .5}]
        
        # Install the components:
        
        #             Axes:
        
        install x using ::blt::component::axis %AUTO% -plot $self -which xaxis
        install y using ::blt::component::axis %AUTO% -plot $self -which yaxis
        
        array set axisComponents [list x $x y $y x1 $x y1 $y]
        
        #           Crosshairs:
        
        install crosshairs using ::blt::component::crosshairs \
            %AUTO% -canvas $stripchart
        
        #        Legend:
        
        install legend using ::blt::component::legend %AUTO% -plot $self
        
        # Grid
        
        install grid using ::blt::component::grid %AUTO% -plot $self
        
        #  Configure and paste the widget:
        
        $self configurelist $args
        set plotId [::Plotchart::createStripchart $stripchart {0 1 .5} {0 1 .5}]
 
        pack $stripchart -fill both -expand 1
        
    }
    
    #---------------------------------------------------------------------------
    #
    # Public methods.
    #
    #------------------------------------------------------------------------
    #
    # Component callbacks.
    #
    
    ##
    # matchingElements
    #   Names of the elements that match a glob pattern.
    #
    # @param pattern - the pattern to match
    #
    # @return a possibily empty list of element names.
    #
    method matchingElements pattern {
        return [array names elements $pattern]    
    }
    
    ##
    # dataRange
    #   Return the range of x/y values.
    #
    # @return two element list of {xmin ymin} {xmax ymax}
    #
    method dataRange {} {
        set minx ""
        set maxx ""
        
        set miny ""
        set maxy ""
        foreach name [array names elements] {
            set data [$elements($name) getData]
            set x [lindex $data 0]
            set y [lindex $data 1]
            
            # Series could be empty:
            
            if {[llength $x] > 0} {
                if {[llength $x] > 1} {
                    set x [join $x ,]
                    set max [expr max($x)]
                    set min [expr min($x)]
                } else {
                    set max $x
                    set min $x
                }
                if {$minx eq ""} {
                    set minx $min
                } else {
                    set minx [expr {min($minx, $min)}]
                }
                if {$maxx eq ""} {
                    set maxx $max
                } else {
                    set maxx [expr {max($maxx, $max)}]
                }
            }
            if {[llength y] > 0} {
                if {[llength $y] > 1} {
                    set y [join $y ,]
                    set max [expr max($y)]
                    set min [expr min($y)]
                } else {
                    set max $y
                    set min $y
                }
                
                if {$miny eq ""} {
                    set miny $min
                } else {
                    set miny [expr {min($miny, $min)}]
                }
                if {$maxy eq ""} {
                    set maxy $max
                } else {
                    set maxy [expr {max($maxy, $max)}]
                }
            }
            
        }
        # Final thing.. if there is no min/max..just put in 0, 10 for both:
        
        if {$minx eq ""} {
            set minx 0.0
            set maxx 10.0
        }
        if {$miny eq ""} {
            set miny 0.0
            set maxy 10.0
        }
        
        return [list [list $minx $miny] [list $maxx $maxy]]
    }    
    ##
    # dataChanged
    #   Called to indicate something has changed with the data.
    #   Our current strategy is a bit brutal:
    #   Kill all the items on the canvas,
    #   Create a new plot based on the shape of the axes
    #   replot all data elements.
    #
    #  @param name - name of data item that changed "" if all changed.
    #                really we treat it as if all changed.
    #
    method dataChanged name {
        $stripchart delete all
        puts "Dimensions:"
        puts [$stripchart cget -height]
        puts [$stripchart cget -width]
        
        set xAxis $axisComponents(x)
        set yAxis $axisComponents(y)
        
        # Get the new axis limits in case that changed:
    

        set xLimits [$xAxis limits]
        set x1 [lindex $xLimits 0]
        set x2 [lindex $xLimits 1]
        if {$x1 == $x2} {
            set x2 [expr $x1 + 1.0]
        }
        set xLimits [list $x1 $x2]
        
        set step [$xAxis cget -stepsize]
        if {$step == 0} {
            set step [expr ceil([lindex $xLimits 1])]
            if {$step == 0} {
                set step 1
            }
        }
        lappend xLimits $step
        puts "xlimits $xLimits"
        
        set yLimits [$yAxis limits]
        set y1 [lindex $yLimits 0]
        set y2 [lindex $yLimits 1]
        if {$y1 == $y2} {
            set y2 [expr $y2 + 1]
        }
        set yLimits [list $y1 $y2]
        set step [$yAxis cget -stepsize]
        if {$step == 0} {
            set step [expr ceil([lindex $yLimits 1])]
            if {$step == 0} {
                set step 1
            }
        }
        lappend yLimits $step
        puts "yLimits $yLimits"

        $plotId deletedata
        
        
        #  Now plot each series:
        
        foreach name [array names elements] {
            set element $elements($name)
            set points [$element getData]
            $element configureSeries $plotId $name
            
            foreach x [lindex $points 0] y [lindex $points 1] {
                puts "$name: ($x,$y)"
                $plotId plot $name $x $y
            }
            
            
        }
        $legend recreate
        
    }
    
    ##
    # axis
    #
    #  Performs operations on axis components.
    #
    # @param operation - The axis operation to perform.
    # @param name      - name of the axis to affect.
    # @param args      - Remainder of the args.
    #
    method axis {operation name args} {
        # Note that create is a special (NO-OP ) case.  Everything else just
        # looks up the axis object and performs the operation on it.
        
        if {$operation eq "delete"} {
            error "Axis deletion is not supported"
        } elseif {$operation eq "names"} {
            set patterns "*"
            if {[llength $args] > 0} {
                set patterns $args
            }
            set result [list]
            foreach glob $patterns {
                set result [concat $result [array names axisComponents $glob]]
            }
            return $result
        }  elseif {$operation ne "create"} {
            set axis $axisComponents($name)
            $axis $operation {*}$args
        } else {
            error "Axis creation not supported"
        }
    }
    ##
    # bar
    #   Bar elements not supported.
    #
    method bar args {
        error "'bar' elements are nto supported."
    }
    ##
    # crosshairs
    #
    #   Manipulate the crosshairs component.
    #
    # @param op - operation.
    # @param args - arguments for the operation.
    #
    method crosshairs {op args} {
        $crosshairs $op {*}$args
    }
    ##
    # element
    #   Operations on elements.
    #
    #   Note that create is handled here.
    #
    # @param op - operation to performn
    # @param name - defines the name.
    # @param args - Other args.
    #
    method element {op name args} {
        
        if {$op eq "create"} {
            # We  auto name the element because names of objects must be
            # unique across all plots.
            #
            set elements($name) \
                [::blt::component::element create %AUTO% -name $name -chart $self {*}$args]
            $self dataChanged $name
        } elseif {$op eq "delete"} {
            set e $elements($name)
            $e destroy
            unset elements($name)
            $self dataChanged ""
        } elseif {$op eq "names"} {
            set patterns "*"
            if {[llength $args] > 0} {
                set patterns $args
            }
            set result [list]
            foreach pattern $patterns {
                lappend result {*}[array names elements $pattern]
            }
            return $result
        } elseif {$op eq "exists"} {
            return [expr [llength [array names elements $args]] != 0]
        } else {
            set e $elements($name)
            $e $op {*}$args
        }
    }
    ##
    # extents - currently not supported.
    #
    method extents args {
        error "The 'extents' method is not supported."
    
    }
    ##
    # grid
    #
    #  Operations on the grid component:
    #
    # @param op - the operation.
    # @param args - Remaining arguments.
    #
    method grid {op args} {
        $grid $op {*}$args
    }
    ##
    # invtransform
    #
    #  Convert pixels to world coordinates.
    #
    # @param x  - X pixel.
    # @param y  - Y pixel.
    #
    # @return 2 element list containing float x, y coordinates.
    #
    method invtransform {x y} {
        return Plotchart::pixelToCoords $stripchart $x $y 
    }
    ##
    # legend
    #
    #   Performs an operation on the legend component.
    #
    # @param op   - operatiuon to perform.
    # @param args - Parameters to send to the operation.
    #
    method legend {op args} {
        $legend $op {*}$args
    }
    ##
    # line
    #  Degenerates to element:
    #
    method line args {
        $self element {*}$args
    }
    ##
    # marker
    #  Not supported.
    #
    method marker args {
        error "The 'marker' component is not supported."
    }
    ##
    # metafile - unsupported
    #
    method metafile args {
        error "'metafile' method is not supported"
    }
    ##
    # postscript
    #   not supported.
    #
    method postscript args {
        error "'postscript' method not supported"
    }
    ##
    # snap
    #  Not supported.
    #
    #
    method snap args {
        error "'snap' method is not supported"
    }
    ##
    # transform
    #    WC->Pixel transformation.
    #
    # @param x - x coordinate to transform.
    # @param y - y coordinate to transform.
    #
    # @return  2 element list o pixel coords.
    #
    method transform {x y} {
        ::Plotchart::coordsToPixel $stripchart $x $y
    }
    ##
    #  The method below are actuall axis operations with fixed
    #  axis names.
    #
    method xaxis {op args} {
        $self axis $op x  {*}$args
    }
    method x2axis {op args} {
        $self axis $op x2 {*}$args
    }
    method yaxis {op args} {
        $self axis $op y {*}$args
    }
    method y2axis {op args} {
        $self axis $op y2 {*}$args
    }
    ##
    # getPlotId
    #
    #  Return plotchart plot id. 
    #
    method getPlotId {} {
        return $plotId
    }
    ##
    # canvas
    #
    #  Returnthe canvas.
    #
    method canvas {} {
        return $stripchart
    }
   
}
