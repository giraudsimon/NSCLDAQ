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


# Provide a snit object that can produce 
# bells with different patterns:
#    set a [bells -interval nms  -pattern interval-list]
#    a cancel
#
#  -interval  - specifies the interval between repeats of the pattern.
#  -pattern   - specifies the delays each beep in the pattern.
#  a destroy   - stops the bells.
#  a cancel   - stops but can restart with
#  a start
#
package require snit
package require Tk
package provide bells 1.0

snit::type bells {
    option -interval  1000
    option -pattern   100
    variable id

    constructor args {
	$self configurelist $args
	$self start
    }
    destructor {
	$self cancel
    }

    method start {} {
	foreach delay $options(-pattern) {
	    bell 
	    after $delay
       
	}
	bell
	set id [after $options(-interval) [mymethod start]]
    }
    method cancel {} {
	after cancel $id
    }
}
