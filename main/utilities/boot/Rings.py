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
# @file   Rings.py
# @brief  Ring management module for boot.
# @author <fox@nscl.msu.edu>

from nscldaq.expconfiguration import project
import ssh
import os

##
# _createRing
#    Create a single ring buffer.  The requirements are that
#    *   We have a public key registered in the remote host so that ssh can login
#        without passwords.
#    *   The DAQBIN env variable is defined here and pionts to the daq bin directory
#    *   The remote system has this version of NSCLDAQ installed in the same
#        directory as here.
#
# @param host   - The host in which we create the ring (via ringbuffer create).
# @param ring   - The name of the ring in that host.
#
def _createRing(host, ring):
    #
    # Build the command
    #
    daqbin     = os.environ['DAQBIN']       # Presumably throws if not defined.
    command    = '%s/ringbuffer create %s' % (daqbin, ring)
    executor   = ssh.Transient(host, command)
    errorLines = executor.error()
    
    #  The error messages we know start with 'ssh:' in the front of the first
    #  error line:
    
    if len(errorLines) > 0:
        firstLine = errorLines[0]
        words = firstLine.split(' ')
        if words[0] == 'ssh:':
            raise RuntimeError('Could not make ring: %s@%s -- %s' % (ring, host, firstLine))

##
# _deleteRing
#
#   Delete a single ring buffer.  Requirements are the same as for _createRing
#   we don't look for failures because it's harmless to leave rings lying around
#   and any host we can't connect to won't have rings anyway.
#
# @param host   - The host in which we create the ring (via ringbuffer create).
# @param ring   - The name of the ring in that host.
#
def _deleteRing(host, ring):
    daqbin  = os.environ['DAQBIN']
    command = '%s/ringbuffer delete %s' %(daqbin, ring)
    executor= ssh.Transient(host, command)


##
# _listRings
#    lists the rings in a database file.
#
# @param database - database filename.
#
def _listRings(database):
    p  = project.Project(database)
    p.open()
    rings = project.Rings(p)
    ringList = rings.list()
    return ringList


#-----------------------------------------------------------------------------
# Public entries.
#



##
# createRings
#    Create the ring buffers required by the database.
#
# @param monitor - The run state  monitor.
# @param database - The database from which the rings will be drawn.
#
def createRings(monitor, database):
    print("Creating rings for database %s" %(database))
    ringList = _listRings(database)
    for ringDef in ringList:
        ringName = ringDef[1]
        ringHost = ringDef[4]
        _createRing(ringHost, ringName)
        
##
# destroyRings
#    Destroy ring buffers created by the experiment.
#
#  TODO:  Really the rings that are destroyed should be the memorized
#         rings that were created since we may be shutting down to reboot
#         for a database change.
#
# @param monitor  - The run state monitor (in case we want to make a transition)
# @param database - The database with the rings.
#
def destroyRings(monitor, database):
    ringList = _listRings(database)
    for ringDef in ringList:
        ringName = ringDef[1]
        ringHost = ringDef[4]
        _deleteRing(ringHost, ringName)


