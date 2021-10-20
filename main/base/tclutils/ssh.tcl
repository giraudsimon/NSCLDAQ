
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
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


# (C) Copyright Michigan State University 1938, All rights reserved 
#
# ssh is a package which supports the issuance of a remote shell
# command on a system with shared filesystem.
#

#NOTE:  This version has a basic understanding of singularity containers.
#       specifically, if the envirionment variable
#       SINGULARITY_CONTAINER is defined, it is assumed to be the path
#       of a singularity container running the application.  It's also assumed
#       that
#         1.   The same container exists in the remote system.
#         2.   The user wants to run the command inside that container environment.
#       This gets done by ssh-ing a singularity exec container-name command.
#       To make things more exciting, if the ~/stagarea is defined,
#       it is read and the container runs with a bindpoint matching the link
#       target.  Additional bindpoints can be defined in the file
#       ~/.singularity_bindpoints which must consist sources which get mapped
#        to the same mount point.
#
#

package provide ssh 2.0
package require Wait
namespace eval  ssh {
	#
	# getSingularityBindings
	#
	#   @return string
	#      This is either --bind bind1,bind2,...
	#   if there are bindings or an empty string if there are none.
	#
	proc getSingularityBindings {} {
		set bindings [list]
		set status [catch {file readlink ~/stagearea} value]
		if {!$status} {
			lappend bindings $value;                  # stagearea link auto-binds.
		}
		if {[file readable ~/.singularity_bindpoints]} {
			# User has additional bind points:
			
			set fd [open ~/.singularity_bindpoints r]
			while {![eof $fd]} {
				set line [string trim [gets $fd]]
				if {$line ne ""} {
					lappend bindings $line
				}
			}
			
			close $fd
		}
		# If at the end of this we have any bindings we need to return a --bind
		# option else an empty string
		
		if {[llength $bindings] > 0} {
			return "--bind  [join $bindings ,]"
		} else {
			return ""
		}
	}
	# actualCommand
	#    If we are in a singularity container the command returned runs
	#    the input command in the container See the NOTE comments above for what
	#    that means.
	#
	# @parma command - the plain old command we're trying to run.
	#
	proc actualCommand {command} {
		if {[array names ::env SING_IMAGE] eq ""} {
			# not in a container env.
			
			return $command
		}
		#
		#  We're in a container:
		
		set container $::env(SING_IMAGE)
		set bindings  [ssh::getSingularityBindings]
		return "singularity exec $bindings $container $command"
	}
	proc shellCommand { } {
        if {[array names ::env SING_IMAGE] eq ""} {
			# not in a container env.
			
			return bash
		}
		#
		#  We're in a container:
		
		set container $::env(SING_IMAGE)
		set bindings  [ssh::getSingularityBindings]
		return "singularity shell $bindings $container "
    }
    #-------------------------------------------------------------------------
    proc ssh {host command} {
		set command [ssh::actualCommand $command]
		set stat [catch {set output [eval exec ssh $host $command]} error]
		if {$stat != 0} {
			append output "\n"  $error
		}
		return $output
	}
    proc sshpipe {host command access} {
		set command [ssh::actualCommand $command]
		lappend command {"2>&1"}
		return [open "|ssh $host $command '2>&1'  " $access]
    }
	
	proc sshcomplex {host command access} {
        set shell [ssh::shellCommand]
		return [open "| echo $command | ssh -t -t $host $shell 2>&1" $access]
    }

    #
    #   sshpid - Uses the Pipe command to open a pipe to the
    #            command.  The pipe has an input and an output end.
    #            The command runs asynchronously.
    #   Parameters:
    #       host   command
    #   Returns:
    #     list containing in order:
    #        pid    - Process ID of the ssh shell.
    #        inpipe - Pipe to read from to get output/error from process.
    #        outpipe- Pipe to write to to send data to the process.
    #
    #
    proc sshpid {host command} {
		set command [ssh::actualCommand $command]

        set pipe [open "|  ssh -o \"StrictHostKeyChecking no\"  $host $command |& cat" a+]
		set pid [lindex [pid $pipe] 0]

		return [list $pid $pipe $pipe]
    }

    namespace export ssh sshpipe sshpid sshcomplex
}
