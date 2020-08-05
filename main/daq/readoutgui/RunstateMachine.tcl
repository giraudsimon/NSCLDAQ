#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file RunstateMachine.tcl
# @brief Run State Machine Package
# @author Ron Fox <fox@nscl.msu.edu>

package provide RunstateMachine 1.0
package require snit

##
# @class RunstateMachine
#
#    This snit object provides a run state machine for the readoutGUI
#    The statemachine relies on action callouts that are called when
#    entering an leaving states.  Callouts are managed as 'bundles'  which are
#    basically namespaces with well defined commands exported.
#    These commands are:
#      *  enter  - Called when entering a state, passed the old state, newstate
#      *  leave  - Called when leaving a state, passed the old state, new state.
#      *  attach - Called when added to the callouts list.  Passed the
#                  current state.
#
#    It is up gto the client to ensure the namespaces registered exist.
#    Note that a side effect of registration is to create a namespace
#    ensemble out of the namespace.
#
# States are:
#    *  NotReady - No data sources are active.
#    *  Starting - The data sources are now being started.
#    *  Halted   - Data sources are active, but the run is not.
#    *  Active   - Data taking is in progress.
#    *  Paused   - Data taking is paused.
#
#  All state transitions are triggered by the transition method.
#  Legality of transitions are checked against the state machine definition
#  and errors thrown if an illegal transition is attempted.
#
# METHODS
#   listCalloutBundles
#   addCalloutBundle
#   removeCalloutBundle
#   transition
#   listStates
#   listTransitions
#   getState
#
#  daqdev/NSCLDAQ#659 - We keep track of the successful bundle calls and, if one
#                       fails we call the optional method failed in the successful
#                       bundles.
#
#  Any methods that start with _ are considered private.  Note that
#
#   _setState  will be used by the tests to force the state machine to a
#              specific state prior to a test.  This method does not invoke
#              any bundle methods, nor does it check the legality of the state
#              transition being forced.
#
snit::type RunstateMachine {
    #--------------------------------------------------------------------------
    #  Class variables:
    
    #
    #   validTransitions defines the legal edges of the state machine.
    #   this is an array indexed by state name with values a list of valid transitions.
    #
    
    typevariable validTransitions -array {
        NotReady {NotReady Starting}
        Starting {NotReady Halted}
        Halted   {NotReady Active}
        Active   {NotReady Halted Paused}
        Paused   {NotReady Halted Active}
    }
    
    # Array indexed by callback name contents are the number of parameters the
    # callback must accept.
    
    typevariable requiredExports -array [list \
        enter 2 leave 2 attach 1]
    
    #---------------------------------------------------------------------------
    #  Instance variables
    
    # Current state.
    
    variable state NotReady
    
    #  List of currently registered bundle namespaces.
    
    variable callouts [list]
    
    ##
    # constructor
    #   Does nothing at this time.
    #
    constructor args {
        
    }
    #---------------------------------------------------------------------------
    # Public methods:
    #
    
    ##
    # listSates
    #
    #  Provides a list of the states the machine knows about. Note that
    #  the states are alphabetized so that the order is predictable.
    #
    # @return list  alphabetized list of allowed states.
    typemethod listStates {} {
        lsort -ascii -increasing [array names validTransitions]
    }
    ##
    # listTransitions
    #
    #   Given a state lists the valid target states for the transition method
    #
    # @param stateName - The state being queried.
    # @return list     - list of valid next states.
    #
    typemethod listTransitions stateName {
        return $validTransitions($stateName)
    }
    ##
    # getState
    #  Returns the current state name
    #
    # @return string - name of current state
    #
    method getState {} {
        return $state
    }
    ##
    # listCalloutBundles
    #
    #   Lists the set of namespaces that are established as callout bundles.
    #   This list defines what happens on state transitions...specifically,
    #   on leaving an old state and on entering a new state.
    #
    method listCalloutBundles {} {
        return $callouts
    }
    
    ##
    # addCalloutBundle
    #
    #   Adds a new callout bundle to the state machine.  To be added (preconditions);
    #   *   The stated namespace must exist.
    #   *   The stated namespace must not already be registered.
    #   *   The stated namespace must export the attach, enter,leave procs.
    #   *   The stated namespace's attach, enter, leave procs must have the
    #       correct number of parameters.
    #
    # @param name - Name of the bundle to add.  This is assumed to be relative to
    #               the global namespace e.g. junk -> ::junk
    # @param where - If supplied, the bundle is added prior to the specified existing
    #                bundle, if not it's appended.
    #
    # @note When successful (postconditions):
    #   *  The bundle name is added to the callouts variable.
    #   *  A namespace ensemble is generated for that bundle.
    #
    method addCalloutBundle {name {where ""}} {
        if {![namespace exists ::$name]} {
            error "No such bundle $name"
        }
        if {[lsearch -exact $callouts $name] != -1} {
            error "$name is already registered as a callback bundle"
        }
        $self _checkRequiredExports $name
        $self _checkRequiredParams $name
        

        #  Now we can make a namespace ensemble from the bundle and
        #  add it to the callout list.
        
        namespace eval ::${name} {
            namespace ensemble create
        }
        
        if {$where eq "" } {
            lappend callouts $name
        } else {
            set idx [lsearch -exact $callouts $where]
            if {$idx == -1} {
                error "Attempt to register callout bundle $name before $where which does not exist"
            }
            set callouts [linsert $callouts $idx $name]
        }
        
        # Finally invoke the attach method:
        
        ::$name attach $state
    }
    
    ##
    # removeCalloutBundle
    #
    #   Remove a callout bundle:
    #   * Ensure the bundle is registered.
    #   * Remove it from the callouts list.
    #   * Undefine its namespace ensemble.
    #
    # @param name - Name of namespace that holds the bundle methods.
    #
    method removeCalloutBundle name {
        set index [lsearch -exact $callouts $name]
        if {$index == -1} {
            error "$name has not been registered"
        }
        set callouts [lreplace $callouts $index $index]
        rename ::${name} {}
    }
    ##
    # transition
    #
    #    Request a state transition.
    #
    # @param to   - Target state.
    #
    # @error the transition is to a prohibited state.
    #
    method transition to {
        if {$to in $validTransitions($state)} {
            $self _callback leave $state $to
            set from $state
            set state $to
            $self _callback enter $from $to
        
        } else {
            error "The transtion from $state to $to is not allowed"
        }
    }

    ##
    # Run a precheck to detect innevitable errors for a transition
    #
    # If a callout bundle implements this method, then there is the possibility
    # for the bundle to report an impending error for an upcoming transition.
    # This method does nothing more than produce a list of all the errors that 
    # have been detected as a list of lists. Each element of the list is a 
    # list with the first element being the name of the callout bundle and the 
    # second element being the error message. In other words, if two callout
    # bundles, e.g. bundle0 and bundle1, report errors the result might be:
    #  
    #  [list bundle0 {error found in bundle0} [list bundle1 {error for bundle1}]]
    #
    # Callout bundles that do not use implement this bundle are simply ignored. On
    # the other handle, a callout bundle that implements this must provide a proc
    # in its namespace that has the following signature:
    #
    # @code
    # proc ::BundleName::precheckTransitionForErrors {from to} {
    #   # code for bundle
    # }
    # @endcode
    #
    # @param to   state to transition to
    # @returns list of detected problems.
    #
    method precheckTransitionForErrors {to} {
      set errors [list]
      foreach cb $callouts {
        set error [_callBundleMethodIfExists $cb precheckTransitionForErrors  $state $to]
        if {$error ne {}} {
            lappend errors [list $cb $error]
        }
      }
      return $errors
    }
    
    #--------------------------------------------------------------------------
    #
    # Private methods
    #
    ##
    # _setState
    #   Set the state (testing method).
    #
    # @param newState - Requested new state
    #
    # @note no callbacks are performed and no legality checking is done.
    #
    method _setState newState {
        set state $newState
    }
    ##
    #  _callback
    #    Perform a callback in the list of callbacks.
    #
    # @param method - Name of the namespace callback method (e.g. enter)
    # @param args     - Arguments to hand to the callback method
    #
    #  daqdev/NSCLDAQ#659 - we keep track of the successful callbacks.
    #  If there is a callback failure:
    #   - Call the failure method on each of the bundles passing the same
    #     args we got (normally this will be from/to)
    #   - re-signal the failure along with the original traceback.
    #   - any failures of the failure method are just absorbed and dropped.
    #
    method _callback {method args} {
        
        set succeeded [list]
        foreach cb $callouts {
            if {[catch {$cb $method {*}$args} msg]} {
                set traceback $::errorInfo
                foreach s [lreverse $succeeded] {
                    catch {_callBundleMethodIfExists $s failure $args} 
                }
                error "$msg from $traceback"
            }
            lappend succeeded $cb
        }
    }
    ##
    # _checkRequiredExports
    #
    #   Checks that a namespace has the required exports for a callback bundle.
    #   Throws an error if not.
    #
    # @param name - Name of the namespace to check
    # @throw If one or more exports is missing.
    #
    method _checkRequiredExports name {
        
        # Make a list of missing exported procs.  The only way to figure out which
        # procs are exported from a namespace is to do a namespace export in the
        # context of the namespace itself:
        
        set exports [namespace eval ::${name} { namespace export}]
        set missingProcs [list]
        
        #
        #  The sort below is done to make the output repeatable/predictble
        #  and hence testable.
        #
        foreach proc [lsort -ascii -increasing [array names requiredExports]] {
            if {[lsearch -exact $exports $proc] == -1} {
                lappend missingProcs $proc
            }
        }
        if {[llength $missingProcs] > 0} {
            set missingList [join $missingProcs ", "]
            error "$name is missing the following exported procs: $missingList"
        }
    }
    ##
    # _checkRequiredParams
    #
    #  Checks that a namespace that is being proposed as a callback bundle
    #  has the right number of parameters for each of the required exported procs.
    #
    # @param name - Path to the namespace relative to ::
    #
    # @throw If one or more required exports has the wrong argument signature.
    #
    method _checkRequiredParams name {
        
        set badProcs [list]
        set actualArgs [list]
        set requiredArgs [list]
        foreach proc [lsort -ascii -increasing [array names requiredExports]] {
            set params [llength [info args ::${name}::${proc}]]
            if {$params != $requiredExports($proc)} {
                lappend badProcs     $proc
                lappend actualArgs   $params
                lappend requiredArgs $requiredExports($proc)
            }
        }
        if {[llength $badProcs] > 0} {
            foreach proc $badProcs required $requiredArgs actual $actualArgs {
                lappend badList       ${proc}(${actual})
                lappend requiredList  ${proc}(${required})
            }
            set badList [join $badList ", "]
            set requiredList [join $requiredList ", "]
            error "$name has interface procs with the wrong number of params: $badList should be: $requiredList"
        }
    }
    #----------------------------------------------------------------------------
    #  Private procs.
    
    ##
    # _callBundleMethodIfExists
    #    If the specified method of the specified bundle exists it is called.
    #    This is used to allow for optional methods see e.g.
    #    issue: daqdev/NSCLDAQ#659
    #
    # @param bundle     - bundle namespace name.
    # @param methodname - Name of the bundle method
    # @param args       - args to pass.
    # @return - whatever the callback returns if it exists, "" if not.
    #
    #   @note if ::$bundle::$methodname exists its' called:
    #            ::$bundle::$methodname {*}$args
    #
    proc _callBundleMethodIfExists {bundle methodname args} {
        set qualifiedProc ::${bundle}::$methodname
        if {[info procs $qualifiedProc] ne ""}  {
            return [$qualifiedProc {*}$args]
        } else {
            return ""
        }
    }
}



