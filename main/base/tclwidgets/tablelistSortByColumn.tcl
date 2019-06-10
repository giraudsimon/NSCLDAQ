#==============================================================================
# Contains the implementation of the tablelist::sortByColumn command.
#
# Copyright (c) 2000-2002  Csaba Nemethi (E-mail: csaba.nemethi@t-online.de)
#==============================================================================
package provide tablelistsort 1.0
namespace eval tablelist {}
#------------------------------------------------------------------------------
# tablelist::sortByColumn
#
# Sorts the contents of the tablelist widget win by its col'th column.  Returns
# the sorting order (increasing or decreasing).
#------------------------------------------------------------------------------
proc tablelist::sortByColumn {win col} {
    #
    # Check the arguments
    #
    if {![winfo exists $win]} {
	return -code error "bad window path name \"$win\""
    }
    if {[string compare [winfo class $win] Tablelist] != 0} {
	return -code error "window \"$win\" is not a tablelist widget"
    }
    if {[catch {::$win columnindex $col} result] != 0} {
	return -code error $result
    }
    if {$result < 0 || $result >= [::$win columncount]} {
	return -code error "column index \"$col\" out of range"
    }

    #
    # Determine the sorting order
    #
    set col $result
    if {$col == [::$win sortcolumn] &&
	[string compare [::$win sortorder] increasing] == 0} {
	set sortOrder decreasing
    } else {
	set sortOrder increasing
    }

    #
    # Sort the widget's contents based on the given column
    #
    if {[catch {::$win sortbycolumn $col -$sortOrder} result] == 0} {
	return $sortOrder
    } else {
	return -code error $result
    }
}
