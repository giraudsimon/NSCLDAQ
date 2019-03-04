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
# @file   CommandTest.py
# @brief  Connect to my localhost state manager and poke commands
#         at it, getting replies back.
# @author <fox@nscl.msu.edu>

from nscldaq.statemanager import StateMonitor
from nscldaq.statemanager import Utilities
import sys
import getpass

username        = getpass.getuser()
requestPort     = Utilities.getPort('localhost', 'StateRequest', username)
transitionPort  = Utilities.getPort('localhost', 'StatePublish', username)

requestUri    = 'tcp://localhost:%d' % (requestPort)
transitionUri = 'tcp://localhost:%d' %(transitionPort)

mon = StateMonitor.StateMonitor(requestUri, transitionUri)

while 1:
    line = sys.stdin.readline()
    if not line:
        break
    
    line = line[0:-1]                # Trim the \n
    info = line.split(' ', 1)        #Command/param.
    reply =''
    if info[0] == 'T':    
        reply = mon.requestTransition(info[1])
    if info[0] == 'R':
        reply = mon.setRun(info[1])
    if info[0] == 'D':
        reply = mon.setTitle(info[1])
    if info[0] == 'r':
        reply = mon.setRecording(info[1].upper() in ['ON', 'TRUE', 'ENABLED'])
        
    print("Reply: %s" % (reply))



