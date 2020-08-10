#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2009.
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

package provide  s800 1.0
package require  snit
package require  portAllocator
package require  Bundles


##
# @brief The master readoutgui logic for handling a remote Readout GUI
#
# Originally this was implemented to remotely control the S800DAQ's RunControl
# program via a defined interface. This same interface is the backbone of
# this class. More recently, the ReadoutGUI has been enhanced to support remote
# control through the ReadoutGUIRemoteControl using the same interface. More
# often than not, in the future this will provide a controlling connection to a
# ReadoutGUIRemoteControl. It is therefore the code that runs in the master
# ReadoutGUI and commands a remote slave.
#
# With that said, it also can receive requests from the slave. There are two
# connections that this maintains much like the way the ReadoutGUIRemoteControl
# class does. One of these is explicitly for send requests (requestfd) to the
# slave and the other is intended for the receipt of requests from the slave
# (replyfd). Both of these connections communicate via symmetric transactions in
# which any request made expects a reply. While waiting for the reply, the class
# implements a soft-blocking scheme in that it processes events but does not
# continue. Only one request socket is allowed.
#
snit::type s800rctl {
  option -host "localhost";	# Host on which the s800 usb rdo runs.
  option -port 8000;		# Port on which the s800 usb rdo listens.
  option -timeout 10000;	# ms to wait for the s800 to reply

  variable socket "";	 	# Socket connected to the s800 rctl server.
  variable requestReplyReceived 0; # flags when complete reply has been received
  variable requestReply "";   # reply received from the slave after request

  typevariable listenSocket "";		# Listens for connections from the slave 
  #  variable replySocket "";	# channel local.	# Socket that can respond to requests from the slave
  typevariable replyReply -array [list];     # Incoming message from slave

  variable ACK "OK"
  variable NAK "FAIL"

  variable readDoneFlag;	# Incremented to indicate socket readable.
  variable timedOut;   	# Incremented by timeout handler.
  variable timeoutId  -1;	# ID of the timeout [after].


  ##
  #  The constructor forms the connection to the -host, -port configured
  #  into the system.
  #   -host is pretty much mandatory as it's not likely the rdo is running on 
  #         localhost.
  #
  #  Sets a listening socket on which to listen for connections from the slave
  #  to use for making requests. The listening socket will be associated with
  #  the service locatable as "s800rctl"
  #
  # on Connection failure, a error is generated.
  #
  constructor args {
    $self configurelist $args

    if {$listenSocket eq ""} {
      set port [$self _getPortForService s800rctl]
      set listenSocket [socket -server [mytypemethod _onConnection] $port]
    }
    $self Connect
  }

  ##
  # Destruction sets the s800 in to master mode and closes the connection
  #
  #
  destructor {

  #
  #  The catch is in case we're being destroyed because we lost
  #  our connection.
  #

    set stat [catch {
      if {$socket ne ""} {
        close $socket
      }
    } msg]
    
  }


  
  ## Retrieve a preallocated port for service or allocate an new one to return
  #
  # @returns port for service
  #
  method _getPortForService {service} {

    set allocator [portAllocator %AUTO%]

    # search through allocated ports for a specific service
    set allocatedLocalPorts [$allocator listPorts]
    foreach portInfo $allocatedLocalPorts {
      if {[lindex $portInfo 1] eq $service} {
        $allocator destroy
        return [lindex $portInfo 0] ;# return the port number
      }
    }

    # we did not find a preallocated port, so allocate one.
    
    set port [$allocator allocatePort s800rctl]
    $allocator destroy
    return $port

  }

  #------------------------------------------------------------------
  #
  #  Operations on the s800:
  #

  ##
  # Set the S800 into slave mode:
  #
  # If a NAK is provided the remainder of the line is thrown as
  # an error:
  #
  method setSlave {} {
		set bundles [BundleManager getInstance]
		$bundles invoke remotecontrol OnEnslaving $options(-host)
    set result [$self Transaction "set slave 1"]
    $self ThrowIfNak $result
		
  }
  ##
  # Set the s800 back into master mode:
  #
  # If a NAK is provided the remainder of the line is thrown as
  # an error.
  #
  method setMaster {} {
		set bundles [BundleManager getInstance]
		$bundles invoke remotecontrol OnFreeing $options(-host)
    set result [$self Transaction "set slave 0"]
    $self ThrowIfNak $result
  }
  ##
  #  Request that a run start.
  #
  # NAKs result in an error throw:
  #
  method begin {} {
    $self masterTransition Active
  }
  ##
  # Request that a run end.
  #
  # NAKs result in error throws:
  #
  method end {} {
    $self masterTransition Halted
  }

  if {0} {
  ##
  # Request that a run pause.
  #
  # NAKs result in error throws:
  #
  method pause {} {
    set result [$self masterTransition Paused]
    $self ThrowIfNak $result
  }

  ##
  # Request that a run pause.
  #
  # NAKs result in error throws:
  #
  method resume {} {
    set result [$self masterTransition Active]
    $self ThrowIfNak $result
  }
  }
  ##
  # Request an init operation.
  #
  # NAKs result in error throws:
  #
  method init {} {
    set result [$self Transaction init]
    $self ThrowIfNak $result
  }

  ##
  # Request a generic state transition
  #
  # @param newState   state to transition to  
  #
  # NAKs result in error throws
  method masterTransition {newState} {
    set result [$self Transaction [list masterTransition $newState]]
    $self ThrowIfNak $result
  }

  ##
  # Set the run number to the appropriate value
  #
  # @param n - run number must be an integer >= 0.
  #
  # Throws an error on NAK.
  # Throws an error on an invalid run number.
  #
  method setRun {n} {
    if {![$self ValidRun $n]} {
      error "ERROR - setRun detected an invalid run number $n"
    }
    set result [$self Transaction "set run $n"]
    $self ThrowIfNak $result
  }
  ##
  # Set the run title.
  #
  # @param title - The title of the new run.
  #
  # Throws an error if a NAK.
  # @note The title has any "'s turned into \" so that the string
  #       is a valid Tcl quoted string.
  #
  #
  method setTitle title {
    set title [$self SubstituteSpecialChars $title]
    set result [$self Transaction "set title \"$title\""]
    $self ThrowIfNak $result
  }

  ##
  # Request recording
  #
  # Throws an error on NAK
  #
  method setRecordingOn {} {
    set result [$self Transaction "set recording 1"]
    $self ThrowIfNak $result
  }
  ##
  # Request recording off
  #
  # Throws an error on NAK.
  #
  method setRecordingOff {} {
    set result [$self Transaction "set recording 0"]
    $self ThrowIfNak $result
  }
  ##
  # Set the file save destination.
  #  The s800 will interpret this as a directory relative to this
  # base directory (usually the user's home directory).  That directory
  # must have S800 write access.
  #
  # @param dir - destination directory.
  #
  # Throws on NAK.
  #
  method setDestination dir {
    set result [$self Transaction "set destination $dir"]
    $self ThrowIfNak $result

  }
  ##
  # Return the current s800 state (see below).
  #
  # @return state string which is one of "active" "inactive"
  #
  method getState {} {
    set result [$self Transaction "get state"]
    $self ThrowIfNak $result
    set tail [lindex $result 1]
    set tail [string trim $tail]
    return $tail
  }



  #------------------------------------------------------------------
  #
  #  Private methods
  #
  ##
  #   Connect to the run control
  # Implicit inputs:
  #   $options(-host) contains the target host.
  #   $options(-port) contains the target port.
  # Implicit outputs:
  #   the socket variable will have the connected socket.
  # Errors:
  #   Connection failure throws an error.
  #
  method Connect {} {
    if {$socket eq ""} {
      set socket [socket $options(-host) $options(-port)]
      chan configure $socket -buffering line -blocking 0
      chan event $socket readable [mymethod _onRequestReadable]      
    } else {
      error "s800rctl::Connect - socket already connected"
    }

  }

  ##
  # Send a command to the run control and get an Ack back
  # ACK is a string whose first characters match the ACK variable contents.
  # NAK is a string whose first characters match the NAK variable contents.
  #
  # @param command - The command to send.
  #
  # @return - two item list the first item is either 'OK' or 'ERROR' depending
  #           on whether or not an ACK or NAK was received.  The second
  #           item is the remainder of the response line from the server.
  # Errors:
  #   If the socket is disconnected and could not be reconnected, an error
  #   is thrown.
  method Transaction {command} {

  #  If the socket is disconnected (empty $socket) try to reconnect:

    if {$socket eq ""} {
      $self Connect;		# Throws error on failure.
    }


    # Try to send the message.  If there's a failure
    # 1. set the socket to null
    # 2. try once to reconnect (throws an error if necesary).
    # 3. If a successful reconnect, re-try the message:
    #
    set requestReply ""
    if {[catch {puts $socket  "$command"} msg]} {
      set socket "";	# In case we live through a connect fail.
      $self Connect
      $self setSlave
      puts $socket "$command";	# Allow the retry to throw an error.
    }

    # wait until the reply has been fully received before proceeding
    # meanwhile process events
    vwait [myvar requestReplyReceived]


    set requestReplyReceived 0

    # Analyze the result:
    return [$self AnalyzeResponse $requestReply]

  }


  
  ## 
  # Read from the socket with a timeout.
  #
  # NO LONGER USED ... but is a very clever idea so I am keeping it around.
  #
  # This is done by 
  # - Establishing a filevent for readability on the socket
  #   which just increments readDoneFlag
  # - Establishing an after timer which increments the read done flag and sets timed out
  # - vwaiting on readDoneFlag.
  # - etc. etc.
  #
  # @param timeout  - ms to wait for a response.
  #
  # @return the data read on success.
  #
  # @throw - if timedout an error string of "Read timed out on s800"
  # @throw - gets on socket failed.
  #
  method ReadWithTimeout timeout {

  # wait for the socket to be readable with a timeout:
    set timedOut 0


    ##
    # Grumble.... none of this works in the presence of I/O that's
    # scheduled via an AFTER because the vwait below allows those to come
    # crashing down on top of us messing everything up.  For now
    # (and probably forever) I'm giving up the timeout in order to keep'
    # the ability to status poll.

    return [gets $socket]


    fileevent $socket readable [list incr ${selfns}::readDoneFlag]
    set timeoutId \
      [after    $timeout "set ${selfns}::timedOut 1; incr ${selfns}::readDoneFlag"]
    vwait ${selfns}::readDoneFlag

    # Reset the fileevent and if we did not time out cancel the timer:

    fileevent $socket readable [list]
    if {!$timedOut} {
      after cancel $timeoutId
    }
    set timeoutId -1;	

    #  Now figure out what happened:

    if {$timedOut} {
      error "Read timed out on s800"
    }
    return [gets $socket]
  }


  ##
  #  Analyze the response message and break it up into 
  #  the chunks described in Transaction above.
  #
  # @param msg - the resonse message.
  #
  # @return list as described in Transaction above.
  #
  method AnalyzeResponse msg {
    set status [lindex $msg 0]
    set tail   [string range  $msg [string length $status] end]
    set tail   [string range $tail  1 end];	# strip off space after status.

    if {$status eq $ACK} {
      set status "OK"
    } elseif {$status eq $NAK} {
      set status "ERROR"
    } else {
      error "Invalid response from server: $msg"
    }
    return [list $status $tail]

  }
  ##
  # Throws an error if NAK
  #
  # @param response - two element list that is the cooked response
  #                   from the s800
  #
  method ThrowIfNak {response} {
    if {[lindex $response 0] eq "ERROR"} {
      error [lindex $response 1]
    }
  }
  ##
  # Ensures a value is a valid run number.
  #
  # @param v - Value to check.
  #
  # @return bool true if valid, false if not.
  #
  method ValidRun v {

  # Run numbers must be valid nonempty integers:

    if {![string is integer -strict $v]} {
      return false
    }
    # ..and those integers cannot be negative:

    if {$v < 0} {
      return false
    }
    return true
  }
  #  Substitute the special string characters for their backslashed equivalents.
  #
  # @param string - input string
  # @return string -substituted string e..g This is a "test" becomes This is a \"test\"
  #
  method SubstituteSpecialChars {string} {
    regsub -all -- {"} $string {\"} string;	# " <-- stupid emacs colorization/tabbing.
    return $string
  }


  ## @brief Handle generic readable events from a channel
  #
  # If the eof condition exists, close the channel. Otherwise, read what can be
  # read and return it. Because the sockets are non-blocking, it is possible
  # that the call to gets did not retrieve the entire message. The return value
  # therefore included the result of whether the full message was received or
  # not.
  #
  # @param fd   the channel handle to read from
  #
  # @retval  Two element list : 
  #             - element 1 indicates if gets was complete, 
  #             - element 2 the data read
  # @retval "" - eof of file condition
  #
  typemethod _onReadable {fd} {
    if {[eof $fd]} {
      # close the channel
      catch {close $fd}
			
    } else {
      # read what we can from the channel
      if {[catch {gets $fd line} len]} {
        set msg "s800rctl::_onReadable unable to read data from peer $len"
        puts stderr $msg
      } else {
        return [list [chan blocked $fd] $line]
      }
    }
  }

  ## @brief Read request from the peer
  #
  # Outsources the reading to the _onReadable method and then handles the
  # result. If the input operation in _onReadable did not return early because
  # of block, then the reply is complete and handled. Otherwise, the data read
  # is simply appended to the replyReply and waiting resumes.
  # 
  # @returns ""
  #
  typemethod _onReplyReadable {replySocket} {

    # read what can be read from the channel
    set readInfo [$type _onReadable $replySocket]
    if {$readInfo in {0 1}} {
      return;             # EOF on input... hopefully someone else discovers problems.
    }
    # append what was read to what was previously read
    append replyReply($replySocket) [lindex $readInfo 1]

    # if the read was complete, handle the received message
    #
    #  Quick notes on readInfo:
    #     Could be a command word.
    #     On eof from the peer it can be either:
    #       -   0 - The close succeeded.
    #       -   1 - the close failed but what are you going to do so we ignore
    #               the result.
    if {![lindex $readInfo 0] && ($readInfo != 1)} {

      
    
      # we have to reset the string replyReply but at the same time
      # use the result in _handleCommand. The issue is that someone could
      # remotely be inciting a transition to NotReady in the _handleCommand
      # method. That would wind up calling S800::stop, which would straight 
      # up destroy this instance. If we reset the replyReply string after,
      # we might try to set replyReply when its instance is long gone. That
      # causes a tcl error because the namespace for replyReply no longer 
      # exists. So we have to copy it into a temporary variable that we 
      # use later in the method. Shoot.
      set command $replyReply($replySocket)
      set replyReply($replySocket) ""

      # only evaluate the command if it was valid
      

      if {$command ne ""}  {
        
        if {[ $type _isValidCommand [lindex $command 0]]} {
        # if we had a valid command, then send the response immediately.
        # Executing the command the peer sent may require further
        # communication and at the moment the peer is waiting for a response
        # in blocking mode. Don't let it continue to block! 
          $type _sendResponse $replySocket "OK"
          set result [$type _handleCommand $command]
        } else {
          $type _sendResponse $replySocket "FAIL invalid command \"$command\""
        }
      }
    }
  }

  ## @brief Read replies from the slave 
  #
  # Outsources the read to the _onReadable method. Once a full message has been
  # received, then set the flag. 
  # 
  #
  method _onRequestReadable {} {

    # read what can be read 
    set readInfo [$type _onReadable $socket]
    
    # append the new data to what has previously been read
    append requestReply [lindex $readInfo 1]

    if {![lindex $readInfo 0]} {
      # the read did not block so we must have received the hole message! Flag
      # it complete.
      set requestReplyReceived 1
    }
  }

  ## @brief Check to see if a verb is something that this will happily act on
  #
  # At the moment, only the end operation is supported.
  #
  # @returns boolean
  # @retval 0 - not valid
  # @retval 1 - valid
  typemethod _isValidCommand {verb} {
    set acceptedVerbs [list end transitionTo]
    return [expr {$verb in $acceptedVerbs}]
  }

  ## @brief Execute a command
  #
  # @param script   the command to execute
  #
  # @returns string containing status and command result
  typemethod _handleCommand {script} {

    # evaluate the script
    set res [catch {uplevel #0 eval $script} msg]

    # check for return status other than 0
    if {$res} { 
      return "FAIL $msg"
    } else {
      # status was 0, formulate a response
      if {$msg ne ""} {
        return "OK $msg"
      } else {
        return "OK"
      }
    }
  }

  ## @brief A unidirectional message
  #
  # @param fd       the channel handle to send response on
  # @param response message to send 
  #
  # @throws error if no channel provided
  typemethod _sendResponse {fd response} {
    if {$fd eq ""} {
    # just in case the socket blew away after reading the message
      set msg "ReadoutGUIRemoteControl::_sendResponse Cannot send response "
      append msg "to peer because it does not exist."
      return -code error $msg 
    } else {
      puts $fd $response
    }
  }

  # _onConnection
  #   Called when a server connection comes in:
  #   -  Close the connection if we already have a client..only one customer at
  #      a time is allowed.
  #   -  Set buffering to line.
  #   -  Set a fileevent to fire when the socket is readable.
  #   -  Save the socket in the clientfd attribute.
  # 
  # @param client     - socket fd open on client.
  # @param clientaddr - IP address of client.
  # @param clientport - Remote Port address.
  #
  typemethod _onConnection {client clientaddr clientport} {
    
      chan configure $client -buffering line
      set replySocket $client
      chan event $replySocket readable [mytypemethod _onReplyReadable $replySocket]
      
      set replyReply($replySocket) ""
  }
}