## A run state machine to use when enslaved by a master ReadoutGUI
#
# This is really just a wrapper around a local RunstateMachine snit::type.
# The difference here is that the transition method is redefined to forward
# requests to a connected master ReadoutGUI. It then provides an extra
# method called masterTransition that is invoked by the master to initiate
# a local transition. 
#
snit::type ClientRunstateMachine {

  component localStateMachine

  delegate method * to localStateMachine
  delegate option * to localStateMachine

  ## @brief Construct the local runstate machine
  #
  constructor {localStMachine} {
    install localStateMachine using set localStMachine
  }

  ## @brief Handle transitions initiated by the local ReadoutGUI
  #
  # If the ReadoutGUI is connected to a master ReadoutGUI (i.e. is enslaved),
  # then the transition request is forwarded to the master. On the other hand,
  # if it is not enslaved, then it simple initiates a state transition using the
  # local runstate machine.
  #
  # @param  to    state to transition to
  method transition to {

    if {[$self _isConnectedToMaster]} {
      # we have not forwarded the request yet to the master
      # so do it and then set flags to handle the next transition
      # locally
      $::RemoteControlClient::control send "transitionTo $to"
    } else {
      $localStateMachine transition $to 
    }

  }

  ## @brief Response to a master-initiated state transition
  #
  # This simple invokes a state transition using the local 
  # runstate machine
  #
  # @param  to  the state the transition to
  #
  method masterTransition to {
        $localStateMachine transition $to
  }

  ##  @brief Determine whether the system is enslaved and connected
  #
  # @returns boolean
  # @retval 0 - not connected to a master ReadoutGUI
  # @retval 1 - connected to a master ReadoutGUI
  #
  method _isConnectedToMaster {} {
    set retval 0

    # check to see if the user has enabled the remote control package.
    # If they have, the RemoteControlClient namespace will exist.
    if {[namespace exists ::RemoteControlClient]} {
      # next we need to check to see if there is actually a connection object
      # in existence. It is possible to not be connected to the master even though
      # the user enable the remote control capability
      if {$::RemoteControlClient::control ne {}} {
      # get the connection status
        set connectionStatus [$::RemoteControlClient::control getConnectionStatus]

        # if the connection is healthy, then we are all set.
        if {[lindex $connectionStatus 1]} { 
          set retval 1
        }
      }
    }

    return $retval
  }
  

}; # end of ClientRunstateMachine snit::type



