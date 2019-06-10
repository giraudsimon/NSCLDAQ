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
#      NSCL
#      Michigan State University
#      East Lansing, MI 48824-1321
#

# This package provides a client to access the 
# readout's port for control operations.
#

package provide usbcontrolclient   1.0

package require snit
package require portAllocator

#
#  Snit type to connect with a controls client.
#  Construction implies connection, construction errors will
#  Typically imply that the server is not available.
#   
# Options and defaults:
#    -server  localhost    - The host to connect to.
#    -port    27000        - The connection port.
#  NOTE:  
#       All options can only be provided at construction time.. after that
#       they have no effect.
# Methods:
#    Get    device channel
#    Set    device channel value
#    Update device
#
#
snit::type controlClient {
  variable connection -1

  option    -server   localhost
  option    -port     27000

  constructor args {
    $self configurelist $args

    if  {[catch {socket $options(-server) $options(-port)} connection]} {
      error $connection
    }
    fconfigure $connection -buffering line

  }
  destructor {
    if {[catch {close $connection} msg]} {  # in case already closed catch.
      error "Error destroying $self : $connection :  $msg"
    }

  }

  # Get device channel
  #      Attempts to get the value of a channel within a device.
  #      The model is that there may be many devices, each with serveral
  #      channels of data (e.g. a power supply may have a voltage set point,
  #      voltage value, current limit interlock state etc.
  #  Returns:
  #      Success - The result from the server  Note that returns that
  #                begin ERROR mean a server detected error.
  #      Failure - Generally because the connection was dropped by the
  #                server e.g. Readout exited.
  #
  method Get {device channel} {
    puts $connection "Get $device $channel"
    set response [gets $connection]
    if {[eof $connection]} {
      return -code error "USB control client has lost connection to server"
    }
    return $response

  }
  # Set device channel value
  #      Attempts to set the value of a channel within a device.
  # Returns:
  #     Success - The result from the server.  Note that returns
  #               beginning with "ERROR" are errors detected by the server.
  #     Failure - Generally because the connection was dropped by the server.
  #
  method Set {device channel value} {
    puts $connection "Set $device $channel $value"
    set response [gets $connection]
    if {[eof $connection]} {
      return -code error "USB control client has lost connection to server"
    }
    return $response
  }
  # Update device
  #     Force the device settings/readings to be udpated.
  #
  method Update device {
    puts $connection "Update $device"
    set response [gets $connection]
    if {[eof $connection]} {
      return -code error "USB control client has lost connection to server"
    }
    return $response
  }
  # Get monitored data.
  #
  method Monitor device {
    puts $connection "mon $device"
    set response [gets $connection]
    if {[eof $connection]} {
      return -code error "USB control client has lost connection to server"
    }
    return $response
  }
}
#  The following are convenience functions that may be used
#  by clients.  These are intended to be used within tk.
#  and make nice graphical error messages if the channel got dropped.
#

#  connect
#     Connects to a server in local host.  Returns the connection
#     object or "" if "" the connection could not be established.
#     in that case, a modal information dialog has been popped up
#     for the convenience of the user.
#
proc connect {} {
  if {[catch {controlClient %AUTO%} client]} {
    tk_messageBox  -icon error -title {No Server}    \
      -message {Unable to connect to control server be sure Readout is runing}
    return ""
  } else {
    return $client
  }
}
#
#  controlOp object ...
#     Performs an arbitrary control operation.
#     If the operation succeeds, the server return value is returned.
#     If the operation fails, a nice dialog is popped up and "" is returned.
#
proc controlOp {object args} {
  if {[catch {eval $object $args} msg]} {
    tk_messageBox -icon error -title {Control op failed} \
      -message "Unable to perform a [lindex $args 0] : $msg"
    return ""
  } elseif {$msg eq ""} {
    tk_messageBox -icon error -title {Control op failed} \
      -message "Unable to perform a [lindex $args 0] : Connection dropped."
    return ""
  } else {
    return $msg
  }
}
##
# Lists the control servers in a specific system.
#
# @param host (could be localhost of course)
# @param prefix (VMUSBReadout or CCUSBReadout)
#
# @return list of pairs.  The first element of each pair is the stuff after
#         the colon in the advertised service (serial number or FirstController
#         for direct connections or the server host for remotely served controllers).
#         The second element in the list is the port number.
#         Only the servers run by the current user are listed.
#
proc listUSBControlServers {host prefix} {
  set manager [portAllocator %AUTO -hostname $host]
  set servers [$manager listPorts]
  $manager destroy

  set controlServers [list]
  set me       $::tcl_platform(user)

  foreach server $servers {
    set  port [lindex $server 0]
    set  app  [lindex $server 1]
    set  user [lindex $server 2]

    # Split the app up into the appName (e.g. VMUSBReadout) and the
    # thing being served (e.g. VM055).

    set appList [split $app :]
    set appName [lindex $appList 0]
    set device  [lindex $appList 1]

    #  It gets added to the list only if the user is me and the
    #  appname matches the prefix:

    if {($user eq $me) && ($appName eq $prefix)} {
      lappend controlServers [list $device $port]
    }
  }
  return $controlServers
}

