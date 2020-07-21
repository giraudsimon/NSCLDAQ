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
# @file MCFD16Common.tcl
# @brief Common utilities for MCFD16 code.
    
package provide MCFD16Common 1.0

# @author Ron Fox <fox@nscl.msu.edu>
#
namespace eval MCFD16Common {
    ##
    # computeTriggerBits
    #    Given a trigger source/veto, returns the appropriate
    #    value to write as the trigger bits.
    #
    # @param trigid - desired trigger id.
    # @param source - Desired trigger source.
    # @param veto   - Veto bool.
    # @return int   - The bits to write to the trigger register.
    #
    proc computeTriggerBits {trigId source veto} {
        if {$trigId ni [list 0 1 2]} {
            set msg "Invalid trigger id argument provided. Must be 0, 1, or 2."
            return -code error -errorinfo MCFD16USB::SetTriggerSource $msg
        }

        set sourceBits [dict create or 1 multiplicity 2 pair_coinc 4 mon 8 pat_or_0 16 pat_or_1 32 gg 128]
        if {$source ni [dict keys $sourceBits]} {
            set msg "Invalid source provided. Must be or, multiplicity, pair_coinc, mon, pat_or_0, pat_or_1, or gg."
            return -code error -errorinfo MCFD16USB::SetTriggerSource $msg
        }

        set value [dict get $sourceBits $source]
        if {[string is true $veto]} {
            set value [expr {$value + 0x40}]
        }
        return $value
    }
}