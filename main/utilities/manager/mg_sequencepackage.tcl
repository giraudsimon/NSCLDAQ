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

package provide sequence 1.0
package require programs;           # Because sequence steps are programs.
package require containers
package require sqlite3
package require snit
package require portAllocator

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
# ::sequence::listReachableStates - List states one can reach from a Specified state.
# ::sequence::listLegalFromStates - Lists states that can transition to a specific state.
# ::sequence::listStates    - Lists the set of known states.
# ::sequence::listLegalNextStates - Given the current state lists legal next states
# ::sequence::isLegalTransition - Determines if a proposed state transition is legal.
# ::sequence::currentState   - Return current state name.
# ::sequence::listSequences - Lists the known sequences.
# ::sequence::listSteps     - List steps in a sequence.
# ::sequence::transition    - Perform  a transition
# ::sequence::runSequence   - Runs a sequence (normally done by
#                             ::sequence::transition this entry supports testing too).
# ::sequence::addMonitor - Adds a monitor for a program.
#
#  What is a monitor:
#    A monitor is a command ensemble with the following subcommands:
#
#    onOutput - invoked when the program the step is attached to has output
#               The database, name of the program and fd are presented (in order)
#                as parameters.
#    onExit   - invoked when the program the step is attached to exits (as
#               determined by an EOF in the fd).  These are passed:
#               the program description dict and file descriptor
#
#  Monitors get attached to step in a squence. They can be attached
#  before the sequence fires up.
#  Note that this package wraps its own hands around each program so that if
#  critical ones exit the system can be shutdown and if a Persistent on shuts down
#  that can be 
#

## This BS is needed by pkg_mkIndex which throws up on references to
#  undefined globals referenced at global level.

if {[array names ::env SERVICE_NAME] eq ""} {
    set ::env(SERVICE_NAME) dummy
    set ::errorInfo dummy
}

namespace eval sequence {
    ::container::_setup
    
    variable step_interval 10.0
    
    variable StepMonitors
    array set StepMonitors [list]
    
    variable currentTransitionManager [list];    # Current transition manager if any.
    variable outputListener;                     # Socket with output listener.
    variable outputClients;                      # list of sockets of output clients.
    variable outputServiceName $::env(SERVICE_NAME)-outputMonitor ; # Name of port manager serice.
}



#---------------------------------------------------------------------
#  Output service setup/handling
proc ::sequence::_startOutputService {} {
    set pm [portAllocator %AUTO%]
    set port [$pm findServer $::sequence::outputServiceName]
    if {$port ne ""} {
        error "Output service '$::sequence::outputServiceName' already in use."
    }
    set ::sequence::outputListener [$pm allocatePort $::sequence::outputServiceName]
    
    $pm destroy
    
    socket -server \
        ::sequence::acceptConnection $::sequence::outputListener
    
}

##
# ::sequence::acceptConnection
#     Called when a client has connected to the output listener service.
#     all we need do is add the socket to the outputClients list.  The
#     output handlers will do the rest by forwading all program output
#     to the list of current clients.  Similarly if a client disconnnects,
#     the output handlers detect that and remove that client fromt he list.
#
# @param socket - socket fd open on the connected client.
# @param host   - host from which the connection was made (ip address).
# @param port   - The port connected.
#
proc ::sequence::acceptConnection {socket host port} {
    lappend ::sequence::outputClients $socket
    fconfigure $socket -buffering line
}



##
# ::sequence::_relayOutput
#    Relay output from programs to the clients in ::sequence::outputClients
#    If a puts on one of those sockets fails it is removed from the list
#    of client sockets and closed.
#
# @param text - text to relay.
#
proc ::sequence::_relayOutput {text} {
    program::_log "Relaying $text to $::sequence::outputClients"
    foreach socket $::sequence::outputClients {
        set status [catch {
            puts $socket $text
            flush $socket
        }]
        if {$status} {
            catch {close $socket}
            set index [lsearch -exact $::sequence::outputClients $socket]
            set ::sequence::outuptClients \
                [lreplace $::sequence::outputClients $index $index]
        }
    }
    program::_log "Clients are: $::sequence::outputClients"
}
catch {::sequence::_startOutputService};    # Again for pkg_mkIndex.
    


#------------------------------------------------------------------------------
#  Snit classes:

