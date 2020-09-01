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


#--------------------------------------------------------------------------------
#
#  Event builder connection manager:
# 
#  This must be run in an event loop such as in a vwait or in 
#  tk
#
#  Two snit objects are defined here:
# 
#   Connection - The state machine that makes up an actual connection.
#                What a connection does with input depends on
#                the state a connection is in.
#                Connections are created in the 'FORMING' state and have the following 
#                additional states:
#                - 'ACTIVE'  - The connection is in a state where it can handle data'
#                              from a data source
#                - 'CLOSED'  - The connection closed normally (as per protocol).
#                - 'LOST'    - The connection was lost without normal close protocol.
#                - 'ERROR'   - The connection saw a protocol error and was dropped from
#                              our end.
#
#  ConnectionManager - Handles TCP/IP level connection requests creating connections
#                      as needed and killing them as well.
#

package require snit
package require EVB::CallbackManager
package require EvbOrderer;	# C/C++ orderer.
package require Observer;	# Observer pattern component

package provide EVB::ConnectionManager 1.0

namespace eval EVB {
    #
    #  Message type codes:
    #
    variable CONNECT    1
    variable FRAGMENTS  2
    variable DISCONNECT 4
    
    variable HeaderSize 8;           # two uint32_t items in the header.
}


##
# Connection object
#
# OPTIONS
#   -state - Current socket state (see above).  This is readonly.
#   -clientaddr        - Client TCP/IP address (this is readonly).
#   -description       - Client description as provided in the the connection negotiation.
#   -socket            - Socket connected to client.
#   -disconnectcommand - Script  to call on disconnect.
#   -fragmentcommand   - Script to call on a fragment
#
# METHODS
#   Only construction and destrution are are public.
#
snit::type EVB::Connection {
    option -state -default FORMING -readonly yes
    option -description -default "" -readonly yes

    option -socket;		#  The socket connected to client.
    option -clientaddr;#  -default "not-set"
    option -disconnectcommand -default [list] -configuremethod _SetCallback 
    option -fragmentcommand   -default [list] -configuremethod _SetCallback

    variable alive 0
    variable callbacks
    variable expecting ""
    variable stateMethods -array [list FORMING _Connect ACTIVE _Fragments]




    constructor args {
        $self configurelist $args
        fconfigure $options(-socket) -blocking 1 -buffering none -translation {binary lf} \
            -encoding binary
        set callbacks [EVB::CallbackManager %AUTO%]
        flush stdout
        $callbacks define -disconnectcommand
        $callbacks define -fragmentcommand
    
        $self _Expecting _Connect FORMING; # We are now expecting a CONNECT command.

    }
    destructor {
        if {$options(-socket) != -1} {
            catch {$self _Close CLOSED}
        }
        $callbacks destroy
    }
    method isAlive {} {return $alive}
    ##
    # tryRead
    #    Check to see if there are data currently buffered in our channel
    #    and if so dispatch.  During this , the -fragmentcommand is disabled.
    #
    method tryRead {} {
        if {$options(-socket) eq "-1"} {
            return
        }
        catch {
            if {[chan pending input $options(-socket)] > 0} {
                $callbacks register -fragmentcommand [list]; # turn off callback
                $self $expecting $options(-socket)
                $callbacks register -fragmentcommand $options(-fragmentcommand); #  Restore.
            }
        }
    }
    ##
    # flowOff
    #   Called to disable reception of data.
    #
    method flowOff {} {
        fileevent $options(-socket) readable [list]
    }
    ##
    # flowOn
    #   Called to enable reception of data.
    #
    method flowOn {} {
        set method $stateMethods($options(-state))
        fileevent $options(-socket) readable [mymethod $method $options(-socket)] 
    }

    #----------------------------------------------------------------------------
    # Configuration
    #

    ##
    # _SetCallback - register a callback handler for an option.
    #
    # @param option - name of the option.
    # @param script - New script to register.
    #
    method _SetCallback {option script} {
        $callbacks register $option $script
        set options($option) $script
    }


    #------------------------------------------------------------------------------
    # Private methods:

    ##
    # _DecodeConnectBody
    #
    #  Decodes the body of a connect message.  This consists of:
    #
    #  - 80 characters of description information.
    #  - uint32 of number of following source ids.
    #  - source ids each in a uint32_t
    #  
    #  The count and source ids are all little endian uint32_t numbers.
    #
    # @param body - The body binary blob.
    # @return list - 2 element list.
    # @retval First list element is the description string.  The second, a list of source ids.
    #
    method _DecodeConnectBody body {
        #  The string and number of bytes:
        
        binary scan $body A80i description sourceCount
        set cursor 84;                # where we start.
        set sources [list]
        for {set i 0} {$i < $sourceCount} {incr i} {
            
            binary scan $body @${cursor}i sid
            lappend sources $sid
            incr cursor 4
        }
        return [list $description $sources]
    }

    ##
    # Read binary data from the socket with a leading uint32_t that contains
    # the number of bytes in the data:
    #
    # @param socket - The socket to read from.
    #
    # @return byte array containing the data read from the socket.
    #
    method _ReadBinaryData socket {
        set count [read $socket 4]; # Read the count.
    
        if {[string length $count] != 4} {
            error "Read of byte array length failed"
        }
        binary scan $count i1 size
    
        if {$size > 0} {
            return [read $socket $size]
        } else {
            return ""
	}
    }

    ##
    # Reads a counted string from the socket turning it into a 
    # Tcl string.
    #
    # @param socket
    # @return string
    # @note If a read on the socket fails an error is thrown.
    #
    method _ReadCountedString socket {
        #
        # Note that [string length] is documented to
        # return the length of a byte array object if that's what it's
        # applied to so it should work below:
        #
        set count [read $socket 4]; # uint32_t.
        if {[string length $count] != 4} {
            error "read of string length failed"
        }
        binary scan $count i1 size
        set stringBytes [read $socket $size]
        if {[string length $stringBytes] != $size} {
            error "read of string itself failed"
        }
        #
        # Decode the string as a tcl string via binary scan
        #
        binary scan $stringBytes a$size result
        return $result

    }

    ##
    # Read/Decode a message whose header and body payloads are ASCII Strings.
    #
    # @param socket - The socket open on the client.
    #
    # @return a 2 element list.  [lindex .. 0] is the header text.
    #         [lindex .... 1] is the body text.
    #
    method _ReadTextMessage socket {
    
        set header [$self _ReadCountedString $socket]
        set body   [$self _ReadCountedString $socket]
    
        return [list $header $body]

    }

    ##
    #   Indicate which method shouild be associated with socket readability as part
    #   of a state transition.
    #
    # @param method - the new method that shouild handle readability.
    # @param newState - The new state value.
    #
    method _Expecting {method newState} {
        fileevent $options(-socket) readable [mymethod $method $options(-socket)] 
        set options(-state) $newState
        set expecting $method
    }
    ##
    # Called to close the connection
    #  - The new state is set.
    #  - The socket is closed.
    #  - The callback, if set, is called.
    #
    # @param newState - New state to set.
    method _Close {newState} {
        set alive 0
        set options(-state) $newState
        fileevent $options(-socket) readable [list]
        close $options(-socket) 
        set options(-socket) -1
    
        $callbacks invoke -disconnectcommand [list] [list]

    }

   
    ##
    #  Input handler for when we are expecting a CONNECT message.
    #  when one is properly handled, we transition to the ACTIVE state.
    #
    # @param socket - the socket on which we are going to get our
    #                 message.
    # @note:  If the message given is not a valid CONNECT,
    #         We just transition to ERROR, and close out
    #
    method _Connect socket {
    
        # Get the header and get the body.
        # The header is a string, but the body is a counted binary block:
    

        if {[catch {read $socket $EVB::HeaderSize} header]} {
            catch {puts $socket "ERROR {Could not read header expecting CONNECT}"}; # Might fail.
            $self _Close ERROR
            return
        }
        if {[eof $socket]} {
            catch puts $socket "ERROR - unable to read header\n"
            puts stderr "EOF on read of header! ignore connection."
            $self _Close ERROR
            return
        }
        #
        #  Decode the header and read the body, if there is one:
        #
        binary scan $header "ii" bodySize msgType

        if {[catch {read $socket $bodySize} body]} {
            puts stderr "Could not read body: $body"
            puts $socket "ERROR {Corrupt body in CONNECT message}"
            $self _Close ERROR
            return
        }
    
    
        # Must be a CONNECT message with a non-zero body.
        #
    
        if {$msgType ne $EVB::CONNECT} {
            catch {puts $socket "ERROR {Expected CONNECT}"}
            $self _Close ERROR
            return
        }
        
        # the body consist of 80 characters of description  and
        # a count of source ids followed by that many source ids.
    
        set alive 1
    
        # Pull out the description and the source id list
    
        set decodedBody [$self _DecodeConnectBody $body]
        set description [lindex $decodedBody 0]
        set sourceIds   [lindex $decodedBody 1]

        puts $socket "OK"
        flush $socket
    
        # Save the description and transition to the active state:
            # Register ourself with the event builder core
    
        set options(-description) $description
        $self _Expecting _Fragments ACTIVE
    
    
        EVB::source $socket {*}$sourceIds
     
    }
    #
    # Expecting fragments if the next message is
    # DISCONNECT, close the socket after responding.
    # If the next message has a FRAGMENTS header
    # pass the socket down to the C/C++ code
    # for further processing.
    #
    # @param socket - socket that is readable.
    #
    method _Fragments socket {
        # Read the header:
        
        if {[catch {read $socket $EVB::HeaderSize} header]} {
            if {[eof $socket]} {
                $self _Close LOST
                return
            } else {
                #  Some other error:
                
                $self $Close LOST
                error "Failure reading FRAGMENTS"
            }
        }
        set result [binary scan $header ii bodySize msgType]
        if {$result != 2} {
            puts stderr "Header only decoded to $result values instead of 2"
            puts stderr "Closing connection due to errors"
            flush stderr
            # Closing connection.
            $self _Close LOST
            return
        }
    
       #  Presumably the most common case is "FRAGMENTS"
    
       if {$msgType == $EVB::FRAGMENTS} {
    
            
            # protocol allows FRAGMENTS here:
            # TODO: Handle errors as a close
            
            # We can read the fragments from the socket:
            
            set fragments [read $socket $bodySize]
            
            # Acknowledging the fragments here allows next bunch to be prepared
            # in the caller.
            
            if {[catch {puts $socket "OK"} msg]} {
                puts stderr "Event orderer failed to ack ok to a data source $msg"
                $self  _Close LOST
            } else {
                flush $socket
            }

            EVB::handleFragments $socket $fragments;    # Handle fragments in C++
    
            $callbacks invoke -fragmentcommand [list] [list]
    
            
        # Protocol allows a DISCONNECT here as well as fragments.. note disconnect
        # has no body data.:

        } elseif {$msgType == $EVB::DISCONNECT} {

            # There is no body in disconnect messages.
            
            puts $socket "OK"
            flush $socket
            $self _Close CLOSED
    
        }  else {
            # Anything else is a crime against The Protocol:
    
    
            puts $socket "ERROR {Unexpected header: $header}"
    
        }
    }

}

