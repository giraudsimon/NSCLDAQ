#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

##
# @file icon.tcl
# @brief Provide application icons for Tk applications.
#


package provide icon 1.0
package require Tk

namespace eval icon {
    set here [file dirname [info script]]
    set defaultPath [file normalize [file join $here .. .. share images]]
}
##
# setIcon
#   Sets the application icon to the specified file.
#
# @param file   - the name of the image file containing the icon.
# @param format - The icon format (defaults to png).
# @param top    - The top level who's icon is being set; defaults to .
#
# @note the file is checked for in order @prefix@/share/image,
#        [pwd]
#        ~
#
proc setIcon {file {format png} {top .}} {
    set searchOrder [list                                   \
	$::icon::defaultPath                                \
	[pwd] ~ ]

    foreach dir $searchOrder {
	set fullName [file normalize [file join $dir $file]]
	if {[file readable $fullName]} {
	    set image [image create  photo -format $format -file $fullName]
	    #
	    #  The after is an attempt to be sure the top level actually
	    #  materializes first.
	    #
	    after 200 wm iconphoto $top $image
	    return
	}
    }
    puts stderr "-- warning could not find icon image file $file in $searchOrder"
}
