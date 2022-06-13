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
# @file   mg_kvstorepackage.tcl
# @brief  API to the configuration database key/value store.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide kvstore 1.0
package require sqlite3


namespace eval kvstore {
}
##
# The public API consists of the following procs:
#
#  kvstore::create    - Create a new key/value - key must not exist.
#  kvstore::modify    - Modify the value of a key - key _must_ exist.
#  kvstore::remove    - Delete a key/value pair -key must exist.
#  kvstore::get       - Return the value of a key.
#  kvstore::listKeys  - List the keys in the store.
#  kvstore::listAll   - List the key/value pairs.

#-----------------------------------------------------------------------------
# Private utilities

##
# ::kvstore::_exists
#
# @param db   - database command.
# @param key  - Key to check for.
# @return boolean - true if key exists in the kvstore.
#
proc ::kvstore::_exists {db key} {
    #
    #  The query will return 1 or 0 since we don't allow duplicates.
    #  
    return [db eval {
        SELECT COUNT(*) FROM kvstore WHERE keyname = $key
    }]
}

#------------------------------------------------------------------------------
# Public entries

##
# kvstore::create
#   Create a new key/value pair - the key must not already exist.
#
# @param db   - database command.
# @param key   - New key.
# @param value - Value to associate with that key.
#
proc kvstore::create {db key value} {
    if {![::kvstore::_exists $db $key]} {
        $db eval {
            INSERT INTO kvstore (keyname, value) VALUES ($key, $value)
        }
    } else {
        error "The key $key is already defined."
    }
}
##
# kvstore::modify
#   Modify the value of an existing key.  The key must exist.
# 
# @param db    - the database command.
# @param key   - THe key to modify.
# @param value - the new value to assign to the key.
#
proc kvstore::modify {db key value} {
    if {[::kvstore::_exists $db $key]} {
        $db eval {
            UPDATE kvstore SET value=$value WHERE keyname = $key
        }
    } else {
        error "The key $key is not defined and cannot be modified."
    }
}
##
# kvstore::remove
#   Remove a key from the key value store.  The key must exist.
#
# @param db  - the database command.
# @param key - The key to remove.
#
proc kvstore::remove {db key} {
    if {[::kvstore::_exists $db $key]} {
        $db eval {
            DELETE FROM kvstore WHERE keyname=$key
        }
    } else {
        error "The key $key does not exist and therefore cannot be removed"
    }
}
##
# kvstore::get
#   Return the value of a key from the store.  The key must exist.
#
# @param db     - The database command.
# @param key    - The key to lookup.
# @return string- The value associated with the key.
#
proc kvstore::get {db key} {
    if {[::kvstore::_exists $db $key]} {
        return [$db eval {
            SELECT value FROM kvstore WHERE keyname = $key
        }]
    } else {
        error "Can't get the values of $key because it does not exist"
    }
}
##
# kvstore::listKeys
#    Return the list of keys that are in the database.
#
# @param db   - The database.
# @return list of strings - the keys in the database.
#
proc kvstore::listKeys {db} {
    $db eval {
        SELECT keyname FROM kvstore
    }
}
##
# kvstore::listAll
#   Return the list of keys/value pairs as a  giant dict:
#
# @param db
# return dict - keys of the dict are the kvstore keys and values are their values.
#
proc kvstore::listAll {db} {
    set result [dict create]
    
    $db eval {
        SELECT keyname, value FROM kvstore
    } kv {
        dict set result $kv(keyname) $kv(value)
    }
    
    return $result
}