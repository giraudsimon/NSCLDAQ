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
# @file   StateMonitor.py
# @brief  State Monitoring class framework.
# @author <fox@nscl.msu.edu>

import zmq
from nscldaq.statemanager import Utilities



##
# @class StateMonitorBase
#
#   Simplest possible state manager base class.  Initial state publication is
#   via call to the overridable initialState method and transitions to the
#   overridable Transition method.
#
#  An initializer callout allows additional poll targets to be registered.
#
#   We also provide ways to make a request of the state manager server.
#
class StateMonitorBase(object):
    ##
    # __init__
    #    Constructor
    #  @param transitionRequest - URI for the transition request port.
    #  @param statePublisher    - URI for the state publisher port.
    #  @param initializer       - Callable to call  just before exiting the
    #                             constructor.  This is passed self.
    #                              from which other stuff can be gotten.
    #
    def __init__(self, transitionRequest, statePublisher, initializer = None):
        super(StateMonitorBase, self).__init__()
        self.zmqContext         = zmq.Context()
        #
        #  Make the transition request socket and connect it to the
        #  state manager
        #
        self._requestSocket  = self.zmqContext.socket(zmq.REQ)
        self._requestSocket.connect(transitionRequest)
        
        # Make the subscription socket and subscribe to transitions
        # and to state publications.
        
        self._stateSocket = self.zmqContext.socket(zmq.SUB)
        self._stateSocket.setsockopt(zmq.SUBSCRIBE, 'STATE:')
        self._stateSocket.setsockopt(zmq.SUBSCRIBE, 'TRANSITION:')
        self._stateSocket.setsockopt(zmq.SUBSCRIBE, 'RUN:')
        self._stateSocket.setsockopt(zmq.SUBSCRIBE, 'TITLE:')
        self._stateSocket.setsockopt(zmq.SUBSCRIBE, 'RECORD:')
        
        self._stateSocket.connect(statePublisher)

        #  Make a poller and add our subscription to it.
        #
        self.poller = Utilities.ZmqFileEventLoop()
        self.poller.register(self._stateSocket, zmq.POLLIN, self._StateMessage)
        self._state = None                 # we don't know the state yet.
        self._runNumber = None              # Don't know the run number.
        self._title     = None              # Nor the title
        self._recording = None              # OR recording state for that matter.
        
        #  Now we can call the client's initializer callback:
        
        if initializer != None:
            initializer(self)
    
    def __del__(self):
        self._requestSocket.close()
        self._stateSocket.close()
        
    #-------------------------------------------------------------------------
    # private/protected utilities
    
    ##
    # _StateMessage
    #    Called when the state manager publishes a new STATE or TRANSITION  message.
    #
    # @param  poller - The poller object that fired us off.
    # @param  object - The object that became readable/writable (the socket).
    # @param  mask   - Mask of events on the object.
    #
    def _StateMessage(self, poller, object, mask):
        msg          = object.recv()
        messageParts = msg.split(':', 1)
        msgType      = messageParts[0]
        msgBody      = messageParts[1]
        
        # If the type was a STATE or TRANSITION request,
        # and the current state was None, update the state,
        # invoke InitialState and remove the 'STATE:' subscription.
        
        
        
        # STATE and TRANSITION types containe the new/current state as the body:
        
        if msgType == 'STATE' :
            self.initialState(msgBody)
        elif msgType == 'TRANSITION':
            self.transition(msgBody)
        elif msgType == 'RUN':
            self.runNumberMsg(msgBody)
        elif msgType == 'TITLE':
            self.titleMsg(msgBody)
        elif msgType == 'RECORD':
            self.recordingMsg(msgBody)
        else:
            raise RuntimeError('Invalid message type %s' % (msgType))
    
    #---------------------------------------------------------------------------
    #  Public entries.
    #
    
    
    ##
    # requestTransition
    #     Called to request a state transition.
    #
    # @param transition - The state transition request string (e.g. FAIL)
    #
    # @return string - the status of the request from the server.
    #
    def requestTransition(self, transition):
        message = 'TRANSITION:%s' % (transition)
        self._requestSocket.send(message)
        reply = self._requestSocket.recv()
        return reply
    ##
    # setRun
    #   Set a new run number:
    #
    # @param run - new run number.
    # @return reply message from server.
    #
    def setRun(self, run):
        message='RUN:%s' % (run)
        self._requestSocket.send(message)
        return self._requestSocket.recv()
    ##
    # setTitle
    #  set a new title:
    #
    # @param title - new run title.
    #
    def setTitle(self, title):
        message='TITLE:%s' % (title)
        self._requestSocket.send(message)
        return self._requestSocket.recv()
    ##
    # setRecording
    #   Set recording on or off.
    #
    # @param state - boolean desired recording state
    #
    def setRecording(self, state):
        message='RECORD:%s' % (bool(state))
        self._requestSocket.send(message)
        return self._requestSocket.recv()
    ##
    # initialState
    #   Called to set the initial state of the system.  This is normally
    #   a callback from _StateMessage but is left public so that it can
    #   be overridden by subclasses (which still should call us):
    #   * self_state is set.
    #   * We unsubscribe from the STATE: messages.
    #
    # @param state - New value for the state.
    #
    def initialState(self, state):
        self._state = state
        self._stateSocket.setsockopt(zmq.UNSUBSCRIBE, 'STATE:')
        
    ##
    # transition
    #    Called when a state transition is detected.  Note that if our current
    #    state is None we treat this as an initialState as well
    #    actual classes override this normally but should call us.
    # @param state -- the new state.  Note that self._state contains the prior
    #                 state.
    #
    def transition(self, state):
        if self._state == 'None':
            self.initialState(state)
        else:
            self._state  = state
    
    ##
    #  recordingMsg
    #    Callback when the recording state is published.
    #preint
    # @param newState
    #
    def recordingMsg(self, newState):
        self._recording = newState.upper() in ['ON', 'TRUE', 'ENABLED', True]
        
    ##
    # runNumberMsg
    #  Callback when a run number is published.
    #
    # @param body - stringified run number.
    #
    def runNumberMsg(self, body):
        self._runNumber = int(body)
    ##
    # titleMsg
    #   Callaback invoked when a title is published:
    #
    # @param body -new title.
    #
    def titleMsg(self, body):
        self._title = body
    
    ##
    # getRecording
    #   @return bool or None - recording state, None if not yet known
    #
    def getRecording(self):
        return self._recording
    
    ##
    # getRunNumber
    #   @return The last run number received or None if we haven't gotten
    #           a run number yet.
    #
    def getRunNumber(self):
        return self._runNumber
    ##
    # getTitle
    #  @return the last title or None if we haven't gotten one yet.
    #
    def getTitle(self):
        return self._title
    
    ##
    # run
    #    Run the state machine.
    #     TODO:  We could continue to take STATE messages and use timeouts
    #            to watch for the state manager exit.
    def run(self):
        while True:
            self.poller.pollForever(10)

