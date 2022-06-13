# Meant to be run on Tclserver.
#
set scriptdir [file dirname [info script]]
lappend auto_path [file join $scriptdir .. TclLibs];   # s.b TclLibs dir.


package require tkutils

#  The user must have supplied a command line argument
#  which is the name of their setup file.
#

tkutils::requireArgs 1 "No Setup File" \
   "You must suppy a command argument that is your epics config file"
set file [lindex $argv 0]

#  And the file must be readable...
#
tkutils::requireReadable $file "Setup file unreadable" \
   "The setup file: $file is not readable"



#  We need to establish our location and source in the
#  scaler GUI support. We assume that we are installed
#  1 level below 'prefix' and that the scaler display
#  GUI support packages are installed in the Scripts
#  dir of that directory.

set me [info script]
if {[file type $me] == "link"} {
    set me [file readlink $me]
}

set mydirectory [file dirname $me]
set bindir     $mydirectory/../bin
set scriptdir  $mydirectory/../TclLibs/epicsdisplay
source $scriptdir/displayepics.tcl

# restartControlPush file
#       Restarts the controlpush program.
# Parameters:
#    file  - setup file.
#
proc restartControlPush file {
   global clientpid
   global TclServerPort
   global bindir

   exec kill -9 $clientpid

   set clientpid [exec $bindir/controlpush --port=$TclServerPort --node=localhost \
                  --interval=5 --authorize $file &]
   bind . <Destroy> "Cleanup %W $clientpid"
}

#
#   Now the setup file is sourced we can start the controlpushp rogram.
#   and set things up so that we will kill it on our exit:
#

puts "Channel setup file: $file"
puts "exec $bindir/controlpush --port=$TclServerPort --node=localhost --interval=5 $file &"
set clientpid [exec $bindir/controlpush --port=$TclServerPort --node=localhost  --interval=5 --authorize $file &]


proc Cleanup {widget pid} {
    if {$widget != "."} return
    exec kill $pid
}

bind . <Destroy> "Cleanup %W $clientpid"

#  Build the GUI:

set chans [readSetupFile $file]
makeGui
fillTable $chans
setupStripChart $chans
setupAlarms $chans
