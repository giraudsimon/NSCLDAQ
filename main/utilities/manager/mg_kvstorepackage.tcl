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

##
# The public API consists of the following procs:
#
#  kvstore::create    - Create a new key/value - key must not exist.
#  kvstore::modify    - Modify the value of a key - key _must_ exist.
#  kvstore::remove    - Delete a key/value pair -key must exist.

