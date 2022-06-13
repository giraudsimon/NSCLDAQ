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
#   This file contains a port allocation class.
#   it operates with the NSCLDAQ port manager to
#   allocate tcp/ip server ports.
#   this is a snit class and therefore depends on
#   tcl8.4 and its associated tcllib.
#
#  To access the port manager we need to know a hostname
#  (in case the applicatin just wants to list the ports used in a
#  remote syste) and a port number.
#   The defaults are:
#          hostname:   localhost
#          port:       value in /var/tmp/daqportmgr/listen.port
#                      if that file exists, else 30000
#  Both of these can be set by the user as construction options
#   e.g. portAllocator create thechad -hostname thechad.nscl.msu.edu -port 1234
#  If a port is allocated, we transparently hold the connection to the server
#  so that the port is held by us.  The connection is not closed if the
#  object is destroyed under the assumption that the application will
#  listen on a service port for the lifetime of the application.

package provide portAllocator 1.0
package require snit

::snit::type portAllocator {
    option   -hostname "localhost"
    option   -port      30000
    variable PortFile   "/var/tmp/daqportmgr/listen.port"
    
    #  Connected is true if we are holding a persistent connection
    #  to the server because we allocated a port.
    #  in that case Socket is the fd open on the socket.
    
    variable Connected 0
    variable Socket
    
    constructor {args} {
        $self configurelist $args
        
        # Use the port-file port value if the user does not
        # provide a -port and the file is readable.
        
        if {[file readable $PortFile] && ([lsearch -exact $args -port] == -1)} {
            set fd [open $PortFile r]
            set $options(-port) [gets $fd]
            close $fd
        }
    }
    #listPorts:
    #  List the ports allocated by the server.
    #  If the socket is connected we use it otherwise
    #  we open a new connection to the server and then
    #  close it when done.
    #    If we are not able to open the socket, the method
    #  delegates error processing to the caller.
    #
    # Returns:
    #  A list, one element per port allocation.  Each element of
    #  the list is a 3 element list that contains:
    #    port application_name user_name
    #  in that order.
    #
    method listPorts {} {
        $self OpenServer
        puts $Socket "LIST"
        flush $Socket
        
        set line1 [gets $Socket]
        
        # line1, should be OK nports
        
        set status    [lindex $line1 0]
        set portcount [lindex $line1 1]
        if {$status != "OK"} {
            $self CloseServer
            error "Status of LIST was not OK: $line1"
        }
        set portlist "";         # portcount could be 0!!
        for {set i 0} {$i < $portcount} {incr i} {
            lappend portlist [gets $Socket]
        }
        
        $self CloseServer
        return $portlist
    }
    ##
    # findServer
    #   Locates a server given the service name and username.
    # @param service - The service we are hunting for.
    # @param user    - Name of the user providing the service, defaults to
    #                  the logged in user.
    # @return mixed  - An integer port number if the service is found or "" if
    #                  not.
    #
    method findServer {service {user {}}} {
        if {$user eq ""} {
            set user $::tcl_platform(user)
        }
        
        # Get all the services that match our servic name
        #  Then hunt for one that matches our desired username.
            
        
        set servers [$self findServerAllUsers $service]
        foreach server $servers {
            set u [lindex $server 0]
            if {$u eq $user} {
                return [lindex $server 1]
            }
        }
        return ""
    }
    ##
    # findServerAllUsers
    #
    #   Locates services that match the requested service name regardless
    #   of the user.  This is needed in order to handle locating
    #   daq servers (e.g. ringmaster) in both test and production environments
    #   in test environments, the ring master might have been run by a developer
    #   while in production it will, in general be run by root.
    #
    #  @param service -name of the service.
    #  @return list of pairs.  The first item in each pair is the user.
    #          the second the port.
    method findServerAllUsers {service} {

        set allocations [$self listPorts]
        set port ""
        set result [list]
        foreach server $allocations {
            set s [lindex $server 1]
            set u [lindex $server 2]
            if {$s eq $service} {
                lappend result [list $u [lindex $server 0]]
            }
        }
        return $result
    }
        
    
    #
    # allocatePort:
    #    Asks the server to allocate a port to the specified
    #    application.
    # Parameters:
    #   application (string):
    #       The name of the application to which the port will be allocated.
    #       Note that the application will be qualified by the user running
    #       the application.
    # Returns:
    #   On success: the number of the port allocated.
    # Errors:
    #   FAIL reason
    #       If the server failed the request.  At present this can be either
    #       because there are no ports available or because the server we're
    #       talking to is not local.
    #   other:
    #       Connection failed.
    #       I/O to server failed due to server disconnect e.g.
    # Side effects:
    #   If the port was successfully allocated, the connection to the
    #   server will remain open and be re-used by other allocatePort and
    #   listPorts services.  This is required by the protocol to retain
    #   port ownership. Once our connection to the server is dropped,
    #   our port will be released for re-use.
    #
    method allocatePort {application} {
	global tcl_platform

        $self OpenServer
        
        # Request a port and get the server reply.
        
        puts  $Socket "GIMME $application $tcl_platform(user)"
        flush $Socket
        set line [gets $Socket]
        
        set status [lindex $line 0]
        if {$status != "OK"} {
            $self CloseServer
            error $line
        }
        
        #  We got a port need to hold on to the connection:
        
        set Connected 1
        
        return [lindex $line 1]
    }
    
    # OpenServer:
    #     utility to open a connection to the server.
    #     If the server is alreay open, this is a no-op.
    # Side Effects:
    #   Socket will contain a socket open on the server.
    method OpenServer {} {
       if {!$Connected} {
            set Socket [socket $options(-hostname) $options(-port)]
            fconfigure $Socket -buffering line
        }
    }
    # CloseServer:
    #   utility to indicate the caller no longer needs a connection
    #   to the server.  If Connected is true, this is a no-op.
    #
    method CloseServer {} {
        if {!$Connected} {
            close $Socket
        }
    }
}

