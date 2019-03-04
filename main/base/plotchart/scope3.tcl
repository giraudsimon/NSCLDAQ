#
#  Timing test of oscilloscope mode.
#  Four traces are plotted.
#  sin(x)
#  cos(x)
#  square wave
#  triangle wave
#
#  Each interval a random shift is put on the data points.
#
#  Interested to see how 'live' plotchart can do this.
#

source plotchart.tcl
package require Tk
package require Plotchart


# --- start patch --
# Patch: deletedata subcommand
#

namespace eval ::Plotchart {
    set methodProc(xyplot,deletedata) DeleteData
}

# DeleteData --
#     Delete the data elements and the associated resources
#
# Arguments:
#     w          Name of the canvas (actually the plot)
#
# Result:
#     None
# Side effects:
#     The data elements in the canvas are removed and the
#     last known data points are removed from the resources.
#     However, the rest (legend, options for data series) is not.
#     This is deliberate: you may simply want to refresh the
#     data.
#
proc ::Plotchart::DeleteData {w} {
    variable data_series
    variable scaling

    foreach elem [array names data_series $w,*,x] {
         unset -nocomplain data_series($elem)
    }

    $w delete data
}

# --- end patch ---

set update 10;		#  Ms between updates.

set square [list]


##
# Create square wave.
# This creates a square wave 1000 points long.
# The period of the square wave is 100 pts.
# (10 cycles).
#
# @return 2 element list.  The first element is
#                          the x coordinate the
#                          second element the y coordinate.
#
proc squareWave {} {
    set flip 50;		# points before polarity flip.
    set magnitude 100;		# Magnitude of the wave.
    set multiplier 1

    for {set i 0} {$i < 1000} {incr i} {
	lappend x $i
	lappend y [expr {$magnitude * $multiplier}]

	# flip from positive to negative polarity etc.

	if {($i % $flip) == 0} {
	    set multiplier [expr {$multiplier * -1}]
	}
    }
    return [list $x $y]
}

##
# Plot the data
#
#  @param period - refresh period.
#  @param plot   - plot chart plot id.
#  @param wf1    - first waveform to plot.
#
#  Each time 400pts with a random starting offset of
#  up to 600pts will be ploted.
#
proc updatePlot {period plot wf1} {

    $plot deletedata

    set offset [expr {int(600*rand())}]
    set last   [expr {$offset + 350}]
    set name [lindex $wf1 0];	# series name.
    set pts  [lindex $wf1 1]
    set x    [lindex $pts 0]
    set y    [lindex $pts 1]


    $plot plotlist $name [lrange $x 0 400] [lrange $y $offset $last]
    after $period [list updatePlot $period $plot $wf1]

}

set square [squareWave];	#  Create the square wave.
canvas .c -width 8i -height 4i
pack .c

set plot [::Plotchart::createXYPlot .c [list 0 400 100] [list -2048 2048 1000]]

updatePlot $update $plot [list square $square]
