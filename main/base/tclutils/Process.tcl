#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#      NSCLDAQ Development Group
#
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

# This software is Copyright by the Board of Trustees of Michigan
# State University (c) Copyright 2015.

# @file  Process.tcl 
# @author Jeromy Tompkins 
#
# ############################################################


package provide Process 2.0

package require snit

puts "Temporary: Process 2.0"

## @brief Encapsulation of a pipeline process
#
# This provides the user with a simple way to handle 
# pipelines. The pipeline is opened during construction of the Process
# instance. The process that actually is run as a pipeline is defined
# by the user with the -command option. The stdout and stderr outputs of the process 
# specified by that option are piped to the cat program, which in turn writes the output
# to stdout. There are two different callbacks that the user
# can register for different events:
#
# -oneof   takes a script to execute when pipeline reaches EOF condition
# -onread  take a script to execute when output has been read from the file.
#          The script should accept a single argument that is the string
#          of output.
#
# It is not possible to open more than one pipeline per instance of this snit::type.
#
snit::type Process {
  option -command {}
  option -oneof {}
  option -onread {}

  variable fd {}

  ## @brief Open the pipeline and register the readable callback
  constructor {args} {
    $self configurelist $args

    set fd [open "| $options(-command) |& cat" r]
    chan configure $fd -blocking 0
    chan configure $fd -buffering line
    chan event $fd readable [mymethod onReadable $fd]
  }

  ##  Close the open channel if it is still open
  destructor {
    if {$fd ne {}} {
      set pids [pid $fd]
      foreach p $pids {
        exec kill $p
      }
      catch {close $fd}
      set fd {}
    }
  }

  ## @brief Callback for readable events on the open pipe
  #
  # On end of file events, the pipeline is closed. After the channel is closed, 
  # the -oneof script is called in the global scope, if it is defined. 
  #
  # If the channel is not in an EOF condition, the script defined by -onread
  # is called in the global scope with the line read from the pipeline as its argument.
  #
  # \param channel  the file channel to read from
  #
  method onReadable channel {
    chan gets $channel line 
    if {[eof $channel]} {
      catch {close $channel}
      set fd {}
      uplevel #0 $options(-oneof) 
    } else {
      if {$options(-onread) ne {}} {
        uplevel #0 $options(-onread) $line
      }
    }
  }

  ## @brief Retrieve the pids of the processes in the pipeline
  method getPIDs {} {
    return [pid $fd]
  }
}
