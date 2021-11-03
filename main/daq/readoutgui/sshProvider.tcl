#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

##
# @file sshProvider.tcl
# @brief Provide a data source on the end of an ssh pipeline.
# @author Ron Fox
#


# unset ::env(DISPLAY);     # Prevent Tk?
package provide SSHPipe_Provider 1.0
package require ssh
package require Wait
package require ReadoutGUIPanel
package require InstallRoot

# Establish the namespace in which the methods live:

namespace eval ::SSHPipe {
    variable parameterization [dict create \
        host [list {Host name}]             \
        path [list {Path to executable}]    \
        wdir [list {Working directory}]    \
        parameters [list {Command parameters}] \
    ]
    #
    #   Array indexed by source id whose contents
    #   are a dict containing:
    #    sshpid   - Process id of the ssh child process.
    #    inpipe   - input pipefd.
    #    outpipe  - output pipefd
    #    alive    - boolean true if alive.
    #    closing  - boolean true if the data source is being closed out.
    #    idle     - bolean true if the data source does not have an open run.
    #    line     - Input line buffer.
    #    parameterization - the parameterization dict.
    #
    variable activeProviders
}
#-----------------------------------------------------------------------------
#
# The procs below satisfy the public interface of a data source provider:
#

##
# parameters
#
#   Provides a description of the parameters the data source requires.
#
# @return dict of parameter descriptions
#
proc ::SSHPipe::parameters {} {
    return $::SSHPipe::parameterization
}
##
# start
#   Starts a data source.
#   * Start an ssh pipe according to the specificatioins in the paramter dict
#   * Fill in an entry in the activeProviders array that describes this
#     data source.
#
# @param params - The parameter dictionary with sourceid added.
#
# @throw - File is not executable on this system
# @note  - The requirement that a file be executable on this system may have to
#          be relaxed at some point.
#
proc ::SSHPipe::start params {
    
    # Extract the parameters from the dict:
    
    set sid [dict get $params sourceid]
    set host [dict get $params host]

    # wdir is a late addition, this code should allow
    # dicts without it to continue to function.

    if {[dict exists $params wdir]} {
        set wd [dict get $params wdir]
    } else {
        set wd ~
        dict set params wdir ~
    }
    set program [dict get $params path]
    set cmdparams  [dict get $params parameters]
    
    # Assume the path is locally valid as well.
    
    if {![file executable $program]} {
        error "SSHPipe data source: Cannot start '$program' as it is not executable"
    }
    
    set starter [file join [InstallRoot::Where] bin start.bash]
    
    #  Start the ssh pipeline:
   

    set pipeinfo [::ssh::sshpid $host "$starter $wd \"$program $cmdparams\""]
    puts " $program : $pipeinfo"
    puts "Full pipe: [pid [lindex $pipeinfo 1]]" 
    
    # Set up our context entry in activeProviders:
    
    set ::SSHPipe::activeProviders($sid) [dict create \
        sshpid           [lindex $pipeinfo 0]     \
        inpipe           [lindex $pipeinfo 1]     \
        outpipe          [lindex $pipeinfo 2]    \
        alive            true                    \
        line             ""                     \
        closing          false                  \
        idle             true                   \
        parameterization $params                \
    ]
    
    #  Set up the listener for input from the program:
    
    fconfigure [lindex $pipeinfo 1] -blocking 0 -buffering none -buffersize 10
    fileevent [lindex $pipeinfo 1] readable [list ::SSHPipe::_readable $sid]

}
##
# check
#
#   Check liveness of a source.
#
#  @param source   - Id of the source to check
#  @return boolean - True if source is alive, false otherwise.
#
proc ::SSHPipe::check source {
    return [dict get $::SSHPipe::activeProviders($source) alive]
}
##
# stop
#   Stop the data source
#   * If the run is active in the source end it.
#   * Issue the exit command.
#   * close the pipe so the EOF fires too
#
# @param source - id of the source to stop.
# @note  The actual clean up is done when the eof is detected on the
#        input pipe.  Here we just mark the dict indicating deletion is ok.
#
proc ::SSHPipe::stop source {

    # Attempt to end any non-halted run.
    
    
    if {[::SSHPipe::_notIdle $source]} {
	puts "Attempting to end."
        ::SSHPipe::_attemptEnd $source
    }
    ::SSHPipe::_send $source exit
    # For good measure kill the beast
    
    catch {exec kill -9 [dict get $::SSHPipe::activeProviders($source) sshpid]}
    Wait -pid [dict get $::SSHPipe::activeProviders($source) sshpid]
    dict set ::SSHPipe::activeProviders($source) closing true
    
}
##
# begin
#   Start a run.  Sets the title, the run number and
#   then starts the run.
#
# @param source - Id of the source.
# @param runNum - number of the run to start.
# @param title  - Title of the run.
#
# @throw - we are no longer connected to the data source.
# @throw - we are not idle.
#
proc ::SSHPipe::begin {source runNum title} {
    
    ::SSHPipe::_errorIfDead $source
    set sourceInfo $::SSHPipe::activeProviders($source)
 
    if {[::SSHPipe::_notIdle $source]} {
        set host [dict get $sourceInfo parameterization host]
        set path [dict get $sourceInfo parameterization path]
        error "A run is already active in $path@$host"
    }
    
    # Set the run metadata title is in [list] to quote it properly.:
    
    ::SSHPipe::_send $source "set run $runNum"
    ::SSHPipe::_send $source "set title [list $title]"
    
    # Start the run and update our state:
    
    ::SSHPipe::_send $source begin
    

    dict set sourceInfo idle false
    set ::SSHPipe::activeProviders($source) $sourceInfo

}
##
# pause
#    Attempt to pause the run.
#
# @param source - Id of the source to pause.
#
# @throw  If we are no longer connected to the data source.
# @throw  If we are idle since we can only pause if we are not idle.
# @note   since we are not maintaining full state we must take the caller's
#         word that we are not already paused.
#
proc ::SSHPipe::pause source {
    ::SSHPipe::_errorIfDead $source
 
    if {![::SSHPipe::_notIdle $source]} {
        set sourceInfo $::SSHPipe::activeProviders($source)

        set host [dict get $sourceInfo parameterization host]
        set path [dict get $sourceInfo parameterization path]
        error "A run is not active in $path@$host so no pause is possible."
    }    
    ::SSHPipe::_send $source pause
}
##
# resume
#  Attempt to resume a paused run
#
# @param source - id of source to resume.
#
# @throw  If we are no longer connected to the data source.
# @throw  If we are idle since we can only resume if we are not idle.
# @note   since we are not maintaining full state we must take the caller's
#         word that we are not already paused.
#
proc ::SSHPipe::resume source {

    if {![::SSHPipe::_notIdle $source]} {
        set sourceInfo $::SSHPipe::activeProviders($source)
        set host [dict get $sourceInfo parameterization host]
        set path [dict get $sourceInfo parameterization path]
        error "A run is not active in $path@$host so no resume is possible."
    }

    ::SSHPipe::_send $source resume    
}
##
# end
#  Attempt to end a run.
#
# @param source - id of source to resume.
#
# @throw  If we are no longer connected to the data source.
# @throw  If we are idle since we can only resume if we are not idle.
# @note   since we are not maintaining full state we must take the caller's
#         word that we are not already paused.
#
proc ::SSHPipe::end source {
    if {![::SSHPipe::_notIdle $source]} {
        set sourceInfo $::SSHPipe::activeProviders($source)
        set host [dict get $sourceInfo parameterization host]
        set path [dict get $sourceInfo parameterization path]
        error "A run is not active in $path@$host so no end is possible."
    }    
    ::SSHPipe::_send $source end
    
    dict set ::SSHPipe::activeProviders($source) idle true

}

