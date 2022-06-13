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



##
# This is the event orderer main object.
# The event orderer is a Tcl package that the user incorporates into their own script
# via:
#   package require EventBuilder
#
# The package contains the top level event builder snit::type and
# code to properly construct and start it.
# Also provided are EVB namespace procs that allow the user
# to establish an appropriate set of callbacks.
#

package provide EventBuilder 1.0
#package require Tk
package require snit
package require EVB::ConnectionManager
package require portAllocator
package require InstallRoot

set DEBUG 0

namespace eval EVB {
    variable eventBuilder
}
##
#  The event builder basically ties everything together in to 
#  a single type that represents the event building application.
#
#  -appsuffix - If non empty this is part of the name registered with the connection
#               manager.
#
snit::type EVB::EventBuilder {
    component connectionManager

    option -appsuffix -default ""

    delegate option -connectcommand    to connectionManager
    delegate option -disconnectcommand to connectionManager
    delegate option -sourcetimeout     to connectionManager
    delegate option -port              to connectionManager
    

    delegate method getConnections to connectionManager

    constructor args {
	$self configurelist $args
	install  connectionManager using  EVB::ConnectionManager %AUTO% -port [$self _GetServerPort]
    }

    #--------------------------------------------------------------------
    # Private methods:
    #

    ##
    # Return the port manager's server port.  This is gotten from the
    # nscldaq port manager.
    #
    # @return int - port number.
    method _GetServerPort {} {
	set appName ORDERER:$::tcl_platform(user)
	if {$options(-appsuffix) ne ""} {
	    append appName : $options(-appsuffix)
	}

	set pa [portAllocator create %AUTO%]

	#
	#  If there's already an event builder registered on this port,
	#  Give the user a chance to not start this one:
	#


	set existing [$pa findServerAllUsers $appName]
	
	if {[llength $existing] > 0} {
	    set reply [tk_messageBox -type yesno -title {Duplicate event builder} \
			   -message "An event orderer named $appName already exists are you sure you want to start?"
		      ]
	    if {$reply eq "no"} {
		exit -1
	    }
	}
	
	set port [$pa allocatePort $appName]
	$pa destroy
	puts stderr "Got port $port for orderer "

	return $port
    }
    
}

#------------------------------------------------------------------------

#  Unbound procs that provide a proecedural interface to the event builder.
#
#

##
#  Creates and starts the event builder.
# @para appsuffix - IF nonempty this is the application suffix passed to the event builder.
#
proc EVB::Start {{appsuffix {}}} {
    if {$appsuffix ne ""} {
	set EVB::eventBuilder [EVB::EventBuilder %AUTO% -appsuffix $appsuffix]
    } else {
	set EVB::eventBuilder [EVB::EventBuilder %AUTO%]
    }
}
##
# Set the connection callback.  The
# connection callback is invoked every time a new client connects to the
# event builder.  The command passed in has the host and description
# string appended to it when called.
#
# @param script - The script to invoke.
#
proc EVB::setConnectionCallback script {
    $EVB::eventBuilder configure -connectcommand [list $script %H %D]
}
##
# Set the disconnect callback.  This is called whenever a data source
# client drops the connection with us.
# The script will have the host and description appended to it.
#
# @param script - the script to invoke.
#
proc EVB::setDisconnectCallback script {
    $EVB::eventBuilder configure -disconnectcommand [list $script %H %D]
}
##
# Get the set of connections. The connection is returned as a list of three
# element sublists.  The elements of each sublist are in-turn,
# - The host of the client.
# - The client description
# - The state of the client connection.
#
#
proc EVB::getConnections {} {
    $EVB::eventBuilder getConnections
}


##
# EVB::setSourceTimeout
#
# Set the source timeout.
#
# @param timeout - seconds of timeout.
#
proc EVB::setSourceTimeout timeout {
    $EVB::eventBuilder configure -sourcetimeout $timeout
}
##
# EVB::getSourceTimeout
#
# Get the source timeout.
#
# @return int timeout
#
proc EVB::getSourceTimeout {} {
    $EVB::eventBuilder cget -sourcetimeout 

}


