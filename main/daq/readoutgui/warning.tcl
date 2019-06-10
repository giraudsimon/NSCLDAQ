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


package provide Diagnostics 1.0

namespace eval Diagnostics {
    variable isTk

    if {[info globals tk_version] == ""} {
	set isTk 0
    } else {
	set isTk 1
    }

    proc Dialog  {title text icon exitbutton} {
	variable isTk
	set window .diagnosticsdialog
	if {!$isTk} {
	    puts stderr $text
	    if {$exitbutton} {
		puts -nonewline "Do you want to exit (yes/no)? "
		flush stdout
		set yesno [string toupper [gets stdin]]
		set yesno [string index $yesno 0]
		if {$yesno == "Y"} exit else return
	    } else {
		return
	    }
	} else {
	    if {$exitbutton} {
		set button [tk_dialog $window $title $text $icon 1 "Continue" "Exit"]
	    } else {
		set button [tk_dialog $window $title $text $icon 0 "Continue"]
	    }
	}
	if {$button != 0} exit
    }


    proc Error {text} {
	Dialog Error $text error 1
    }
    proc Info {text} {
	Dialog Information $text info 0
    }
    proc Warning {text} {
	Dialog Warning $text warning 1
    }

    namespace export Error Info  Warning
}