##
#  Connection manager object.  Creating one of these
#  creates a connection manager on the specified port
#  which will create Connection objects each time a connection arrives.
#  and will destroy them as they die.
#
# OPTIONS
#                the port manager.
#   -port - port on which we are listening for connections.
#   -connectcommand - script to call when a connection has been added.
#                    Substitutions:
#                     %H - Host that is connecting with us.
#                     %O - Connection object created to manage this connection.
#   -disconnectcommand - Script to call when a connection has been lost.
#                     %O - Connection object created to manage this connection.
#                     %H - Host from which the client came.
#                     %D - Connection descdription.
#   -sourcetimeout - Number of seconds of not getting a fragment
#                    from a data source after which it is considered timed out.
#
#                     
# METHODS:
#  getConnections - List the connections and their states.
#
snit::type EVB::ConnectionManager {
    component TimeoutObservers

    option -port;		# Port on which we listen for connections.
    option -connectcommand   -default [list]  -configuremethod _SetCallback
    option -disconnectcommand -default [list] -configuremethod _SetCallback
    option -sourcetimeout    -default 60

    variable serverSocket;	           # Socket run by us.
    variable connections -array {};        # Key is connection, value last received timestamp.
    variable lastFragment;		   # When the last fragment arrived.
    variable callbacks;		           # Callback manager.
    variable timedoutSources [list];	   # list of connections that are timed out.
    variable accepting  1;                 # when zero we are flowed off.
   

    delegate method addObserver    to TimeoutObservers as addTimeoutObserver
    delegate method removeObserver to TimeoutObservers as removeTimeoutObserver


    constructor args {
        set result [catch {
        $self configurelist $args; # To get the port.
    
        set lastFragment [clock seconds]
    
        #
        # Set up the callback manager.
        #
        set callbacks [EVB::CallbackManager %AUTO%]
        $callbacks define -connectcommand
        $callbacks define -disconnectcommand
    
        set serverSocket [socket -server [mymethod _NewConnection] $options(-port)]
    
        # watch timeouts at 1/2 the timeout interval:
    
        after [$self _TimeoutCheckInterval] [mymethod _CheckSourceTimeouts]
    
        install TimeoutObservers using  Observer %AUTO%
            } msg]
            if {$result} {
                puts stderr "$msg\n $::errorInfo"
                exit
            }
            EVB::onflow add [mymethod _FlowOn] [mymethod _FlowOff]
    }
    destructor {
        foreach object [array names connections] {
            $object destroy
            unset connections($object)
        }
        close $serverSocket
        $callbacks destroy
        # $TimeoutObservers destroy
    }
    #-----------------------------------------------------------------
    # Public methods:
    #

    ##
    # Get the list of connections
    #
    # @return list of three element sublists (possibly empty).
    # @retval Each list element contains in order:
    #  - Client's host.
    #  - description of client.
    #  - Connection state string.
    #  - "yes" if the connection is in the timedout list
    #    "" if not.
    method getConnections {} {
	set result [list];	# possibly empty return list.
	foreach connection [array names connections] {
	    set host  [$connection cget -clientaddr]
	    set desc  [$connection cget -description]
	    set state [$connection cget -state]
	    set timedout ""
	    if {[lsearch -exact $timedoutSources $connection] != -1} {
		set timedout "yes"
	    }

	    lappend result [list $host $desc $state $timedout]
	}
	return $result
    }
    #-----------------------------------------------------------------
    #  Private methods.
    
    ##
    # _FlowOn
    #   Enable the acceptance of data from source.
    # @param sock - if not an empty string the specific socket
    #                to flow on.  If an empty string all are flowed on.
    #
    method _FlowOn {{sock ""}} {
        set accepting 1
        foreach connection [array names connections] {
            if {($sock eq "") || ([$connection cget -socket] eq $sock)} {

                $connection flowOn
            }
        }
    }
    ##
    # _FlowOff
    #   Disable the acceptance of data from sources.
    # @param sock - the specific socket to flow off.  ""
    #               means flow off all of them.
    method _FlowOff { {sock ""}} {
        set accepting 0
        foreach connection [array names connections] {
            if {($sock eq "") || ([$connection cget -socket] eq $sock)} {

                $connection flowOff
            }
        }
    }

    ##
    # Client disconnect is just removing it from the list of connections
    # and invoking the disconnect callback.  Once all that dust has settled,
    # the connection object is destroyed, and its socket closed.
    #
    #
    # @param object - connection object name.
    # @note  Since it should not be possible for an object to disconnect that
    #        is not in our list, we toss an error if we are handed one.
    #
    method _DisconnectClient object {

	if {[array names connections $object] ne ""} {

	    $callbacks invoke -disconnectcommand [list %O %H %D] \
		[list $object [$object cget -clientaddr] [$object cget -description]]
	    
	    # If the connection is stalled, remove it from the stalled list:

	    set stallIndex [lsearch -exact $timedoutSources $object]
	    if {$stallIndex != -1} {
		set timedoutSources [lreplace $timedoutSources $stallIndex $stallIndex]
	    }

	    unset connections($object)
	    $object destroy
	} else {
	    error "BUG - $object not in ConnectionManager known connections"
	}
    }


    ##
    #  React to client connections:
    #  - A new Connection object is created to interact witht the client.
    #  - The object is aded to the connections list.
    #  - The object's -disconnectcommand is configured to invoke
    #    _DisconnectClient
    #  - We invoke the -connectcommand callback.
    #
    #
    #  @param sock - socket connected to the client
    #  @param client - Client IP addrerss.
    #  @param cport - client port (we could care less).
    #
    method _NewConnection {sock client cport} {
        set connection [EVB::Connection %AUTO% -socket $sock  -clientaddr $client]
        set connections($connection) [clock seconds]; # connection's last frag time to be not-timedout.
        $connection configure -disconnectcommand [mymethod _DisconnectClient $connection]
        $connection configure -fragmentcommand   [mymethod _RecordFragment $connection]
        $callbacks invoke -connectcommand [list %H %O] [list $client $connection]
            if {!$accepting} {
                $connection flowOff;            # Event builder can't take more data.
            }
        }
        ## 
        # We very carefully ensured the callbacks registered with the callback manager
        # are the same as the options that set/get them.
        # this ensures that we can have a single configuremethod take care of both of them.
        # 
        # @param option - Option being set (callback name).
        # @param value  - New value for the option (the callback).
        #
        method _SetCallback {option value} {
            $callbacks register $option $value
            set options($option) $value
        }
        ##
        # _RecordFragment
        #
        #  Record the time at which the most recent fragment arrived and which 
        #  connection received it.
        #
        # @param connection - the connection that just got the fragment.
        #
        method _RecordFragment connection {
            set connections($connection) [clock seconds]
            set lastFragment [clock seconds]
        
            if {$connection in $timedoutSources} {
                EVB::reviveSocket [$connection cget -socket]
                set cindex [lsearch -exact $timedoutSources $connection]
                set timedoutSources [lreplace $timedoutSources $cindex $cindex]
            }
            # Now ensure none of the connections gets starved by calling their
            # tryRead -- this is because I think under heavy unbalanced load, the
            # event dispatcher may not be fair, only dispatching the first readable socket
            # even if other sockets are readable...and if that socket becomes readable...
            #
            foreach c [array names connections] {
                if {$c != $connection && ([$c isAlive])} {
                    $c tryRead
                }
        }
    }
    ##
    #  Check for lack of data on data sources;
    #  - If we've not had any data in a timeout period, nobody's timed out.
    #  - If we have had data within a timeout but a source has not contributed,
    #  - It's timed out.
    #
    method _CheckSourceTimeouts {} {

	set previouslyTimedOut $timedoutSources

	# Only care if we're getting fragments:

	set now [clock seconds]
	if {($now - $lastFragment) < $options(-sourcetimeout)} {
	    set timedoutSources [list]; # Assume it's all just peachy.
	    foreach connection [array names connections] {
		if {($now - $connections($connection)) > $options(-sourcetimeout)} {
		    lappend timedoutSources $connection
		}
	    }
	}

	# Invoke observers for each newly timed out source
	# Invocation is of the form:
	#   observer STALLED connection

	foreach source $timedoutSources {
	    if {$source ni $previouslyTimedOut} {
		catch {$TimeoutObservers invoke STALLED $source} msg
		EVB::deadsource [$source cget  -socket]; # Indicate source dead to fragment handler.
	    }
	}

	# invoke observers that are no longer  stalled:
	# Invocation is of the form.
	#    observer UNSTALLED $source
	#
	foreach source $previouslyTimedOut {
	    if {$source ni $timedoutSources} {
		catch {$TimeoutObservers invoke  UNSTALLED $source} msg
	    }
	}


	# Reschedule:

	after [$self _TimeoutCheckInterval] [mymethod _CheckSourceTimeouts]
    }
    ##
    # compute the timeout check interval:
    #
    # @return ms to next timeout check.
    #
    method _TimeoutCheckInterval {} {
	return [expr {int(1000*$options(-sourcetimeout)*2.0)}]
    }
}

