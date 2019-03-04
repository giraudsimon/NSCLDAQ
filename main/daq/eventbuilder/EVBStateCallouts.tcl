
package provide EVBStateCallouts 11.0


## @brief State machine callouts for the event builder
#
# Normally in the readout callouts, the state machine is not
# aware of the event builder. Instead, the EVB is set up using
# the readout callouts mechanism with the evbcallouts package.
# In reality, there is nothing special that happens in the 
# evbcallouts.tcl that cannot be encapsulated into set of state
# machine callouts.
#
#
namespace eval EVBStateCallouts {

  ## No-op.
  #
  # We want to configure this elsewhere so we will pass
  #
  proc attach {state} {
  }

  ## @brief Handle state entrances
  #
  # End run type operations are treated as if they are an end run
  # Any transition to NotReady kills the event builder.
  #
  # @param from   the previous state
  # @param to     the new state
  #
  proc enter {from to} {
    if {($from in [list Active Paused]) && ($to eq "Halted")} {
      EVBC::onEnd
    } 

    # This transition is a failure mode... we shouldn't expect 
    # an end run. Because EVBC::onEnd blocks until an end run,
    # it is not safe here because we could deadlock waiting for an 
    # END_RUN that never comes along.
    if {($from in [list Active Paused Halted]) && ($to eq "NotReady")} {
      EVBC::stop
    } 

  }

  ## Leave a state
  #
  # The only thing treated here is the transtion into Active
  # This is understood as a begin run type operation.
  #
  # @param from   the previous state
  # @param to     the new state
  #
  proc leave {from to} {

    if {($from eq "Halted") && ($to eq "Active")} {
      EVBC::onBegin
    }

  }

  ## @brief Register the package to the state machine
  #
  # This is just a convenience method for registering the 
  # callouts to the state machine.
  #
  proc register {} {
    set sm [::RunstateMachineSingleton %AUTO%]
    $sm addCalloutBundle EVBStateCallouts
    $sm destroy
  }

  # The necessary exports required by the state machine.
  namespace export attach enter leave

}