## @brief A snit::type that delegates methods to the appropriate Runstate machine
#
# This is an implementation of the State design pattern. The type contains both a
# normal RunstateMachine and a ClientRunstateMachine instance. Depending on whether
# the user has enabled the remote control package, the delegation will proceed through
# either the ClientRunstateMachine instance or the RunstateMachine. 
#
# This object is used by the RunstateMachineSingleton.
#
snit::type RemoteControllableRunstateMachine {
  component localStateMachine     ;# instance of a RunstateMachine
  component clientStateMachine    ;# instance of a CLientRunstateMachine

  component currentStateMachine   ;# "pointer" to the appropriate RunstateMachine

  delegate method * to currentStateMachine
  delegate option * to currentStateMachine

  ## @brief Initialize our run state machines
  #
  constructor args {
    install localStateMachine using RunstateMachine %AUTO%
    install clientStateMachine using ClientRunstateMachine %AUTO% $localStateMachine

    install currentStateMachine using set localStateMachine

    $self configurelist $args
  }


  ## @brief Destroy the run state machines
  #
  destructor {
    $localStateMachine destroy
    $clientStateMachine destroy
  }

  ## @brief Switch to state machine appropriate for the enslaved state
  #
  # @param value  boolean indicating if new state is an enslaved state (use values 0 or 1)
  #
  method setSlave {value} {
    if {$value} {
      set currentStateMachine $clientStateMachine
    } else {
      set currentStateMachine $localStateMachine
    }
  }

  ## @brief Check whether the instance delegates to a runstate machine for remote control ops
  #
  # @return boolean 
  # @retval 0 - local control
  # @retval 1 - remote controllable
  method isSlave {} {
    return [expr {$currentStateMachine eq $clientStateMachine}]
  }
  
}

