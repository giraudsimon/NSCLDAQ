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
# @file   mg_evlogEditModel.tcl
# @brief  Model code needed by the event log editor.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide evlogeditmodel 1.0
package require eventloggers
package require containers
package require snit
package require sqlite3;                #  *We* probably don't need this.

##
# @class evlogEditModel
#   Provide access to the database bits and pieces we need to edit event loggers.
#   This is intended to be used by an evlogEditController.
#
# OPTIONS:
#    *  -database  - sqlite3 command open on the experiment configuration
#                    database.
# METHODS:
#    listContainers   - List the names of the containers that have been defined.
#    listEventLogs    - Return the event log listings.
#    updateEventLogs  - Update the event log data with the listing supplied.
# NOTE:
#    The class is quit minimal.  It only supports wholesale replacement
#    of all the event loggers.  This fits in with the way the editor view works
#    It may not fit in well with other applications.
#
snit::type evlogEditModel {
    option -database
    
    constructor args {
        $self configurelist $args
    }
    #--------------------------------------------------------------------------
    # Private utilities.
    #
    
    ##
    # _clearLoggers
    #   Deletes all logger definitions.
    #
    method _clearLoggers {} {
        set loggers [$self listEventLogs]
        foreach logger $loggers {
            ::eventlog::rm $options(-database) [dict get $logger id]
        }
    }
    ##
    # _addLogger
    #    Given a dictionary definition of a logger adds it to the database.
    #
    # @param def - dictionary definition of a logger as might come from e.g.
    #              eventlog::listLoggers - note the 'id' key is not needed.
    #
    method _addLogger {def} {
        ::eventlog::add $options(-database) \
            [dict get $def daqroot] [dict get $def ring] [dict get $def destination] \
            [dict create                                          \
                host            [dict get $def host]              \
                partial         [dict get $def partial]           \
                critical        [dict get $def critical]          \
                enabled         [dict get $def enabled]           \
                container       [dict get $def container]         \
            ]
    }
    #--------------------------------------------------------------------------
    # Public methods.
    #
    
    ##
    # listContainers
    #     Lists the names of the containers currently defined.
    #
    # @return list of strings.
    # @note This method will fail messily if the database option  is not
    #       properly established first.
    #
    method listContainers {} {
        set containers [::container::listDefinitions $options(-database)]
        set result [list]
        foreach item $containers {
            lappend result [dict get $item name]
        }
        
        return $result
    }
     
    ##
    # listEventLogs
    #    Provides a listing of all of the event log definitions.
    #    this is a thin wrapper to eventlog::listLoggers.
    #
    method listEventLogs {} {
        return [eventlog::listLoggers $options(-database)]
    }
    ##
    # updateEventLogs
    #    Updates eventlogs.  Replaces all logger definitions with the new
    #    ones passed in.  Note this is done within a transaction so it's all
    #    or nothing.
    #
    # @param newDefs  - new logger definitions  This is a list of dicts
    #                   in the form of listEventLogs but the id value,
    #                   if present, is ignored.
    # @note The -database option must have been established or this will
    #       noisily fail.
    #
    method updateEventLogs {newDefs} {
        $options(-database) transaction {
            $self _clearLoggers
            foreach logger $newDefs {
                $self _addLogger $logger
            }
        };                                  # commits if successful.
    }
}




