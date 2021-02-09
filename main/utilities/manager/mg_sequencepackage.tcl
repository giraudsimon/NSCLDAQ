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
#             Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file   mg_sequencepackage.tcl
# @brief  Package providing sequence access.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide mg_sequencepackage 1.0
package require programs;           # Because sequence steps are programs.
package require containers
package require sqlite3

##
# This package provides an API for state transitions and associated sequences.
# the concept is that associated with each state transition is one or more
# sequences that must be executed.  Sequences are ordered 'steps'  each step
# is a program invocation with an optional pre delay and post delay.
# Programs that are transient have their output monitored until they silently die.
# there are, however, two classes of non-transient programs:
#  *  Persistent programs are monitored until they die and, while death is reported,
#     All that is done is to report the death.
#  *  Critical programs are monitored until they die and, death results in a
#     shutdown transition which requires a restart of all persistent programs.
#  You can imagine that a system consists of a mix of persistent and critical
#  components.  State transitions fire off programs that communicate the transitions
#  to the components that need to know about them in a manner specific to that
#  component.   Those programs execute in an order and timing determined
#  by their sequence.  A state transition can have more than one sequence
#  attached to it.  The sequences themselves are executed in the order of their
#  primary keys (order in which they were defined).  Attaching multiple sequences
#  to a single state transition is just and organizational tool rather than
#  providing any larger significance.
#
#  The api lives in the ::sequence:: namespace and consists of the following
#  entries:
#
# ::sequence::newState - Creates a new state.
# ::sequence::newTransition - Creates a new legal transition.
# ::sequence::add    - Adds a new sequence.
# ::sequence::addStep- Adds a new step to the end an existing sequence.
# ::sequence::addMonitor - Adds a monitor for a program.
# ::sequence::rmvState - Remove a state - note associated sequences are also
#                        destroyed as are legal transitions with this state as a
#                        to value.
# ::sequence::rmvTransition - remove a legal transition.
# ::sequence::rmvSequence   - Remove a sequence and all of its steps.
# ::sequence::rmvStep       - Removes an existing step.
# ::sequence::insertStep    - Inserts a new step between existing steps
# ::sequence::prependStep   - Inserts a new step prior to the existing first step.
#
# ::sequence::listStates    - Lists the set of known states.
# ::sequence::listLegalNextStates - Given the current state lists legal next states
# ::sequence::isLegalTransition - Determines if a proposed state transition is legal.
# ::sequence::listSequences - Lists the known sequences.
# ::sequence::listSteps     - List steps in a sequence.
# ::sequence::transition    - Perform  a transition
# ::sequence::runSequence   - Runs a sequence (normally done by
#                             ::sequence::perform this entry supports testing too).
#
    
namespace eval sequence {
    ::container::_setup
    set  tempdir $::container::tempdir
}
