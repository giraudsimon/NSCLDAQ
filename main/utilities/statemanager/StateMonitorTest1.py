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
# @file   StateMonitorTest1.py
# @brief  Simple test program for the StateMonitorBase class.
# @author <fox@nscl.msu.edu>

from nscldaq.statemanager import StateMonitor
import sys

## usage:
#  python StateMonitorTest1 statemanager-req-uri state-manager-pub-uri
#

requestUri = sys.argv[1]
pubUri     = sys.argv[2]

class Mymonitor(StateMonitor.StateMonitorBase):
    def __init__(self, transitionRequest, statePublisher, initializer=None):
        super(Mymonitor, self).__init__(transitionRequest, statePublisher, initializer)
        print("__init__ done")

    def initialState(self, state):
        print("Initial state: %s" % (state))
        super(Mymonitor, self).initialState(state)
    def transition(self, state):
        print('Transition to %s ' % (state))
        super(Mymonitor, self).transition(state)
    def runNumberMsg(self, msg):
        super(Mymonitor, self).runNumberMsg(msg)
        print("Run number: %d" % (self.getRunNumber()))
    def titleMsg(self, msg):
        super(Mymonitor, self).titleMsg(msg)
        print("Title: %s" % (self.getTitle()))
    def recordingMsg(self, msg):
        super(Mymonitor, self).recordingMsg(msg)
        print("Recording %s" % (msg))
        



def initfunc(monitor):
    print("initfunc called")
    
mon = Mymonitor(requestUri, pubUri, initfunc)
mon.run()