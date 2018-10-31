package require Process

set readoutHost charlie
set channelFile /scratch/fox/daq/master.git/main/cookbooks/epicspush/vmusbreadout/channels.dat

set here [file dirname [info script]]

proc OnStart {} {
    puts "Killing old proc"
    Process ctlkill -command [list ssh $::readoutHost killall controlpush]
    puts "Starting chanlog in host charlie - where Readout is"
    Process ctlpush -command [list  \
       ssh $::readoutHost $::here/controlpush.sh
    ]
    puts "Running"
}
