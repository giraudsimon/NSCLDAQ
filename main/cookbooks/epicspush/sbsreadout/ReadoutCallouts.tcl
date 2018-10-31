package require Process

set readoutHost charlie
set channelFile /scratch/fox/daq/master.git/main/cookbooks/epicspush/sbsreadout/channels.dat


proc OnStart {} {
    puts "Killing old proc"
    Process ctlkill -command [list ssh $::readoutHost killall controlpush]
    puts "Starting chanlog in host charlie - where Readout is"
    Process ctlpush -command [list \
       ssh $::readoutHost $::env(DAQBIN)/controlpush -p12000 -n$::readoutHost -i5 $::channelFile \
    ]
    puts "Running"
}
