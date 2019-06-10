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
# @file   MonitorTest.py
# @brief  Test program to monitor the publications of a state manager.
# @author <fox@nscl.msu.edu>


from nscldaq.statemanager import Utilities
import zmq
context = zmq.Context()

socket = Utilities.subscribe(context, True, True)
state  = None

while True:
    msg = socket.recv()
    info = msg.split(':', 1)
    
    if info[0] == 'STATE':
        if state != info[1]:
            print("State is now: %s" % (info[1]))
            state = info[1]
    if info[0] == 'TRANSITION':
        print("Transition: %s " % (info[1]))
        state = info[1]
    if info[0] == 'RUN':
        print('Run number: %s' % (info[1]))
    if info[0] == 'TITLE':
        print('Title is: %s' % (info[1]))



