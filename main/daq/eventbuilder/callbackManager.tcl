#
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



##
#  This package provides a callback manager.
#  The idea is that callbacks can be registered by name and then invoked
#  along with a set of substitution variables and their value.
# 
package require snit
package provide EVB::CallbackManager 1.0

namespace eval EVB {}

##
# The callback manager:
#
# OPTIONS:
#
# METHODS:
#  define   - Defines a new callback name
#  register - Registers a callback.
#  get      - Returns the callback associated with a name.
#  invoke   - Invokes a callback.
#
snit::type EVB::CallbackManager {
    variable callback -array [list]

    #-----------------------------------------------------------------
    #
    #   Public methods:

    

    ##
    # Define a new callback.
    #
    # @param cbName - name of the callback.
    #
    method define cbName {
	if {![catch {$self _CheckValidCallback $cbName}]} {
	    error "Duplicate callback"
	}
	set callback($cbName) [list]
    }
    ##
    # Register a callback handler .
    #
    # @param cbName - Name of the callback (must have been defined).
    # @param script - Script to run for the callback.
    #
    method register {cbName script} {
	$self _CheckValidCallback $cbName
	set callback($cbName) $script
    }
    ##
    # Return the script associated with a callback.
    #
    # @param cbName - name of the callback.
    #
    method get {cbName} {
	$self _CheckValidCallback $cbName
	return $callback($cbName)
    }
    ##
    # Invoke a callback script.
    #
    # @param cbName - name of the callback.
    # @param substitutions - a list of substitution strings.
    # @param values  - For each substituion string the value that will
    #                  be substituted for it each time it occurs in the
    #                  callback script.
    # @note The callback is invoked at the global level.
    #
    method invoke {cbName substitutions values} {
	$self _CheckValidCallback $cbName
	set command $callback($cbName)
	if {$command ne ""} {
	    set command [$self _Substitute $command $substitutions $values]
	    uplevel #0 $command
	}
    }

    #----------------------------------------------------------
    # Private methods:

    ##
    # Throw an error if a callback name is not value.
    # @param cbName - the name of the callback.
    #
    method _CheckValidCallback cbName {
	if {[array names callback $cbName] eq ""} {
	    error "No such callback"
	}
    }
    #
    # Does a set of global substitutions on a string.
    #
    # @param string - input string.
    # @param patterns - The patterns that will be modified.
    # @param values   - The new values for each pattern.
    # @return string with substitutions applied.
    method _Substitute {string patterns values} {
	foreach pattern $patterns value $values {
	    set string [regsub -all $pattern $string [list $value]]
	}
	return $string
    }
}
