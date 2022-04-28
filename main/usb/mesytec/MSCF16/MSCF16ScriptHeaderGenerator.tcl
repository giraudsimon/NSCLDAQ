#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2015.
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


package provide mscf16scriptheadergenerator 1.0

package require snit

## @brief A snit::type to generate the header portion of saved file
#
# Depending on whether the user specified a usb or mxdcrcbus protocol, the 
# first few lines in the saved file must be different to reflect the different
# ways to instantiate. This snit::type handles the difference and can determine
# what the appropriate text to add should be according to the MSCF16AppOptions.
#
snit::type MSCF16ScriptHeaderGenerator {

  option -name -default ::dev

  variable _options ;# the MSCF16AppOptions to refer to

  ## @brief Construct and record the options
  #
  # @param appoptions  the MSCF16AppOptions instance name manage by MSCF16GuiApp
  #
  # @returns name of constructed snit::type
  constructor {appoptions args} {
    set _options $appoptions 

    $self configurelist $args
  }

  ## @brief Generate a header
  # 
  # The user of this snit::type is expected to call this method rather than the
  # others. Doing so allows the logic of the snit::type to handle how to
  # properly generate the header according the options.
  #
  # @returns list of statements that compose the header
  method generateHeader {} {

    set header [$self generateUSBHeader]

    return $header
  }

  ## @brief Specialized method for constructing a USB protocol header
  #
  # Generates a header that instantiates an MSCF16USB type with the appropriate
  # serial file name.
  #
  # @returns list of statements that compose the header
  method generateUSBHeader {} {
    set header [list]
    lappend header {package require mscf16usb}
    lappend header [list set serialFile [$_options cget -serialfile]]
    lappend header {}
    lappend header "if \{!\[file exists \$serialFile\]\} \{"
    lappend header {  puts "Serial file \"$serialFile\" provided but does not exist."}
    lappend header {  exit}
    lappend header "\}"
    lappend header "MSCF16USB [$self cget -name] \$serialFile"
    return $header
  }

  if {0} {
    # this may one day be useful if someone wants rc-bus support. I am 
    # going to keep it around as a comment.
    #

  ## @brief Specialized generator for the mxdcrcbus type protocol
  #
  # This will generate statements that when added to a file can be executed to
  # instantiate an MCFD16RC driver whose proxy is of type MXDCRCProxy. The proxy
  # receives parameters given by the -host, -port, -module, and -devno options.
  # 
  # @returns list of statements that compose the header
  method generateMxDCRCBusHeader {} {
    set header [list]

    lappend header {package require mcfd16rc}
    lappend header {package require mcfd16channelnames}
    lappend header [list MXDCRCProxy ::proxy -server [$_options cget -host] \
      -port [$_options cget -port] \
      -module [$_options cget -module] \
      -devno [$_options cget -devno]] 
    lappend header {# use the proxy created to construct an MCFD16RC} 
    lappend header "MCFD16RC [$self cget -name] ::proxy"

    return $header
  }
  }
}
