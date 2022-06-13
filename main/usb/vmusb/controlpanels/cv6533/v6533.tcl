#
#   This snit type manages a V6533 over an
#   existing socket connected to the 
#   server.
#  Methods:
#   monitor     - Return monitored data.
#   setpoint    - Set the requested voltage.
#   setilimit   - Set the current limit.
#   on          - Set channel on 
#   off         - Set channel off
#   setTripTime - Set trip condition time.
#   setVmax     - Set channel software voltage limit.
#   setRupRate  - Set the channel ramp up rate.
#   setRdnRate  - Set the channel ramp down rate.
#   setOffMode  - Set the power off mode.
#   getGlobalVmax - Get the global max voltage.
#   getGlobalImax - Get global current limit.
#   getSetpoints - get all voltage setpoints.
#   getIlimit    - Get all current limits.
#   getVact       - Get actual voltage.
#   getIact       - Get actual currents.
#   getStatus     - Get channel statuses.
#   getTripTimes   - get channel trip times.
#   getSoftVlimit  - Get channel soft v limits.
#   getRdnRate     - Get ramp down rate.
#   getRupRate     - Get the ramp up rate.
#   getOffMode     - Get power down mode.
#   getPolarities  - Get channel polarities.
#   getTemps       - Get channel temperatures.
#   getOnRequest   - Return whether the channel is requested
#                    to be on.
#
#  Note that monitor is the preferred way to get
#  information about a channel as it will not interrupt
#  data taking in progres..
package provide v6533Driver 1.0
package require snit

