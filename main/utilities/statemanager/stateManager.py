#!/usr/bin/env python



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
# @file   stateManager.py
# @brief  Main for the state manager application.
# @author <fox@nscl.msu.edu>

##
# The state manager application supplies a central server that manages
# the state of an experiment.

from nscldaq.statemanager import StateMachine, GenericState, Utilities
from nscldaq.portmanager import PortManager

import zmq



#----------------------------------------------------------------------------
#  Globally useful functions:




#-----------------------------------------------------------------------------
# Program entry points.
#

# We need to obtain the following server ports:
#   StatePublish - Publishes the state and the transitions.
#   StateRequest - Send state transition requests here.
#
# TODO:  Command line parameters can override these names.
#
portMgr = PortManager.PortManager('localhost')
publishPort = portMgr.getPort('StatePublish')
requestPort = portMgr.getPort('StateRequest')

#  Create the command request/reply port and the state publish port:

zmqContext    = zmq.Context(1)
requestSocket = Utilities.createRequestPort(zmqContext, requestPort)
publishSocket = Utilities.createPublishPort(zmqContext, publishPort)


## Set up the states in the state machine and run it.

Utilities.stateMachine = StateMachine.StateMachine()
Utilities.stateMachine.add_state('NotReady', GenericState.State)
Utilities.stateMachine.set_start('NotReady')

Utilities.stateMachine.add_state('Ready', GenericState.State)
Utilities.stateMachine.add_state('Booting', GenericState.State)
Utilities.stateMachine.add_state('Active',  GenericState.State)

## Dummy end state:
#

Utilities.stateMachine.add_state('Exit', None, 1)


##
#  The dicts below map the valid state transitions for each state.
#  they consist of a set of request strings and the state that results
#  from receiving them.   These are used by GenericState.State to know
#  how to respond to state transitions.

# State manager upper cases its states.

NotReadyTransitions = {'BOOT' : 'Booting', 'EXIT'  : 'Exit'}
BootingTransitions  = {'FAIL' : 'NotReady', 'UP'   : 'Ready'}
ReadyTransitions    = {'FAIL' : 'NotReady', 'STOP' : 'NotReady', 'BEGIN' : 'Active'}
ActiveTransitions   = {'FAIL' : 'NotReady', 'END' : 'Ready'}


Transitions         = {'NOTREADY' : NotReadyTransitions,    \
                       'BOOTING'  : BootingTransitions,     \
                       'READY'    : ReadyTransitions,       \
                       'ACTIVE'   : ActiveTransitions       \
                       }


##
# Run the state machine:

Utilities.stateMachine.run(
    {'publish' : publishSocket, 'request': requestSocket,
     'transitions' : Transitions})

# We fall through here from the exit state.  Publish the transition and
# close up shop:

# Utilities.publishTransition(publishSocket, 'Exit')

requestSocket.close()
publishSocket.close()
zmqContext.destroy()
zmqContext.term()



