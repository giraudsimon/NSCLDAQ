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

package provide ChannelLabel 1.0

package require snit
package require Tk

lappend auto_path [file join [file dirname [info script]] .. TclLibs]

package require Utils



## @brief ttk::entry widget with default value
#
# This will not allow the user to leave the widget empty unless the
# -defaultstring option is {}. Basically, whenever a user is done editing the
# text in the ttk::entry, this checks to see whether it is either empty or all
# whitespace (i.e. space characters). If it is non-empty, then the user's new
# string is left alone. However, if it was determined to be empty, the
# textvariable is set to the value of -defaultstring.
#
snit::widgetadaptor ChannelLabel {

  option -defaultstring -default "Text" ;#!< the default value to fall back to

  # delegate all options and methods to hull, this is really to behave just like
  # a ttk::entry
  delegate option * to hull
  delegate method * to hull

  ## @brief Install hull and parse the options.
  constructor {args} {
    installhull using ttk::entry -validate focus \
      -validatecommand [mymethod ValidateName %P] \
      -invalidcommand [mymethod ResetChannelName %W]

    # user's args should override this.
    $self configurelist $args
  }

  ## @brief Check whether a channel name contains non-whitespace characters
  # 
  # This is called when a channel entry loses focus
  #
  # @param name   candidate string 
  #
  # @return boolean
  # @retval 0 - string was empty or all whitespace
  # @retval 1 - otherwise
  method ValidateName {name} {
    return [Utils::nonemptString $name]
    set name [string trim $name]
    set good [expr [string length $name]!=0]

    return $good
  }

  ## @brief Reset channel to a simple string
  #
  # Typically called with ValidateName returns false. It will set the string
  # to "Ch#".
  #
  # @returns ""
  method ResetChannelName {widget} {
    set str [$widget cget -textvariable]
    set $str [$self cget -defaultstring]
  }
}

