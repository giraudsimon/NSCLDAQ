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
# @file   eventlog_wrapper.tcl
# @brief  Wraps eventlog instances for the manager.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
#  This script wraps the event logger when its used by the program manager.
#  The expectation is that:
#   daqsetup has been run and therefore all env. variables relevant to that
#   have been defined (specfically DAQBIN).
#   In addition the following script variables have been defined and are in env:
#   -   RECORD_PARTIAL - This is a boolean which is true if the events should just
#                        be recorded into a directory that is a soup of event
#                        files... like the 11.x multilogger e.g.  If false,
#                        the event files will be managed as they were for
#                        the 'normal' event file logger.
#
#   -   RECORD_DEST   - Where the event files will be recorded.  If RECORD_PARTIAL
#           is true, this is the directory in which event file soup is made.
#           if not, this is the top level of the directory tree that
#           holds the managed event files.
#   -   RECORD_SRC  - the URL from which the ring items are logged.
#   -   RUN_NUMBER  - expected run number.
#
#
# @note This script only records a single run.
#

#   Pull in some env variables we require:

set partial     $::env(RECORD_PARTIAL)
set destination $::env(RECORD_DEST)
set source      $::env(RECORD_SRC)
set runNumber   $::env(RUN_NUMBER)
set daqbin      $::env(DAQBIN)

# Some global definitions

set SEGMENT_SIZE 100000g
set done         0;         # Modified when managed logging is finished.
set afterId     -1

#-------------------------------------------------------------------------
# Partial logging code.

##
# _multilog
#   Do multilog like logging.  The event logger is told to
#   Do a one-shot log into the directory with a prefix identifying the date/time
#   and a very very large segment size;  100000g   corresponding to
#   100TB.
#
# @param ring   - Ringbuffer URI.
# @param dest   - Destination for logging.
#
 proc _multilog {src dest} {
    set command [file join $::daqbin eventlog]
    set prefix [clock format  [clock seconds] -format {%Y%b%d-%H%M%S-}]
    append command  \
       " --source=$src --path=$dest --segmentsize=$::$SEGMENT_SIZE" \
       " --oneshot --prefix=$prefix"
    
    # For this we can do a synchronous exec -- with ouput and error
    # relayed to our stdout:
    
    exec $command >&@ stdout 
 }
 
#------------------------------------------------------------------------
# Full logging code.
#

##
# _makeTree
#    Make the full directory tree.
#
# @param dest - destination top level directory.
#
proc _makeTree {dest} {
    file mkdir experiment complete
    file mkdir experiment/current
}
##
# _cleanExistingFiles
#    If there are existing event files...well these are really just
#    links so we clean them up.  To be a bit better than blindly killing
#    of *.evt, we kill off run-*.evt
#
# @param dest - root directory of experiment tree.
# @note This only applies to managed directories, not to partial recordings.
#
proc _cleanExistingFiles {dest} {
     set links [glob -nocomplain [file join $dest current run-*.evt]]
     file delete {*}$links
}
##
# _monitorFiles
#    Watches the destination directory of a full event log to see if
#    we need to create new event file links in the current directory.
#
# @param evdir - the event file dir
# @note $::destination/current is where the links must be made.
# @note we reschedule ourself every second with ::afterId maintaining the
#       id of the reschdule.
#
proc _monitorFiles {evdir} {
    set existing [glob -nocomplain [file join $evdir run_*.evt]]
    set current [file join $::destination current]
    foreach file $existing {
        set name [file tail $file]
        set linkname [file join $current $name]
        if {![file exists $linkname]} {
            file link -symbolic $linkname $file
        }
    }
    
    set ::afterId [list _monitorFiles $evdir]
}
##
# _finalizeRun
#    - All evt links in the current dir get moved to complete.
#    - All metadata in the current directory gets copied to the run directory.
#
# @param run  - run number of the run that's just ended.
#
proc _finalizeRun {run} {
    set complete [file join $::destination complete]
    set current  [file join $::destination experiment current]
    set events   [file join $::destiniation experiment $run]
    
    ##
    #  Take care of the event file links - protect against no links.
    #
    set links [glob -nocomplain [file join $current run-*.evt]]
    if {[llength $links] > 0} {
        file copy {*}$links $complete
        file delete {*}$links
    }
    # Everythying else must be copied via tar xzfH
    
    set status [catch {exec sh -c \
      (cd $current; tar czf - --dereference .) | \
      (cd $complete tar --warning=no-timestamp xzpf .) } msg]
    if {$status} {
        puts "Failed to copy metadata for $run : $msg"
    }
}
    

##
# _eventlogInput
#   Receives control when the pipe connected to a full event logger
#   has input available.
#   -  Input is relayed to stdout.
#   -  On EOF:
#      * We close the pipe.
#      * We stop monitoring the directory.
#      * We do the file movements we're required to do.
#      * We increment the ::done variable so that the event loop can be
#        exited along with this program.
#
# @param fd   - File descriptor open on the event log pipe.
# @param run  - The run number.
proc _eventlogInput {fd run} {
    if {[eof $fd]} {
        close $fd
        _finalizeRun $run
        after cancel $::afterId
        incr ::done
    } else {
        puts [gets $fd]
    }
    
}

##
# _fullyLog
#   Log a run to a managed directory tree.
#   The event logger is started and as files pop into the run directory,
#   they are linked into current.  When the logger exits:
#   - The links are moved to complete.
#   - All files in the current directory tree are tar-ed over into
#     the run directory.
#
# @param source      - URL of ring buffer.
# @param destination - Top level of managed directory tree.
# @param run         - expected run number.
# @note - The final, primary directory for the eventlogger destination is
#         $destination/experiment/$run
#
proc _fullyLog {source destination run} {
   #  Figure out the eventlog command:
   
   set destdir [file join $destination experiment $run]
   set command [file join $::daqbin  eventlog]
   append command
      " --source=$source --path=$destdir --segmentsize=$::SEGMENT_SIZE " \
      " --oneshot"
   
   set fd [open "|$command 2>@1 | cat"]
   fconfigure $fd -buffering line
   fileevent $fd readable [list _eventlogInput $fd $run]
   _monitorFiles $destdir
   
   vwait ::done
   
}
   
 

#-------------------------------------------------------------------------
# Entry point.

#  if not partial, ensure there's a correct directory structure.

if {!$partial} {
    _makeTree $destination
    _cleanExistingFiles $destination;   # care for existing current.
    
}

if {$partial} {
    _multilog $source $destination 
} else {
    _fullylog $source $destination $run
}
