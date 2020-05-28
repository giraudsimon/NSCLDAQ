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

#
#   This file supplies the unique instance package.
#   The package supports using a lock file to ensure that
#   this program is will be a unique instance of the program
#   for this user on all systems that share this user's
#   home directory.
#
#   The package lives in the namespace: UniqueInstance
#   and exports the following globals:
#      Unique App - If ~/.$App.lock exists, returns 0
#                   otherwise creates the file containing
#                   pid@host and returns 1 if sucessful.
#      IsMe   App - Invokes Unique.  If the return value is 0,
#                   indicating the lock file exists,
#                      returns 0 if the contents of the lock
#                      file are pid@host otherwise return 0.
#                    If the return value from Unique is 1
#                    returns 1.
#     WhoElse App - Returns the contents of the lock file if
#                   if it exists or errors if not.
#     Exit    App - If IsMe removes the lock file and exits.
#                   Applications using UniqueInstance should
#                   use Exit rather than exit in order to
#                   clean up the lockfile.
#
package provide UniqueInstance 1.0
# package require Wait
namespace eval UniqueInstance {
    variable home
    #   Constructs the lock file name:
    #
    proc LockFile {App} {
	variable home
	set lockfile $home/.
	append lockfile $App .lock
	return $lockfile
    }
    #   See header comments:  Ensures this is a unique instance
    #   of this application.
    #
    proc Unique {App} {
	variable home
	set lockfile [LockFile $App]
	if [file exists $lockfile] {
	    return 0
	} else {
	    set fd [open $lockfile w]
	    puts $fd [pid]@[exec hostname]
	    close $fd
	    return 1
	}
    }
    # See header comments: Ensures that if there is an existing
    # lockfile it's all about us.
    proc IsMe {App} {
	variable home
	if {[Unique $App] == 0} {
	    set lockfile [LockFile $App]
	    set fd [open $lockfile r]
	    set contents [read $fd]
	    close $fd
	    if {$contents == "[pid]@[exec hostname]\n"} {
    		return 1
	    } else {
    		return 0
	    }
	} else {
	    return 1
	}
    }
    #
    #  See header comments: indicates who else is running this
    #  Application.
    #
    proc WhoElse {App} {
	set lockfile [LockFile $App]
	set fd [open $lockfile r]
	set contents [read $fd]
	close $fd
	return $contents
    }
    #  See header comments, removes the application lock and
    #  exits.

    proc Exit {App {from "unknown"}} {
	set lockfile [LockFile $App]
	file delete $lockfile
	# catch Wait
	exit
    }

    namespace export Unique IsMe Exit WhoElse
}
set UniqueInstance::home $env(HOME)








