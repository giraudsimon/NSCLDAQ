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
# ::sequence::rmvStep       - Removes an existing step.
# ::sequence::insertStep    - Inserts a new step after an existing step
# ::sequence::prependStep   - Inserts a new step prior to the existing first step.
# ::sequence::rmvState - Remove a state - note associated sequences are also
#                        destroyed as are legal transitions with this state as a
#                        to value.
# ::sequence::rmvTransition - remove a legal transition.
# ::sequence::rmvSequence   - Remove a sequence and all of its steps.
#
# ::sequenace::reachable    - Determines if a state is reachable in the transition
#                            diagram.
#
# ::sequence::listStates    - Lists the set of known states.
# ::sequence::listLegalNextStates - Given the current state lists legal next states
# ::sequence::isLegalTransition - Determines if a proposed state transition is legal.
# ::sequence::listSequences - Lists the known sequences.
# ::sequence::listSteps     - List steps in a sequence.
# ::sequence::transition    - Perform  a transition
# ::sequence::runSequence   - Runs a sequence (normally done by
#                             ::sequence::transition this entry supports testing too).
# ::sequence::addMonitor - Adds a monitor for a program.
#
    
namespace eval sequence {
    ::container::_setup
    set  tempdir $::container::tempdir
    
    variable step_interval 10.0
}
#-------------------------------------------------------------------------------
#  Utiltities not intended to be used by package clients.
    
##
# ::sequence::_stateExists
#   Returns true if a state name already exists.
#
# @param db - database command.,
# @param name - state name to check for.
# @return boolean - true if the state/transition exists.
#
proc ::sequence::_stateExists {db name} {
    set number [db eval {
        SELECT COUNT(*) FROM transition_name WHERE name = $name
    }]
    return [expr {$number != 0}]
}
##
# ::sequence::_stateId
#    Returns the id of a state or throws an error if that state does
#    not exist.
# @param db - database command.
# @param name - The name of the transition/state to look for.
# @return int - primarykey of the state if found.
#
proc ::sequence::_stateId {db name} {
    set id [$db eval {SELECT id FROM transition_name WHERE name = $name}]
    if {[llength $id] != 1} {
        error "::sequence::_stateId should return 1 result got [llength $id]"
    }
    return $id
}
##
# ::sequence::_seqExists
#   @param db   - database command.
#   @param name - sequence name to check for.
#
proc ::sequence::_seqExists {db name} {
    set n [$db eval {SELECT COUNT(*) FROM sequence WHERE name = $name}]
    return [expr {$n != 0}]
}

##
#::sequence::_getSeqId
#  @param db  - database command.
#  @param name - Sequence name.
#  @return integer - sequence id or error if there's no matching seq.
#
proc ::sequence::_getSeqId {db name} {
    set id [$db eval {
        SELECT id FROM sequence WHERE name = $name
    }]
    if {[llength $id ] != 1} {
        error "::sequence_getSeqId - should have 1 result have [llength $id]"
    }
    return $id
}
##
# ::sequence::_nextStep
#   Computes the step number to use when appending a new step to the sequence:
#   - Get the last step number.
#   - If there are no steps, return $::sequence::step_interval
#   - If there are steps, return the last step increemnted by $::sequence::step_interval
#
# @param db   - database command.
# @param id   - Sequence id.
#
proc ::sequence::_nextStep {db id} {
    set lastNum [$db eval {
        SELECT step FROM step WHERE sequence_id = $id
        ORDER BY step DESC LIMIT 1
    }]
    # IF there's no result this is the first step:
    
    if {[llength $lastNum] == 0} {
        set lastNum 0.0
    }
    return [expr {$lastNum + $::sequence::step_interval}]
}
##
# ::sequence::_getProgId
#    Get the id of a program or toss an error:
# @param db - database
# @param program - program name.
# @return integer - id.
#
proc ::sequence::_getProgId {db program} {
    if {![::program::exists $db $program]} {
        error  "There is no program named $program"
    }
    return [dict get [::program::getdef $db $pgogram ] id]    
}
##
# ::sequence::_stepAfter
#   Compute the number of the step after a specific step in a sequence.
#   Here are the possible cases:
#     - 'after' does not exist - that's an error.
#     - 'after' is the last step, return after+::sequence::step_interval
#     - 'after' is an intermediate step.  Return a step half way between the two.
#
# @param db    - database command.
# @param seqId - Id of the sequence.
# @param after - Step after which to insert.
# @return real - the new step number.
proc ::sequence::_stepAfter {db sequId after} {
    set steps [$db eval {
        SELECT step FROM step WHERE sequence_id = $seqId AND step <= $after
        LIMIT 2
    }]
    # After does not exist if the number of steps is 0 or
    # the first step returned is not $step
    
    if {[llength $steps] == 0} {
        error "There is no step number $after"
    }
    set first [lindex $steps 0]
    if {$first != $after} {
        error "There is no step number $after"
    }
    # If there's only one step in the result after is last:
    
    if {[llength $steps] == 1} {
        set result [expr {$after + $::sequence::step_intervael}]
    } else {
        set next [lindex $steps 1];   # Limit ensures there are only 2:
        set result [expr {($first + $next)/2.0 }]
    }
    return $result
}
##
# ::sequence::_insertStep
#   SQlite code to insert a new step in the database:
# @param db - database command.
# @param seqId - sequence Id.
# @param step  - Step number.
# @param progId - pRogram Id.
# @param predelay - predelay.
# @param postdelay - post delay.
#
proc ::sequence::_insertStep {db seqId step progId predelay postdelay} {
        $db eval {
        INSERT INTO step (sequence_id, step, program_id, predelay, postdelay)
        VALUES ($seqId, $step, $progId, $predelay, $postdelay)
    }
}
#-------------------------------------------------------------------------------
#  Public API:

