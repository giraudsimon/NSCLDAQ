#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

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
# @file scalerUtil.tcl
# @brief Scaler utilities used to factor code duplicated between modules.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide scalerUtil 1.0
package require Utils

##
# formatElapsedTime
#   Turns a number of seconds into an elapsed time of the form>:
#     days hours:min:sec
#
# @param secs  - the seconds to formta
# @return string - The formatted elapsed time.
#
proc formatElapsedTime secs {

    return [Utils::formatDeltaTime $secs]

}