## init
#
# Simply sends whatever program on the other end of the pipe an init command.
#
# @param source - id of source to init
#
proc ::SSHPipe::init id {
  if {$id ni [array names ::SSHPipe::activeProviders]} {
    set msg "SSHPipe::init Source id does not exist."
    return -code error $msg
  }
  ::SSHPipe::_send $id init
}


##
# capabilities
#    Returns a dict with the capabilities of SSHPipe data sources.
#
#  This fills in the following:
#  *   canPause        - true
#  *   runsHaveTitles  - true
#  *   runsHaveNumbers - true
#
proc ::SSHPipe::capabilities {} {
    return [dict create canPause true runsHaveTitles true runsHaveNumbers true]
}
#-------------------------------------------------------------------------------
# Private utilities:
#

##
# _readable
#
#   Called when the input pipe from a file data source is ready to be read.
#   * If there's an EOF on the input,
#      - cancel us
#      - mark the source dead in the activeProviders array.
#      - Flush any data in the line buffer.
#      - Log source exit to the GUI.
#      - Wait on the ssh pid.
#   * If no EOF
#      - Read what we can from the inpipe appending it to the line buffer
#      - If the line buffer has endlines, output them as log entries.
#
# @param source  - Id of the source that just fired.
#
proc ::SSHPipe::_readable source {
    set sourceInfo $::SSHPipe::activeProviders($source)
    set fd [dict get $sourceInfo inpipe]
    catch {
        if {[eof $fd]} {
            ::SSHPipe::_sourceExited $source
        } else {
            ::SSHPipe::_readInput $source
        }
    }
}
##
# _sourceExited
#
#   Called to process EOF on a data source input pipe.  This indicates the
#   source exited.  See _readable for the full details of our actions.
#
# @param source  - Source id of the source that exited.
#
proc ::SSHPipe::_sourceExited source {

    set sourceInfo $::SSHPipe::activeProviders($source)

    fileevent [dict get $sourceInfo inpipe] readable ""
    
    dict set sourceInfo alive false
    
    set input [dict get $sourceInfo line]
    set host  [dict get $sourceInfo parameterization host]
    set path [dict get $sourceInfo parameterization path]
    
    if {$input ne  ""} {
        ReadoutGUIPanel::Log SSHPipe@$host:$source output $input
        dict set sourceInfo line ""
    }
    ReadoutGUIPanel::Log SSHPipe@$host:$source warning  \
        "Source $path@$host exited"


    
    catch {close [dict get $sourceInfo inpipe]} message
      
    if {[dict get $sourceInfo closing]} {
        array unset ::SSHPipe::activeProviders $source
    } else {
        set ::SSHPipe::activeProviders($source) $sourceInfo
    }
    #
    #  Remove the source from the active providers array
    #
    array unset ::SSHPipe::activeProviders($source)
    
    
}
##
# _readInput
#
#  Read input from a data source pipe.
#  If at least one complete line has been received output all complete
#  lines as log messages to the console.
#
# @param source - Id of the source to read from.
#
proc ::SSHPipe::_readInput source {
  
    set sourceInfo $::SSHPipe::activeProviders($source)
    set host [dict get $sourceInfo parameterization host]
    set path [dict get $sourceInfo parameterization path]
    
    set input [read [dict get $sourceInfo inpipe]]
    if {[string length $input] > 0} {
        append data [dict get $sourceInfo line ] $input
        if {[string first "\n" $data] != -1} {
            set data [::SSHPipe::_LogCompleteLines $source $host $data]
        }
        dict set sourceInfo line $data
        
    }
    
    set ::SSHPipe::activeProviders($source) $sourceInfo                                          
}

