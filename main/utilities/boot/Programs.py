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
# @file   Programs.py
# @brief  Program management for boot application.
# @author <fox@nscl.msu.edu>

import os
import fcntl
import zmq
import time
from nscldaq.expconfiguration import project
from nscldaq.boot             import ssh, config

_RunningPrograms = list()     # List of active program objects.

##
# _listPrograms
#   List the programs a given database file.
#
# @param database - path to the database file.
# @return         - iterable container from project.Programs.list()
#
def _listPrograms(database):
    dbase  =   project.Project(database)
    dbase.open()
    progs  =   project.Programs(dbase)
    return progs.list()
    
##
# @class Program
#
# Private class for maintaining a program.
#
#
class Program:
    ##
    #  constructor
    #    Starts up the program via a persistent shell:
    # @param monitor - The state monitor (we register input on stdout,stderr for calbacks)
    # @param host    - Host on which the command runs.
    # @param command - The base command.
    # @param args    - The command arguments
    # @param workingDir - The working directory in which the program runs.
    # @param env     - Dict defining additional env variables to define.
    #
    def __init__(self, monitor, host, command, args, workingDir, env):
        self._ssh           = ssh.shell(host)
        self._expectingExit = False            # This is true when we ask for an exit.
        self._monitor       = monitor          # so we can unregister.
        self._setupEnv(env)
        self._setWd(workingDir)
        self._startProgram(command, args)
        self._registerCallbacks(self._ssh)
        _RunningPrograms.append(self)           # Self registration.
    
    ##
    #  destructor
    #    need to unregister our callbacks from the poller.
    #    Note that this must be called at garbage collection time in response
    #    to the object being removed from _RunningPrograms.
    #
    def __del__(self):
        stdout = self._ssh.stdout()
        stderr = self._ssh.stderr()
        poller = self._monitor.poller
        poller.unregister(stdout)
        poller.unregister(stderr)
        
    # class.Program private utility methods:
    
    ##
    # _setupEnv
    #   Set up the program environment.  This is done by
    #   *  Invoking . $DAQBIN/daqsetup in the remote system.
    #   *  Doing export's for each item in the env map.
    #
    # @param env  - Map of envvarName -> value pairs.
    #
    def _setupEnv(self, env):
        self._ssh.writeLine('. %s/daqsetup.bash' % (os.getenv('DAQROOT')))
        for var in env.keys():
            self._ssh.setenv(var, env[var])

    
    ##
    # _setWd
    #
    #   Set the working directory (just a cd <thing>)
    #
    # @param wd - the desired working directory
    #
    def _setWd(self, wd):
        self._ssh.writeLine('cd %s' % (wd))
    ##
    # _startProgram
    #   starts the actual target program.
    #
    # @param program - path to the program (argv[0]).
    # @param args    - ordered list of program arguments.
    #
    # @note runner.bash is a script that runs the program and then exits the shell
    #       when the command exits.  This ensures that causes endfiles on the
    #       stdout/stderr pipes.
    #
    def _startProgram(self, program, args):
        command = '. $DAQBIN/runner.bash %s %s' % (program, ' '.join(map(lambda x: "'" + x + "'", args)))
        self._ssh.writeLine(command)
        self._command = command
        
    
    ##
    # _registerCallbacks
    #   Set readable callbacks on the shell's stdout and stderr.
    #
    # @param shell - The ssh shell whose stdout/stderr we want to monitor.
    #
    def _registerCallbacks(self, shell):
        poller = self._monitor.poller
        poller.register(shell.stdout(), zmq.POLLIN, self._programOutput)
        poller.register(shell.stderr(), zmq.POLLIN, self._programOutput)

    #  Set a file object to on blocknigh mode.
    
    def _setNonblock(self, file):
        # fd = file.fileno()
        fl = fcntl.fcntl(file, fcntl.F_GETFL)
        fcntl.fcntl(file, fcntl.F_SETFL, fl | os.O_NONBLOCK)
        return fl
    
    def _setFlags(self, file, flags):
        fcntl.fcntl(file, fcntl.F_SETFL, flags)
    
    ##
    # _programOutput
    #   Called by the poller/event loop when our program's stdin/stderr
    #   is readable
    #   * Try to read a line - If we can, discard it.
    #   * If reading a line shows that the file descriptor is at EOF (was closed)
    #     then force a fail transition, and unregister us from the program list
    #     because we are alread gone.    
    #
    # @param poller - The poller that called us.
    # @param f      - The file descriptor that is now readable.
    # @param mask   - Mask of events (zmq.POLLIN).
    #
    # @note - on EOF we also unregister the file descriptor to ensure our events
    #         don't starve other event sources.
    # @note - If self._expectingExit is true we don't attempt the state
    #         transition but do unregister as this means we got the because we were
    #         asked to end the program.
    #
    def _programOutput(self, poller, f, mask) :
        oldflags = self._setNonblock(f)
        l = f.read()
        self._setFlags(f, oldflags)
        if len(l) == 0:
            print("Eof detected")
            poller.unregister(f)
            if not self._expectingExit:
                print("Unexpected program exit... forcing FAIL transition")
                self._monitor.requestTransition('FAIL')
            #
            #  The try block is because we'll see this for stdout and
            #  stderr closing.
            #
            try:
                _RunningPrograms.remove(self)
            except:
                pass              # Might already be gone.
        else:
            print("%s: '%s'" % (self._command, l))
    
    # class.Programs - public entries.
    
    ##
    # exit
    #   sends the 'exit' string to the program in hopes that will kill it off.
    #
    def exit(self):
        self._expectingExit = True
        try:
            self._ssh.writeLine('exit')       # Exit bash if program exits.
        except:
            pass                              # Maybe it's already exited.
    ##
    #  intr
    #    Send an intr signal to the program (control-C)
    #
    def intr(self):
        self._expectingExit = True
        try:
            self._ssh.stdin().write('\x03')    # Don't have trailing \n.
            self._ssh.writeLine('exit')        # ^C probably wipes out pending exit.
        except:
            pass                               # Maybe it's already exited...

