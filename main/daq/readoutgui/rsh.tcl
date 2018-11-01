
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
package provide ssh 1.0
package require Wait
namespace eval  ssh {

    proc ssh {host command} {
	set stat [catch {set output [eval exec ssh $host $command]} error]
	if {$stat != 0} {
	    append output "\n"  $error
	}
	return $output
    }
    proc sshpipe {host command access} {
	lappend command {"2>&1"}
#	return [open "|ssh $host $command  " $access]
	return [open "|ssh $host $command '2>&1'  " $access]
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


        set pipe [open "|  ssh -o \"StrictHostKeyChecking no\"  $host $command |& cat" a+]
	set pid [lindex [pid $pipe] 0]

	return [list $pid $pipe $pipe]
    }

    namespace export ssh sshpipe sshpid
}
