#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#    Jeromy Tompkins
#	   NSCL
#	   Michigan State University
#	   East Lansing, MI 48824-1321


package provide mcfd16factory  1.0

package require snit
package require mcfd16usb
package require mcfd16rc
package require mcfd16commandlogger
package require mcfd16memorizer
package require mcfd16guiapp

## @brief A factory for producing snit::type in the MCFD16 family
#
# There are 4 different snit::types that implement the same public interface
# and can be used interchangably (if the return value is not important). This
# provides a centralized location for creating these objects. Thereby removing
# the creation logic from other snit::types. The logic for creating an
# MCFD16-type object is dependent on the MCFD16AppOptions. Given the application
# options, the factory will use the extra information in the options to properly
# construct it. For example, an MCFD16USB snit::type must have a serial file
# associated with it and that is specified in the app options.
#
snit::type MCFD16Factory {

  variable _appOpts ;# the MCFD16AppOptions

  ## @brief Construct
  #
  # It is expected that the MCFD16AppOptions that are being passed in will
  # contain sensible option for the type of values the user requests to create.
  # In other words, there will be problems if the user did not set the
  # -serialfile option in the MCFD16AppOptions passed in and then tried to
  # call create with an argumet "usb".
  #
  # @param opts   the MCFD16AppOptions 
  #
  constructor {opts} {
    set _appOpts $opts
  }

  ## @brief The Factory method
  #
  # There are 4 allowed types that can be generated and they are created with
  # the following arguments:
  #
  #  Argument       Type returned
  #  usb        --> MCFD16USB
  #  mxdcrcbus  --> MCFD16RC with an MXDCRCProxy
  #  cmdlogger  --> MCFD16CommandLogger
  #  memorizer  --> MCFD16Memorizer
  #
  # Ownership of instance is transferred to the user.
  #
  #  @important If the user specifies cmdlogger, they need to pass in the file
  #  channel that the cmdlogger will use as a second parameter.
  #
  # @param proto    the type of object desired
  # @param extra    optional parameter (needed for cmdlogger)
  #
  # @returns a new instance of an MCFD16-type snit::type
  #
  # @throws error if the user provided an argument other than the 4 described
  method create {proto {extra {}}} {

    set instance ""
    switch $proto {
      usb { set instance [$self createUSB] }
      mxdcrcbus { set instance [$self createMxDCRCBus] }
      cmdlogger { set instance [$self createCmdLogger $extra] }
      memorizer { set instance [$self createMemorizer] }
      default {
        return -code error "MCFD16Factory::create passed unknown type."
      }
    }

    return $instance
  }

  ## @brief Utility method for creating an MCFD16USB  
  #
  # It assumes the presence of a non-empty -serialfile option.
  # Ownership of instance is transferred to the user.
  #
  # @returns a new MCFD16USB instance
  method createUSB {} {
    return [MCFD16USB %AUTO% [$_appOpts cget -serialfile]]
  }

  ## @brief Creates an MCFD16RC controller with an MXDCRCProxy object
  #
  # This can construct an object that is intended to interact with an MCFD16
  # device over the RC-bus. This particular method sets it up to communicate
  # using an MxDC as a proxy for the communication. It does so using the
  # slow-controls server of the VMUSBReadout program. For this fact, proper
  # construction requires that a module of type mxdcrcbus is loaded into the
  # server using
  #
  # @code
  # Module create mxdcrcbus name
  # Module config name -base <MxDC base address>
  # @endcode
  #
  # To properly instantiate this, the value of the -host, -port, -module, and
  # -devno options must be reasonable. The first two of these will be given
  # reasonable defaults. The -module option is mandatory and the -devno defaults
  # to 0, which may or may not be reasonable.
  #
  # Ownership of instance is transferred to the user.
  #
  # @returns an MCFD16RC instance that uses an MXDCRCProxy instance
  method createMxDCRCBus {} {
    set prxy [MXDCRCProxy %AUTO% -server [$_appOpts cget -host] \
                     -port [$_appOpts cget -port] \
                     -module [$_appOpts cget -module] \
                     -devno [$_appOpts cget -devno]]
    return [MCFD16RC %AUTO% $prxy]
  }

  ## @brief Create a command logger instance
  #
  # @param channel  the channel object to write to
  #
  # @returns a unique MCFD16CommmandLogger instance
  #
  method createCmdLogger {channel} {
    return [MCFD16CommandLogger %AUTO% $channel]
  }

  ## @brief Create an MCFD16Memorizer
  #
  # Ownership of instance is transferred to the user.
  #
  # @returns a unique instance of MCFD16Memorizer
  method createMemorizer {} {
    return [MCFD16Memorizer %AUTO%]
  }
}
