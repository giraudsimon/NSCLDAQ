# set up the runvars from the
# channels.dat file.

set f [open channels.dat r]
while {![eof $f]} {
    set line [gets $f]
    if {$line ne ""} {
	monvar add EPICS_DATA($line)
	monvar add EPICS_UNITS($line)
	monvar add EPICS_UPDATED($line)
    }
}
puts "Monitored variables:"
puts [monvar list]
close $f


#   Start a Tcl server:

package require TclServer

proc printError {ch cmd msg} {
    puts "Error : $msg executing '$cmd'"
}

proc printConnection {sock ip port} {
    puts "Accepting connection from $ip on $port"
    return 1
}
lappend auto_path /usr/opt/daq/11.3-006/TclLibs


set server [TclServer %AUTO% -port 12000 -onconnect printConnection -onerror printError]
$server start



