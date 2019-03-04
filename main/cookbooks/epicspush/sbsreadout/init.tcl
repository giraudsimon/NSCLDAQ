set f [open channels.dat r]
while {![eof $f]} {
    set line [gets $f]
    if {$line ne ""} {
	runvar EPICS_DATA($line)
	runvar EPICS_UNITS($line)
	runvar EPICS_UPDATED($line)
    }
}
puts "Monitored variables:"
puts [runvar -list]
close $f
