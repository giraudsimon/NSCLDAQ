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
# @file   EVBMonitor.tcl
# @brief  Program to provide a REST UI to the event builder.
# @author Ron Fox <fox@nscl.msu.edu>
#
if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}

package require EVBRESTClient
package require EVBRestUI
package require EVBRestControllers


##
# The UI Is based on the MVC architecture implemented in the packages required
# above.  Our main task is to:
#   *  Create/Layout the views above.
#   *  Create a client model.
#   *  Build controllers that link the client models with the views.
#   *  Schedule updates for all the controllers.
#
#------------------------------------------------------------------------------
# Utility procs
#

##
# usage
#   Output an error message and program usage and exit.
#
# @param msg - error message that will precede the usage message.
# @note all output is to stderr.
#
proc usage {msg} {
    puts stderr $msg
    puts stderr "Usage: "
    puts stderr "    \$DAQBIN/EVBMonitor host user ?service?"
    puts stderr "Provides a graphical user interface to monitor event builder"
    puts stderr "statistics Where:"
    puts stderr "   host   - is the host the event builder is running in"
    puts stderr "   user   - is the user the event buidler is running under"
    puts stderr "   service- is an optional service name the event builder is"
    puts stderr "            is advertising its REST interface on.  The defaul"
    puts stderr "            is the standard REST interface service name (ORDERER_REST)"
    
    exit -1
}
#------------------------------------------------------------------------------
# _update
#   Perform an update in all controllers and reschedule .
#
# @param controllers - list of controllers each controller's update method
#                      will be called.
# @param interval    - optional update interval in ms.  This defaults to
#                      2000 which provides a 2 second update interval.
#
proc _update {controllers {interval 2000}} {
    foreach controller $controllers {
        $controller update
    }
    after $interval _update $controllers $interval
}

##
# _buildGUI
#    Build the GUI and lay it out.  The GUI consist of a top part
#    that contains a notebook of most of the views and a bottom part that
#    shows the connection list and flow control state.  Each view is built
#    and bound into a controller that connects it to the underlying model.
#
# @param model   - The model object that gets data from the event builder.
# @return list   - list of controller object names.  The entire
#                  GUI can be refreshed by interating over this list and
#                  invoking the update method on each element. see _update
#                  above.
#
proc _buildGUI {model} {
    set result [list]
    ttk::notebook .notebook
    ttk::frame    .bottom
    
    # STock the notebook.
    
    # Stock the bottom frame:
    
    ConnectionView .bottom.connections
    lappend result \
        [EVBConnectionController %AUTO% -model $model -view .bottom.connections]
    
    FlowControlView .bottom.flow
    lappend result [EVBFlowController %AUTO% -model $model -view .bottom.flow]
    
    grid .bottom.connections -sticky nsew
    grid .bottom.flow -sticky nsw
    
    grid .notebook
    grid .bottom -sticky nsew
    
    
    return $result
}

# 
#------------------------------------------------------------------------------

#  Entry point
#

#  Process the command line parameters:

if {[llength $argv] < 2} {
    usage "At least the 'host' and 'user' command parameters must be supplied."
}
set host [lindex $argv 0]
set user [lindex $argv 1]
set argv [lrange $argv 2 end]
if {[llength $argv] > 1} {
    usage "At most the 'host', 'user' and 'service' command paramters can be supplied"
}

# Create the model client object:

if {[llength $argv] == 0} {
    #Default service:
    
    set model [EVBRestClient %AUTO% -host $host -user $user]
} else {
    #  Service provided:
    
    set service [lindex $argv 0]
    set model [EVBRestClient %AUTO% -host $host -user $user -service $service]
}

#  Create the GUI elements, lay them out, build controllers and bind the model
#  controllers and views:

set controllers [_buildGUI $model]

#  Now we just need to get the updates rolling:

_update $controllers


