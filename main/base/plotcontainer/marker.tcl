#******************************************************************************
#
# CAEN SpA - Software Division
# Via Vetraia, 11 - 55049 - Viareggio ITALY
# +390594388398 - www.caen.it
#
#***************************************************************************//**
# 

# @file   marker.tcl
# @brief  Plotchart marker objects
#

package provide Plotchart::marker 1.0
package require Plotchart 3.0.0;            # Includes the Plotchart:: namespace.
package require snit

##
# @class marker
#
#   Provides a pointing marker that can be put on plotchart
#   canvases
#   The marker is a triangle which can point any of
#   up, down, left, or right.
#
# OPTIONS
#  -plot  - Plotchart handle on which the marker should be displayed.
#  -size  - Size in pixels of the 'extent' of the marker.
#  -direction 'up', 'down', 'left', 'right' the orientation of the marker.
#  -color - Color of the marker.
#  -filled - True to fill the marker, false to draw the marker in outline only.
#            At this point in time there is no support for a separate fill
#            color from the outline.
# METHODS
#
#   drawAt  - Draw the marker with the point at the specified real coordinates.
#   moveTo  - Move the marker so that the point is at the specified real coords.
#   coords  - Returns the real coordinate position of the marker.
#
#
snit::type Plotchart::marker {
    option -plot      -default "" -readonly yes
    option -size      -default 5     -configuremethod _Redraw
    option -direction -default right -configuremethod _Redraw
    option -color     -default black -configuremethod _Redraw
    option -filled    -default 1     -configuremethod _Redraw
    
    # Keeps track of the last known location of the marker.
    # This allows the marker to be redrawn if its attributes change.
    #  A "" means the marker never was drawn.
    #
    variable pointerX  ""
    variable pointerY  ""
    
    variable markerId  "";       # Canvas Id of marker.
    
    ##
    # constructor
    #   Create the marker  We need to ensure that the plot
    #   was specified.
    #
    # @param args - the option value set.
    #
    constructor args {
        $self configurelist $args
        if {$options(-plot) eq ""} {
            error "-plot option required at construction time."
        }
    }
    ##
    # destructor
    #  Undraw the marker.
    #
    destructor {
        set canvas [$options(-plot) canvas]
        $canvas delete $markerId
    }
    ##
    # drawAt
    #
    #  Draw at the specified set of plot world coordinates.
    #
    # @param x - X world coordinate for the point.
    # @param y - Y world coordinate of the poiint.
    #
    # @note the pointerX and pointerY variables are updated with the
    #       position of the pointing part of the marker.
    #
    method drawAt {x y} {
        set pointerX $x
        set pointerY $y
        
        set coords [$self _getMarkerCoordinates]
        
        # Figure out the marker options:
        
        set cmdopts [list -outline $options(-color)]
        if {$options(-filled)} {
            lappend cmdopts -fill $options(-color)
        } else {
            lappend cmdopts -fill ""
        }
        # Now we can get the canvas and draw the marker:
        
        set canvas [$options(-plot) canvas]
        if {$markerId ne ""} {
            $canvas delete $markerId
        }
        
        set markerId [$canvas create polygon {*}$coords {*}$cmdopts]
    }
    ##
    # moveTo
    #   Move the marker pointer to the specified world coordinates.
    #
    # @param x  X coordinate of the point in world coordinates.
    # @param y  Y coordinate of the point in world coordinates.
    #
    method moveTo {x y} {
        
        set xyDeltas [$self _Delta $x $y]
        set canvas [$options(-plot) canvas]
        
        set pointerX $x
        set pointerY $y
        
        $canvas move $markerId [lindex $xyDeltas 0] [lindex $xyDeltas 1]
    }
    ##
    # coords
    #   Fetch the marker coordinates.
    #
    # @return list - [list $pointerX $pointerY]
    #
    method coords {} {
        return [list $pointerX $pointerY]
    }
    
    #--------------------------------------------------------------------------
    #
    #  Private methods.
    #
    
    ##
    # _getMarkerCoordinates
    #
    #  Returns the canvas coordinates of the marker.  These are the
    #  three points that define the marker triangle.  This method takes into
    #  account not only the pointerX and pointerY variables but also the
    # -direction option that determines where the other points are relative
    # to this one.
    #
    # @return List of coordinates that can be {*}'d into the canvas create
    #         command.
    #
    method _getMarkerCoordinates {} {
        set canvascoords [$self _Transform $pointerX $pointerY]
        set x [lindex $canvascoords 0]
        set y [lindex $canvascoords 1]
        
        lappend coords $x $y
        set size $options(-size)
        
        # Brute force for now...I suppose I could use an afine transform
        # to rotate the object from some reference position to the final position
        # but I'd still have to chose/calculate the matrix based on the
        # -direction anyway
        
        set direction $options(-direction)
        
        if {$direction eq "left"} {
            set rx [expr {$x + $size}]
            set uy [expr {$y + $size}]
            set ly [expr {$y - $size}]
            
            lappend coords $rx $uy $rx $ly
            
        } elseif {$direction eq "right"} {
            set lx [expr {$x - $size}]
            set uy [expr {$y + $size}]
            set ly [expr {$y - $size}]

            lappend coords $lx $uy $lx $ly
            
        } elseif {$direction eq "up"} {
            set ly [expr {$y + $size}];   #X11 coordinates.
            set lx [expr {$x - $size}]
            set rx [expr {$x + $size}]
            
            lappend coords $lx $ly $rx $ly
            
        } elseif {$direction eq "down"} {
            set uy [expr {$y - $size}];   #X11 coordinates.
            set lx [expr {$x - $size}]
            set rx [expr {$x + $size}]
            
            lappend coords $lx $uy $rx $uy
            
        } else {
            error "Invalid -direction value: $direction "
        }
        return $coords
    }
    ##
    # _Delta
    #
    #   Given a world coordinate x/y pair calculates the distance (dx,dy) that
    # this is from the current pointer position in canvas coordinates.
    # The canvas coordinates are truncated to integers so they can be used
    # directly with the canvas move subcommand.
    #
    # @param x - New X world coordinates.
    # @param y - New Y world coordinates.
    #
    # @return 2 element list containing the dx dy values.
    #
    method _Delta {x y} {
        set current [$self _Transform $pointerX $pointerY]
        set new     [$self _Transform $x $y]
        
        set dx [expr {([lindex $new 0] - [lindex $current 0])}]
        set dy [expr {([lindex $new 1] - [lindex $current 1])}]
                
        return [list $dx $dy]
    }
    ##
    # _Transform
    #
    #  Transform world coordinates to pixels.  The pixels are truncated to
    #  integers so that they can be directly used by canvas commands.
    #
    #  @param x - World coordinate x
    #  @param y - World coordinate y
    #
    # @return 2 element list of pixel x/y coordinates.
    #
    method _Transform {x y} {
        set c [$options(-plot) canvas]
        set pixels [Plotchart::coordsToPixel $c $x $y]
        set x [expr {int([lindex $pixels 0])}]
        set y [expr {int([lindex $pixels 1])}]
        
        return [list $x $y]
        
    }
    ##
    # _Redraw
    #    Called to reconfigure an option that would force a redraw of the marker.
    #
    #  @param  option  - Name of reconfigured option.
    #  @param  value   - New value of the option.
    #
    method _Redraw {option value} {
        set options($option) $value
        
        # If we already have an object it needs  to be redrawn with the new
        # configuration.  A redraw is the simplest way to do this given that the
        # -direction may change.
        
        if {$markerId ne ""} {
            $self drawAt $pointerX $pointerY
        }
        
    }
}