##
# @class slowControlsPrompter
#
# Snit megawidget that prompts for a control server.  The
# widget has the following format:
#
#  +-----------------------------------------------+
#  | +----------------------------+^|              |
#  | | list box of servers        | |              |
#  | +----------------------------+V|              |
#  | <hostname entry>   <port entry>               |
#  |                                               |
#  +-----------------------------------------------+
#  | [ Ok ]  [Update] [Default Port]  [Cancel]     |
#  +-----------------------------------------------+
#                                              
#  When started, the hostname is localhost and the port entry contains 27000
#  and the  list box, contains a list of advertised servers if any on localhost
#  The following buttons have autonomous action:
#
#  Update - Updates the list box of servers from the host in the hostname
#  entry (e.g. enter spdaq44 in hostname and click update to get a list of
#  advertised servers).
#  Default port -Loads 27000 (the default server port) into the port entry.
#
#  In addition, doubleclicking an item in the list box of servers, loads it into
#  the hostname/port entry (both the host name and the port are updated as the
#  user may have typed something into host without clicking update and then
#  double clicked a server.)
#
# OPTIONS:
#   -defaultport   - The default port number.
#   -host          - Sets/gets the hostname entry field.
#   -port          - Sets/gets the port entry field.
#   -okcmd         - Script executed on the OK button click.
#   -cancelcmd     - Script executed on the Cancel command click.
#   -type          - Type of service..shouild be VMUSBReadout or CCUSBReadout.
#
# METHODS:
#    update        - Forces an update of the listbox from the current values
#                    of -host.  Note this is done automatically by the constructor
#                    after configuration options have been processed.
#
# SUBSTITUTIONS:
#   %W           - The widget
#
# IMPLEMENTATION NOTE:
#   * We use a widget adaptor to allow the hull to be a ttk::frame
#   * where they exist we use ttk widgets.
#   * Methods whose names begin with _ should be considered private.
#     All other methods are public.
#
snit::widgetadaptor slowControlsPrompter {
  component form
  component command

  option -defaultport 27000
  option -host        localhost
  option -port        27000
  option -okcmd       [list]
  option -cancelcmd   [list]
  option -type        VMUSBReadout

  variable services   [list];        # services in the list box.
  variable listedHost [list];        # host listed in the list box.

  ##
  # constructor
  #   * Layout the widget.
  #   * establish listbox bindings (double click).
  #   * process the command options.
  #   * stock the list box.
  #
  # @param args - configuration args (e.g. -port 1234).
  #
  constructor args {

    installhull using ttk::frame

    # Layout the widget in two subframes:
    # form - is the top part with the form elements.
    # command - is the bottom with the buttons.
    #
    install form using    ttk::frame $win.form
    install command using ttk::frame $win.command

    # The form has for 4 columns, the list box spans 3 columns its scroll
    # bar lives in the 4'th the entries and their labels each live in
    # a single column.

    listbox $form.list -height 8 -selectmode single \
      -yscrollcommand [list $form.scroll set]
    ttk::scrollbar $form.scroll -orient vertical \
      -command [list $form.list yview]

    ttk::label $form.hostl  -text Hostname:
    ttk::entry $form.host   -textvariable ${selfns}::options(-host) \
      -width 10
    ttk::label $form.portl  -text Port:
    ttk::entry $form.port   -textvariable ${selfns}::options(-port) \
      -width 10

      # Layout the form.

    grid $form.list -row 0 -column 0 -columnspan 3 -sticky nsew
    grid $form.scroll -row 0 -column 3 -sticky nsw

    grid $form.hostl -row 1 -column 0 -sticky ew
    grid $form.portl -row 1 -column 1 -sticky ew

    grid $form.host -row 2 -column 0 -sticky ew
    grid $form.port -row 2 -column 1 -sticky ew

    grid rowconfigure    $form {0 1 2}   -weight 1
    grid columnconfigure $form {0 1 2 3} -weight 1

    # The command widget has 4 colums one with each button as shown
    # in the layout figure:

    # Layout the command form.

    button $command.ok      -text Ok     -command [mymethod _Dispatch -okcmd]
    button $command.update  -text Update -command [mymethod update]
    button $command.default -text {Default port} \
      -command [mymethod _DefaultPort]
    button $command.cancel  -text Cancel -command [mymethod _Dispatch -cancelcmd]


    grid $command.ok $command.update $command.default $command.cancel -sticky ew
    grid rowconfigure    $command {0}       -weight 1
    grid columnconfigure $command {0 1 2 3} -weight 1

    # Layout the entire widget

    grid $form      -sticky ew
    grid $command   -sticky ew
    grid columnconfigure $win 0 -weight 1
    grid rowconfigure    $win {0 1} -weight 1

    # Add a double-1 binding to the listbox to _SelectServer.
    # which loads -port and -host with the characteristics
    # of the service selected.

    bind $form.list <Double-1> [mymethod _SelectServer]

    # Process the configuration optionsL

    $self configurelist $args

    # Stock the listbox:

    $self update
  }
  #--------------------------------------------------------------------------
  #
  #  Public methods:

  ##
  # update
  #
  #   Updates the list box with the set of servers (if any) in the
  #   the current -host value.  Note that if a connection cannot be established
  #   with the host; the list box is unmodified and an error dialog
  #   is autonomously displayed.
  #
  method update {} {
    set host $options(-host)

    # The catch block detects an inability to talk to the port manager:

    if {[catch {listUSBControlServers $host $options(-type)} ports]} {
      tk_messageBox -icon error -title "Can't list ports" -parent $win \
        -message "Unable to get ports for $host : $ports"
      return
    }

    #  Ok we have a list ... could be empty but that's just how it is.
    #  Set our internal variables:

    set services $ports
    set listedHost $host

    $form.list delete 0 end
    foreach server $services {
      $form.list insert end [lindex $server 0]
    }
  }
  #-------------------------------------------------------------------------
  #
  #  Private methods:

  ##
  # _Dispatch
  #
  #  Dispatches to a user callback stored in an option after applying all
  #  the command substitutions.
  #
  # @param option -name of the option that holds the script.
  #
  method _Dispatch option {
    set command $options($option)
    if {$command ne ""} {
      set command [$self _Substitute $command]
      uplevel #0 $command
    }
  }
  ##
  # _DefaultPort
  #
  #   Sets the port back to the default port value.
  #
  method _DefaultPort {} {
    set options(-port) $options(-defaultport)
  }
  ##
  # _SelectServer
  #
  #  Double click handler in the list box.  Sets -host and -port to the
  #  selected item in the list box.
  #
  method _SelectServer {} {
  #
  #  Figure out which one is selected:
  #
    set itemNumber [$form.list curselection]
    if {$itemNumber ne ""} {
      set info [lindex $services $itemNumber]
      set port [lindex $info 1]
      set options(-host) $listedHost
      set options(-port) $port
    }
  }
  ##
  # _Subsitute
  #
  # Perform any script substitutions that are done for dispatches.
  # See SUBSTITUTINOS in the class comments for descriptions of what is
  # substituted for what.
  #
  # @param cmd - Input command.
  #
  # @return string - cmd with the substitutions applied.
  #
  method _Substitute cmd {
    return [string map [list %W $win] $cmd]
  } 
}
