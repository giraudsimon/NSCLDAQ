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
#             Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file tkutils.tcl
# @brief Utilities that require Tk
# @author Ron Fox <fox@nscl.msu.edu>
#
package require Tk
package provide tkutils 1.0

namespace eval tkutils {
    ##
    # requireArgs
    #    Require a minimum number of args else display an error
    #    tk_messageBox and exit.
    # @param minArgs  - minimum argument count in script.
    # @param title    - error dialog title.
    # @param msg      - Error message text.
    #
    proc requireArgs {minArgs title msg} {
        if {[llength $::argv] < $minArgs} {
            tk_messageBox  -type ok -icon error \
                -title $title -message $msg
            exit -1
        }
    }
    ##
    # requireReadable
    #   Require a file to be readable.  If not display an error
    #   tk_messageBox and exit in error.
    #
    # @param file   - path to requested file.
    # @param title  - message box title
    # @param msg    - the message text
    proc requireReadable {file title msg} {
        if {![file readable $file]} {
            tk_messageBox -type ok -icon error -title $title -message $msg
            exit -1
        }
    }
        
    
}