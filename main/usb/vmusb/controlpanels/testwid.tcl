source gdgwidget.tcl

proc wup {wid chan value} {
    $wid setWidth $chan [incr value]
}
proc wdown {wid chan value} {
    $wid setWidth $chan [incr value -1]
}
proc dup {wid chan value} {
    $wid setDelay $chan [incr value]
}
proc ddown {wid chan value} {
    $wid setDelay $chan [incr value -1]
}

gdgwidget .g -title {Testing gdg widget} \
    -upwidth    [list wup .g]  \
    -downwidth  [list wdown .g] \
    -updelay    [list dup  .g] \
    -downdelay  [list ddown .g]


pack .g