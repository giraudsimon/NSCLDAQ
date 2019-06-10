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
# @file   ssh.py
# @brief  run ssh on a pipe.
# @author <fox@nscl.msu.edu>

import subprocess
import select
import fcntl
import os


##
#  This file contains support for using ssh to run command lines in remote
#  systems. A prerqequisite for this to work is that the user must have
#  a public user user key installed in the remote system to ensure that ssh
#  will not need to prompt for username/password.
#
#  Support is provided for two sorts of programs:
#
#  * Transient - These are programs that run, produce output/errors and exit
#                for example /usr/opt/daq/current/bin/ringbuffer create george
#  * Persistent - These are programs that run and stay alive. They may produce
#                 output, and error messages.  They may also get input on their
#                 stdin across the pipe.
#


##
# @class sshPrimitive
#
#   Base class that all other classes are built on top of.
#   This does little more than construct the right subprocess.Pipe object.
#
class sshPrimitive:
    ##
    # constructor
    #
    #  Create the ssh pipeline:
    #
    # @param host    - The host on which the command runs.
    # @param command - The command string.
    #
    def __init__(self, host, command):
        self._command = 'ssh -t -t %s "%s"' % (host, command)
        self._pipe    = subprocess.Popen(
            self._command, shell=True,
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        self._stdin = self._pipe.stdin
        self._stdout= self._pipe.stdout
        self._stderr= self._pipe.stderr
        
        #  Set stdout and stderr to nonblocking mode:
        
        
    #
    # Destructor
    #  Send a control-C to the remote program just in case
    #  and close all file descriptors:
    #
    def __del__(self):
        # Try block in case the remote program has already closed stdin.
        
        try:
            self._stin.write('\x03')       
        except:
            pass
        
        self._stdin.close()
        self._stdout.close()
        self._stderr.close()
        
    ##
    # stdin
    #   Gets the file descriptor associated with the stdin pipe.
    #
    # @return fd
    #
    def stdin(self):
        return self._stdin
    ##
    # stdout
    #   Gets the file descriptor associated with the stdout pipe.
    #
    def stdout(self):
        return self._stdout
    ##
    # stderr
    #   Gets the error file descriptor
    #
    def stderr(self):
        return self._stderr
    
##
# @class Transient
#
#   This class encapsulates a transient command.  An example of a transient
#   command is ls, or the ringbuffer command.
#
class Transient:
    ##
    # constructor
    #
    # @param host    - Host in which the command runs.
    # @param command - The command to run.
    #
    def __init__(self, host, command):
        self._ssh = sshPrimitive(host, command)
    
    ##
    #  _getLines
    #    returns a list containing linex read from a file:
    #
    # @param fd  - File descriptor to read from
    #
    def _getLines(self, fd):
        result = list()
        for l in fd:
            result.append(l)
        return result
    
    
    ##
    #  output
    #   Returns the output from the command as a list of text lines.
    #
    def output(self):
        return self._getLines(self._ssh.stdout())
    ##
    # error
    #  Returns error output from the command as a series of text lines
    #
    def error(self):
        return self._getLines(self._ssh.stderr())
    

##
# @class shell
#
#  Runs a shell.  stdin, stdout and stderr pipes are available for
#  use or there are methods to push commands to the shell.
#
#
class shell:
    ##
    # constructor
    #   @param host  - Name of host.
    #
    def __init__(self, host):
        self._ssh = sshPrimitive(host, "")
        self._stdin = self._ssh.stdin()
        self._stdout= self._ssh.stdout()
        self._stderr= self._ssh.stderr()
        
        # The pipes get set to non blockin gmode.
        


    #   Accessors for the file descriptors:
     
    def stdin(self):
        return self._stdin
    def stdout(self):
        return self._stdout
    def stderr(self):
        return self._stderr
    
    # read/write operations:
    
    ##
    # writeLine
    #   Write a command to the shell.
    #
    # @param command - command to write, a "\" is appended to it.
    #
    def writeLine(self, command):
        cmd = command + "\n"
        self._stdin.write(cmd)
        self._stdin.flush()
        
    ##
    # readOutput
    #    Reads a line from stdout  This will block if there's nothing available
    #
    # @return string - empty if EOF.
    #
    def readOutput(self):
        return self._stdout.readline()
    ##
    # readError
    #    Reads a line for stderr. this will block if no data is available.
    #
    # @return string - empty if EOF.
    #
    def readError(self):
        return self._stderr.readline()
        
    ##
    # setenv
    #   Set an environment variable:
    #   * We must be at the shell prompt.
    #   * The shell must be an sh dervived shell as this does an
    #   * name=b followed by export name
    #
    # @param name - Environment variable name to set.
    # @param value - New value for the name.
    #
    def setenv(self, name, value):
        command1 = "%s=%s" % (name, value)
        command2 = "export %s" % (name)
        self.writeLine(command1)
        self.writeLine(command2)
        
    ##
    # intr
    #  Send the INTR signal (control-C) to the stdin.
    #
    def intr(self):
       self._stdin.write("\x03")
