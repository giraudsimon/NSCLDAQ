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
# @file nameMap.tcl
# @brief Provide a name to object mapping generic class.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide namemap 1.0
package require snit

##
# @class nameMap
#
#   Provide a mapping from names to values (like an array but with 'nicer' error'
#   semantics)
#
# METHODS:
#
#   add key value - adds a new key/value pair.
#
snit::type nameMap {
    variable mapping -array [list]
    constructor args {

    }
    ##
    # add
    #   Add a new key value pair.
    #
    #  @param key - The keyword.
    #  @param value - The value
    #  @return [list $key $value]
    #  @throw error if key already is in the map (DUPKEY - Duplicate key).
    #
    method add {key value} {
        if {[array names mapping $key] eq "" } {
            set mapping($key) $value
            return [list $key $value]
        }
        error "$key already exists" nameMap::add DUPKEY
    }
    ##
    # get
    #   Get the value of a key:
    #
    # @param key - The key whose value should be fetched.
    # @return the value fetched.
    # @throw NOSUCHKEY - if there is no such key in the map.
    #
    method get {key} {
        if {[array names mapping $key] eq ""} {
            error "$key has no mapping" nameMap::get NOSUCHKEY
        }
        return $mapping($key)
    }
    ##
    # list
    #   Return a list of the names that have mappings:
    #
    method list {} {
        return [array names mapping]
    }
}
