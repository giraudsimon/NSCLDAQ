#!/usr/bin/tclsh

# Make the packages findable

lappend auto_path .
lappend auto_path /scratch/fox/daq/8.1test/TclLibs

package require VhsWidgets
package require iSegVhs
package require PollManager

pollManager pm -period 500
pm start

vhs create hv

VhsWidgets::channel .c -device hv -channel 0
pack .c

pm add [list .c update]

# Status screen:

toplevel .status
VhsWidgets::channelStatus .status.s -device hv -channel 0
pack .status.s

pm add [list .status.s update]