##
# @class RunstateMachineSingleton
#
#   This class should be created rather than a RunstateMachine.  It is basically
#   a facade that enforces the singleton pattern on the RunstateMachine type.
#
#
snit::type RunstateMachineSingleton {
    component StateMachine
    delegate method * to StateMachine
    delegate option * to StateMachine
    
    typevariable actualObject ""
    
    ##
    #  The constructor creates actuaObject if necessary
    #  once done with that, installs it as the StateMachine
    #
    constructor args {
        if {$actualObject eq ""} {
            set actualObject [RemoteControllableRunstateMachine %AUTO%]
        
        }
        # The delegates and this magic take care of the rest.
        
        install StateMachine using set actualObject
        
    }
}
namespace eval RunStateMachineConvenience {
    
}
##
# RunStateMachineConvenience::removeBundle
#   Convenience proc to remove a callout bundle from the state machine
#  singleton
#
proc RunStateMachineConvenience::removeBundle {name} {
    set sm [RunstateMachineSingleton %AUTO%]
    $sm removeCalloutBundle $name
    $sm destroy
}


##------------------------------------------------------------
# Convenience functions
#
namespace eval Pending {
    variable pendingState None;    # During state transitions this will indicate the next state.
}

##
# Global start proc to transition from NotReady -> Starting -> Halted
#
#  The transition from NotReady to Halted involves an intermediate 
#  transition through the Starting state. We therefore perform two
#  transitions in this proc. If either of them fail, the state 
#  machine is forced to do an emergency transition to NotReady.
#
proc start {} {


  set machine [RunstateMachineSingleton %AUTO%]
  set state [$machine getState]
  if {$state eq "Starting"} return;                      # don't allow double start.
  # Transition NotReady -> Starting
  set Pending::pendingState Starting
  if { [catch { $machine transition Starting } msg] } {
    set trace $::errorInfo
    forceFailure
    $machine destroy
    error "start failed with message : $msg : $trace"
  }

  $machine destroy
  
  set Pending::pendingState Halted
  # Transition Starting -> Halted
  after idle {
    set machine [RunstateMachineSingleton %AUTO%]
    if {[catch {$machine transition Halted} msg]} {
      set trace $::errorInfo
      forceFailure
      $machine destroy
      error "transition to halted failed with message : $msg : $trace"
    }
    $machine destroy
  }
  set Pending::pendingState None

}

