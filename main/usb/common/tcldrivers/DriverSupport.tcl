

package provide DriverSupport 1.0

namespace eval ::DriverSupport {
}

##
# checkRange
#  Checks the range of a numeric value against optinoal limits.
#  note that this is normally considered an internal proc,
#  however you can call it from outside.
#
# @param value   - The value to check, any numeric type is ok.
# @param low     - If this is not "" this is the smallest value allowed.
# @param high     = If this is not "" this is the largest value allowed.
#
# @throw - Errro if value fails a range check.
#
# @note missing trailing parameters are allowed and treated as "".
#
proc ::DriverSupport::checkRange {value {low ""} {high ""} } {

  if {($low ne "") && ($value < $low)} {
    error "$value must be >= $low"
  }

  if {($high ne "") && ($value > $high)} {
    error "$value must be <= $high"
  }    

}


##
# validInt 
#  Validates an integer value with possible range checking.
#
# @param value - The value to check.
# @param low   - Optional parameter if non-empty the smallest integer value can take.
# @param high  - Optional parameter if non-empty the largest integer value can take.
#
# @return -nothing useful-
#
# @throw - Error if out of range.  If you don't catch this, eventually
#          the XXUSB framework catches it and aborts the run-start.
#
proc ::DriverSupport::validInt {value {low ""} {high ""}} {
    if {![string is integer -strict $value]} {
	error "Invalid integer parameter $value"
    }
    ::DriverSupport::checkRange $value $low $high

}
##
# validReal
#   Validates a real value with possible range checking.'
#
# @param value - The value to check.
# @param low   - The smallest number value can have (inclusive).
# @param high  - The largest number value can have (inclusive).
#
# @return -nothing useful-
#
# @throw - if value is not textually a valid real, or if a limit is nonempty and
#          the value does not make the limit.
#
proc ::DriverSupport::validReal {value {low ""} {high ""}} {
    if {![string is double -strict $value]} {
	error "Invalid real parameter: $value"
    }
    ::DriverSupport::checkRange $value $low $high
}
##
# validEnum
#    Validates an enumerated value to ensure it is in an allowed set:
#
# @param value - The value to check
# @param set   - List of allowed values in the set.
#
# @return -nothing useful-
# 
# @throw - I value is not in set throws an error.
#
# 
proc ::DriverSupport::validEnum {value set} {
    if {$value ni $set} {
	error "$value must be one of {[join $set {, }]}"
    }
}

##
# validBool
#    Validates a boolean value
#
# @param value
#
# @return -nothing useful-
#
# @throw If value does not pass string is boolean -strict
#
proc ::DriverSupport::validBool {value} {
    if {![string is boolean -strict $value]} {
	error "$value is not a valid boolean"
    }
}

##
# validList
#    Validates a list with an arbitrary checker
#
# @param value   - The list of values.
# @param fewest  - Fewest elements the list can have.
# @param most    - Largest no. of elements the list can have.
# @param checker - name of proc that checks the value of each list element
# @param args    - Remainder of the args are passed in to the checker.
#                  e.g checker is callded $checker $value {*}$args
# 
# @return -nothing useful-
#
# @throw If the list size does not fit the requirements of fewest/most.
# @throw Obviously if a list element fails.  These are caught munged a bit
#        to make the message clearer and then rethrown.
#
proc ::DriverSupport::validList {value {fewest ""} {most ""} {checker ""} {args ""} } {
    if {![string is list $value]} {
	error "$value must be a valid Tcl list and is not"
    }

    #  Check the list length constraints:

    set length [llength $value]
    set result [catch {
	::DriverSupport::checkRange $length $fewest $most
    } msg]
    if {$result} {
	error "List length of {$value} invalid: length $msg"
    }

    if {$checker ne ""} {
	foreach element $value {
	    if {[catch {$checker $element  {*}$args} msg]} {
		set errorMessage "Element of {$value} failed type check: $msg"
		error $errorMessage
	    }
	}
    }
}
##
# validIntList:
#    Validates a list of integers.
#   
# @param value   - the list of values
# @param fewest  - If not empty the fewest number of list elements allowed.
# @param most    - If not empty the maximum number of list elements allowed.
# @param low     - If not empty the minimum value for all the list elements.
# @param high    - If not empty, the maximum value for all list elements.
#
# @return -nothing useful-
#
# @throw error if any of the constraints are not met.
#
proc ::DriverSupport::validIntList {value {fewest ""} {most ""} {low ""} {high ""}} {
    DriverSupport::validList $value $fewest $most DriverSupport::validInt $low $high
}
##
# validBoolList 
#   validates a list of booleans:
#
# @param value   - the list of values
# @param fewest  - If not empty the fewest number of list elements allowed.
# @param most    - If not empty the maximum number of list elements allowed.
# @return -nothing useful-
#
# @throw error if any of the constraints are not met.
#
proc ::DriverSupport::validBoolList {value {fewest ""} {most ""}} {
    DriverSupport::validList $value $fewest $most DriverSupport::validBool
}

