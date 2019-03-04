# test_axis.tcl --
#     Test the drawing of the axis: nice rounded values?
#
#     NOTE:
#     Negative values require floor() instead of ceil()!
#
#     NOTE:
#     Problem with right axis!
#
#     TODO:
#     Floor and Ceil and less stringent check for bounds!
#
source plotchart.tcl
package require Plotchart

grid [canvas .c1] [canvas .c2]
grid [canvas .c3] [canvas .c4]

#
# Create the font
#
set p1 [::Plotchart::createXYPlot .c1 {0.12 10.4 1.0}    {-0.12 10.4 2.5}   ]
set p2 [::Plotchart::createXYPlot .c2 {10.12 -10.4 -2.0} {-5.1 -4.5 0.1}    ]
set p3 [::Plotchart::createXYPlot .c3 {-0.12 10.4 2.5}   {0.12 10.4 1.0}    ]
set p4 [::Plotchart::createXYPlot .c4 {-5.1 -4.5 0.1}    {10.12 -10.4 -2.0} ]
set p5 [::Plotchart::createRightAxis $p2 {-5.99 -4.5 0.1}]

$p2 plot data  10.0 -5.0
$p2 plot data -10.0 -5.0
$p5 dataconfig data -colour green
$p5 plot data  10.0 -4.7
$p5 plot data -10.0 -4.7
