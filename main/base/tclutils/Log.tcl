#
#   This snit class supports the server log functionality
#   of the Port Manager.
#     We create a snit type with public methods:
#        logConnection        - Log server connection requests.
#        logRelease           - Log port releases
#        logAllocationFailure - Log failures to allocate a port.
#        logInvalidRequest    - Log invalid server requests.
#
#

package provide Log 0.1
package require snit
package require log

snit::type Log {
    option     -filename /var/log/portmanager.log;  # The file logged to.
    variable   logfd ""
    
    # Construction: Open the log file and setup the log facility.
    
    constructor {args} {
        $self configurelist $args
        
        #  Open the log file ... if it cannot be opened,
        #  use stderr as the error channel:
        #

        if {[catch {open $options(-filename) a+} logfd]} {
            set name [file tail $options(-filename)]
            set name [file join ~ $name]
            set logfd [open $name a+]
        }
        ::log::lvChannelForall $logfd
        #
        #  Enable all log levels
        #
        foreach level [::log::levels] {
            ::log::lvSuppress $level 0
        }
        
    }
    
    # Destruction: Close the log file.
    
    destructor {
        if {$logfd != "stderr"} {
            close $logfd
        }
    }
    #----------------- public methods ------------------------
    #
    # logAllocation
    #    Log the allocation of a port.
    #
    #  Parameters:
    #    port  (int in):
    #       The port allocated.
    #    application (string in)
    #       The name of the application that was given the port.
    #    user (string in)
    #       The name of the user that is running the application.
    #
    method logAllocation {port application user} {
        set message [timestamp]
        append message " Port $port was allocated to $application run by $user"
        ::log::log info $message
        flush $logfd
    }
    # logConnection:
    #     Log a connection to the server.
    # Parameters:
    #    host (string in)
    #      Host that connected to us.  This could be either an ip or a
    #      DNS name depending on whether or not it could be translated.
    #
    method logConnection {host} {
        set message [timestamp]
        append message " Connection accepted from $host"
        ::log::log info $message
        flush $logfd
    }
    # logRelease
    #    Log the release of a port by a client.
    # Parameters:
    #   port (int in):
    #       The port that was released.
    #   application (string in)
    #       The application that had been granted the port.
    #   user (string in)
    #       The user that was running the application
    #
    method logRelease {port application user} {
        set message [timestamp]
        append message " Port $port was released by $application run by $user"
        ::log::log info $message
        flush $logfd
    }
    # logAllocationFailure
    #   Logs failure to allocate a port.
    # Parameters:
    #   application (string in)
    #       The name of the application that was trying to get a port.
    #   user (string in)
    #       The user that was running the application.
    #  reason (string in)
    #      The reason the allocation failed.
    #NOTE:
    #   We will create an alert message.
    #
    method logAllocationFailure {application user reason} {
        set message [timestamp]
        append message " Unable to allocate a port for $application run by $user because: $reason"
        ::log::log alert $message
        flush $logfd
    }
    # logInvalidRequest
    #   Log an invalid request by a client.  This is an alert level as streams of
    #   invalid requests may indicate a denial of service attack e.g.
    #
    # Parameters:
    #   client  (string in)
    #       IP address or name of client if it can be gotten.
    #   request (string in)
    #       The request that is invalid.
    #
    method logInvalidRequest {client request} {
        set message [timestamp]
        append message " $client issued an illegal request: $request"
        ::log::log alert $message
        flush $logfd
    }
    #----------------------------------------------------------
    #
    #  Local function to return a timestamp of the form:
    # mon dd, yyyy hostname
    #
    proc timestamp {} {
        set now [clock seconds]
        set message [clock format $now -format "%B %d, %Y %T "]
        append message [info hostname]
        
        return $message
    }
}