##
# @class sequence::SequenceRunner
#
#   These objects are responsible for running sequences.  They execute steps and
#   delays from inside an external event loop.  Without that loop the sequence
#   will, in fact, stall.
#
# OPTIONS:
#    -database   - The database command - must be valid the entire duration of
#                  the sequence.
#    -name       - Name of the sequence.
#    -steps      - Steps in the sequence (readonly).
#    -endcommand - Command to execute when the sequence completes.
#                  This command recieves the object command, a reason 
#                  for completion that is one of:
#                  *  NORMAL - normal completion.
#                  *  ABORT  - The sequence aborted.
#                  and finally, if ABORT was the reason a human readable reason
#                  for the abort.
# METHODS:
#    currentStep - return index of current step error if inactive.
#    isActive
#    start     - Start sequencde execution.
#    abort     - External request to abort sequence execution.
#
snit::type sequence::SequenceRunner {
    option -database -default [list]
    option -name     -default [list]
    option -steps    -default [list] -readonly 1
    option -endcommand -default [list]
    
    variable currentStep 0;      # index into steps of current step.
    variable active      0;      # True if we've been started.
    variable afterid    -1;      # Current after id.
    variable sequenceId  -1;     # Information about the sequence.
    
    constructor args {
        $self configurelist $args
        
        #  We need non-null steps a database command and a name:
        
        if {$options(-database) eq ""} {
            error "sequence::SequenceRunner constructed with empty database command"
        }
        if {$options(-name) eq ""} {
            error "sequence::SequenceRunner constructed with blank sequence name"
        }
        set sequenceId [::sequence::_getSeqId $options(-database) $options(-name)]
        
    }
    destructor {
        catch {;                   # as recommended by snitfaq.
            if {[$self isActive]} {
                $self abort
            }
        }
    }
    
    #---------------------------------------------------------------------------
    #  public methods:
    
    ##
    # isActive
    #   @return Boolean - true (1) if the sequence is actively executing.
    #
    method isActive {} {
        return $active
    }
    ##
    # currentStep
    #   Returns the index into -steps that is currently executing.
    # @return integer
    # @note if the sequence is not active an error is raised.
    #
    method currentStep {} {
        if {![self $isActive]} {
            error "sequence::SequenceRunner::currentStep $options(-name) is inactive"
        }
    }
    ##
    # start
    #    Starts execution of the sequencde.
    #    If the sequence is active, this is an error
    # @note - the assumption is that there is an event loop to dispatch the
    #    after completions.
    #
    method start {} {
        if {[$self isActive]} {
            error "sequence::SequenceRunner::start - $options(-name) is already active."
        }
        
        # Special case - if there are no sequence steps, we're already done:
        
        
        if {[llength $options(-steps)] == 0} {
        
           after 0 [mymethod _dispatchEnd OK]
        } else {
            set currentStep 0
            set active      1
            set afterid     [after 0 [mymethod _preDelay]];   #schedule from the event loop.    
        }
        
    }
    ##
    # abort
    #    Abort the sequence. An error is thrown if the sequence is not active.
    #
    # @param reason - optional - reason for the abort.
    method abort {{reason {Programatically aborted}}} {
        if {![$self isActive]} {
            error "sequence::SequenceRunner::abort - $options(-name) is not active"
        }
        after cancel $afterid
        set afterid -1
        set active 0
        
        $self _dispatchEnd ABORT $reason
    }
    #--------------------------------------------------------------------------
    # non-public methods:

    #  The methods below reflect that progression of step states:
    #  *  _preDelay is called to initiate the pre-run delay that may be defined by
    #     the step.
    #  *  _runProgram initiates the program and schedules _postDelay
    #  *  _postDelay schedules _stepDone to run after any post delay in the step.
    #  *  _stepDelay detects the end of the sequence and depending on that:
    #    - Either starts the next step by incrementing currentStep and scheduleing
    #      _preDelay or
    #    - sets active to 0 and executes any -endcommand.
    #
    # @note that the only method that can fail is _runProgram which
    #       if program::run fails will abort the sequence by invoking abort.
    #
    
    ##
    # _preDelay
    #    Schedule the program to run after the preDelay value.
    #
    method _preDelay {} {
        set step [lindex $options(-steps) $currentStep]
        set waitSecs [dict get $step predelay]
        set afterid [after [expr {$waitSecs * 1000}] [mymethod _runProgram]]
        
    }
    ##
    # Start the step program and then immediately schedule the post delay
    #
    method _runProgram {} {
        set step [lindex $options(-steps) $currentStep]
        set stepNum [dict get $step step]
        set seq $options(-name)
        set seqid [::sequence::_getSeqId $options(-database) $seq]
        set programName [dict get $step program_name]
        set status [catch {
            ::program::run $options(-database)  $programName [list   \
                ::sequence::_outputHandler $options(-database) \
                    [::sequence::_monitorIndex $seqid $stepNum] \
            ]
        } msg]
        if {$status} {
            $self abort "Unable to start program $programName : $msg"
        }
        
        # Now that the program is up and running schedule the post delay:
        
        set afterid [after 0 [mymethod _postDelay]]
    }
    ##
    #  Pause sequence execution the length of any post run delay specified by the
    #  current step.
    #
    method _postDelay {} {
        set step [lindex $options(-steps) $currentStep]
        set waitSecs [dict get $step postdelay]
        set afterid [after [expr {$waitSecs * 1000}] [mymethod _stepDone]]
    }
    method _stepDone {} {
        incr currentStep
        if {$currentStep < [llength $options(-steps)]} {
            # More steps
            
            set afterid [after 0 [mymethod _preDelay]]
            
        } else {
            # Done.
            
            set active 0
            $self _dispatchEnd OK
        }
    }
    
    ##
    # _dispatchEnd
    #    Dispatch any end script
    #
    # @param why - why the  sequence ended.
    # @param reason - Reason for abort
    #
    method _dispatchEnd {why {reason {}} } {
        set userscript $options(-endcommand)
        if {$userscript ne ""} {
            lappend userscript $self $why $reason
            uplevel #0 $userscript
        }
    }
}
##
# @class sequence::TransitionManager
#    Class that manages state transitions.
#    The assumption is that an event loop is running in the idle parts of the
#    application.  This allows scheduling of the sequence elements.
#
# OPTIONS
#   -database - the database command (read only)
#   -type     - Transition type.     (read only)
#   -endscript - Optional called when the transition completes.  See
#                ::sequence::transition to get the call sequence of this script.
# METHODS
#     start   - Start the transition.
#     abort   - abort the transition.
#     isActive - True if active.
#     currentSequence - name of current sequence.
#
snit::type sequence::TransitionManager {
    option -database -default [list] -readonly 1
    option -type     -default [list] -readonly 1
    option -endscript -default [list]
    
    variable Sequences [list];    # List of sequences to execute in the transition.
    variable SequenceIndex 0;     # Which sequence we're executing.
    variable active        0;     # Nonzero when active.
    
    variable CurrentSequence [list];  # Current SequenceRunner object.
    
    ##
    # constructor
    #    After processing options, make sure we have a database and transition name
    #    - list all sequences
    #    - Add those who are for our transition into Sequences
    # @note an explicit 'start' method call is required to begin executing
    #       the transition.
    #
    # @param args - arguments passed to the constructor - these are just options.
    #
    constructor args {
        $self configurelist $args
        
        # Ensure we have the required configuration options.
        
        if {$options(-database) eq "" } {
            error "sequence::TransitionManager - Necessary -database option was not supplied"
        }
        if {$options(-type) eq "" } {
            error "sequence::TransitionManager - necessary -type option was not supplied"
        }
        # Fetch the list of sequences and filter them out into
        # Sequences (we only need the names):
        
        set allSequences [::sequence::listSequences $options(-database)]
        foreach seq $allSequences {
            if {[dict get $seq transition_name] eq $options(-type)} {
                lappend Sequences [dict get $seq name]
            }
        }
    }
    #--------------------------------------------------------------------------
    # Public methods.
    
    ##
    # start
    #   Start executing the transition.  This means that
    #   - We set active 1
    #   - We set the sequenceIndex -> 0
    #   - We start sequence execution.
    #
    method start {} {
    
        if {$active} {
            error "Transition to $options(-type) is already active."
        }
        if {[llength $Sequences] == 0} {
    
            #  This after ensures that if someone runs an evnt loop that
            #  depends on e.g. a completion routine to notify done, that
            #  this works.
            
            after 0 [mymethod _completeTransition OK]
        } else  {
        
            set active 1
            set SequenceIndex 0
            $self _executeSequence
        }
    }
    ##
    # abort
    #   Abort the execution of the transition:
    #   - We must be active.
    #   - We need to abort the sequence that's current.
    #   - We need to let any end handler script know we've aborted.
    #
    method abort {} {
        if {!$active} {
            error "Attempting to abort $options(-type) transition which is not active"
        }
        $CurrentSequence abort
        $CurrentSequence destroy
        
        set active 0
        set SequenceIndex 0
        $self _completeTransition ABORTED
    }
    ##
    # isActive
    # @return bool - true if we are active.
    #
    method isActive {} {
        return $active
    }
    ##
    # currentSequence
    #  @return string name of current sequence
    #  @note an error is raised if we are not active.
    #
    method currentSequence {} {
        if {!$active} {
            error "Cannot get current sequence on inactiv $options(-type) transition"
        }
        return [lindex $Sequence $CurrentSequence]
    }
    #--------------------------------------------------------------------------
    # Private utilities.
        
    ##
    # _executeSequence
    #   Executes the current sequence:
    #   - Fetches the sequence steps.
    #   - Creates a sequence::SequenceRunner object to run the sequence
    #     with _sequenceEnded as its endcommand.
    #   - Starts the sequence going.
    #
    # @note the assumption is that ultimately we will return to an event loop.
    #
    method _executeSequence {} {
        set seqName [lindex $Sequences $SequenceIndex]
        set steps \
            [::sequence::listSteps $options(-database) $seqName]
        set CurrentSequence [sequence::SequenceRunner %AUTO%                  \
            -database $options(-database) -name $seqName -steps $steps        \
            -endcommand [mymethod _sequenceCompleted]]
        $CurrentSequence start
    }
    ##
    # _sequenceCompleted
    #    Called when a sequence completes.
    #    - If the sequence completed properly, we just Go to the next
    #      sequence if there is one else complete the
    #      transition normally.
    #    - If the sequence aborted, we complete the transition with an abort
    #      status _unless_ the type is SHUTDOWN in which case we treat it as
    #      a normal completion.
    #
    # @param seq - the sequence runner.
    # @param reason - completion status.
    # @param failureReason - Reason for ABORT if $reason = ABORT.
    # @note  If the transition completes (success or failure), we call
    #        ::sequence::_TransitionComplete to execute transition complete business
    #        logic.  That proc gets the failure code... note that
    #        ::sequence::currentTransitionManager already holds our transition.
    #
    method _sequenceCompleted {seq reason failureReason} {
        $CurrentSequence destroy;      # clean up - don't need it anymore.
        
        if {$reason eq "ABORT"} {
            if {$options(-type) ne "SHUTDOWN"} {
                $self _completeTransition "FAILED - $failureReason"
            }
        }
        #  If we're here the sequence either completed properly or we're shutting
        #  down and we need to keep on keeping on.
        
        incr SequenceIndex
        if {$SequenceIndex < [llength $Sequences]} {
            $self _executeSequence;          # next sequence.
        } else {
            # done!
            
            $self _completeTransition OK
        }
    }
    ##
    # _completeTransition
    #   - Call any user end transition script.
    #   - Invoke ::sequence::_TransitionComplete to do all the mandatory
    #     'business logic'.
    #
    # @param how - how the transition completed. (OK, ABORTED, SHUTDOWN)
    #
    method _completeTransition {how} {
        
        # Run any user end transition script.
        
        set userscript $options(-endscript)
        if {$userscript ne ""} {
            lappend userscript $options(-database) $self $how
            uplevel #0 $userscript
        }
        
        #  Business logic method:
        
        ::sequence::_TransitionComplete $how
    }
    
}

