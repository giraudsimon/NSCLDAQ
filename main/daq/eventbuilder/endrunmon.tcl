#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file endrunmon.tcl
# @brief Monitor rings for end of run.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
#  This file contains the EndrunMon package.  It provides an API/implementation
#  that can monitor a ring for a specific number of end runs.  When these
#  are seen, a condition variable is signalled and the thread doing the monitoring
#  exits.
#
# API:
#   *  startMonitor    - begins the end run monitor thread.
#   *  setEndRunCount  - Sets the number of end run items expected.
#   *  incEndRunCount  - Increments the number of end run items expected
#   *  decEndRunCount  - Decrements the number of end run items expected.
#   *  waitEndRun      - Blocks until the end run items required have been seen.
#   *  abort           - Abort the end run thread after next item.
#

package provide EndrunMon 1.0
package require Thread
package require evbcallouts
package require RunstateMachine
package require ui

##
# Create the namespace and any variables needed in the parent thread.
#
namespace eval ::EndrunMon {
    variable mutex   [thread::mutex create]
    variable condVar [thread::cond create]
    variable tid     ""
}
##
#  Create the thread shared variables we need.  They are elements of the varname
#  EndrunMon
#

tsv::set EndrunMon endsExpected 0;             # Number of end runs expected.
tsv::set EndrunMon abort        0;             # Set nonzero to abort thread.
 
 
##
# setEndRunCount
#   Sets the number of end run items expected by the monitor thread.  This can be
#   dynamically modified by the monitor which will signal when the number it has
#   seen is >= the number requested.
#
# @param value  - new number of end runs expected.
#
proc EndrunMon::setEndRunCount value {
    tsv::set EndrunMon endsExpected $value    
}
##
# incEndRunCount
#    Increment the number of end runs expected by the monitor thread.
#
proc EndrunMon::incEndRunCount {} {
    tsv::incr EndrunMon endsExpected
}
##
# decEndRunCount
#   Decrement the number of end runs expected by the monitor thread.
#
proc EndrunMon::decEndRunCount {} {
    tsv::incr EndrunMon endsExpected -1
}

##
# startMonitor
#   Start the monitor thread.  
#   -  It is an error to do this when the thread is already active
#      (nonempty tid)
#   -  The mutex is taken.  We don't release it until we
#      wait for the condvar in waitEndRun so that the signal can't happen
#      before we wait.
#
# @param ringUrl - the URI of the ring to be monitored.
#

