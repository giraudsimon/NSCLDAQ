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
# @file   StateMonitorTest2
# @brief  Test the StateMonitor framework class.
# @author <fox@nscl.msu.edu>


from nscldaq.statemanager import StateMonitor
import sys

## usage:
#  python StateMonitorTest1 statemanager-req-uri state-manager-pub-uri
#

requestUri = sys.argv[1]
pubUri     = sys.argv[2]


# We'll just use unbound methods for our callbacks:

def NotReadyHandler(monitor, fromState, toState, cbarg):
    print('Not Ready state entered: %r -> %r  (%s)' %(fromState, toState, cbarg))


def ReadyHandler(monitor, fromState, toState, cbarg):
    print('Ready state entered: %r -> %r  (%s)' %(fromState, toState, cbarg))

def newRun(run):
    print("Run no changed to: %d" % (run))
def newTitle(title):
    print("Title changed to '%s'" % (title))

def recordingChanged(state):
    print("Recording is  now: %s" %(state))
    
mon = StateMonitor.StateMonitor(requestUri, pubUri)
mon.register('NotReady', NotReadyHandler, 'poof')
mon.register('Ready', ReadyHandler, 'poof poof')
mon.setRunNumberHandler(newRun)
mon.setTitleHandler(newTitle)
mon.setRecordingHandler(recordingChanged)

mon.run()