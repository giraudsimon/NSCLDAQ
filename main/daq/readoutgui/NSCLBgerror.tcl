#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file NSCLBgerror.tcl
# @brief bgerror handler that logs to file.
# @author Ron Fox <fox@nscl.msu.edu>
#

#
#  Usage:
#     package require NSCLBgerror
#     Optionally NSCLBgerror::setLogFile  newLogFileName
#

package provide NSCLBgerror 1.0
package require log;                 # Does the actual output.


namespace eval NSCLBgerror {
    variable logFile $::env(HOME)/nscldaqerrors.log
    variable logFd   ""
}


##
#  bgerror only makes sense if this is Tk
#  if it isn't we don't do anything.  Otherwise we
#  force a load of the library bgerror.tcl file:
#
if {[catch {package present Tk}]  == 0} {
    
    if {[info command bgerror] eq ""} {
        catch bgerror;              # not called right but loads it.    
    }
    rename ::bgerror NSCLBgerror::bgerror
    set NSCLBgerror::logFd [open $NSCLBgerror::logFile a]
    log::lvChannelForall $NSCLBgerror::logFd

    ##
    # Our bgerror just logs the message to the log file 
    # with the error traceback and passes control to the
    # system default bgerror:
    #
    
    proc bgerror {msg} {
        set stamp [clock format [clock seconds]]
        ::log::log error "$stamp: $msg \n  $::errorInfo"
        NSCLBgerror::bgerror $msg
    }
} else {
    puts stderr "This package is only usable in Tk applications."
}

##
# NSCLBgerror::setLogFile
#    The existing log file is closed.
#    The new logfile is opened.
#    The new log channel is set:
#
# @param newLogFile -- path to the new logfile.
#
proc NSCLBgerror::setLogFile newLogFile {
    set newFd [open $newLogFile a];   # errors reported with oldmsg.
    log::lvChannelForall $newFd
    
    close $NSCLBgerror::logFd
    set NSCLBgerror::logFile $newLogFile
    set NSCLBgerror::logFd   $newFd
}




