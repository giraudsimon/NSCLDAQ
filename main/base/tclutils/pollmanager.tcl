#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#

#  This file provides a poll manager that can dispatch
#  procs at periodic intervals.  It is used to deal with
#  the lack of scalability in [after].  In this case,
#  a single [after] can be used to fire off multiple
#  actions.
#
# OPTIONS:
#      -period  ms         - Sets the polling period in milliseconds.
#                            Default is 1000ms.
# METHODS:
#      add  script         - Adds a script to run whenever the after fires.
#                            returns a script id that is unique within this
#                            pollmanager object.  No promises are made about
#                            the order in which the scripts are fired.
#      remove id           - Removes the script identified by the id from the
#                            poll set.
#      stop                - Stop polling
#      start               - Start polling (initially the object is _not_ polling).
# NOTE:
#   Multiple stop/starts are harmless, stop when stopped is a no-op,
#   start when running is an no-op.
#


package provide PollManager 1.0
package require snit

snit::type pollManager {
    
    option   -period  1000
    
    variable afterId   {}
    variable scripts
    variable scriptSerial 0
    
    constructor args {
        $self configurelist $args
    }
    destructor {
        $self stop
    }
    
    #--------------------------------------------------------------------------
    #   Start  polling.
    #
    method start {} {
        if {$afterId eq ""} {
            set afterId [after $options(-period) [mymethod DispatchScripts]]
        }
    }
    #--------------------------------------------------------------------------
    #   Stop polling.
    #
    method stop {} {
        if {$afterId ne ""} {
            after cancel $afterId
            set afterId [list]
        }
    }
    #---------------------------------------------------------------------------
    # Add a new script to the list of scripts that will be executed.
    #
    # Parameters:
    #   script     - The script to execute when the after fires.
    # Returns:
    #   an Id that can be used to remove the script from the list.
    #
    method add script {
        set index "${self}_[incr scriptSerial]"
        set scripts($index) $script
        return $index
    }
    #-------------------------------------------------------------------------
    #
    #  Removes the script indicated by the id from the list of processed scripts
    #
    method remove id {
        if {[array names scripts $id] eq ""} {
            error "No script with the id $id exists in this pollManager"
        }
        unset scripts($id)
    }
    
    #========================= Private methods =================================
    #---------------------------------------------------------------------------
    #
    #  dispatch the scripts.  Scripts are stored in the array scripts
    #
    method DispatchScripts {} {
        #  Dispatch all the scripts:
        
        foreach index [array names scripts] {
            set script $scripts($index)
            eval $script;                        # Let errors be known!
        }
        #
        #  Schedule next event- done here though imprecise so that error scripts will
        #  stop the polling!!... preventing periodic error messages.
        #
        set afterId [after $options(-period) [mymethod DispatchScripts]];
        

    }
}