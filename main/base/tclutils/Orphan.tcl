#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

##
#  @file Orphan
#  @brief  Provide a script to be called when the caller is orphaned.
#  @author Ron Fox<fox@nscl.msu.edu>

package provide orphan 1.0
package require Wait
package require snit

##
# @class Orphan
#    Snit class/type that provides the ability to trigger a script when
#    the process running the script is orphaned.  A process is considered
#    orphaned when the process's parent id changes from when the
#    object is intantiated.
#
# @note There's a race condition;  If the process is orphaned prior to
#       establishing the script, the script won't be triggered.
#
snit::type Orphan {
    option -script [list]
    
    variable initialPid
    variable afterid [list]
    
    ##
    # constructor
    #   - process the parameters.
    #   - Save the initial parent id.
    #   - Establish an after to monitor for changes in the parent id.
    #
    constructor args {
        set initialPid [ppid]
        $self configurelist $args
        set afterid [after 1000 [mymethod _monitorPPID]        ]
    }
    ##
    # destructor
    #    Kill off any script we've scheduled for later
    #
    destructor {
        if {$afterid ne ""} {
            catch {after cancel $afterid}
        }
    }
    ##
    # _monitorPPID
    #    Called periodically to decide if it's time to start fire off the
    #    after script:
    #
    method _monitorPPID {} {
        if {$initialPid != [ppid]} {
            set afterid "";                    # We're not rescheduling.
            if {$options(-script) ne ""} {
                uplevel #0 $options(-script)
            }
        } else {
            set afterid [after 1000 [mymethod _monitorPPID]]
        }
    }
}