proc begin {} {
    set machine [RunstateMachineSingleton %AUTO%]
    set state [$machine getState]
    if {$state eq "Active"} return;              # don't allow double begin.
    set Pending::pendingState Active
    if { [catch { $machine transition Active } msg] } {
        set trace $::errorInfo
        forceFailure
        error "begin failed with message : $msg : $trace"
  }
  set Pending::pendingState None
  $machine destroy
}

proc end {} {
  set machine [RunstateMachineSingleton %AUTO%]
    set state [$machine getState]
    if {$state eq "Halted"} return;              # don't allow double end
    set Pending::pendingState Halted
    if { [catch { $machine transition Halted } msg] } {
        set trace $::errorInfo
        forceFailure
        error "end failed with message : $msg : $trace"
  }
  set Pending::pendingState None
  $machine destroy
}

proc pause {} {
  set machine [RunstateMachineSingleton %AUTO%]
  set Pending::pendingState Paused
  if { [catch { $machine transition Paused } msg] } {
    set trace $::errorInfo
    forceFailure
    error "pause failed with message : $msg : $trace"
  }
  set Pending::pendingState None
  $machine destroy
}

proc resume {} {
  set machine [RunstateMachineSingleton %AUTO%]
  set Pending::pendingState Active
  if { [catch { $machine transition Active } msg] } {
    set trace $::errorInfo
    forceFailure
    error "resume failed with message : $msg : $trace"
  }
  set Pending::pendingState None
  $machine destroy
}

proc forceFailure {} {
  set machine [RunstateMachineSingleton %AUTO%]
  set Pending::pendingState NotReady
  if { [catch { $machine transition NotReady } msg] } {
    set trace $::errorInfo
    error "Transition to not ready failed with message : $msg : $trace"
  }
  set Pending::pendingState None
  $machine destroy    
}

proc transitionTo {to} {
  set machine [RunstateMachineSingleton %AUTO%]
  set Pending::pendingState $to
  set retCode [catch [$machine transition $to] msg]
  $machine destroy

  if {$retCode} {
    return -code error $msg
  } else {
    set Pending::pendingState None
    return $msg
  }
}
