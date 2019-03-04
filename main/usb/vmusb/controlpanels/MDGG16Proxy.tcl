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


package provide mdgg16proxy 1.0

package require snit
package require usbcontrolclient


## @breif Client for the mdgg16 VMUSBReadout slow-controls module
#
# This is intended to work with the mdgg16 slow-controls module in the 
# VMUSBReadout. It therefore depends on the present of that server. 
#
# The interface currently provided is very minimal and can be added to if more
# functionality is required from the MDGG-16 device. At the moment there is
# only a desire to use it as a programmable OR.
#
# This owns a single component.
#
snit::type MDGG16Proxy {

  option -module -default {} ;#!< the name of the slow-controls module

  component _comObj ;#!< the client connection to slow-controls

  delegate option * to _comObj

  ## @brief Establish connection with server and parse options
  #
  constructor {args} {
    if {[catch {install _comObj using controlClient %AUTO%}]} {
      set msg "Failure starting client to VMUSBReadout slow-controls server. "
      append msg "Is the server running? Is the module name correct?"
      return -code error $msg
    }

    $self configurelist $args
  }

  ## @brief Destroy the connection object
  #
  destructor {
    catch {$_comObj destroy}
  }

  ## @brief Write the value for the Logical OR AB register
  #
  # It is the callers responsibility to provide an argument that is no larger
  # than a 32-bit unsigned integer can hold.
  #
  # @param value  value to write
  #
  # @returns response from the slow-controls server
  method SetLogicalORAB {value} {
    return [$_comObj Set [$self cget -module] "or_ab" $value]
  }

  ## @brief Write the value for the Logical OR CD register
  #
  # It is the callers responsibility to provide an argument that is no larger
  # than a 32-bit unsigned integer can hold.
  #
  # @param value  value to write
  #
  # @returns response from the slow-controls server
  method SetLogicalORCD {value} {
    return [$_comObj Set [$self cget -module] "or_cd" $value]
  }

  ## @brief Read the value for the Logical OR AB register
  #
  # @returns response from the slow-controls server (should be an integer)
  method GetLogicalORAB {} {
    return [$_comObj Get [$self cget -module] "or_ab"]
  }

  ## @brief Read the value for the Logical OR CD register
  #
  # @returns response from the slow-controls server (should be an integer)
  method GetLogicalORCD {} {
    return [$_comObj Get [$self cget -module] "or_cd"]
  }

}
