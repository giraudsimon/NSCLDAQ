#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#      NSCLDAQ Development Group
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file offlineProvider.tcl
# @author Jeromy Tompkins <tompkins@nscl.msu.edu> 

package provide Offline_Provider 1.0
package require OfflineEVBInputPipeline 11.0 
package require ui


## @namespace Offline
#
# @brief Provides a data source data source that cats an evt file into a ring.
# 
# At the moment, this is intended for use with the Offline EVB/Orderer. It 
# launches a pipeline that consists of the following:
# 
# cat file0.evt file1.evt ... | unglom -id val | frag2ring --strip | stdintoring ring
#
# The result of this is a stream of data that has been unzipped into ring items
# passed into the final stage of event building. If the data file contains data 
# recorded following multiple stages
# of event building, then this will not split the deepest data into its fragments.
# 
namespace eval ::Offline {
  variable localParams
  variable pipeline
}

##
# parameters
#     Returns the set of parameters that are required to create the source
#    - unglomid   the id to give to items missing body headers after they are 
#                 unzipped
#    - ring       the name of the ring to dump data into at the end of the 
#                 pipeline
#    - file       the list of files to concatenate together into cat
#
proc ::Offline::parameters {} {
    return [dict create unglomid [list {Unglom ID}] ring [list {Ring name}] file [list {Files}] ]
}

##
# start
#    Start the data source.  This means configuring the parameters
#    and constructing the pipeline object. It does not actually launch
#    the pipeline though.
#
# @param params - Parameterization of the source.
#
proc ::Offline::start params {

  variable localParams
  variable pipeline 

  set localParams [OfflineEVBInputPipeParams %AUTO%]
  $localParams configure -unglomid [dict get $params unglomid]
  $localParams configure -inputring [dict get $params ring]
  $localParams configure -file [dict get $params file]

  set pipeline [OfflineEVBInputPipeline %AUTO%]

}
##
# check - See if we are still alive:
#
# This always says that it is alive. 
#
# @todo check whether the pipleline has exited unexpectedly and then
#       return that info
#
# @param id - the source id.
#
proc ::Offline::check id {
  return 1
}

##
# stop - stop the source by destroying the pipeline object. 
# 
# @param id - source id.
#
proc ::Offline::stop id {
  variable pipeline
  catch {$pipeline destroy}
}
##
# begin - start a run.
#
# Launch the pipeline.
#
# @param id - source id
# @param run - run numer
# @param title
#
proc ::Offline::begin {id run title} {
  variable pipeline
  variable localParams 
  $pipeline launch $localParams
}
##
# end
#   End the run
# @param id - the source id
#
proc ::Offline::end id {}

##
# init 
#  Initialize - no-op 
# @param id - the source id
#
proc ::Offline::init id {}


##
# capbilities - the s800 capabilities:
#
proc ::Offline::capabilities {} {
}