##
# @class ShutdownManager
#    Special transition manager for shutting the system down.
#
snit::type ::sequence::ShutdownManager {
    component manager;       # Transition manager.
    
    #  I'm less certain now that there's really a difference
    #  Since TransitionManager has scads of special SHUTDOWN transition cases.
    
    delegate option * to manager
    delegate method * to manager
    constructor args {
        install manager using ::sequence::TransitionManager %AUTO% \
            -type SHUTDOWN {*}$args
    }
}
    


#-------------------------------------------------------------------------------
#  Utiltities not intended to be used by package clients.

##
# ::sequence::_TransitionComplete
#    Called to complete a state transition. Not in order:
#    - Log the new state (on success)
#    - Start a shutdown transition (on failure).
#    - Destroy the transition
#    - set the value of ::sequence::currentTransitionManager appropriately.
# @param how - how the transition completed.
#       We hope for OK.
#
proc ::sequence::_TransitionComplete {how} {
    if {$::sequence::currentTransitionManager eq ""} {
        return;             # Was aborted in some way.
    }
    set db [$::sequence::currentTransitionManager cget -database]
    set transition [$::sequence::currentTransitionManager cget -type]
    $::sequence::currentTransitionManager destroy
    set ::sequence::currentTransitionManager [list]

    # If a failure (SHUTDOWN can't fail) then start a SHUTDOWN transition.
    
    if {$how ne "OK"} {
        ::sequence::transition $db SHUTDOWN
    } else {
        
        # If this was a shutdown transition, we need to get the list of
        # all running programs and initiate kills on them.
        
        if {$transition eq "SHUTDOWN"} {
        
            foreach active [::program::activePrograms ] {
                catch {::program::kill $db $active};   # in case it's died since.
            }
            
        }

        # Update the database to the new state:
        
        set newStateId [$db eval {
            SELECT id FROM transition_name WHERE name = $transition
        }]
        $db eval {
            UPDATE last_transition SET state = $newStateId
        }

        #  Wait for i/o indicating program exit:
        
        
        
    }
}