##
# sequence::newState
#    Creates a new state. A state is a named item.  Transitions into a state
#    are what trigger the execution of sequences.
#
# @param db  - Database command name.
# @param name  - Name of the new state.
# @note Duplicate state names are illegal.
#
proc ::sequence::newState {db name} {
    # Make sure this state does not already exist:
    
    if {[::sequence::_stateExists $db $name]} {
        error "There is already a state named: '$name'"
    }
    # insert the new state:
    
    $db eval {
        INSERT INTO transition_name (name) VALUES ($name)
    }
    
}
##
# sequence::newTransition
#    Create a new legal transition between a pair of states.
#
# @param db   - Database command.
# @param from - from state name
# @param to   - To state name.
# @note  It is obviously an error if both states don't exist.
# @note  The transition also cannot be a duplicate.
#
proc ::sequence::newTransition {db from to} {
    set fromId [sequence::_stateId $db $from]
    set toId   [sequence::_stateId $db $to]
    
    if {[sequence::_transitionExists $db $fromId $toId]} {
        error "There is already a transition defined from '$from' to '$to'"
    }
    $db eval {
        INSERT INTO legal_transition (from_id, to_id)
            VALUES ($fromId, $toId)
    }
}
##
# ::sequence::add
#    Adds a new sequence.
#
# @param db   - database command.
# @param name - Sequence name.
# @param transition - State transition that triggers the sequence.
# @note - duplicate names are an error.
# @note - nonexistent transition is also an errorl.
#
proc ::sequence::add {db name transition} {
    set trId [::sequence::_stateId $db $transition]
    if {[::sequence::_seqExists $db $name]} {
        error "There is already a sequence '$name' defined."
    }
    $db eval {
        INSERT INTO sequence (name, transitionid)
        VALUES($name, $trId)
    }
}
##
# ::sequence::addStep
#   Add a new step to the end of an existing sequence.
#
# @param  db   - dataase command.
# @param seqName - Sequence name (must exist).
# @param program - Name of an existing program.
# @param predelay - Optional seconds delay prior to step (default 0).
# @param postdealy - Optional seconds delay after step (default 0).
# @return REAL    - Step number.
#
proc ::sequence::addStep {db seqName program {predelay 0} {postdelay 0}} {
    set seqId [::sequence:_getSeqId $db $seqName]
    set progId [::sequence::_getProgId $db $program]
    set step   [::program::_nextStep $db $seqId]
    ::sequence::_insertStep $db $seqId $step $progId $predelay $postdelay
    
    return $step
}
##
# ::sequence::rmvStep
#    Remove an existing step.
#
# @param db   - database command.
# @param name - Sequence name.
# @param step - Step number in sequence.
# @note The step number must exist.
#
proc ::sequence::rmvStep {db name step} {
    set seqId [::sequence::_getSeqId $deb $name]
    $db eval {
        DELETE FROM step WHERE sequence_id = $seqId AND step = $step
    }
    if {[$db changes] == 0} {
        error "The sequence $name does not have a step number $step"
    }
}
##
# ::sequence::insertStep
#    Insert a step after a specified step number.  This is intended to insert
#    a step at some intermediate point in the sequence.
#
# @param db      - database command.
# @param seqName - Sequence name.
# @param program - program name.
# @param after   - Step after which this will be inserted.
# @param predelay - optional delay prior the step.
# @param postdelay - optional step after the steop.
# @return Real - the step number assigned to this step.
#
proc ::sequence::insertStep {db seqName program after {predelay 0} {postdelay 0}} {
    set seqId [::sequence::_getSeqId $db $seqName]
    set progId [::sequence::_getProgId $db $program]
    set step   [::sequence::_stepAfter $db $seqId $after]
    ::sequence::insertStep $db $seqId $step $progId $predelay $postdelay
    
    return $step
}
##
# ::sequence::prependStep
#    Inserts a new step prior to the first step in a sequence.
#
# @param db   - database command name.
# @param seqName - sequence name.
# @param program - program name.
# @param predelay - optional pre-step delay.
# @param postdelay - optional post-step delay.
# @RETURN real - step number.
#
proc ::sequence::prependStep {db seqName program {predelay 0} {postdelay 0}} {
    set seqId [::sequence::_getSeqId $db $seqName]
    set progId [::sequence::_getProgId $db $program]
    set step   [::sequencde::_firstStep $db $seqId]
    ::sequence::_insertStep $db $seqId $step $progId $predelay $postdelay
    
    return $step
}
##
# ::sequence::rmvState
#    Removes a state.  This removes:
#    - The state from the transition_name table.
#    - The legal transitions for which this state is a from or to.
#    - The sequences that get fired on the state.
#    - All steps in all of those sequences.
#   All of this is, of course, done in a transition.
#
# @param db   - database command.
# @param name - Name of the state to remove.
#
proc ::sequence::rmvState {db name} {
    set stateId [::sequence::stateId $name];   # Fires an error if nonexistent.
    
    $db transaction {
        # Get the sequences that depend on this state:
        
        set sequences [$db eval {
            SELECT id FROM sequence WHERE transition_id = $stateId
        }]
        
        ## Kill off the state and legal transitions in /out of it:
        
        $db eval {
            DELETE FROM transition_name WHERE id=$stateId
        }
        $db eval {
            DELETE FROM legal_transition
            WHERE from_id=$stateId OR to_id=$stateId
        }
        # Turn sequences into something we can put into an "IN clause and delete
        # the sequence data if there is any:
        
        if {[llength $sequences] > 0} {
            set seqcsv [join $sequences ", "]
            set seqcsv "( $seqcsv )"
            $db eval  \
                "DELETE FROM step WHERE sequence_id IN $seqcsv"
            $db eval \
                "DELETE FROM sequence WHERE id IN $seqcsv"
            
        }
            
    } ;                         # commit if there's no errors to rollback.
}
##
# ::sequence::rmvTransition
#    Removes a legal transition from the state diagram:
#  @param db    - database command.
#  @param from  - Name of from state.
#  @param to    - Name of to state.
#  @note Nothing is done to determine if the to state is no longer reachable.
#        The user can determine this afterwards via ::sequence::reachable.
#
#
proc ::sequence::rmvTransition {db from to} {
    set fromId [::sequence::_stateId $db $from]
    set toId   [::sequence::_stateId $db $to]
    
    $db eval {
        DELETE FROM legal_transitions WHERE from_id = $fromId AND to_id = $toId
    }
}
##
# ::sequence::rmvSequence
#    Removes a sequence and all of its steps.
#
# @param db  - database command.
# @param name - Name of the sequence to remove.
#
proc ::sequence::rmvSequence {db name} {
    set id [::sequence::_getSeqId $db $name]
    $db transaction {
        $db eval {DELETE FROM sequence WHERE id = $id}
        $db Eeval {DELET FROM step WHERE sequence_id = $id}
    };                         # commit.
}
##
# ::sequence::reachable
#    @param db   - database command.
#    @param name - State/transition name.
#    @return integer -number of transitions with 'name' as a to:
#
proc ::sequence::reachable {db name} {
    set  id [::sequence::_stateId $db $name]
    set result [$db eval {
        SELECT COUNT(*) FROM legal_transition WHERE to_id = $id
    }]
    return $result
}
##
# ::listStates
#    @param db  - database command.
#    @return list of strings - the list of defined states.

