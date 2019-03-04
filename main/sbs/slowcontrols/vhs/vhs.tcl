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
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#



# Support for the VHS series of iSeg High voltage power
# supplies.  This is provided as a snit object.
#

package provide iSegVhs 1.0

package require Vme
package require snit

#
#   This implements the snit object for the device.
#   Note that the addressing for this module is a bit weird.
#   There's a hardware jumper that, when installed makes the
#   device sit at 0x4000 shortio space.
#   There are then a pair of registers you can use to program the
#   module's new base address ... or rather the address it will have
#   when the jumper is removed and the module re-powered.
#   While we can provide a mechanism to set the base address, we can't
#   assure that the jumper is in, so we'll require that the user already
#   has gotten access to the device, rather than make that a type method.
#   In any event:
#
# OPTIONS:
#      -base      - Sets the module base address. This must be a 16 bit value.
#                   If not provided, the base address is assumed to be 0x4000,
#                   the factory default setting.
# METHODS:
#    getModuleStatus          - Gets a list of the bits in the module status
#                               register that are in non 'normal' states.
#                               TempBad        - Module temperature is bad.
#                               SupplyBad      - Power supply is bad.
#                               ModuleBadState - Module is in a bad state.
#                               EventActive    - An event is active with the mask set.
#                               SafetyLoopOpen - The safety loop is open.
#                               Ramping        - One or more channels are ramping.
#                               Failed         - Module has failed in some way.
#
#  These methods set/get the state of the bits in the Module Control register.
#  If the optional parameter is not provided, no change is made.  The state of the
#  bit is returned (1/0) by each function
#
#   killEnable ?on | off?     - Enables or disables the module kill on over
#                               voltage or current trips.
#   fineAdjust ?on | off?     - Enables or disables the fine adjustment (to regulate in the
#                               presence of temperature drift (should probably always be on).
#   clearKill                 - Clear the kill condition in all the channels (set only).
#
#  This set of functions reads/controls the module event status register.
#  
#   haveTempNoGoodEvent       - Returns true if there's a temp no good event.
#   haveSupplyNotGoodEvent    - Returns true if there's a power supply V not good event.
#   haveSafetyLoopOpenEvent   - Returns true if there's a safety loop open event.
#   clearTempNoGoodEvent      - If the overtemp condition no longer exists clears the event.
#   clearSupplyNotGoodEvent   - If the power supplies are now good, clears that event.
#   clearSafetyLoopOpenEvent  - If the safety loop is open, clears that event.
#
#  Controls/reads the module event mask register.  This register determines which events
#  set the EventActive bit in the module status register.
#
#    eventMask                - Returns the bits that are set in the event mask register.  These
#                               can be any of:
#                                   EnableTemperatureEvent    - Bad temp results in an event.
#                                   EnablePowerEvent          - Bad input voltage posts an event.
#                                   EnableSafetyLoopEvent     - Open safety loop posts an event.
#    allowTemperatureEvent ?on | off?
#                             - Enables/Disables the posting of a temperature event.
#    allowPowerEvent ?on | off?- Enables/Disables the posting of a bad power supply event.
#    allowSafetyEvent ?o | off?- Enables/Disables the potingof a bad safety loop event.
#
#    getChannelsWithEvents     - lists the channels that have active events.
#    getEventEnabledChannels   - lists the channels that can have active events.
#    maskChannelEvents  mask   - Sets the module event mask. Each bit in the mask represents
#                                a channel.  Set bits enable the channel to produce events.
#                                Channel 0 is the lsb etc.
#
#   setRampSpeed  pct          - Sets the voltage ramp speed in percentage of target voltage.
#   getRampSpeed               - Returns the ramp speed in percentage of the target voltage.
#   getVoltageLimit            - Returns the voltage limit (in absolute volts  - this is different
#                                from just getting the contents of the VoltageMax register.
#   getCurrentLimit            - Returns the current limit (mA).  Note this is different
#                                from returning the CurrentMax register.
#   get5VLevel                 - Returns the 5V power supply value in V.
#   getP12VLevel               - Returns the +12V power supply value in V.
#   getN12VLevel               - Returns the -12V power supply value in V.
#   getAmbientTemperature      - Returns the temperature of the board in degrees C.
#   serialNumber               - Returns the device serial number
#   firmwareVersion            - Return the firmware version as a.b.c.d
#   channelCount               - Returns the number of channels the board has.
#                                This assumes the channels are stuffed contiguously
#                                from channel 0 upwards.
#   getChannelStatus n         - Returns a list of the bits that are set channel n's
#                                status register.  Note that if channel n does not exist,
#                                an error is raised.  The bit names in the list returned are:
#                                VoltageLimit       - Exceeded voltage limit.
#                                CurrentLimit       - Exceeded current limit.
#                                Tripped            - Current tripped.
#                                ExternalInhibit    - External inhibit set
#                                VoltageBounds      - Exceeded voltage bounds (regulation).
#                                CurrentBounds      - Exceeded current bounds regulation)
#                                VoltageControlled  - Voltage control is active (?)
#                                CurrentControlled  - Current control is active (?)
#                                EmergencyOff       - Emergency shutdown performed.
#                                On                 - Channel is on.
#                                Ramping            - Channel is ramping.
#                                InputError         - Input error (?)
#  emergencyOff n              - Turns channel 1 off without a ramp down.
#  On           n              - Ramp channel n to target voltage.
#  Off          n              - Ramp channel n to 0.
#  
#  getChannelEvents n           - Returns a list of the events active on a channel:
#                                VoltageLimitEvent    - Voltage limit exceeded.
#                                CurrentLimitEvent    - Current limit exceeded.
#                                TripEvent            - Trip set due to current limit
#                                ExternalInhibitEvent - Externally inhibitied.
#                                VoltageBoundsEvent   - Voltage out of bounds event (regulation)
#                                CurrentBoundsEvent   - Current out of bounds event (regulation)
#                                ControlledVoltageEvent- Voltage is controlled event
#                                ControlledCurrentEvent- Current is controlled event.
#                                EmergencyOffEvent    - Emergency off generated event.
#                                RampEndedEvent       - Ramp ended event.
#                                TurnedOffEvent       - Event due to module off.
#                                InputErrorEvent      - Input error event.
#  resetChannelEvents  n mask    Resets the events specified by the mask.  THis can be a mask of e.g.
#                                vhs::ChannelEventVoltageLimit
#  getChannelEventMask n         Returns a list in the identical format of getChannelEvents,
#                                but the list shows which events have their mask bits set in
#                                the channel event mask register.
#  setChannelEventMask n bitlist - sets the channel event mask register as per the bits in
#                                  bit list (for channel n).  Bitlist has the same format
#                                as for getChannelEvents.
#  getChannelVoltage n           - Returns voltage set point for channel n
#  setChannelVoltage n v         - Sets the channel n voltage target to v Volts.
#  getChannelCurrentTrip n       - Returns the channel n current trip point in mA.
#  setChannelCurrentTrip n i     - Set the current trip point for channel n to i mA
#  getMeasuredVoltage     n       - Returns the current voltage for channel n in V
#  getMeasuredCurrent     n       - Returns the measured current for channel n in mA.
#  getVoltageBounds      n       - Returns the regulation tolerance for channel n's voltage.
#  setVoltageBounds   n v        - Set the regulation tolerance for channel n's voltage.
#
#  setAllVoltages     v          - Sets all channels to the same voltage.
#  setAllCurrentTrips i          - Sets the current trip values for all channels.
#  setAllVoltageBounds v         - Set the voltage regulation tolerance for all channels.
#  setAllCurrentBounds i         - Set the current regulation tolerances for all channels.
#  emergencyOffAll               - Shuts down all channels via emergency off.
#  onAll                         - Turn on all channels.
#  offAll                        - Turn off all channels.
#
#  setModuleBaseAddress newA     - Sets the module base address to newA.. 
#                                  I believe this only take effect the next power cycle with the
#                                  address jumper removed.
#
snit::type vhs {
    option -base   0x4000;              # Base address (static).
    
    
    #  Register map;
     
         # Module data:
    
    variable ModuleStatus                0
    variable ModuleControl               2
    variable ModuleEventStatus           4
    variable ModuleEventMask             6
    variable ModuleEventChannelStatus    8
    variable ModuleEventChannelMask   0x0a
    variable ModuleEventGroupStatus   0x0c
    variable ModuleEventGroupMask     0x10
    variable VoltageRampSpeed         0x14
    variable CurrentRampSpeed         0x18
    variable VoltageMax               0x1c
    variable CurrentMax               0x20
    variable SupplyP5                 0x24
    variable SupplyP12                0x28
    variable SupplyN12                0x2c
    variable Temperature              0x30
    variable SerialNumber             0x34
    variable FirmwareRelease          0x38
    variable PlacedChannels           0x3c
    variable DeviceClass              0x3e

    # ChannelOffsets is an array of offsets to the base of each channel
    # The remainder of the defs are offsets into each block of channel data.
    # FirstChannelBase is the base address of the first block of channels.
    # ChannelSize      is the number of bytes in each channel block

    variable ChannelOffsets
    variable FirstChannelBase         0x60
    variable ChannelSize              0x30
    variable ChannelStatus            0x00
    variable ChannelControl           0x02
    variable ChannelEventStatus       0x04
    variable ChannelEventMask         0x06
    variable ChannelVoltageSet        0x08
    variable CurrentTrip              0x0c
    variable VoltageMeasure           0x10
    variable CurrentMeasure           0x14
    variable VoltageBounds            0x18
    variable CurrentBounds            0x1c
    variable VoltageNominal           0x20
    variable CurrentNominal           0x24
    
    # Group register information:
    
    variable SetVoltageAllChannels   0x2a0
    variable SetCurrentAllChannels   0x2a4
    variable SetVoltageBoundsAll     0x2a8
    variable SetCurrentBoundsAll     0x2ac
    variable EmergencyOffAllChannels 0x2b0
    variable AllChannelsOn           0x2b4
    
    variable FirstVariableGroup      0x2c0
    variable VariableGroupCount      32
    variable VariableGroupSize       4
    variable VariableGroupAddresses;            # Array of base addresses of variable groups.
    
    
    
    variable GroupMemberList         0;         # Offset to group memberlist in a group.
    variable GroupTypeSettings       2;         # Offset to group type/settings word in a group.
    
    #  Special addresses we care about:
    
    variable NewBaseAddress           0x3a0
    variable NewBaseAddressXor        0x3a2
    variable NewBaseAddressAccepted   0x3a6 
    
    # Constant definitions
    
    variable VHS0x0Class             0x14;       # Correct device class.
    variable ModuleStatusBitNames;               # Array indexed by bit value to return module status bit name.
    variable ModuleEventMaskBitNames;            # "" but for the event mask bits.
    variable BitNumbers;                         # "" but just the bit numbers.
    variable ChannelStatusBitNames;              # "" But for the channel status regs.
    variable ChannelEventBitNames;               # "" but for the bits ina channel status register.
    
    typevariable ModuleControlKillEnable    0x4000;     # Module control register kill enable.
    typevariable ModuleControlSetAdjustment 0x1000;     # Enable/disable fine adjustment
    typevariable ModuleControlClearKill     0x0040;     # Clear kill signals.
    
    typevariable ModuleEventTempNoGood         0x4000
    typevariable ModuleEventPowerSupplyNoGood  0x2000
    typevariable ModuleEventSafetyLoopNoGood   0x0400

    typevariable ChannelControlEmergencyOff    0x0020
    typevariable ChannelControlOn              0x0008
    
    # Bits in the channel event mask register (also status regiseter)
    
    typevariable ChannelEventVoltageLimit     0x8000
    typevariable ChannelEventCurrentLimit     0x4000
    typevariable ChannelEventTrip             0x2000
    typevariable ChannelEventInhibit          0x1000
    typevariable ChannelEventVoltageBounds    0x0800
    typevariable ChannelEventCurrentBounds    0x0400
    typevariable ChannelEventControlledVoltage 0x0080
    typevariable ChannelEventControlledCurrent 0x0040
    typevariable ChannelEventEmergencyOff      0x0020
    typevariable ChannelEventRampEnded         0x0010
    typevariable ChannelEventEventOnOff        0x0008
    typevariable ChannelEventInputError        0x0004
    
    
    #  Group definitions:
    
    #     Group types:
    
    typevariable SetGroupType      0x0000
    typevariable StatusGroupType   0x4000
    typevariable MonitorGroupType  0x8000
    typevariable TimeoutGroupType  0xc000
    
    #  SetGroupType specific stuff:
    
    typevariable SetGroupOneShot   0x0000
    typevariable SetGroupPermanent 0x0100
    
    typevariable SetVset           0x0010
    typevariable SetIset           0x0020
    typevariable SetVbnds          0x0040
    typevariable SetIbnds          0x0050
    typevariable SetOn             0x00a0
    typevariable SetEmrgCutOff     0x00b0
    typevariable Cloning           0x00f0
    
    # Status group specific bit fields
    
    typevariable ChkIsOn                 0x0030
    typevariable ChkIsRamping            0x0040
    typevariable ChkIsControlledCurrent  0x0060
    typevariable ChkIsControlledVoltage  0x0070
    typevariable ChkIsCurrentBounds      0x00a0
    typevariable ChkIsVoltageBounds      0x00b0
    typevariable ChkIsExternalInhibit    0x00c0
    typevariable ChkIsTrip               0x00d0
    typevariable ChkIsCurrentLimit       0x00e0
    typevariable ChkIsVoltageLimit       0x00f0
    
    #  Monitoring group specific values:
    
    typevariable MonAtLeastOneClear      0x0000
    typevariable MonAtLeastOneSet        0x0100
    
    typevariable MonitorIsOn                0x0030
    typevariable MonitorIsRamping           0x0040
    typevariable MonitorIsControlledCurrent 0x0060
    typevariable MonitorIsControlledVoltage 0x0070
    typevariable MonitorIsCurrentBounds     0x00a0
    typevariable MonitorIsVoltageBounds     0x00b0
    typevariable MonitorIsExternalInhibit   0x00c0
    typevariable MonitorIsTrip              0x00d0
    typevariable MonitorIsCurrentLimit      0x00e0
    typevariable MonitorIsVoltageLimit      0x00f0
    
    typevariable MonitorActionEvent         0x0000
    typevariable MonitorActionRampDown      0x1000
    typevariable MonitorActionEmgOffGroup   0x2000
    typevariable MonitorActionEmgOffModule  0x3000
    
    # Timeout group specific values:
    
    typevariable TimeoutActionEvent         0x0000
    typevariable TimeoutActionRampDown      0x1000
    typevariable TimeoutActionEmgOffGroup   0x2000
    typevariable TimeoutActionEmgOffModule  0x3000
    
    constructor args {
        $self configurelist $args
        
        # Map the address range and ensure this is a Vhs module
        # We will use our name_vme for the VME space name
        
        vme create ${self}_vme -device shortio $options(-base) 0x400
        
        set class [${self}_vme get -w $DeviceClass]
        if {$class != $VHS0x0Class} {
            vme delete ${self}_vme
            error "VME address $options(-base) does not have a VHS module"
        
        }
        # set up the modules status bit names.
        # this is an array that is indexed by a bit value and contains the name of the
        # associated module status bit... these are actually the inverse of the bits.
        
        set ModuleStatusBitNames([expr 1 << 14])     TempBad
        set ModuleStatusBitNames([expr 1 << 13])     SupplyBad
        set ModuleStatusBitNames([expr 1 << 12])     ModuleBadState
        set ModuleStatusBitNames([expr 1 << 11])     EventActive
        set ModuleStatusBitNames([expr 1 << 10])     SafetyLoopOpen
        set ModuleStatusBitNames([expr 1 << 9])      Ramping
        set ModuleStatusBitNames([expr 1 << 8])      Failed
        set ModuleStatusBitNames([expr 1 << 7])      CommandsComplete
        
        # Set bit names for the module bitmask:
        
        set ModuleEventMaskBitNames([expr 1 << 14]) EnableTemperatureEvent
        set ModuleEventMaskBitNames([expr 1 << 13]) EnablePowerEvent
        set ModuleEventMaskBitNames([expr 1 << 10]) EnableSafetyLoopEvent
        
        # Bit numbers good for 32 bits too:
        
        for {set i 0} {$i < 31} {incr i} {
            set BitNumbers([expr 1 << $i])  $i
        }

	for {set i 0} {$i < 12} {incr i} {
	    set ChannelOffsets($i) [expr $FirstChannelBase + $i*$ChannelSize]
	}
	# Bit names for the channel status registers.
	#
	
	set ChannelStatusBitNames([expr 1 << 15])  VoltageLimitExceeded
	set ChannelStatusBitNames([expr 1 << 14])  CurrentLimitExceeded
	set ChannelStatusBitNames([expr 1 << 13])  TripSet
	set ChannelStatusBitNames([expr 1 << 12])  ExternalInhibit
	set ChannelStatusBitNames([expr 1 << 11])  VoltageOutOfRegulation
	set ChannelStatusBitNames([expr 1 << 10])  CurrentOutOfRegulation
	set ChannelStatusBitNames([expr 1 <<  7])  VoltageControlActive
	set ChannelStatusBitNames([expr 1 <<  6])  CurrentControlActive
	set ChannelStatusBitNames([expr 1 <<  5])  EmergencyOff
	set ChannelStatusBitNames([expr 1 <<  4])  Ramping
	set ChannelStatusBitNames([expr 1 <<  3])  On
	set ChannelStatusBitNames([expr 1 <<  2])  InputError
	
        foreach item [array names ChannelStatusBitNames] {
            set ChannelEventBitNames($item) $ChannelStatusBitNames($item)
        }
        
        
        
        for {set i 0} {$i < $VariableGroupCount} {incr i} {
            set VaribleGroupAddresses($i)  [expr $i * $VariableGroupSize + $FirstVariableGroup]
        }
        
    }
    destructor {
        catch {vme delete ${self}_vme}
    }
    
    #-------------------------------------------------------------------
    #
    #   getModuleStatus          - Gets a list of the bits in the module status
    #                               register that are in non 'normal' states.
    #                               TempBad        - Module temperature is bad.
    #                               SupplyBad      - Power supply is bad.
    #                               ModuleBadState - Module is in a bad state.
    #                               EventActive    - An event is active with the mask set.
    #                               SafetyLoopOpen - The safety loop is open.
    #                               Ramping        - One or more channels are ramping.
    #                               Failed         - Module has failed in some way.
    #
    method getModuleStatus {} {
         set status [${self}_vme get -w $ModuleStatus]
         set result [BitMaskToNameList [expr ~$status] ${selfns}::ModuleStatusBitNames]
         return $result
    }
    #-------------------------------------------------------------------------
    #
    # Clear set/clear the kill enable or just get its state.
    #
    # Parameters:
    #   args   - If supplied, this must be a single element that is a valid boolean
    #            describing what state to set the kill enable bit (true enables, false disables).
    #            If not supplied, only the value of the bit is returned (as a boolean).
    # Returns:
    #   true - If kill is enabled,
    #   false- If kill is not enabled.
    #
    method killEnable args {
        return [$self controlRegisterBit    killEnable                  \
                                            $ModuleControl              \
                                            $ModuleControlKillEnable    \
                                            $args]
    }
    #
    #  Sets or just gets the fine adjustement on/off.
    # Parameters:
    #     args    - This is either empty or a single element list containing
    #               a boolean that defines the desired state of the fine adjust.
    #               (true to enable, false to disable).
    # Returns:
    #     The state of the fine adjustment control after the dust has settled.
    #
    method fineAdjust args {
        return [$self controlRegisterBit fineAdjust                  \
                                         $ModuleControl              \
                                         $ModuleControlSetAdjustment \
                                         $args]
    }
    #--------------------------------------------------------------------------------
    #
    #  Clears any kill signals going into the channels.  There is no gaurantee the
    #  kills won't just come back due to the conditions the module is operating under.
    #
    #  No parameters are required.. no values are returned.
    method clearKill {} {
        set now [${self}_vme get -w $ModuleControl]
        set now [SetBit $now $ModuleControlClearKill 1];   # Unconditionally set the bit.
        ${self}_vme set -w $ModuleControl $now
        after 50
        set now [SetBit $now $ModuleControlClearKill 0];   #Cycle the bit back to zero...
        ${self}_vme set -w $ModuleControl $now
    }
    #--------------------------------------------------------------------------
    # haveTempNoGoodEvent
    #    Returns true if a temperature bad event has been signalled by the
    #    module.
    #
    method haveTempNoGoodEvent {} {
        set events [${self}_vme get -w $ModuleEventStatus]
        return [BitIsSet $events $ModuleEventTempNoGood]
    
    }
    #---------------------------------------------------------------------------
    #
    #  Returns true if a power supply voltage not good event has been posted
    #  by the module.
    #
    method haveSupplyNotGoodEvent {} {
        set events [${self}_vme get -w $ModuleEventStatus]
        return [BitIsSet $events $ModuleEventPowerSupplyNoGood]
    }
    #----------------------------------------------------------------------------
    #
    #  Returns true if the module has signalled a safety loop open event.
    #
    method haveSafetyLoopOpenEvent {} {
        set events [${self}_vme get -w $ModuleEventStatus]
        return [BitIsSet $events $ModuleEventSafetyLoopNoGood]
    }
    #----------------------------------------------------------------------------
    #
    #  Clear any temp no good event. Note that if the temp is still bad the event
    #  will just get reposted right away.
    #
    method clearTempNoGoodEvent {} {
        ${self}_vme set -w $ModuleEventStatus $ModuleEventTempNoGood
        
    }
    #---------------------------------------------------------------------------
    #
    #  Clear any power supply not good event.  Note that if the supply voltages
    #  are still bad the event will just get reposted right away.
    #
    method clearSupplyNotGoodEvent {} {
        ${self}_vme set -w $ModuleEventStatus $ModuleEventPowerSupplyNoGood
    }
    #--------------------------------------------------------------------------
    #
    # Clear any event resulting from an open safety loop.  Note that if the
    # safety loop is still open, this event will just get re-posted right away.
    #
    method clearSafetyLoopOpenEvent {} {
        ${self}_vme set -w $ModuleEventStatus $ModuleEventSafetyLoopNoGood
    }
    #---------------------------------------------------------------------------
    #
    #  Returns the bits that are set in the event mask.   These bits determine
    #  which events can be posted.
    #
    # Return Value:
    #    A list (potentially empty) of strings describing the set bits.
    #    the set bit values can be:
    #
    #                                   EnableTemperatureEvent    - Bad temp results in an event.
    #                                   EnablePowerEvent          - Bad input voltage posts an event.
    #                                   EnableSafetyLoopEvent     - Open safety loop posts an event.   
    #
    method eventMask {} {
        set register [${self}_vme get -w $ModuleEventMask]
        return [BitMaskToNameList $register ${selfns}::ModuleEventMaskBitNames]
        
    }
    #---------------------------------------------------------------------------
    #
    #   Gets or sets the state of the temperature event mask bit.
    # Parameters:
    #   args   - At most a single boolean parameter that specifies the new state
    #            of the temperature mask.
    # Returns:
    #    The final state of the temperature mask bit.
    #
    method allowTemperatureEvent args {
        return [$self controlRegisterBit allowTemperatureEvent   \
                                         $ModuleEventMask        \
                                         $ModuleEventTempNoGood  \
                                         $args]
    }

    #-------------------------------------------------------------------------
    #
    #  Gets or sets and gets the state of the Power off event enable bit.
    #
    # Parameters:
    #     args    - At most a single boolean. If provided, the boolean controls
    #               the new state of the power event mask.
    # Returns:
    #    The final state of the power event mask bit.
    #
    method allowPowerEvent args {
        return [$self controlRegisterBit allowPowerEvent \
                                        $ModuleEventMask \
                                        $ModuleEventPowerSupplyNoGood \
                                        $args]
    }
    #--------------------------------------------------------------------------------
    #
    #  Enables or queries the state of the safety event mask.
    #
    # Parameters:
    #    args  - At most one boolean that sets the new requested state of the
    #            Safety event enable bit.
    # Returns:
    #    The final state of that bit.
    #
    method allowSafetyEvent args {
        return [$self controlRegisterBit    allowPowerEvent             \
                                            $ModuleEventMask            \
                                            $ModuleEventSafetyLoopNoGood \
                                            $args]
    }
    #---------------------------------------------------------------------------
    #
    #  Returns a list of the channel numbers that have events posted.
    #  this is really just a decode of the ModuleEventChannelMask into
    #  bit numbers.
    #
    method getChannelsWithEvents {} {
        set mask [${self}_vme get -w $ModuleEventChannelStatus]
        return [BitMaskToNameList $mask ${selfns}::BitNumbers]
    }

    #--------------------------------------------------------------------------
    #
    #   Return a list of the channels that are enabled to produce events.
    #
    method getEventEnabledChannels {} {
        set mask [${self}_vme get -w $ModuleEventChannelMask]
        return [BitMaskToNameList $mask ${selfns}::BitNumbers]
    }
    #--------------------------------------------------------------------------
    #
    #  Set the channel mask according to the mask.
    #
    method maskChannelEvents mask  {
        ${self}_vme set -w $ModuleEventChannelMask $mask
    }
    #--------------------------------------------------------------------------
    #
    #  Set the ramp speed to some percent of the nominal voltage/second.
    #  This must be no larger than 20.00
    # Parameters:
    #    pct     - Percentage of max rated value of the channel (nominal voltage
    #              the manual calls it to ramp per second.
    #
    method setRampSpeed pct {
	$self writeFloat $VoltageRampSpeed $pct
    }
    #---------------------------------------------------------------------
    #  
    # Read the ramp speed from the module.  It is returned in the same for
    # it was gotten, pct of voltage hardware max.
    #
    method getRampSpeed {} {
	return [$self readFloat $VoltageRampSpeed]
    }
    #--------------------------------------------------------------------
    #
    # Return the voltage limit in volts.  Note tht this is different
    # than just readin the voltage limit register, which just returns
    # the percentage of the hardware max capability dialed in on the pot.
    # we take that percentage and multiply it by the max capability from chan 0.
    #
    method getVoltageLimit {} {
	set pct [$self readFloat $VoltageMax]
	set max [$self readFloat [$self channelOffset 0 $VoltageNominal]]

	return [expr $max * $pct/100.0]
	
    }

    #-----------------------------------------------------------------
    # 
    # Return the current limit in the same way as the voltage limit.
    #
    method getCurrentLimit {} {
	set pct [$self readFloat $CurrentMax]
	set max [$self readFloat [$self channelOffset 0 $CurrentNominal]]

	return [expr $max * $pct/100.0]
    }
    #----------------------------------------------------------------
    #
    #  Return the voltage level on the 5V power supply of the board.
    #
    method get5VLevel {} {
	return [$self readFloat $SupplyP5]
    }
    #----------------------------------------------------------------
    #   Returns the voltage level on the 12 V power supply of the
    #   board.
    #
    method getP12VLevel {} {
	return [$self readFloat $SupplyP12]
    }
    #---------------------------------------------------------------
    #   Returns the voltage level on the -12V power supply of the
    #   board.
    #
    method getN12VLevel {} {
	return [$self readFloat $SupplyN12]
    }
    #-------------------------------------------------------------
    # 
    # Return the board temperature sensor value in degrees C.
    #
    method getAmbientTemperature {} {
	return [$self readFloat $Temperature]
    }
    #-------------------------------------------------------------
    #
    # Return the board serial number.
    #
    method serialNumber {} {
	return [$self getLong $SerialNumber]
    }
    #-------------------------------------------------------------
    #
    #  Return the board firmware release.  We need to guess about
    #  the order of the bytes here.. we will assume that the
    #  serial number is big-endian.. that is the high byte of the
    #  first word is the first digit.
    #
    #  The result is returned as a w.x.y.z string.
    #
    method firmwareVersion {} {
	set high [${self}_vme get -w $FirmwareRelease]
	set low  [${self}_vme get -w [expr $FirmwareRelease + 2]]

	set w    [expr ($high >> 8) &0xff]
	set x    [expr $high & 0xff]
	set y    [expr ($low >> 8)  &0xff]
	set z    [expr $low & 0xff]

	return "$w.$x.$y.$z"
    }
    #--------------------------------------------------------------
    #
    # Return the number of channels in the module.
    # This can be at most 12.  We assume the channels are stuffed
    # contiguously from bottom to top
    #
    method channelCount {} {
	set channelMask [${self}_vme get -w $PlacedChannels]
	set channels 0
	for {set i 0} {$i < 12} {incr i} {
	    if {($channelMask & [expr 1 << $i]) == 0} break
	    incr channels
	}
	return $channels
    }
    #---------------------------------------------------------------
    #
    #  Returns the bit names of the bits set in the channel status
    #  Register for the specified channel
    #
    # Parameters:
    #    channel - Number of the channel to check it will be an 
    #              error to choose a channel that does not exist
    #
    method getChannelStatus channel {
	$self validChannel $channel
	set status [${self}_vme get -w [$self channelOffset $channel $ChannelStatus]]
	return [BitMaskToNameList $status ${selfns}::ChannelStatusBitNames]
    }
    #----------------------------------------------------------------
    # 
    #  Turn off the channel without a ramp.. we do this by removing on
    #  and cycling the emergency off.  At the end, the
    #  control register will be 0.
    # 
    method emergencyOff channel {
	$self validChannel $channel
	set offset [$self channelOffset $channel $ChannelControl]

	${self}_vme set -w $offset $ChannelControlEmergencyOff
	after 50
	${self}_vme set -w $offset 0

    }
    #-------------------------------------------------------------------
    #
    #  Turn on a channel by setting the on bit in a channel control
    #  register.
    #
    method channelOn channel {
        $self validChannel $channel
        set offset [$self channelOffset $channel $ChannelControl]
        
        ${self}_vme set -w $offset  $ChannelControlOn
    }
    #------------------------------------------------------------------
    #
    # Turn a channel off by turning off all the bits in its control register.
    #
    method channelOff channel {
        $self validChannel $channel
        set offset [$self channelOffset $channel $ChannelControl]
        
        ${self}_vme set -w $offset  0
    }
    #---------------------------------------------------------------------
    #
    #   Return a list of the names of event conditions on a channel by decoding
    #   its event register.
    #
    method getChannelEvents channel {
        $self validChannel $channel
        set mask [${self}_vme get -w [$self channelOffset $channel $ChannelEventStatus]]
        return   [BitMaskToNameList $mask ${selfns}::ChannelEventBitNames]
    }
    #--------------------------------------------------------------------------------------
    #  Resets selected channel events.
    # Parameters:
    #   chan    - Number of channel to affect.
    #   mask    - mask of events to clear.
    #
    method resetChannelEvents {chan mask} {
        $self validChannel $chan
        ${self}_vme set -w [$self channelOffset $chan $ChannelEventStatus] $mask
    }
    #--------------------------------------------------------------------------------------
    #  Reset all events that are resettable:
    #
    method resetAllChannelEvents channel {
        $self validChannel $channel
        ${self}_vme set -w [$self channelOffset $channel $ChannelEventStatus] 0xffff
    }
    #--------------------------------------------------------------------------------------
    #
    #  Return the mask of bits that are set in  a channel mask register.
    #

    method getChannelEventMask channel {
        $self validChannel $channel
        return [${self}_vme get -w [$self channelOffset $channel $ChannelEventMask]
    }
    #---------------------------------------------------------------------------------
    #
    #  Set a new value for the channel even mask:
    #
    method setChannelEventMask {channel mask} {
        $self validChannel $channel
        
        ${self}_vme set -w [$self channelOffset $channel $ChannelEventMask] $mask
    }
    #----------------------------------------------------------------------------------
    #
    #  set the channel target voltage to a specific value.  If the channel is on,
    #  a ramp to the new target voltage will start.
    # Parameters:
    #    channel   - Number of the channel to set.
    #    value     - New target in floating point volts.
    #
    method setChannelVoltageSet {channel value} {
        $self validChannel $channel
        
        set offset [$self channelOffset $channel $ChannelVoltageSet]
        $self writeFloat $offset $value
    }
    #---------------------------------------------------------------------------------
    #
    #  Return the voltage setting for the specified channel.  See getMeasuredVoltage
    #  to return the actual voltage.  This is the requested voltage the module will
    #  ramp towards if the channel is on.
    #
    method getChannelVoltageSet channel {
        $self validChannel $channel
        $self readFloat [$self channelOffset $channel $ChannelVoltageSet]
    }
    #---------------------------------------------------------------------------
    #  Get the value of the current trip level for the specified channel
    #
    method getChannelCurrentTrip channel {
        $self validChannel $channel
        return [$self readFloat [$self channelOffset $channel $CurrentTrip]]
    }
    #--------------------------------------------------------------------------
    #
    #  Set a new value for the channel current trip.
    #
    method setChannelCurrentTrip {channel value} {
        $self validChannel $channel
        $self writeFloat [$self channelOffset $channel $CurrentTrip] $value
    }
    #--------------------------------------------------------------------------
    #
    #  Get the measured voltage from the channel
    #
    method getMeasuredVoltage channel {
        $self validChannel $channel
        return [$self readFloat [$self channelOffset $channel $VoltageMeasure]]
    }
    #--------------------------------------------------------------------------
    #
    #  Returns the measured current from the specified channel.
    #
    method getMeasuredCurrent channel {
        $self validChannel $channel
        return [$self readFloat [$self channelOffset $channel $CurrentMeasure]]
    }
    #-------------------------------------------------------------------------
    #
    #   Returns the voltage regulation tolerance for the specified channel
    #
    method getVoltageBounds channel {
        $self validChannel $channel
        return [$self readFloat [$self channelOffset $channel $VoltageBounds]]
        
    }
    #-------------------------------------------------------------------------
    #
    #   Sets new voltage regulation bounds.  If, when the ramp is done,
    #   the voltage gets more than bounds away from the target voltage,
    #   error events can be generated. (ChannelEventVoltageBounds).
    #   
    method setVoltageBounds {channel bound} {
        $self validChannel $channel
        $self writeFloat [$self channelOffset $channel $VoltageBounds] $bound
    }
    #----------------------------------------------------------------------------
    #
    #  Returns the bounds on the allowed current.  I >think< this determines,
    #  along with the target current, what constitutes an over current trip?
    #
    method getCurrentBounds channel {
        $self validChannel $channel
        return [$self readFloat [$self channelOffset $channel $CurrentBounds]]
    }
    #---------------------------------------------------------------------------
    #
    #   Sets the current bounds on a channel.  I >think< that the current bounds
    #   in conjunction with the 'target current' determine the point at which
    #   a current out of bounds event will occur (and hence when an associated
    #   monitor group might trip the module.
    #
    method setCurrentBounds {channel bound} {
        $self validChannel $channel
        $self writeFloat [$self channelOffset $channel $CurrentBounds] $bound
    }
    #--------------------------------------------------------------------------
    #
    #  Get nominal voltage:
    #
    method getNominalVoltage {channel} {
        $self validChannel $channel
        return [$self readFloat [$self channelOffset $channel $VoltageNominal]]
    }
    #--------------------------------------------------------------------------
    # Get nominal current:
    #
    method getNominalCurrent channel {
        $self validChannel $channel
        return [$self readFloat [$self channelOffset $channel $CurrentNominal]]
    }
    #--------------------------------------------------------------------------
    #
    #   Sets all voltages to a common value. This is done via the fixed
    #   group registers.
    #
    
    method setAllVoltages {volts} {
        $self writeFloat $SetVoltageAllChannels $volts
    }
    #--------------------------------------------------------------------------
    #
    #  Returns the voltage last set to all channels.  Note that I have no idea
    #  what this will do if the an individual channel voltage was set prior to
    #  reading this register.. it is probabl only safe to count on this value
    #  meaning anything after doing a setAllVoltages.
    #
    method getAllVoltages {} {
        return [$self readFloat $SetVoltageAllChannels]
    }
    #-------------------------------------------------------------------------
    #
    #  Sets the output current for all channels
    #
    method setAllCurrents current {
        $self writeFloat $SetCurrentAllChannels $current
    }
    #-------------------------------------------------------------------------
    #
    # Returns the common output current register contents.
    #
    method getAllCurrents {} {
        return [$self readFloat $SetCurrentAllChannels]
    }
    #-------------------------------------------------------------------------
    #
    #  Sets the voltage bounds for all channels.
    #
    method setAllVoltageBounds {bounds} {
        $self writeFloat $SetVoltageBoundsAll $bounds
        
    }
    #------------------------------------------------------------------------
    #
    #   Returns the voltage bounds for all channels
    #
    method getAllVoltageBounds {} {
        return [$self readFloat $SetVoltageBoundsAll]
    }
    #------------------------------------------------------------------------
    #
    #  Sets the current bounds for all channels.
    #
    method setAllCurrentBounds {bounds} {
        $self writeFloat $SetCurrentBoundsAll $bounds
    }
    #------------------------------------------------------------------------
    #
    # Returns the current bounds for all channels.
    #
    method getAllCurrentBounds {} {
        return [$self readFloat $SetCurrentBoundsAll]
    }
    #-------------------------------------------------------------------------
    #
    #  "Scram" all channels to zero... no ramping is done.
    #
    method emergencyOffAll {} {
        ${self}_vme set -w $EmergencyOffAllChannels 1
    }
    #--------------------------------------------------------------------------
    #
    #   Turn on all channels.
    #
    method onAll {} {
        ${self}_vme set -w $AllChannelsOn 1
    }
    #--------------------------------------------------------------------------
    #
    #  Turn off all channels (with ramp).
    #
    method offAll {} {
        ${self}_vme set -w $AllChannelsOn 0
    }
    #---------------------------------------------------------------------------
    #
    #  Change the module base address.
    #  Once this is done, one must be sure the factory base address jumper is
    #  removed from the module and then the module power cycled to get its new
    #  address
    #
    method setModuleBaseAddress newAddress {
        set newAddressXor [expr ~$newAddress]
        ${self}_vme set -w $NewBaseAddress    $newAddress
        ${self}_vme set -w $NewBaseAddressXor $newAddressXor
        
        # Not sure if this is needed but...
        
        ${self}_vme get -w $NewBaseAddressAccepted
        
    }
    #---------------------------------------------------------------------------
    #
    #  Create a set group.  The caller can use the typevariables for that group
    #  for his/her parameters:
    # Parameters:
    #    groupNumber         - Number of the group to create (0-31).
    #    members             - Bitmask of the channels that belong in the group.
    #    track               - Boolean that's true if the settings group permanently tracks the master
    #    trackWhat           - Describes what to track e.g. $vhs::SetVset to track the set voltage.
    #    masterChannel       - The master channel of the group.
    #
    method createSetGroup {groupNumber members track trackWhat masterChannel} {
        
        # set up the group type/settings mask:
        
        $self validChannel masterChannel;              # Master channel must exist after all.
        set typeSettings $SetGroupType;                # Start with the group type.
        set typeSettings [expr $typeSettings | ($track ? $SetGroupOneshot : $SetGroupPermanent)]
        set typeSettings [expr $typeSettings | $trackWhat | $masterChannel]
        
        # Now set up the group:
        
        $self setupGroup $groupNumber $members $typeSettings
    }
    #----------------------------------------------------------------------------
    #
    #  Creates  a status group. Status groups simply set the group status bit
    #  corresponding to the group when the condition being monitored is set
    #  in any of the channels.  You can think of that bit as reporting the logical
    #  or of the statuses being monitored for the members.
    #
    # Parameters:
    #   groupNumber      - Number of the group to setup [0-31]
    #   members          - Bitmask of group members.
    #   whichStatus      - typevariable value the describes what status is being monitored.
    #                      e.g. $vhs::ChkIsTrip
    #
    method createStatusGroup {groupNumber members whichStatus} {
        
        # set up the type/settings mask:
        
        set typeSettings [expr $whichStatus | $StatusGroupType]
        
        $self setupGroup $groupNumber $members $typeSettings
    }
    #------------------------------------------------------------------------------
    #
    #  Create a monitor group.  Monitor groups are like status groups but with
    #  actions that can be associated with the condition being monitored.
    #  For example, if a monitor group channel member reports a voltage regulation
    #  problem, the group can be shut down with an emergency off.
    #
    # Parameters:
    #    groupNumber     - Number of the group to set up, must be in the set [0-31]
    #    members         - Bit mask of channels that belong to the group.
    #    what            - Condition being monitored, e.g $vhs::MonitorIsOn
    #    sense           - The sense of the monitor.  This should be one of
    #                      $vhs::MonAtLeastOneClear or $vhs::MonAtLeastOneSet
    #                      to understand this consider what = $vhs::MonitorIsOn
    #                      if $vhs::MonAtLeastOneClear is used, then this is monitoring
    #                      the event of at least one channel in the group turning off.
    #                      if $vhs::MonAtLeastOneSet nonitors the condition of at least
    #                      one channel turning on.
    #   action            - The autonomous action to take if the condition what/sense
    #                       are satisfied, e.g. $vhs::MonitorActionRampDown
    #
    #                       
    method createMonitorGroup {groupNumber members what sense action} {
        
        #  Setup the type/Settings mask
        
        set typeSettings [expr $what | $sense | $action | $MonitorGroupType]
        
        $self setupGroup $groupNumber $members $typeSettings
    }
    #---------------------------------------------------------------------------------
    #
    #  Return the value of the 32 bit ModuleEventGroupStatus longword.  This
    #  describes which groups are reporting status/event information.
    #
    #
    method getGroupStatus {} {
        return [$self  getLong $ModuleEventGroupStatus]
    }
    #--------------------------------------------------------------------------------
    #
    #  Attempts to reset all the events/status bits in the group event/status register
    #  Note that any events still pending will immediately light up again.
    #
    method resetGroupStatus {} {
        $self setLong $ModuleEventGroupStatus 0xffff
    }
    #------------------------------------------------------------------------------
    #
    #  Sets bits that enable the setting of bits in module event group status register
    #  if groups have status/events to report.
    #
    method setGroupEventMask mask {
        $self setLong $ModuleEventGroupMask $mask
    }
    #------------------------------------------------------------------------------
    #
    #  Reports the value of the 32 bit ModuleEventGroupMask register.
    #
    method getGroupEventMask {} {
        $self getLong $ModuleEventGroupMask
    }
    
    #--------------------------------------------------------------------------
    #   Utilities and factorizations:
    #
    
    # Control the state of a single register bit if a control is supplied else
    # just return the bit:
    # Parameters:
    #     methodName     - Name of calling method (for error messages)
    #     offset         - Register offset in which the bit is (register is assumed a word)
    #     mask           - Mask of the bit to control
    #     control        - List of 0 or 1 elements 0 elements no control 1 element must
    #                      be a bool that specifies the desired state of the bit.
    #
    method controlRegisterBit {methodName offset mask control} {
        if {[llength $control] != 0} {
            set newState [GetSingleBoolean $control $methodName]
            set register [${self}_vme get -w $offset]
            set register [SetBit $register $mask $newState]
            ${self}_vme set -w $offset $register
            
            #  Some registers require a bit of time to propagate:
            
            after 50
        }
        set register [${self}_vme get -w $offset]
        return [BitIsSet $register $mask]
    }
    # Set a long value into a register. The assumption is 16 bit access to the
    # device with big-endian ordering of the words and the interface taking care of
    # bit ordering in the device.
    #
    method setLong {offset value} {
        set low  [expr $value & 0xffff]
        set high [expr ($value >> 16) & 0xffff]
        
        ${self}_vme set -w $offset $high
        ${self}_vme set -w [expr $offset + 2] $low
        
    }
    #  Get a longword value from an offset. The assumption is the
    #  the same as for setLong
    #
    method getLong offset {
        set high [${self}_vme get -w $offset]
        set low  [${self}_vme get -w [expr $offset + 2]]
        
        set result [expr $low | ($high << 16)]
        return $result
    }
    # Write a floating point register:
    #
    method writeFloat {offset value} {
	set value [FloatToHex $value]
	$self setLong $offset $value
    }
    # Read a floating point register:
    #
    method readFloat offset {
	set hwValue [$self getLong $offset]
        return [RegisterToFloat $hwValue]
    }
    # Return the offset for a specific register in a specific channel
    # Caller is responsible for channel range checking.
    #
    method channelOffset {channel register} {
	return [expr $ChannelOffsets($channel) + $register]
    }
    # Throws an error if the channel is not valid

    method validChannel channel {
	set numChans [$self channelCount]

	if {($channel < 0) || ($channel >= $numChans)} {
	    error "There are $numChans channels and you attempted channel $channel"
	}
    }
    # Stuff to setup group registers... we'll treat these as writeonly for now.
    #
    proc validGroup group {
        if {($group < 0) || ($group > 31) } {
            error "Group numbers must be in the range of 0-31 inclusive, got $group"
        }
                                
    }
    method writeGroupMembership {group mask} {
        validGroup $group
        set offset [expr $VariableGroupAddresses($group) + $GroupMemberList]
        ${self}_vme set -w $offset $mask
    }
    
    method writeGroupTypeSetting {group mask} {
        validGroup $group
        set offset [expr $VariableGroupAddresses($group) + $GroupTypeSettings]
        ${self}_vme set -w $offset $mask
    }
    
    method setupGroup {group membership typeSettings} {
        $self writeGroupMembership  $group $membership
        $self writeGroupTypeSetting $group $typeSettings
    }
    # Set/clear the bit whose mask is 'bit' in the 'mask' depending on the boolean
    # state of onoff.
    # Return the result.
    #
    proc SetBit {mask bit onoff} {
        if {$onoff} {
            set mask [expr $mask | $bit]
        } else {
            set mask [expr $mask & (~$bit)]
        }
        return $mask
    }
    #
    #  Get the single boolean arg from the input list or complain.
    #  The caller must ensure there's at least one list element
    #
    proc GetSingleBoolean {l function} {
        set n [llength $l]
        set value [lindex $l 0]
        if {($n != 1) || ![string is boolean $value]} {
            error "$function: Needs at most one boolean parameter but got: $l"
        }
        return $value
    }
    # Returns true if a specific bit is set in a mask
    #
    proc BitIsSet {mask bit} {
        return [expr {($mask & $bit) != 0}]
    }
    #
    #  Returns a names list for a mask given the mask and the name of an
    #  array keyed by the bit mask that returns names of the bits.
    #
    proc BitMaskToNameList {mask nameArray} {
        set result [list]
        for {set bit [expr 1 << 15]} {$bit != 0} {set bit [expr $bit >> 1]} {
            if {($bit & $mask) != 0} {
                if {[array names $nameArray $bit] ne ""} {
                   lappend result [set ${nameArray}($bit)]
                }
            }
        }
        return $result
    }
    # Takes a floating point number and returns it as a
    # hexadecimal equivalent IEEE value...with leading 0x and all.
    # so it can be interpreted by tcl as an int.
    #
    proc FloatToHex fValue {
        global tcl_platform
        
        set binaryFloat [binary format f $fValue]
        binary scan $binaryFloat H* initialHex
        
        # If we are little endian we need to reverse the result
        # Bytewise:
        
        if {$tcl_platform(byteOrder) eq "littleEndian"} {
            set result ""
            for {set index 6} {$index >= 0} {incr index -2} {
                append result [string range $initialHex $index [expr $index+1]]
            }
            set initialHex $result   
        }
        return "0x$result"
    }
    # Takes a longword registe value and converts it into a float.
    #
    proc RegisterToFloat value {
	global tcl_platform
	if {$tcl_platform(byteOrder) eq "littleEndian"} {
	    set binaryFloat [binary format i $value]
	} else {
	    set binaryFloat [binary format I $value]
	}
	binary scan $binaryFloat f result

	return $result
    }

}