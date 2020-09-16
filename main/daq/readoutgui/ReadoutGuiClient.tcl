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
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

package provide ReadoutGuiClient 1.0
package require portAllocator
package require snit

##
# @file   ReadoutGuiClient.tcl
# @brief  Provide support for remote clients of a ReadoutGUI.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
# readoutGUIControlPort
#
#  Get the port associated with a readout gui remote control port running in
#  a specified host under a specified user.
#
# @param host - the host running the server.
# @param service - the service to find; defaults to ReadouGUIRemoteControl
# @param user - Optional username of the process providing the service.
#               Defaults to the current user
# @return mixed
# @retval integer - Port number for the service.
# @retval ""      - There is no process providing that service run by the user
#
proc readoutGUIControlPort {host {service ReadoutGUIRemoteControl} {user {}}} {
    set a [portAllocator %AUTO% -hostname $host]
    if {$user eq ""} {
        set user $::tcl_platform(user)
    }
    set port [$a findServer $service $user]
    $a destroy
    return $port
    
}
##
# readoutGUIOutputPort
#   Same as above but returns the port of the output monitor service
#
# @param host - the host running the server.
# @param service - Service to lookup, defaults to ReadoutGUIOutput
# @param user - Optional username of the process providing the service.
#               Defaults to the current user
# @return mixed
# @retval integer - Port number for the service.
# @retval ""      - There is no process providing that service run by the user
#
proc readoutGUIOutputPort {host {service ReadoutGUIOutput} {user {}}} {
    set a [portAllocator %AUTO% -hostname $host]
    if {$user eq ""} {
        set user $::tcl_platform(user)
    }
    set port [$a findServer $service $user]
    $a destroy
    return $port
}


##
# @class ReadoutGUIOutputClient
#
#  Client for the readoutGUI output server.
#
# OPTIONS
#   -host - Host on which the server is running.
#   -user - User running the ReadoutGUI we're going to monitor.
#   - outputcmd - If not empty a command to invoke when there
#                 is output from the host.  The text is quoted and appended to the
#                 command.
#   -closecmd   - If not empty, a command to invoke when the server closes its
#                 connection with us.
#
snit::type ReadoutGUIOutputClient {
    option -host localhost
    option -user $::tcl_platform(user)
    option -outputcmd [list]
    option -closecmd  [list]
    option -service -default ReadoutGUIOutput -readonly 1
 
    variable clientfd -1
    
    
    ##
    # Constructor
    #   Just configures the options because we make connection another stage.
    #   this allows the application to retry in the event the server is not
    #   yet ready.
    #
    constructor args {
        $self configurelist $args
    }
    ##
    # destructor
    #    If clientfd is open close it:
    #
    destructor {
        if {$clientfd != -1} {
            close $clientfd
        }
    }
    ##
    # connect
    #   Try to locate the server port and if possible connect
    #   and set up the fileevent.
    #
    # @return boolean
    # @retval true  - Success
    # @retval false - Unable to connect (usually server not ready)
    #
    method connect {} {
        #
        #  It's an error to connect if we're connected:
        #
        if {$clientfd != -1} {
            error "ReadoutGUIOutputClient - already connected."
        }
        #
        #  Figure out which port is the listener
        #
        set port [readoutGUIOutputPort $options(-host) $options(-service) $options(-user)]
        if {$port eq ""} {
            return false
        }
        #  Try to connect:
        
        set failure [catch {
            set clientfd [socket $options(-host) $port]
            fconfigure $clientfd -buffering line
            fileevent $clientfd readable [mymethod _onReadable]
        }  msg]
        return [expr {!$failure}]
        
    }
    ##
    # _onReadable
    #    Called when connection is readable.  This means either the
    #    server has exited or we have a line of output text.
    # 
    method _onReadable {} {
        if {[eof $clientfd]} {
            $self _onExit
        } else {
            $self _onInput
        }
    }
    ##
    # _onInput
    #
    #   Called when there's input on the socket.  The input
    #   is read and then the -outputcmd script (if defined) is
    #   called.
    #
    method _onInput {} {
        set data [gets $clientfd]
        set cmd $options(-outputcmd)
        if {$cmd ne ""} {
            uplevel #0 $cmd "{$data}"
        }
    }
    ##
    # _onExit
    #   Called when the peer disconnects.  Closes the socket and,
    #   if defined, calls the -closecmd script.
    #
    method _onExit {} {
        close $clientfd
        set clientfd -1;     # To allow reconnect attempt.
        
        set cmd $options(-closecmd)
        if {$cmd ne ""} {
            uplevel #0 $cmd
        }
    }
}

