source displayepics.tcl
alarmConfiguration .t -channel harry \
                      -enable  1    \
    -command [list ok %W %C]  -cancelcommand [list cancel %W %C]


proc ok {w c} {
    puts "$w ok for $c"
    destroy .t

}
proc cancel {w c} {
    puts "$w cancel for $c"
    destroy .t
}
.t modal