#-----------------------------------------------------------------------------
# Public entries.

##
# startPrograms
#    For each program in the specified database:
#    *  Starts is up via an ssh using pipes for in/out/error.
#    *  Registers Records the existence of the program.
#    *  Registers stdout and stderr to be monitored via the state monitor's
#       poller to fire a procedure when the fd is readable.
#
# @param monitor -- The monitor object (includes the poller).
# @param database - The database we would process if we were driven by it.
#
def startPrograms(monitor, database):

    print('Starting programs in %s ' % (database))
    #
    # The env we're going to pass tells the program how to be a client of the
    # statemanager, Note that $DAQBIN/daqsetup will be run in the shell as well.
    
    env = {                                                 \
        'TRANSITION_REQUEST_URI'      : config.remoteRequestUri,   \
        'TRANSITION_SUBSCRIPTION_URI' : config.remoteStateUri      \
    }
    
    programListing = _listPrograms(database)
    for program in programListing:
        path = program['path']
        wd   = program['working_dir']
        host = program['host_name']
        args = program['args']
        p    = Program(monitor, host, path, args, wd, env)

##
# stopPrograms
#
# Attempt to stop all running programs.  By now the caller has given state aware
# programs a few seconds to stop and we're going to do the following:
# *   Send an exit command to each program
# *   Wait a bit.
# *   Send an INTR signal to all that remain.
#
#  The code assumption in this module is that this is happening due to a state
#  transition into NotReady.
#
# @param monitor -- The monitor object (includes the poller).
# @param database - The database we would process if we were driven by it.
#
def stopPrograms(monitor, database):
    for program in _RunningPrograms:
        print("Sending exit to %s" % program._command)
        program.exit()
    time.sleep(3)                          # Give them a chance to exit.
    for program in _RunningPrograms:
        print("Sending intr to %s" % program._command)
        program.intr()                     # Give them a hard kill.
    