##
# _notIdle
#
# @param source   - Id of the source to check.
# @return boolean - true if the source is not in the idle state else false.
#
proc ::SSHPipe::_notIdle source {
    return [expr {![dict get $::SSHPipe::activeProviders($source) idle]}]
}
##
# _attemptEnd
#
#   Attempt to end a run that is active.
#
# @param source - id of source to end.
#
proc ::SSHPipe::_attemptEnd source {
    ::SSHPipe::_send $source end
    dict set ::SSHPipe::activeProviders($source) idle true
}
##
# _send
#
#  Send a message to a sourcde
#
# @param source -id of source to send.
# @param msg    - The message to send.
#
proc ::SSHPipe::_send {source msg} {
    set fd [dict get $::SSHPipe::activeProviders($source) outpipe]
    puts  $fd "$msg"
    flush $fd
}
##
# _close
#    Close the output data file
#
# @param source - source id.
#
proc ::SSHPipe::_close source {

    close [dict get $::SSHPipe::activeProviders($source) outpipe]
}
##
# _errorIfDead
#
#  @param source - Source to check
#  @throw If we are no longer connected to the source or it is shutting down:
#
proc ::SSHPipe::_errorIfDead source {
    set sourceInfo $::SSHPipe::activeProviders($source)
    
    set host       [dict get $sourceInfo parameterization host]
    set program    [dict get $sourceInfo parameterization path]
    
    if {(![::SSHPipe::check $source]) || [dict get $sourceInfo closing]} {
        error "SSHPipe source $program@$host is closing or no longer running"
    }
     
}
##
# _LogCompleteLines
#
#  Logs the complete lines from a string to the ReadoutGUIPanel text widget
#
# @param source - The id of the source logging the data
# @param host   - The host we're controlling.
# @param lines  - Text that has t least one \n in it.
# @return string - (possibly empty) residual string defined as the stuff in lines
#                  after the very last \n.
#
proc ::SSHPipe::_LogCompleteLines {source host lines} {
    set lastIndex [string last "\n" $lines ]
    foreach line [split [string range $lines 0 [expr {$lastIndex - 1}]] "\n"] {
        ReadoutGUIPanel::Log SSHPipe@$host:$source output "$line\n"
    }
    
    return [string replace $lines 0 $lastIndex]
}
