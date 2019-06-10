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
# @file   Utilities.py
# @brief  General utility methods
# @author <fox@nscl.msu.edu>

import zmq
import getpass
import subprocess
from nscldaq.portmanager import PortManager


##
#  Global state machine variables (only valid in the stateManager app)
#
#   stateMachine - The instance of the state machine.
#   title        - Current run title.
#   run          - Current Run number.
#   recording    - True if the run is being recorded.
#

stateMachine = None
title        = None
run          = None
recording    = False

##
# _user
#   Given a possibily default user returns the actual user.
#
# @param user - None if default is desired.
# @return string
#
def _user(user):
    #
    #  Default the user to us.
    #
    if user == None:
        user = getpass.getuser()
        
    return user

##
# _getPort
#   Get the published port of the state manager:
#
# @param host - host on which the manager is running
# @param service - service name.
# @param user    - user name.
# @return port   - the port number.
#
def _getPort(host, service, user):
    pm = PortManager.PortManager(host)
    info = pm.find(service=service, user=user)
    if len(info) != 1:
        raise RuntimeError("Can't figure out the connection parameters for the state manager")
    port = info[0]['port']
    
    return port

##
# getPort
#   make _getPort public.
# 
def getPort(host, service, user):
    return _getPort(host, service, user)

##
# createPublishPort
#   Create the zmq state publisher port.
#
# @param context - zmq context
# @param port    - The port on whichwe will listen:
#
def createPublishPort(context, port):
    socket  = context.socket(zmq.PUB)
    binding = 'tcp://*:%d' %port
    socket.bind(binding)
    return socket

##
# createRequestPort
#   Creates the zmq port on which requests for state transitions, etc
#   will be recieved.
# @param context - ZMQ context.
# @param port  - The port.
# @return zmq socket.
#
def createRequestPort(context, port):
    socket  = context.socket(zmq.REP)
    binding = 'tcp://*:%d' % port
    socket.bind(binding)
    return socket

##
# connectRequestPort
#   Connect to the zmq port on which requests for state transitions are
#   transmitted for a state manager (this is a 'REQ') socket.
#
# @param context - zmq context object.
# @param host    - Host on which the state manager is running (default localhost)
# @param service - Service name (defaults to StateRequest)
# @param user    - User running the state manager, defaults to this user.
#
# @return zmq.socket - Socket on which requests can be sent and on which
#                      status will be received.
#
def connectRequestPort(context, host='localhost', service='StateRequest', user=None):
    user = _user(user)
    
    #  Figure out the port for the state manager's requests:
    
    port = _getPort(host, service, user)
    uri  = 'tcp://%s:%d' %(host, port)
    
    socket = context.socket(zmq.REQ)
    socket.connect(uri)
    
    return socket
##
# subscribe
#   Create a socket that subscribes to the state manager state/transition
#   broadcasts
#
# @param context         - zmq context object.
# @param wantTransitions - True if you want to get transition notifications.
# @param wantState       - True if you want state notifications.
# @param host            - Which host the state manager is running on (default =localhost)
# @param service         - Set to override the default service name.
# @param user            - Set to override the default username.
#
# @return zmq.socket     - which can be recv'd to get subscription updates.
#
def subscribe(context, wantTransitions, wantState, host='localhost', service='StatePublish', user=None):
    
    user = _user(user)
    
    port = _getPort(host, service, user)
    uri  = "tcp://%s:%d" % (host, port)
    
    socket = context.socket(zmq.SUB)
    socket.connect(uri)
    
    # Set the requested subscriptions
    
    if wantTransitions:
        socket.setsockopt(zmq.SUBSCRIBE, 'TRANSITION:')
        socket.setsockopt(zmq.SUBSCRIBE, 'RUN:')
        socket.setsockopt(zmq.SUBSCRIBE, 'TITLE:')
    if wantState:
        socket.setsockopt(zmq.SUBSCRIBE, 'STATE:')
        
    return socket


##
# checkRequest
#
#   Use a poller to wait for input on the specified port.
#   There are two timeouts:  The total amount of time to wait and
#   how long each wait should be before calling the callback.
#   the callback would normally be used to invoke publishState
#   This allows a long total block time while continuing to periodically
#   tell the world our state e.g.
#
# @param socket        - socket on wich we are polling.
# @param pollTimeout - milliseconds for each poll request.
# @param maxPolls    - Total number of polls before returning.
# @param callback    - Callback to invoke between polls.  Set this to None if you
#                      don't want a callback.
# @param cbArg       - Argument passed to the callback.
#
# @return  boolean   - True if the poll says there's input
#                      False if maxPolls were done without input being ready.
#
def checkRequest(socket, pollTimeout, maxPolls, callback, cbArg):
    poller = zmq.Poller()
    poller.register(socket, zmq.POLLIN)
    for i in range(0, maxPolls):
        events = poller.poll(pollTimeout)
        if callback != None:
            callback(cbArg)
        if len(events) > 0:
            return True
    #
    #  Did our max polls:
    
    return False
    
    
## publishState
#
#  Publishes the current state of the system.
#   @param socket  - socket on which to publish the state.
#   @note          - State publications are messages that look like:
#                    STATE:current-state
#
def publishState(socket):
    
    # Make sure the title/run are known:
    
    if title != None:
        publishTitle(socket, title)
    if run != None:
        publishRun(socket, run)
    publishRecording(socket, recording)
    message = 'STATE:%s' % (stateMachine.getState())
    socket.send(message)

##
# publishRecording
#   Publishes the state of the recording state
# @param sock - the zmq socket to which the publication occurs.
# @param value - Value to publish.
#
def publishRecording(sock, value):
    message = 'RECORD:%s' % (value)
    sock.send(message)
    