##
# ::sequence::_outputHandler
#    Handle program output from a program.
#  - If there is an output handler pass data on to it.
#  - If EOF, and the program is critical, force as SHUTDOWN transition.
#
# @param db    - database command.
# @param monitorIndex - index into ::sequence::StepMonitors of any user level
#                monitor.
# @param program - Name of the program that is providing output.
# @param fd      - File descriptor that is readable that will give program output.
#
proc ::sequence::_outputHandler {db monitorIndex program fd} {
    #   If there is a user program monitor let it handle the input.
    
    
    if {[array names ::sequence::StepMonitors $monitorIndex] eq $monitorIndex} {
        set monitor $::sequence::StepMonitors($monitorIndex)
        uplevel #0 [list $monitor onOutput $db $program $fd]
    } else {
        set blocking [chan configure $fd -blocking]
        chan configure $fd -blocking 0
        set text "$program: [gets $fd]"
        program::_log $text
        catch  {::sequence::_relayOutput $text} msg
        program::_log "Relay : $msg"
        chan configure $fd -blocking $blocking
    }
    
    #  If we're at an EOF on input:
    #  1. Call any monitor onExit object
    #  2. If the exiting program is critical, force a shutdown.
    #  3. Let the caller close stuff etc. as ::program::_outputWrapper does take
    #      care of details like that.
    #  
    
    if {[eof $fd]} {
        ::program::_log "Sequence output handler sees EOF."
        
        set programInfo [::program::getdef $db $program ]
        
        if {[array names ::sequence::StepMonitors $monitorIndex] eq $monitorIndex} {
            set monitor $::sequence::StepMonitors($monitorIndex)
            uplevel #0 [list $monitor onExit $db $programInfo $fd]
        }
        if {[dict get $programInfo type] eq "Critical"} {
            ::sequence::_relayOutput "$program: - critical program exited"
            ::program::_log "Critical program exited"
            # Note that this could be us shutting down so we'll catch this:
            catch {::sequence::transition $db SHUTDOWN} ;    # Critical so shutdown everything.
        } else {
            ::sequence::_relayOutput "$program: - exited (non critical)"
        }

    }
}  
##
# ::sequence::_stateExists
#   Returns true if a state name already exists.
#
# @param db - database command.,
# @param name - state name to check for.
# @return boolean - true if the state/transition exists.
#
proc ::sequence::_stateExists {db name} {
    set number [$db eval {
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
    return [dict get [::program::getdef $db $program ] id]    
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
proc ::sequence::_stepAfter {db seqId after} {

    set steps [$db eval {
        SELECT step FROM step WHERE sequence_id = $seqId AND step >= $after
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
        set result [expr {$after + $::sequence::step_interval}]
    } else {
        set next [lindex $steps 1];   # Limit ensures there are only 2:
        set result [expr {($first + $next)/2.0 }]
    }
    return $result
}
##
# ::sequence::_firstStep
#   Determine the step number to assign a pre-pended (new first) step.
#   If there is no first step yet, we just use the step interval value.
#   If there is already a first step, we half it's value to assign the new
#   step number.
#
#  @param db   - database command.
#  @param id   - Sequence id
#  @return Real - first step.
#
proc ::sequence::_firstStep {db id} {
    set currentFirst [$db eval {
        SELECT step FROM step WHERE sequence_id = $id
        ORDER BY step ASC LIMIT 1
    }]
    set result $::sequence::step_interval
    if {[llength $currentFirst] == 1} {
        set result [expr {$currentFirst / 2.0}]
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
##
# sequence::_transitionExists
#   @param db  - database command.
#   @param fromid - Id of from state.
#   @param toid   -id of to state.
#   @return bool - true if this transition already exists.
#
proc sequence::_transitionExists {db fromid toid} {
    set count [db eval {
        SELECT COUNT(*) from legal_transition
        WHERE from_id=$fromid AND to_id=$toid
    }]
    return [expr {$count > 0}]
}
##
# ::sequence::_monitorIndex
#    Construct an index used to insert a monitor in the step monitor array.
#
# @param seqId - sequenceId.
# @param step  - Step number.
# @return string - index for that combination.
#
proc ::sequence::_monitorIndex {seqId step} {
    return $seqId:$step
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
        INSERT INTO sequence (name, transition_id)
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
    set seqId [::sequence::_getSeqId $db $seqName]
    set progId [::sequence::_getProgId $db $program]
    set step   [::sequence::_nextStep $db $seqId]
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
    set seqId [::sequence::_getSeqId $db $name]
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
    ::sequence::_insertStep $db $seqId $step $progId $predelay $postdelay
    
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
    set step   [::sequence::_firstStep $db $seqId]
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
    set stateId [::sequence::_stateId $db $name];   # Fires an error if nonexistent.
    
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
#  @note Nor is anything done to determine there actually _is_ a transition as
#        specified by the parameters.
#
#
proc ::sequence::rmvTransition {db from to} {
    set fromId [::sequence::_stateId $db $from]
    set toId   [::sequence::_stateId $db $to]
    
    $db eval {
        DELETE FROM legal_transition WHERE from_id = $fromId AND to_id = $toId
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
        $db eval {DELETE FROM step WHERE sequence_id = $id}
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
# ::sequence::listReachableStates
#    List states that can be reached from a specific state.
#
# @param db   - database command.
# @param state- Initial state from which we list the reacable ones.
# @return list of strings - possible from states.
#
proc ::sequence::listReachableStates {db state} {
    $db eval {
        SELECT name FROM transition_name
        WHERE id IN (
            SELECT to_id FROM legal_transition
            INNER JOIN transition_name ON from_id = id
            WHERE name = $state
        )
    }
}
##
# ::sequence::listLegalFromStates
#   List the states that can be precursors to a specific state.
# @param db  -  database command.
# @param state - State we want the precursors for.
# @return list of strings - state names that can transition to the state.
#
proc ::sequence::listLegalFromStates {db state} {
    $db eval {
        SELECT name FROM transition_name
        WHERE id IN (
            SELECT from_id FROM legal_transition
            INNER JOIN transition_name ON to_id = id
            WHERE name = $state
        )
    }    
}

##
# ::sequence::listStates
#    @param db  - database command.
#    @return list of strings - the list of defined states.
#
proc ::sequence::listStates {db} {
    return [$db eval {
        SELECT name FROM transition_name
    }]
}
##
# ::sequence::listLegalNextStates
#    Given the current state, return the names of the states that
#    are legal next states.
# @param db  - database command.
# @return  list of strings - the valid next state strings.
#
proc ::sequence::listLegalNextStates {db} {
   # What is our current state:
   
   set currentId [$db eval {SELECT state FROM last_transition}]
   
   return [$db eval {
        SELECT name FROM transition_name
        INNER JOIN legal_transition ON to_id = transition_name.id
        WHERE from_id = $currentId
    }]
   
}
##
# ::sequence::isLegalTransition
#   @param db   - database command.
#   @param next - Proposed next state.
#   @return bool- True if valid.
#
proc ::sequence::isLegalTransition {db next} {
    set legal [::sequence::listLegalNextStates $db]
    return [expr {$next in $legal}]
}
##
# ::sequenceCurrentState
#    Return current state name.
# @param db - database command.
# @return string - name of current transition.
#
proc ::sequence::currentState {db} {
    db eval {
        SELECT name FROM transition_name
        INNER JOIN last_transition ON state = id
    }
}
##
# ::sequence::listSequences
#     List the set of sequences.
# @param db  - database command.
# @return list of dicts - Each element of the list describes one sequence
#             and contains the following keys:
#            *  id   - Sequence primary key.
#            *  name - Name of the sequence.
#            *  transition_name - Name of the transition that fires the sequence.
#            *  transition_id    - Id of the transition that fires off the sequence.
#
proc ::sequence::listSequences {db} {
    set result [list]
    
    $db eval {
        SELECT sequence.id AS id, sequence.name AS name,
        transition_name.id AS trid, transition_name.name AS trname
        FROM sequence
        INNER JOIN transition_name ON transition_name.id = sequence.transition_id
    } values {

        set element [dict create                                         \
            id   $values(id)                                             \
            name $values(name)                                           \
            transition_name $values(trname)                              \
            transition_id  $values(trid)                                \
        ]
        lappend result $element
    }
    
    return $result
}
##
# ::sequence::listSteps
#     List the steps in a sequence.
#  @param db  - database command.
#  @param name - name of a sequence.
#  @return list of dicts - Each dict describes a single sequence. step
#          and has the following keys:
#          *   step   - Real step number (the list will be sorted by these)
#          *   program_name - Name of the program in the step.
#          *   program_id   - id of the program in the step.
#          *   predelay     - Seconds delay prior to the step.
#          *   postdelay    - Seconds delay after the step
#
proc ::sequence::listSteps {db name} {
    set seqid [::sequence::_getSeqId $db $name]
    
    set result [list]
    
    $db eval {
        SELECT step, program_id, predelay, postdelay, program.name AS prname
        FROM step
        INNER JOIN program on program.id = step.program_id
        WHERE step.sequence_id = $seqid
        ORDER by step ASC
    } values {

        set item [dict create                                        \
            step         $values(step)                               \
            program_name $values(prname)                             \
            program_id   $values(program_id)                         \
            predelay     $values(predelay)                           \
            postdelay    $values(postdelay)                          \
        ]
        lappend result $item
    }
    
    return $result
}
##
# ::sequence::addMonitor
#   Adds a command ensemble monitor to the program that runs in a step
#   of a sequence:
#
# @param db - database
# @param seq - Sequence name.
# @param step - Step number  (Real).
# @param monitor - Monitor command ensemble-- if empty cancels any monitor.
# @return string -prior monitor if any (empty string  if none)
# @note Step monitors are held in arrays locally.  They are not stored in the
#       database and therefore must be refreshed each time an application using
#       this facility starts.
#
proc ::sequence::addMonitor {db seq step {monitor {}}} {
    set seqId [::sequence::_getSeqId $db $seq]
    
    # Ensure there is a step in that sequence:
    
    set haveStep [$db eval {
        SELECT COUNT(*) from step WHERE step=$step AND sequence_id = $seqId
    }]
    if {!$haveStep} {
        error "$seq does not have a step numbered '$step'"
    }
    set monitorIndex [::sequence::_monitorIndex $seqId $step]
    set prior [list]
    if {[array names ::sequence::StepMonitors $monitorIndex] ne ""} {
        set prior $::sequence::StepMonitors($monitorIndex)
    }
    if {$monitor eq ""} {
        array unset ::sequence::StepMonitors $monitorIndex
    } else {
        set ::sequence::StepMonitors($monitorIndex) $monitor
    }
    
    return $prior
}
##
# ::sequence::runSequence
#    Runs a sequence.  Note the steps in a sequence are run via the event
#    loop so that I/O from early program steps is available as the sequence
#    runs and a critical program exit can, cause a sequence abort and
#    SHUTDOWN transition.
#
# @param db    - data base command - must be valid the entire duration of the
#                sequence execution (or shutdown execution if that happens).
# @param name  - Name of the sequence to run
# @param endProc - if provided will be called when the sequence either completes
#                or aborts.
#                See the SequenceRunner snit::type for more information about how
#                this is called.
# @return sequence::SequenceRunner  object command running the sequence.
#
proc ::sequence::runSequence {db name {endproc {}}} {
    # Get sequence information then create and start a sequence runner:

    set result "";                          # Retval if no steps.
    set seqSteps [::sequence::listSteps $db $name];    # Fails if invalid seq.
    set result [::sequence::SequenceRunner %AUTO% \
        -database $db -name $name -steps $seqSteps -endcommand $endproc]
    $result start
    return $result
}    
##
# ::sequence::transition
#     Run a transition.  Transitions are run semi-asynchronously (from the
#     event loop).  A few corner cases:
#     - With one exception, only one transition can be started - that is it is
#       an error to call this method when a transition has not yet completed.
#     - A SHUTDOWN transition can always be started and will interrupt any executing
#       transition; aborting it BUT:
#     - Any attempt to start a SHUTDOWN transition when one is already in progress
#       is ignored.  This is because as shutdowns  happen, programs that are
#       critical will be stopped and stopping them will trigger an attempt
#       to start another SHUTDOWN.
#     - SHUTDOWN transitions are special in that:
#       *  They do have sequences but those sequences can only contain
#          transient programs (e.g. to cleanly stop stuff).
#       *  After all steps in all SHUTDOWN sequences are run;
#          the program package is asked to stop all running programs.
#          bringing the system to a complete stop.
#    Hopefully those rules are not too confusing.  Transitions are managed
#    by a pair of snit::type objects; Ordinary transitions (not SHUTDOWN) are
#    handeled by a ::sequence::TransitionManager object while SHUTDOWN transitions
#    are handled by a ::sequence::ShutdownManager object since they operate somewhat
#    differently; see above.
#
#   @param db           - database command.
#   @param transition   - Name of transition.
#   @param endscript    - Optional script called when transition completes.
#   @note The endscript is called with the following parameters:
#       -   The database command.
#       -   The transition manager object.
#       -   A status which can be either OK, ABORTED or FAILED indicating how
#           the transition ended. Note that SHUTDOWN does not admit to failures.
#
#   @return int - 0 if the transition was started -1 if the transition was
#                   redundant (e.g. SHUTDOWn when state was SHUTDOWN.)
proc ::sequence::transition {db transition {endscript {}}} {
    
    #  If there's a shutdown in progress ignore it. Any other transition in
    #  progress is an error:
    
    if {$::sequence::currentTransitionManager ne ""} {
        set currentType [$::sequence::currentTransitionManager cget -type]
        if {($transition eq "SHUTDOWN") && ($currentType eq "SHUTDOWN") } {
            
            return  -1
        }
        if {$transition ne "SHUTDOWN"} {
            error "A transition of type $currentType is still in progress."
        }
    }
    #  If this is a shutdown and there's a current transition;
    #  abort it now:
    #
    if {$::sequence::currentTransitionManager ne ""} {
        catch {$::sequence::currentTransitionManager abort;}
        catch {$::sequence::currentTransitionManage destroy}
        set ::sequence::currentTransitionManager [list]
    }
    
    #  Require the transition be legal.
    
    if {![::sequence::isLegalTransition $db $transition]} {
        error "Invalid transition request: [::sequence::currentState $db] to $transition"
    }
    
    #  Now create and start the correct transition type:
    
    if {$transition eq "SHUTDOWN"} {
        #  If the current state is SHUTDOWN don't do anything:
        
        if {[::sequence::currentState $db] eq "SHUTDOWN"} {
            return -1
        }
        
        
        set ::sequence::currentTransitionManager [                            \
            sequence::ShutdownManager %AUTO% -database db                     \
                -endscript $endscript                                         \
        ]
    } else {
        set ::sequence::currentTransitionManager [                            \
            ::sequence::TransitionManager %AUTO% -database db -type $transition \
                -endscript $endscript
        ]   
    }
    #  Start the transition and let the event loop do the rest:
    
    $::sequence::currentTransitionManager start
    
    return 0
}