proc EndrunMon::startMonitor ringUrl {
    if {$::EndrunMon::tid ne ""} {
        error "Monitor thread is already active!"
    }
    thread::mutex lock $::EndrunMon::mutex
    
    #
    # Create the thread, send it our auto path and
    # get it to require the packages it needs:
    #
    
    set EndrunMon::tid [thread::create -joinable]
    thread::send $::EndrunMon::tid [list set auto_path $::auto_path]
    thread::send $::EndrunMon::tid {
        package require Thread
        package require TclRingBuffer
    }
    #  Send the proc that is the thread's action:
    
    thread::send $::EndrunMon::tid {
        ##
        # monitorRing
        #   takes begin/end data from a ring.
        #   when the nesting level goes to zero, signal the cond var.
        #
        # @param ringurl - Url for the ring to monitor
        # @param mutex   - mutex handle that guards the condvar.
        # @param condvar - condition variable handle to signal.
        #
        proc ::monitorRing {ringurl mutex condvar} {
            ring attach $ringurl
            set ercount 0
            while {1} {

                set item [ring get -timeout 2 $ringurl [list 2]];  #end run only.
                set expectedEnds [tsv::get EndrunMon endsExpected]
                if {$item ne {}} {
                    if {[dict get $item type] eq "End Run"} {
                        incr  ercount
                    }
                }

                #
                #  Signal if we've seen enough end runs.
                #
                if {$ercount >= [tsv::get EndrunMon endsExpected]} {
                  thread::mutex lock $mutex
                  thread::cond  notify $condvar
                  thread::mutex unlock $mutex
                  break
                }

                #
                # Handle requests to abort without signalling.
                #
                if {[tsv::get EndrunMon abort]} {
                    break
                }

            }
            
            ring detach $ringurl
            tsv::set EndrunMon abort 0;     # in case we were aborted.
            thread::release
        }
    }
    thread::send -async $::EndrunMon::tid [list monitorRing $ringUrl $EndrunMon::mutex $::EndrunMon::condVar]
    
}
##
# waitEndRun
#   Wait for the end run to occur.
#   This means
#   - waiting for the monitor thread to signal the condition variable.
#   - joining the thread (as it will exit soon after).
#
proc ::EndrunMon::waitEndRun {} {
    if {$::EndrunMon::tid eq ""} {
        error "Monitor thread was not started use EndrunMon::startMonitor to do so."
    }
    # Wait on the condition variable
    # and on the thread exit.
    #
    
    set ui [::RunControlSingleton::getInstance]
    $ui configure -state disabled
    
    while {[thread::exists $::EndrunMon::tid]} {
        thread::cond wait $EndrunMon::condVar $::EndrunMon::mutex 300
        update;              #Keep UI alive.
    }
    # Thread exited so:
    
    thread::join $EndrunMon::tid
    
    #
    #  indicate the thread is gone and free the mutex.
    #
    set EndrunMon::tid ""
    thread::mutex unlock $EndrunMon::mutex
    
    $ui configure -state normal
}
##
# abort
#   Abort the monitor thread an wait for it to exit.
#   we release the mutex first in case the last end run
#   is arriving as we abort.
#
proc ::EndrunMon::abort {} {
    if {$::EndrunMon::tid == ""} {
        error "::EndrunMon::abort - monitor thread is not running."
    }
    tsv::set EndrunMon abort 1
    thread::mutex unlock $::EndrunMon::mutex
    thread::join $::EndrunMon::tid
    
    # Mark the thread as non existent:
    
    set ::EndrunMon::tid ""
}
#----------------------------------------------------------------------------
# EndunMon callout bundle.
# This section of code registers a callout bundle.  Since in general it
# runs on package require it's going to be before the event builder's bundle.
# Here's the set of transitions we do:
#
#  Leaving Halted -> Active: Start the monitor thread...
#  Entering Halted from Active or Puased: Wait for the end run to complete.
#  Entering NotReady - If the monitor thread is running, abort it.
#

##
#  attach
#    Called when we are registered as a callback bundle
#    We're not going to do anything.
#
# @param current - the current state.
#
proc EndrunMon::attach current {
    
}

##
# enter
#   Called when a state is being entered.
#     If Halted is being entered from Active or Paused,
#   wait for the run to end if the monitor thread
#   is active (if we attached in an active state, it won't be).
#     If NotReady is entered from any other state, if the monitor thread
#   is running abort it..and pray that we're going to be able to
#   see something in the ring before the end of days.
#
# @param from - prior state
# @param to   - state we are entering.
#
proc EndrunMon::enter {from to} {
    
    if {($to eq "Halted") && ($from in [list Active Paused])} {
        if {$::EndrunMon::tid ne ""} {
            ::EndrunMon::waitEndRun
        }
    }
    if {$to eq "NotReady"} {
        if {$::EndrunMon::tid ne ""} {
            ::EndrunMon::abort
        }
    }
}
##
# leave
#   A state is being left;
#   Leaving Active or Paused for Halted starts the monitor to wait for the
#   end of run.  We do this so that we're not monitoring unless the run is
#   actually being ended. This is because otherwise we can rate limit acquisition.
#
# @param from - the state being left.
# @param to   - the state being entered.
#
# @note  [EVBC::applicationOptions cget -destring] is the ring to monitor.
# TODO:  Should really provide a mechanism to make this available that is
#        not event builder specific.
#

proc EndrunMon::leave {from to} {
#    if {($from eq "Halted") && ($to eq "Active")  && ($::EndrunMon::tid eq "")} {}
    if {(($from eq "Active") || ($from eq "Paused")) &&  \
        ($to eq "Halted") && ($::EndrunMon::tid eq "")} {
        set ring tcp://localhost/[$::EVBC::applicationOptions cget -destring]
        ::EndrunMon::startMonitor $ring
    }
}

namespace eval EndrunMon {
    namespace export enter leave attach
}


##
# register the bundle to the state machine before a specific callout bundle
#
# @param beforeBundle   name of callout bundle you want to insert this before
#
proc EndrunMon::register {{beforeBundle {}}} {
  set sm [RunstateMachineSingleton %AUTO%]
  if {$beforeBundle eq {}} {
    $sm addCalloutBundle EndrunMon
  } else {
    $sm addCalloutBundle EndrunMon $beforeBundle
  }
  $sm destroy
}
