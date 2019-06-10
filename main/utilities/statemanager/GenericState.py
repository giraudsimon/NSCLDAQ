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
# @file   GenericState.py
# @brief  Code that handles typical states.
# @author <fox@nscl.msu.edu>

from nscldaq.statemanager import Utilities


##
# State
#   This code handles a generic state.  The request port is polled for
#   transitions.  Legal transitions are taken while illegal transitions are
#   rejected.
#
# @param cargo  - Dict with the following keys/values:
#                 * publish - The publish socket.
#                 * request - The request socket.
#                 * transitions- A dict whose keys are states and whose
#                   values are themselves dicts containing keys that are valid
#                   transition requests and values that are the target state name.
# @return pair   - The new state name and cargo again.
#
def State(cargo):
    # Figure out what state I'm in by asking the state machine
    
    state = Utilities.stateMachine.getState()
    
    # Fish out what we need from the cargo:  the request socket,
    # the publish socket and the appropriate transtion map:
    
    print("%s entered" % (state))
    publishSock = cargo['publish']
    requestSock = cargo['request']
    transitions = cargo['transitions'][state]
    
    while True:
        while not Utilities.checkRequest(requestSock, 1000, 1000, Utilities.publishState, publishSock):
            pass
        # There's a request:
        
        info    = Utilities.getRequest(requestSock)
        if info != None:
            action = info[0]
            param  = info[1]
            
            # Dispatch based on the action:
            
            if action == 'TRANSITION':    # State transition request
                if param in transitions.keys():
                    # Legal transition:
                    nextState = transitions[param]
                    requestSock.send('OK')
                    Utilities.publishTransition(publishSock, nextState)
                    return nextState.upper(), cargo     # Exit state.
                else:
                    # Invalid state transition:
                    
                    validTransitions = ', '.join(transitions.keys())
                    requestSock.send(
                        'FAIL - Valid transitions requests are %s' % (validTransitions)
                    )                
                    
            elif action == 'RUN':     # New run number must be +-ive integer
                try:
                    param = int(param)
                    isInteger = True
                except:
                    isInteger = False
                if isInteger and (param >= 0):
                    Utilities.run = param
                    Utilities.publishRun(publishSock, param)
                    requestSock.send('OK')
                else:
                    requestSock.send('FAIL Run number must be a positive integer')
            elif action == 'TITLE':     # New title.
                Utilities.title = param
                Utilities.publishTitle(publishSock, param)
                requestSock.send('OK')
            elif action == 'RECORD':
                Utilities.recording = param.upper() in ['ON', 'TRUE', 'ENABLED']
                Utilities.publishRecording(publishSock, Utilities.recording)
                requestSock.send('OK')
            
            
            