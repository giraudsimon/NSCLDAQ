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

package require Tk
package require Plotchart 2.0.1

set update 50;		#  Ms between updates.

set square [list]
set sine   [list]


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
# Create a sine wave.
# Wave is 1000 pts long
# Period is 100pts (2pi/100 is the argument increment to sin).
# Wave is offset by 300 units.
#
# @return 2 element list  First element are the x coordinates.
#          second elements the y coordinates.
#
proc sineWave {} {
    set pi 3.141592654;		# close enough.
    set magnitude 100

    for {set i 0} {$i < 1000} {incr i} {
	lappend x $i
	lappend y [expr {($magnitude*sin($i/($pi*2.0)) + 300)}]
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
set time 0

proc updatePlot {period  args} {
    set plot $::plot
   # $c delete all

    $plot deletedata

    #set plot [::Plotchart::createXYPlot $c [list 0 400 100] [list -2048 2048 1000]]
    set offset [expr {int(600*rand())}]
    set last   [expr {$offset + 350}]

    foreach wf $args color [list black red] {
	set name [lindex $wf 0];	# series name.
	set pts  [lindex $wf 1]
	set x    [lindex $pts 0]
	set y    [lindex $pts 1]
	
	
	$plot plotlist $name [lrange $x 0 400] [lrange $y $offset $last]
        $plot dataconfig $name -color $color
    }

    after $period [list updatePlot $period  {*}$args]
}



set square [squareWave];	#  Create the square wave.
set sine   [sineWave]

canvas .c -width 8i -height 4i
pack .c

set plot [::Plotchart::createXYPlot .c [list 0 400 100] [list -2048 2048 1000]]
updatePlot $update  [list square $square] [list sine $sine]