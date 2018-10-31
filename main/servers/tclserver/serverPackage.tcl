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


# @file serverPackage.tcl
# @brief Pure Tcl package that allows embedded TclServers in scripts.
# @author Ron Fox <fox@nscl.msu.edu>


package provide TclServer 1.0
package require snit


##
# @class TclServer
#
#   This class implements a self contained Tcl Server.
#   A Tcl server accepts connections over a TCP/IP port and then
#   processes commands received over established connections.
#   A requirement of running this is an event loop based application as
#   connection event and data available events are all event driven.
#
# OPTIONS:
#   -port   - Port on which the server listens for connections.
#   -onconnect  - User script that is invoked when a connection is received.
#             the script is given all the parameters the socket -server accept
#             script is given.  If it returns false, the connection will be closed
#             if true, the connection is set up to receive commands.
#  -onerror - User script called when a command results in an error
#             passed the socket, the command string and the error message.
#             If returns true, the socket
#             remains open otherwise it is closed.
# METHODS:
#   start   - Starts the server on the -port configured.  Once started, -port cannot
#             be reconfigured, however -hosts and -onconnect may be.
#   stop    - Stops listening for new connections.
#   close   - Closes all active connections.
#
# NOTE:
#   destruction implies a stop and a close.
#
snit::type TclServer {
    option -port  -default ""  -configuremethod _changePort
    option -onconnect -default ""
    option -onerror   -default ""
    
    # Internal state:
    
    
    variable state      inactive;    # start sets active and _changePort cares
    variable serverFd   "";         # When active, this is the server listen fd    
    variable connections -array [list];   # indices sockets contents command being built
    
    ##
    # constructor
    #   Perform configuration and that's it:
    #
    # @param args   - Configuration parameters.
    #
    constructor args {
        $self configurelist $args
    }
    ##
    # Destructor, stop and close:
    #
    destructor {
        
        $self close
        if {$active} {
            self $stop
        }
    }
    #--------------------------------------------------------------------------
    #  Public methods:
    #
    
    ##
    # start
    #    Starts the server listening on the port.
    #    * Must be inactive.
    #    * -port must be a valid integer.
    #    * sets state -> active.
    #    * sets serverFd -> result of the socket -server command.
    #
    method start {} {
        if {$state eq "inactive" } {
            if {[string is integer -strict $options(-port)]} {
                set serverFd [socket -server \
                    [mymethod _onConnection] $options(-port)]
                set state active
            } else {
                error "To start the server, the port must be an integer but it is: $options(-port)"
            }
            
        } else {
            error "Server is already listening on $options(-port) and must first be stopped"
        }
    }
    ##
    #  stop
    #     Stop listening for new connections.
    #     *  Must be active.
    #     *  Sets serverFd -> "" after closing it. 
    #     *  Sets state->inactive
    #
    method stop {} {
        if {$state eq "active"} {
            close $serverFd
            set serverFd ""
            set state inactive
        } else {
            error "To stop the server it must be active and is not"
        }
    }
    ##
    #  close
    #     Closes all connected clients.  Any partial commands are discarded.
    #
    #  @note The state need not be active as client connections are allowed to
    #        linger after stop is invoked.
    #  @note The state is not changed as a result of this.
    #
    method close {} {
        foreach socket [array names connections] {
            close $socket
        }
        array  unset connections *
    }
    #---------------------------------------------------------------------------
    #  Local methods:
    
    ##
    # _onConnection
    #   Called when a new connection has been received.
    #   * If defined -onconnect is called.
    #   * If that returns true, the channel is set up for line buffering and
    #     nonblocking.
    #   * An entry in the connections array is crated. 
    #   * A file event is set on it for _onInput.
    method _onConnection {channel clientaddr clientport} {
        set accept true
        if {$options(-onconnect) ne ""} {
            set accept [uplevel #0 $options(-onconnect) $channel $clientaddr $clientport]
        }
        if {$accept} {
            fconfigure $channel -buffering line -blocking 0
            set connections($channel) ""
            fileevent $channel readable  [mymethod _onInput $channel]
        } else {
            close $channel;             # Rejected the connection.
        }
    }
    ##
    # _onInput
    #
    #   Called when input is available on a connection.
    #   * Append input to the accumulated input
    #   * If the input could syntactically be a complete command (info complete)
    #      - execute the command
    #      - clear out the command string.
    #
    #  @param channel - the socket open on the connection
    #  @note if the command throws an error the -onerror script is called if
    #        it is defined else the socket is silently closed.
    #
    method _onInput      {channel} {
        
        # Close the channel if the peer closed it:
        
        if {[eof $channel]} {
            close $channel
            array unset connections $channel    
        } else {
            append connections($channel) [read $channel]
            if {[info complete $connections($channel)] && ($connections($channel) ne "")} {
                $self _executeCommand $channel
            } 
        }
    }
    ##
    # _changePort
    #
    #   Called when -port option is configured.   This is only allowed when the
    #   state is inactive.
    #
    # @param optname - Name of the option being configured (-port).
    # @param optval  - Value.
    #
    method _changePort   {optname optval} {
        if {$state eq "inactive" } {
            if {([string is integer -strict $optval]) && ($optval > 0)} {
                set options($optname) $optval
            } else {
                error "The -port value must be a positive integer and is $optval"
            
                # Not checking for port in use or privileged, the socket command will
                # discover that on start.
            }
        } else {
            error "Cannot change the -port option when the server is listening"
        }
    }
    ##
    # _executeCommand
    #   Execute the command in the channel's line element
    #   *  If error invoke -onerror and close up the channel if that returns false.
    #   *  blank out the command text.
    #   *  Send the result back in blocking mode to the peer.
    #
    # @param channel - connection socket
    # 
    method _executeCommand channel {
        set command $connections($channel)
        set connections($channel) ""
        puts "Executing '$command'"
        if {[catch {uplevel #0 $command} msg] == 0 } {
            fconfigure $channel -blocking 1
            puts $channel $msg
            flush $channel
            fconfigure $channel -blocking 0
        } else {
            if {$options(-onerror) ne ""} {
                set keepOpen [uplevel #0 $options(-onerror) $channel [list $command] [list $msg]]
                if {!$keepOpen} {
                    close $channel
                    array unset connections $channel
                }
            }
        }
    }
    
}