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
# @file  mg_evlogpackage.tcl
# @brief API to manage event loggers.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide eventloggers 1.0
package require sqlite3

##
# We export the following procs:
#
# eventlog::add          - Add a new logger to the list.
# eventlog::list         - List defintions of all loggers.
# eventlog::rm           - Remove a logger from the list.
# eventlog::enable       - Enable an individual logger.
# eventlog::enableAll    - Enable all loggers.
#


#  Create the namespace in which the procs live.

namespace eval eventlog {
    
}
#------------------------------------------------------------------------------
# Private procs:

##
# _getOption
#    Gets an option from an option dictionary with a default value:
#
# @param dict  - dictionary value containing options.
# @param key   - Key containing the option.
# @param default - Value to return if the dict does not have the key.
#
proc ::eventlog::_getOption {dict key default} {
    
    if {[dict exists $dict $key]} {
    
        return [dict get $dict $key]
    } else {
    
        return $default
    }
}
##
# ::eventlog::_ensureNotDuplicate
#    Error if the definition of a logger duplicates an existing one with
#    respect to either the ring or the destination
#
# @param db   - Database command.
# @param dest - Recording destination directory (or tree).
# @param src  - Recording source ringbuffer.
#
proc ::eventlog::_ensureNotDuplicate {db dest src} {
    set c [$db eval {
        SELECT COUNT(*) FROM logger WHERE ring = $src OR destination = $dest
    }]
    if {$c > 0} {
        error \
          "There is already a logger defined that has either '$src' as a source \
or '$dest' as a destination"
        
    }
}
##
# ::eventlog::_getContainerId
#    If a container is not null, returns its id.   It is an error for
#    a non-empty container name to not exist.
#
# @param db   - database command.
# @param name - Container name. "" for no container.
# @return mixed
# @retval ""  - If $name eq ""
# @retval integer - Id of the container name if it exists.
#
proc ::eventlog::_getContainerId {db name } {
    if {$name eq ""} {
        return "";                # Null id.
    }
    set existing [::container::listDefinitions $db]
    
    foreach def $existing {
        if {[dict get $def name] eq $name} {
            return [dict get $def id]
        }
    }
    error "There is no container named '$name'"
}
#----------------------------------------------------------------------------
#  Public entry points:

##
# eventlog::add
#     add a new event logger.
# @param db     - Database command.
# @param root   - Root of DAQ (in container if the event logger runs in a container)
# @param source - Data source (ring buffer URI).
# @param destination - Where the event data are stored.
#                 * For a partial logger, this is the director in which the
#                   event files are made.
#                 * For a full logger, this is the top of the managed directory
#                   tree.
# @param options - Dict of options.
#                  The default value of the options parameter is an empty dict.
#                  The keys we pay attention to are:
#        -  host   - Host in which the logger runs.  localhost is the default.#
#        -  partial - Boolean that if true indicates this logger acts like a v11
#                    multi-loggers (that is it does ot attempt to manage the filesystem).
#                    Defaults to false (full management of directory tree)
#        -  critical - Boolean that, if true, indicates that if the logger fails,
#                     a shutdown will be initiated.  True by default.
#        -  enabled - Boolean that, if true, indicates the loggers will log
#                     the next run.  This is true by default.
#        -  container - Name of the container the logger is run in.  This is
#                     empty by default, indicating the logger is run in the
#                     host environment.
# @return id   - Id of the event logger in the table.
#
# @note it is illegal to:
#    -   Have a duplicate destination directory.
#    -   Have a duplicate ringbuffer source.
#    -   Specify a container that does not exist.
# @note that the destination directory is created if it does not yet exist.
#
proc ::eventlog::add {db root source destination {options {}}} {
    
    # Handle the options>
    
    set host [::eventlog::_getOption $options host localhost]
    set partial [::eventlog::_getOption $options partial 0]
    set critical [::eventlog::_getOption $options critical 1]
    set enabled  [::eventlog::_getOption $options enabled 1]
    set container [::eventlog::_getOption $options container ""]
    
    ::eventlog::_ensureNotDuplicate $db $destination $source
    set container [::eventlog::_getContainerId $db $container]
 
 
    $db eval {
        INSERT INTO logger
            (
                daqroot, ring, host, partial, destination,
                critical, enabled, container_id
            ) VALUES (
                $root, $source, $host, $partial, $destination, $critical,
                $enabled, $container    
            )
    }
    return [$db last_insert_rowid]
}
##
# ::eventlog::list
#    Return a list containing the definitions of all event loggers.
#
# @param db  - database command.
# @return list of dicts - Each list element is a dict that describes one
#              event logger.  THe dict keys are:
#        - id      - Id of the logger (Primary key).
#        - daqroot - DAQ Root directory of the logger.
#        - ring    - URI of the data source ring buffer.
#        - host    - Name/IP address of the host in which the logger runs.
#        - partial - Boolean indicating if  the logger is a partial logger.
#        - destination - Destination directory to which events are logged.
#        - critical - Boolean that's true if the logger is critical.
#        - enabled  - True if the logger is currenly enabled.
#        - container- Name of the container.
#        - container_id - Id of the container.
# @note - we use knowledge of the container schema.
#
proc ::eventlog::list {db} {
    set result [list]
    $db eval {
        SELECT logger.id AS loggerid, daqroot, ring, host, partial, destination,
               critical enabled, container_id, container.name As container_name
        FROM logger
        INNER JOIN container ON container_id = container.id
    } values {
        lappend result [dict create                                         \
            id $values($loggerid)      daqroot $values(daqroot)             \
            ring $values(ring)         host $value(host)                    \
            partial $values(partial)   destination $values(destination)     \
            critical $vales(critical)  enabled $values(enabled)             \
            container $values(container_name)                               \
            container_id $values(container_id)                              \
        ]
    }
    
    
    return $result
}
##
# ::eventlog::rm
#   Remove an event logger.
#
# @param db   - Database command.
# @param id   - Id of the logger to remove.
#  It is an error if the logger does not exist.
#
proc ::eventlog::rm {db id} {
    $db eval {DELETE FROM logger WHERE id = $id}
    if {[$db changes] == 0} {
        error "There is no event logger with the id '$id'"
    }
}

    


    


    