##
# @class StateMonitor
#    This is the class most people should use.  With it users can specify
#    callbacks for specific state transitions.  Those callbacks get
#    the knowledge of the from/to states as well as us as a parameter.
#
class StateMonitor(StateMonitorBase):
    ##
    # __init__
    #   Constructor.  initialize the base class and set up the
    #   dispatch map as empty.
    #  @param transitionRequest - URI for the transition request port.
    #  @param statePublisher    - URI for the state publisher port.
    #  @param initializer       - Callable to call  just before exiting the
    #                             constructor.  This is passed self.
    #                              from which other stuff can be gotten.
    #
    def __init__(self, transitionRequest, statePublisher, initializer = None):
        super(StateMonitor, self).__init__(
            transitionRequest, statePublisher, initializer
        )
        self._dispatch = dict()
        self._runNoHandler  = None
        self._titleHandler  = None
        self._recordingHandler = None
        
    #--------------------------------------------------------------------------
    # Public methods.
    
    ##
    # register
    #   Declares a desire to know about transitions into a specific state.
    #
    # @param state    - The state we want transitions to.
    # @param callable - A callable invoked when we transition into that state.
    #                    The callable, when called is parameterized by:
    #                    *  This object.
    #                    *  The from state - if None, this is an initial
    #                       state indicating we just joined the state machine.
    #                    *  The to state.
    #                    *  An arbitrary parameter passed in (see cbarg below).
    # @param cbarg    - An parameter that is passed without interperetation to
    #                   the callable.
    #
    def register(self, state, callable, cbarg):
        self._dispatch[state.upper()] = [callable, cbarg]
    ##
    # unregister
    #   Removes the registration of a handler for a state transition.
    #
    # @param state - The target state we no longer want handled.
    #
    def unregister(self, state):
        del self._dispatch[state.upper()]
    ##
    # setRunNumberHandler
    #
    #   Defines a callable that will be invoked when the run number changes or
    #   first becomes known.
    #
    # @param callable - The callable to invoke with the new run number or
    #                   None to disable the existing callback.
    #
    def setRunNumberHandler(self, callable):
        self._runNoHandler = callable
        
    ##
    # setTitleHandler
    #   Defines the callable that will be invoked when the title string changes.
    #
    # @param callable - The callable to invoke or None to disable all callbacks.
    #
    def setTitleHandler(self, callable):
        self._titleHandler = callable
        
    ##
    # setRecordingHandler
    #    Defines a callback invoked when the recording state changes.
    #
    # @param callable - the callable to invoke. or None to disable.
    #
    def setRecordingHandler(self, callable):
        self._recordingHandler = callable
    ##
    # initialState
    #   Override of base class initial stae.
    #   In addition to all of the actions of the base class's initialState
    #   method, if there is a handler for the state it will be invoked with a
    #   prior state as None
    #
    # @param state - New state.
    #
    def initialState(self, state):
        super(StateMonitor, self).initialState(state)
        state = state.upper()
        if state in self._dispatch.keys():
            callable = self._dispatch[state][0]
            param    = self._dispatch[state][1]
            
            callable(self, None, state, param)
    ##
    # transition
    #  Overrides the base class transition method.
    #  * save the prior state.#
    #  * run the base class method.
    #  * If the prior state was not None, invoke
    #    any handler the target state has.
    #
    # @param state - new state name.
    #
    def transition(self, state):
        priorState = self._state.upper()                # In base class...
        state = state.upper()
        super(StateMonitor, self).transition(state)
        if priorState != None and state in self._dispatch.keys():
            callable = self._dispatch[state][0]
            param    = self._dispatch[state][1]
            callable(self, priorState, state, param)
            
    ##
    # runNumberMsg
    #   Called on a run message.
    #   If the run number changes, and there's a handler invoke it.
    #
    # @param body  - the body of the msg.
    #
    def runNumberMsg(self, body):
       oldNo = self.getRunNumber()
       super(StateMonitor, self).runNumberMsg(body)
       current = self.getRunNumber()
       if (oldNo != current) and (self._runNoHandler != None):
           self._runNoHandler(current)
    ##
    # titleMsg
    #   Called on a title message.  Action is similar to the
    #   runNumberMsg override.
    #
    # @param body - the message body.
    #
    def titleMsg(self, body):
        oldTitle = self.getTitle()
        super(StateMonitor, self).titleMsg(body)
        currentTitle = self.getTitle()
        if (oldTitle != currentTitle) and (self._titleHandler != None):
            self._titleHandler(currentTitle)
    ##
    # recordingMsg
    #    Called on a recording msg
    #
    # @param body - the message body.
    #
    def recordingMsg(self, body):
        oldState = self.getRecording()
        super(StateMonitor, self).recordingMsg(body)
        currentState = self.getRecording()
        if currentState != oldState and (self._recordingHandler != None):
            self._recordingHandler(currentState)