##
# EVB::_NotifyEOF
#
#  Called when a data source pipeline becomes readable.
#  If an EOF is seen
#  - the fd is closed,
#  - an error dialog is popped up via tk_messageBox
#
# @param fd - The file descriptor open on the pipe.
# @param who - Identifies who is running.
#
proc EVB::_NotifyEOF {fd who} {
    if {[eof $fd]} {
	close $fd
	tk_messageBox -icon error -title "Pipe EOF" -message "$who exited" -type ok
    } else {
	if ($::DEBUG) {
	    puts stderr [read $fd];	# debugging.
	} else {
	    read $fd
	}
    }
}
##
# EVB::startRingFragmentSource
#
#  Starts a fragment source that gets data from a ring buffer.
#
#  @param ringUrl   - The URL of the ring from which data will be gotten.
#  @param extractor - The shared object that extracts timestamps from the
#                     event fragments that come from the ring.
#                     (this will pass through "file normalize"
#                      before being passed to the --timestampextractor
#                      switch of the ringFragmentSource program
#  @param id        - Id of the source this represents (should be a number
#                     unique across all data sources.  The uniqueness is
#                     not enforced at this time.
# @param desc       - Description of the data source.
# @param exitmsg    - Becomes part of the exit message if the data source
#                     goes away.
#
#
# @note - If the fragment source exits (EOF on its stdout), an error dialog
#         is emitted.
# @note - any data that comes on stdout/stderr is ignored.
# @note - See http://wiki.tcl.tk/1241 for the trick used to capture both
#         stdout and stderr.
# @note - It is not up to us to set up the ring's producer.
#
proc EVB::startRingFragmentSource {ringUrl extractor id desc exitmsg} {
    set port [$EVB::eventBuilder cget -port]

    # Figure out where the timstamp extractor is

    set top [InstallRoot::Where]; # Top of installation.
    set extractor [file normalize $extractor]

    # Create the ringfragment source command:

    set fragsrcExe [file join $top bin ringFragmentSource]

    set desc [regsub -all  " " $desc {\ }]
    set fragsrc "$fragsrcExe --evbhost=localhost --evbport=$port --ids=$id --info=$desc --ring=$ringUrl \
       --timestampextractor=$extractor"


    # Start the command pipeline (here's where the cited wikipage comes in handy)


    set fd [open "| $fragsrc |& cat" "r"]
    fconfigure $fd -blocking 0
    fileevent  $fd readable [list EVB::_NotifyEOF  $fd $exitmsg]
    
}

##
# EVB::startS800FragmentSource
#
#  Starts the S800 fragment data source.
#
# @param ringUrl - the URL of the s800 ring into which the s800 data are being 
#                  sucked from the S800 readout system.
#
# @param id      - Id of the data source.
# @param desc    - client description, defaults to "S800 USB data"
#
# @note - It is not up to us to set up the feed to the ring from the s800!!!
#
proc EVB::startS800FragmentSource {ringUrl id {desc {S800 USB data}}} {
    set top [InstallRoot::Where]; # Top of installation.
    set extractor [file join $top lib libS800TimeExtractor.so]

    EVB::startRingFragmentSource $ringUrl $extractor  $id $desc "S800 Fragment source"


}

##
# EVB::startGretinaFragmentSource
#
#  Starst a Gretina fragment data source from the ring into which the gretina
#  tap pipeline puts data.  Doing it this way ensures that I don't have to re-start
#  the source when Gretina runs end.
#
# @param ringurl - URL of the Gretina ring.  It is up to other people
#                  (e.g. ReadoutGUI) to ensure the producer for this ring is started.
# @param id      - Source id of the Gretina ring.
# @param desc    - client description, defaults to "GRETINA GEB data"
#
proc EVB::startGretinaFragmentSource {ringurl id {desc {GRETINA GEB data}}} {
    #
    # Locate the timestamp extractor:

    set top [InstallRoot::Where]
    set extractor [file join $top lib libGretinaTimeExtractor.so]

    EVB::startRingFragmentSource $ringurl $extractor $id $desc "GRETINA Fragment source"
}
