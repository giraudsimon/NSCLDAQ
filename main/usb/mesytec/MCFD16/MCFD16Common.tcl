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
    # errorOnInvalidTriggerId
    #   Throws an error if the trigger id is not valid.
    # @param trigId  - id of the trigger.
    #
    proc errorOnInvalidTriggerId trigId {
        if {$trigId ni [list 0 1 2]} {
            set msg "Invalid trigger id argument provided. Must be 0, 1, or 2."
            return -code error -errorinfo MCFD16USB::SetTriggerSource $msg
        }
    }
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
        errorOnInvalidTriggerId $trigId
        
        set sourceBits [dict create or 1 multiplicity 2 pair_coinc 4 mon 8 pat_or_0 16 pat_or_1 32 gg 128]
        if {$source ni [dict keys $sourceBits]} {
            set msg "Invalid source provided. Must be or, multiplicity, pair_coinc, mon, pat_or_0, pat_or_1, or gg."
            return -code error -errorinfo MCFD16XX::SetTriggerSource $msg
	}


        set value [dict get $sourceBits $source]
        if {[string is true $veto]} {
            set value [expr {$value + 0x40}]
        }
        return $value
    }
    ##
    #  getTriggerFields
    #    Given a trigger specification returns the source code
    #    and veto enabled flag.
    #
    # @param code - the code from the device.
    # @return list - [lst source vetoenabled]
    # @note might not look like this is worth it but if the
    #       bit allocation changes in a firmware turn it centralizes
    #       that knowledge.
    #
    proc getTriggerFields {code} {
        set vetoEnabled [expr {($code&0x40)!=0}]
        set source      [expr {$code&0xbf}]
        return [list $source $vetoEnabled]
    }
    ##
    # triggerSourceName
    #    Takes the trigger selector and translates it to a trigger
    #    name
    #
    # @param source - source id.
    # @return string - trigger source name.
    #
    proc triggerSourceName {source} {
        set sourceNameMap [dict create 0 none  1 or 2 multiplicity 4 pair_coinc 8 \
                                    mon 16 pat_or_0 32 pat_or_1 128 gg]
        set sourceName [dict get $sourceNameMap $source]
        return $sourceName
    }
    ##
    # validateTriggerOrId
    #
    #  @param patternId id of the trigger or pattern.
    #  @throw error if the pattern is invalid.
    #
    proc validateTriggerOrId {patternId} {
        if {$patternId ni [list 0 1]} {
            set msg "Invalid pattern id argument provided. Must be 0 or 1."
            return -code error -errorinfo MCFD16XX::SetTriggerOrPattern $msg
        }
    }

}

