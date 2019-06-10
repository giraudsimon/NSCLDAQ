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
# @file   boot.py
# @brief  NSCLDAQ user software boot program.
# @author Ron Fox <fox@nscl.msu.edu>

##
# This script boots up the user's software for an experiment.
# It does this in response to state transition requests to the Ready state
# from the experiment state manager.
# It also shuts down the user's software in the event of a transition to the
# not ready state.
#
#  The elements started include:
#  *  Creation of ring buffers associated with the experiment.
#  *  Programs associated with the experiment.
#
#  The elements shut down include:
#  *  Programs associated with the experiment
#  *  Deletion of ring buffers associated with the experiment
#
#  Experiment definition databases are used to deterime what is part of the
#  experiment (see Usage below).
#
#
# Usage:
#   boot --host server-hostname --state-service state-publication-service-name \
#         experiment-database [...]
#
# Where:
#   --host  Specifies the host on which the state manager runs.  If not
#           supplied, this defaults to 'localhost'
#   --state-service Specifies the service name of the state publication port
#           if not specified, this defaults to 'StatePublish'
#   --transition-service - Specifies the service name of the state transition
#           port.  If not specified, defaults to 'StateRequest'.
#
#  experiment-database - are the paths to one or more experiment databases
#           whose contents will determine the make up of the experiment.
#           at start, elements of the experiment are taken from each database
#           in command line order.  At stop time this order is reversed.
#           Thus, for example, you could specify an experiment that couples
#           detectors together as three databases, one for each detector system's
#           daq and one for whatever elements are needed to couple the systems.
#          e.g. boot $S800SW/s800.db $CAESARSW/ceasar ~/config/s800-ceasar-coupling
#           
# Note:
#   Short options are also defined:  -n for --host -s for --state-service
#   Additional switches are defined that don't actually run the program:
#   -h, --help  - Outputs some Usage help text for the program to stdout and
#                 exits.
#   -v, --version - Outputs the program version to stdout and exits.
#
# Note:
#    The defaults for --host, --state-service, --transition-service are correct
#    for a statemanger started with defaults in the same host as the boot host.

import argparse
import getpass
import os.path
import sys
import socket
import time
from nscldaq.statemanager     import StateMonitor
from nscldaq.portmanager      import PortManager
from nscldaq.expconfiguration import project
from nscldaq.boot             import Rings, Programs, config


#-------------------------------------------------------------------------------
#  Program entry point:

##
# notReadable
#   Determins if a file is readable.
#
# @param path - path to a file.
#
# @return boolean - true if the file is not readable.
#
def notReadable(path):
    return not os.access(path, os.R_OK)

# parsargs
#    Parse the command line arguments:
#
def parsargs():
    parser = argparse.ArgumentParser(
        description='Boot/shutdown experiment based on state manager'
    )
    parser.add_argument('database', help='Experiment database', nargs='*')
    parser.add_argument(
        '-v', '--version', help='Print program version and exit',
        action='store_const', const=1
    )
    
    parser.add_argument(
        '-n', '--server', help='State manager host (name or IP address)',
        default='localhost'
    )
    parser.add_argument(
        '-s', '--state-service', help='State publication service name',
        default='StatePublish'
    )
    parser.add_argument(
        '-t', '--transition-service', help='State transition request service name',
        default='StateRequest'
    )
    args  = parser.parse_args()
    
    #  If --version is not specified we need at least one database:
    
    if (args.version == None) and (len(args.database)== 0) :
        print("error: too few arguments:")
        parser.print_help()
        exit(1)
    
    return args

##
# constructUri
#    Given a host and service name:
#    * Looks up the service in the specified host (exception if not found).
#    * Constructs a tcp://hostname:service-port URI for the service.
#
# @param host    - Name of the host in which the service supposedly runs.
# @param service - The service name, the user is assumed to be the one running
#                  the program.
#
def constructUri(host, service):
    pm          = PortManager.PortManager(host)
    serviceInfo = pm.find(service=service, user=getpass.getuser())
    if len(serviceInfo) != 1:
        raise RuntimeError(
            'Could not uniquely determine the port for %s@%s' % (service, host)
        )
    
    return "tcp://%s:%s" % (host, serviceInfo[0]['port'])
    
    

##
# startup
#   Start up the experiment
#   *  Ensure the rings are created.
#   *  Start all the programs.
# @note Exceptions lead to a request to FAIL
#       that will let us shutdown whatever we managed to already start.
#
# @param monitor - The state monitor.
# @param prio    - prior state (None if this is a initial state).
# @param current - Current state (should be case blind match to NotReady)
# @param databases - List of database files that describe what's to be started.
#
def startup(monitor, prior, current, databases):
    try:
        for database in databases:
            Rings.createRings(monitor, database)
        
        for database in databases:
            Programs.startPrograms(monitor, database)
        
        # Success so mark the system as up:
        
        monitor.requestTransition('UP')
    except Exception as e:
        #TODO:  The write below should log in a zmq way.
        sys.stderr.write(
            'Experiment startup failed: %s , transition to NotReady requested' %
                (e)
        )
        monitor.requestTransition('FAIL')

def shutdown(monitor, prior, current, databases):
    
    
    # Only shutdown if this is a transition rather than initial state or loop
    # back to NotReady from NotReady (since we're already shutdown in the latter)
    # case
    
    if prior not in (None, 'NotReady'):   
        print("Shutting down experiment")
        print("Sleeping to let state aware programs exit:")
        time.sleep(2)
        print("Actively killing processes")
        Programs.stopPrograms(monitor, database)
        print("Killing rings")
        Rings.destroyRings(monitor, database)

##
#  This allows unit testing to be done on elements of the main program.
#
if __name__ == '__main__':
    args = parsargs()
    
    # --version means print version and exit:
    
    if args.version != None:
        print("NSCL Boot manager version %s" % (args.version))
        exit(0)
    
    # Save the database name list:
    
    databases = args.database
        
    # Ensure all databases are readable files....at this time we don't require
    # they all be project databases...that requirement is made at startup
    # and shutdown.
    
    
    for database in databases:
        if notReadable(database):
            raise RuntimeError('The database file %s is not readable' % (database))
    
    # Build the state manager URI's and create the state monitor object.
    
    requestUri = constructUri(args.server, args.transition_service)
    stateUri   = constructUri(args.server, args.state_service)
    monitor = StateMonitor.StateMonitor(requestUri, stateUri)
    
    ##
    #  We also want to provide state/request URI's that are
    #  good to use by remote hosts...thism eans substituting our host name for
    #  the server if the server is 'localhost'  The recipe for getting the fqdn
    #  comes from Tom Ekberg's reply in
    #  http://stackoverflow.com/questions/4271740/how-can-i-use-python-to-get-the-system-hostname
    #
    
    server = args.server
    if server == 'localhost':
        server = socket.gethostbyaddr(socket.gethostname())[0]
        
    config.remoteRequestUri = constructUri(server, args.transition_service)
    config.remoteStateUri   = constructUri(server, args.state_service)
    
    # Register our transition requests.
    
    monitor.register('NOTREADY', shutdown, databases)
    monitor.register('BOOTING',  startup,  databases)
    
    #  Run the state monitor's event loop.  The rest of the program
    #  is driven off that.
    
    monitor.run()