##
# publishTransition
#
# Publishes a state transition.  This is a message of the form:
# 'TRANSITION:newstate'
#
# @param socket - socket on which the publication message is sent.
# @param newState - new state we are transitioning into.
#
def publishTransition(socket, newState):
    message = 'TRANSITION:%s' % (newState)
    socket.send(message)
##
# publishRun
#   Publish a new run number.  This is a message of the form:
#   RUN:number
#
# @param socket - Socket on which to publish
# @param run    - Run  number.
#
def publishRun(socket, run):
    message = 'RUN:%d' % (run)
    socket.send(message)

##
# publishTitle
#   Publish a new title.  This is a messag of the form:
#   TITLE:title string.
#
# @param socket - publication socket.
# @param title  - new title.
#
def publishTitle(socket, title):
    message = 'TITLE:%s' % (title)
    socket.send(message)


##
# getRequest
#   Returns the next request from the socket.
#   Requests are of the form: KEYWORD:parameter
#   where allowed keyword/parameters are:
#   * TRANSITION - Request a state transition, the parameter is the transition
#                  requested.
#   * RUN        - Set a new run number, the parameter is the new run number.
#   * TITLE      - Set a new title, the parameter is the new title.
#
# @param socket The request socket.
#    
# @return mixed-type
# @retval Pair of request, and argument if the request type is legal.  In that
#          case the caller must reply to the request.
# @retval None - If the request type is illegal.  In that case we send a FAIL
#          to the socket to save the caller the trouble.
#
def getRequest(socket):
    validRequests = ['TRANSITION', 'RUN', 'TITLE', 'RECORD']
    line = socket.recv()
    list = line.split(':', 1)           # This allows : in the argument.
    if list[0] in validRequests:
        return list
    else:
        socket.send('FAIL - Invalid request must be one of %s' % (', '.join(validRequests)))
        return None
                    

##
# remote
#   Start a remote command off in a target node.
#   *  The user is assumed to have password-less logins via user certs and ssh
#
# @param command    - The command to run.
# @param host       - IP or DNS name of the host on which to run it.
#
# @return Popen object.  See e.g. https://docs.python.org/2/library/subprocess.html
#                        17.1.2 for information about that.
#
def remote(command, host='localhost'):
    command = ['ssh', host, command]
    return subprocess.Popen(
        command, stdin = subprocess.PIPE, stdout = subprocess.PIPE,
        stderr = subprocess.PIPE, close_fds = True
    )

#----------------------------------------------------------------------------
#  Classes:
#

##
# @class ZmqFileEventLoop
#
#   This class provides an event loop that knows how to wait for ZMQ and
#   file events.  The event loop allows callers to register file events
#   or zmq events and to associate callables with the events.  We make use
#   of the fact that the zmq poller can poll on "any Python object having a
#   fileno() method that returns a valid file descriptor." eliminating the
#   need to maintain separate pollers for both fds and the zmq sockets.
#
#
class ZmqFileEventLoop:
    ##
    # Constructor
    #
    def __init__(self):
        self.poller = zmq.Poller()
        self.dispatch = dict()
        self._fdsMap  = dict()
        
    ##
    # register
    #
    #  Register an event.  An event consists of:
    #  *   A zmq socket or file descriptor.
    #  *   An event mask from the bits zmq.POLLIN, and zmq.POLLOUT
    #      indicating the events which are interesting for the socket/fd
    #  *   A handler that is called when an event has been detected.
    #
    # @note  A second registration of an existing fd/socket overwrites the
    #        the prior one.r
    #
    # @param object   - The item to register.  Anything that has a fileno() method
    #                   is strictly speaking ok as long as that methdo returns
    #                   file descriptors.
    # @param events   - A mask of the bits zmq.POLLIN, zmq.POLLOUT  If not bits
    #                   are set, this is equivalent to an unregister call for
    #                   object.
    # @param handler  - A python callable that is invoked when the object
    #                   satisfies the conditions in the event mask.  The
    #                   handler is passed the following parameters in order:
    #                   *   This event loop.
    #                   *   The object that fired the event.
    #                   *   mask - a mask of the events that fired.
    #                       this can only have the bits that were registered for
    #                       the triggered object.
    #
    def register(self, object, events, handler):
        if (events != 0):
            self.poller.register(object, events)
            self.dispatch[object] = handler
            try:
                self._fdsMap[object.fileno()] = object  # Files have fileno
            except:
                pass                                    # But not zmq sockets.
        else:
            self.unregister(object)
    ##
    # unregister
    #    Removes a registration of an object from the poller.
    # @param object - object to remove.
    #
    # @note if the object is not registered, we don't throw an error..
    #
    def unregister(self, object):
        if object in self.dispatch.keys():
            del self.dispatch[object]
            self.poller.unregister(object)
    ##
    #  poll
    #
    #  Polls  once and dispatches as described.
    #
    # @param timeout - usec for which to block on the poll call.
    #
    def poll(self, timeout):
        readyList = self.poller.poll(timeout)
        for info in readyList:
            object   = info[0]
            mask     = info[1]
            if object in self._fdsMap.keys():
                object = self._fdsMap[object]     # Files just give the fileno
            callback = self.dispatch[object]
            callback(self, object, mask)
    
    ##
    # pollForever
    #   Polls forever (well almost forever).
    #
    # @param timeout   - ms to block for  each poll.
    # @param idler     - Method to call between polls (None if you don't want that)
    #                    Idler is passed:
    #                    * This poller.
    #                    If idler returns False, this method returns.
    #
    def pollForever(self, timeout, idler=None):
        while True:
            self.poll(timeout*1000)    # Timeout is microseconds.
            if idler != None:
                if not idler(self):
                    return
                
                

        
        
