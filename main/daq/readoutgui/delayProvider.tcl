#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#      NSCL Data Acquisition Group 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

##
# @file delayProvider.tcl
# @brief A mechanism for inserting a delay between begins received by 
#        data providers. 
# @author Jeromy Tompkins 
#

package provide Delay_Provider 1.0

# Establish the namespace in which the methods and state live and
# define the state variables.
#

namespace eval ::Delay {
  variable destroyCmd 0 ;#< After id for destroying progress dialog
  
  variable instances;    # holds parameterizations of each instance
  array set instances [list] ; # indexed by source id.
  
}

#------------------------------------------------------------------------------
#  Public interface of the data source provider.

##
# parameters
#    Returns the set of parameters that are required to create the source.
#    For now these are:
#    - delay  - The time to delay in milliseconds 
#
proc ::Delay::parameters {} {
    return [dict create delay [list {milliseconds to delay on begin}] \
                        enddelay [list {milliseconds to delay on end}]]
}

##
# start
#   Do nothing.
#
# @param params - Dict containing the parameterization of the source.
#
#
proc ::Delay::start params {
  
  set id [dict get $params sourceid]

  #  Error for duplicated id:
  #
  if {[array names instances $id] ne "" } {
    error "Delay already has an instance $id"
  }
  
  set delayTime [dict get $params delay]
  set endDelayTime [dict get $params enddelay]
  set ::Delay::instances($id) [list $delayTime $endDelayTime]
}
##
# check
#   Check the status of the connection to the source.
#   Do nothing
#
# @param id - Source id of the provider instance.
# @return always true 
#
proc ::Delay::check id {
  return 1
}
##
# stop
#   Stop the data source.  Remove the id from the array
#
# @param id - Id of the source to close.
#
proc ::Delay::stop id {
  if {[array  names ::Delay::instances $id] eq ""} {
    error "Delay::stop - $id is not a known source"
  }
  array unset ::Delay::instances $id
}
##
# begin
#   Wait for the specified amount of time in the delay parameter
#
# @param id - Source id.
# @param run  - Run number desired.
# @param title - Title desired.
#
proc ::Delay::begin {id run title} {
  if {[array  names ::Delay::instances $id] eq ""} {
    error "Delay::begin- $id is not a known source"
  }
  
  set config $::Delay::instances($id)
  set delayTime [lindex $config 0]
  
  ::Delay::_delayWithFeedback $delayTime
}

##
# end
#   Wait the end delay.
#
# @param id - Id of the source to end.
#
proc ::Delay::end id {
  if {[array  names ::Delay::instances $id] eq ""} {
    error "Delay::end - $id is not a known source"
  }
  set config $::Delay::instances($id)
  set endDelayTime [lindex $config 1]
  
  ::Delay::_delayWithFeedback $endDelayTime
}

##
# init
#   Do nothing
#
proc ::Delay::init id {}

##
# capabilities
#   Returns a dict describing the capabilities of the source:
#   - canpause - false
#   - runsHaevTitles - true
#   - runsHaveNumbers - true
#
# @return dict - as described above.
#
proc ::Delay::capabilities {} {
    return [dict create \
        canPause        true    \
        runsHaveTitles  true    \
        runsHaveNumbers true    \
    ]
}

proc ::Delay::_delayWithFeedback {duration} {
  if {[winfo exists .delay]} {
    catch {after cancel $::Delay::destroyCmd}
    destroy .delay

  } 
  toplevel .delay
  ttk::label .delay.title -text "Delay in progress"
  ttk::progressbar .delay.progress -orient horizontal -mode determinate \
    -maximum $duration -value 0
  grid .delay.title -sticky new -padx 8 -pady 8
  grid .delay.progress -sticky new -padx 8 -pady 8
  grid rowconfigure .delay 0 -weight 1
  grid columnconfigure .delay 0 -weight 1
  
  
  

  set increment [expr $duration/10]
  if {$increment == 0} {set increment 1}
  
  for {set elapsed 0} {$elapsed < $duration} {incr elapsed $increment} {
    after $increment
    .delay.progress configure -value $elapsed
    update
  }
  .delay.progress configure -value $duration
  .delay.title configure -text "Delay complete"
  update
  set ::Delay::destroyCmd [after 1000 destroy .delay]
  
}