snit::type v6533 {
    option -socket;		# socket <--> tclserver.
    option -name;		# Name of device.

    constructor args {
	$self configurelist $args
    }
    #---------------------------------------
    #  Public methods:
    #

    # Fetch the monitored data from the
    # server. 
    # Returns:
    #   Result of the mon command applied to
    #   this device:
    #
    method monitor {} {
	set msg [$self subName "mon %N"]
	return [$self sendMessage $msg]
    }
    # Set the requested setpoint value
    # Parameters:
    #   channel   - Number of the channel to use.
    #   value     - New setpoint value.
    # Returns:
    #   result from the server.
    #
    method setpoint {channel value} {
	set msg [$self subName "Set %N v$channel $value"]
	return [$self sendMessage $msg]
    }
    # Set the current limit for a channel.
    # Parameters:
    #  channel   - Number of the channel to use.
    #  value     - New current limit value.
    # Returns:
    #   result from the server.
    #
    method setIlimit {channel value} {
	set msg [$self subName "Set %N i$channel $value"]
	return [$self sendMessage $msg]
    }
    # Turn a channel on. The channel will ramp up
    # to the set point at the ramp up speed set for
    # the channel.
    # Parameters:
    #  channel  - The channel to turn on.
    # Returns:
    #   result from the server.
    # Note:
    #  Turning on a channel that is already on has no
    #  effect and is not treated as an error.
    #
    method on channel {
	set msg [$self subName "Set %N on$channel yes"]
	return [$self sendMessage $msg]
    }
    # Turn a channel off.  Call sequence is the same
    # as for on however the channel is either set
    # immediately to 0V or ramped down at the 
    # ramp down rate depending on the off mode.
    #
    # Note: 
    #  Turning an off channel off has no effect and is not
    #  an error.
    
    method off channel {
	set msg [$self subName "Set %N on$channel no"]
	return [$self sendMessage $msg]
    }
    # Set the time a channel can be out of
    # specification before it trips off
    # Parameters:
    #   channel - Number of channel to set
    #   value   - New trip time value.
    # Returns:
    #   result string from server.
    #
    method setTripTime {channel value} {
	set msg [$self subName "Set %N ttrip$channel $value"]
	return [$self sendMessage $msg]
    }
    # Set the software voltage limit on a channel.
    # If the setpoint for a channel is set above this
    # value, it is limited to this value.
    # Parameters:
    #   channel  - The channel to set.
    #   value    - The new value for the voltage limit
    # Returns:
    #  result message from the server.
    #
    method setVmax {channel value} {
	set msg [$self subName "Set %N svmax$channel $value"]
	return [$self sendMessage $msg]
    }
    # Set the ramp up rate for a channel.  This determines
    # the rate at which the actual voltage is ramped to
    # the setpoint when a channel is initially turned on.
    # Parameters:
    #   channel   - The channel to set.
    #   value     - New ramp up rate value.
    # 
    method setRupRate {channel value} {
	set msg [$self subName "Set %N rup$channel $value"]
	return [$self sendMessage $msg]
    }
    # Set the ramp down rate for a channel.
    # If the turn off mode (See setOffMode) is in ramp,
    # this determines the rate at which the channel voltage
    # ramps to zero when the channel is turned off
    # Q? Does a trip ramp or just always shut off?
    # Parameters:
    #   channel  - Channel to set.
    #   value    - New ramp down rate.
    #
    method setRdnRate {channel value} {
	set msg [$self subName "Set %N rdown$channel $value"]
	return [$self sendMessage $msg]
    }
    # Set the way in which a channel turns off.
    # Parameters:
    #   channel - the channel to modify.
    #   value   - New value "kill" or "ramp" for
    #             the power off mode.
    # Returns:
    #   result string from the server.
    #
    method setOffMode {channel value} {
	set msg [$self subName "Set %N pdownmode$channel $value"]
	return [$self sendMessage  $msg]
    }
    #
    #  Return the global maximum voltage for
    # the device channels:
    # 
    # Returns:  
    #   The result from the seruver
    #
    method getGlobalVmax {} {
	set msg [$self subName "Get %N globalmaxv"]
	return [$self sendMessage $msg]
    }
    # Return the global current maximum
    # Returns:
    #   The result from the server:
    #
    method getGlobalImax {} {
	set msg [$self subName "Get %N globalmaxI"]
	return [$self sendMessage $msg]
    }
    #  Get the channel voltage set points.
    #  This method gets all of them.
    # Returns:
    #   Result from the server.
    #
    method getSetpoints {} {
	set msg [$self subName "Get %N v"]
	return [$self sendMessage $msg]
    }
    # Get the channel current limits.
    #
    method getIlimit {} {
	set msg [$self subName "Get %N i"]
	return  [$self sendMessage $msg]
    }
    # Get the actual voltages from each channel
    # Return:
    #   Result from server.

    method getVact {} {
	set msg [$self subName "Get %N vact"]
	return [$self sendMessage $msg]
    }
    #
    #  Get the current draw of each of the channels.
    #
    method getIact {} {
	set msg [$self subName "Get %N iact"]
	return [$self sendMessage $msg]
    }
    #  Get the channel status word
    # Returns:
    #   response from the server.
    #
    method getStatus {} {
	set msg [$self subName "Get %N status"]
	return  [$self sendMessage $msg]
    }
    # Get the channel trip times.
    # Returns:
    #   Response from the server.
    #
    method getTripTimes {} {
	set msg [$self subName "Get %N ttrip"]
	return  [$self sendMessage $msg]
    }
    # Get the Soft voltage limits for each channel.
    # Returns
    #   Response from the server.
    #
    method getSoftVlimit {} {
	set msg [$self subName "Get %N svmax"]
	return  [$self sendMessage $msg]
    }
    # Get the ramp down rate for each channel
    # Returns:
    #   response from the server.
    #
    method getRdnRate {} {
	set msg [$self subName "Get %N rdown"]
	return  [$self sendMessage $msg]
    }
    # Get the ramp up rate for each channel.
    # Returns:
    #   response from the server.
    #
    method getRupRate {} {
	set msg [$self subName "Get %N rup"]
	return  [$self sendMessage $msg]
    }
    # Get the power down mode for all the channels.
    # Returns:
    #   Response from the server.
    #
    method getOffMode {} {
	set msg [$self subName "Get %N pdownmode"]
	return  [$self sendMessage $msg]
    }
    # Get the channel polarity settings.
    # Returns:
    #   response from the server.
    #
    method getPolarities {} {
	set msg [$self subName "Get %N polarity"]
	return  [$self sendMessage $msg]
    }
    # Get the channel temperature readings.
    # Returns:
    #   Response from the server.
    #
    method getTemps {} {
	set msg [$self subName "Get %N temp"]
	return  [$self sendMessage $msg]
    }
    # Return the on/off requests for all the channels.
    # Returns:
    #   Response from the server.
    #
    method getOnRequest {} {
	set msg [$self subName "Get %N on"]
	return  [$self sendMessage $msg]
    }

    #---------------------------------------
    # Private methods.
    #

    #
    #  This method sends data to the server
    #  and returns the response
    # Parameters:
    #   msg  - The message to send (a newline is
    #          appended to the message).
    # Implicit inputs:
    #   options(-socket) - The socket on which
    #                      communication occurs.
    # Returns:
    #    The reply from the server.
    #
    method sendMessage msg {
	set socket $options(-socket)
	puts  $socket "$msg"
	flush $socket
	set result [gets $socket]
	return $result
    }
    #  Encode the name of the device into a message
    #  everywhere %N occurs is replaced by the name of
    #  the device.
    # Parameters
    #   msg   - Message that is substituted into.
    # Implicit Inputs:
    #  options(-name) - Then name to substitute.
    # Returns:
    #  The message with substitution performed.
    #
    method subName msg {
	set name $options(-name)
	regsub -all {%N} $msg $name msg
	return $msg
    }
}
