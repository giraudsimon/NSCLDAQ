#!/bin/bash

. /etc/profile             # Define epics vars.
. /usr/opt/daq/11.3-006/daqsetup.bash  # Pick the version of NSCLDAQ

printenv |grep EPICS

$DAQBIN/controlpush -p12000 -i5 -nlocalhost /scratch/fox/daq/master.git/main/cookbooks/epicspush/sbsreadout/channels.dat

