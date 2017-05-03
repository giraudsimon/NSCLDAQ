#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file evbRdoCallouts10.tcl
# @brief Magic to allow user's to run the 10.x event builder callouts.


package provide evbcallouts10 1.0


#  Isolate our stuff to prevent collisions:

namespace eval ::RdoCallouts10 {
    variable libpath10

    # We are in toplevel/TclLibs/something...we need to look for nscldaq-10.x installations
    # above toplevel:

    set ::RdoCallouts10::here [file normalize [file dirname [info script]]]
    puts "--------------------------------"
    set daq10s  [glob -nocomplain [file join $::RdoCallouts10::here ../../../10.2*]]
    if {$daq10s eq ""} {
       error "No NSCLDAQ-10.x installed"
    }
    set ::RdoCallouts10::libpath10 [file normalize [file join [lindex [lsort -decreasing \
       [glob -nocomplain [file join $::RdoCallouts10::here ../../../10.2*]]] 0] TclLibs]]
    puts "Path: $::RdoCallouts10::libpath10"

    # There should only be one 11.* in the auto path:

    set ::RdoCallouts10::index [lsearch -glob $::auto_path *11.*]
    set ::auto_path [lreplace $::auto_path $::RdoCallouts10::index $::RdoCallouts10::index $::RdoCallouts10::libpath10]
    puts $::auto_path

    # The package paths are already cached so:

    package forget evbcallouts
    package forget EVB::CallbackManager
    package forget EVB::ConnectionManager
    package forget EVB::GUI
    package forget EVB::Late
    package forget EVB::barriers
    package forget EVB::connectionList
    package forget EVB::inputStatistics
    package forget EVB::outputStatistics
    package forget EVBUtilities
    package forget EvbOrderer
    package forget EventBuilder
    package forget Observer

    # this requires the 10.x event builder.

    package require evbcallouts


    #  This is a bit of dirt that should replace the Method used to get
    #  the ring URL.  We can't just set env(RINGNAME) because we also
    #  want to force the system to localhost:
    
    namespace eval ::Experiment {
        proc spectrodaqURL system {
            
            set destRing [$::EVBC::applicationOptions cget -destring]
            puts "Calling the right url getter: $destRing"
            catch [list ringbuffer create $destRing] msg;  #ensure ring exists first.
            puts "Ringmsg $destRing: $msg"
 
            return "tcp://localhost/$destRing"
        }
    } 
}
