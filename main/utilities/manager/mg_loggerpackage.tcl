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
# @file   mg_loggerpackage.tcl
# @brief  Define event logging programs.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide eventloggers 1.0
package require containers
package require sqlite3

namespace eval eventloggers {
    
}

##
# Really this is a set of special purpose programs that are responsible for
#  logging event data from a ring buffer to some chunk of the filesystem.
# There are two types of loggers: Full loggers function very much like the
# NSCLDAQ event loggers in the sense that after a run has been completely written,
# filesystem manipulations are done so that a run view and experiment view
# can be maintained.  In addition, metadata associated with each run can be maintained
# in the run view for that run.
#
# Partial loggers are like the loggers that start with NSCLDAQ-11's multilogger.
# they just maintain event logs as a soup of files in a directory.
#
# The API consists of the following entries:
#
# eventloggers::add    - Add a new eventlogger.
# eventloggers::enableLogger - Sets the enable flag for an event logger.
# eventloggers::disableLogger - Clears the disable flag of a  single logger.
# eventloggers::enable   - Enable recording.
# eventloggers::disable  - Disable recording.
# eventloggers::delete - Remove an event loggers.
# eventloggers::list    - List the defined event loggers
# eventloggers::isEnabled - determines if event logging is enabled.
# eventloggers::startRun  - Call this before taking data to run all the
#                           loggers.
#

#-------------------------------------------------------------------------------
# Private utilities

##
# _requireId
#    Error if a logger with the specified id does not exist:
#
# @param db  - database command.
# @param id  - Id to require.
#
proc ::eventloggers::_requireId {db id} {
    if {![$db eval {SELECT COUNT(*) FROM logger WHERE id=$id}]} {
        error "There is no logger with the id $id"
    }
}
##
# _setEnableFLag
#  @param db   - Database command
#  @param id   - id of logger.
#  @param state - new flag state:
#
proc eventloggers::_setEnableFlag {db id state} {
    $db eval {
        UPDATE logger SET enabled=$state WHERE id=$id
    }    
}

#------------------------------------------------------------------------------
# Public entry points.

##
# eventloggers::add
#    Add a new event loggers.  Loggers all start disabled:
#
# @param db            - database command.
# @param daqrootdir    - Root of NSCLDAQ from which the logger runs.
# @param ring          - URI of ring buffer that will be recorded.
# @param host          - Host in which the event logger will run.
# @param destination  - Where in the filesystem the data are logged.
#                        See partial as the meaning differs depending on the
#                        state of that flag:
#                       *  if true: This is the directory in which the event logs
#                          are all written.
#                       *  if false;  This is the top level of a stagearea.
# @param options        * Options dict with the following optional keys:
#     -  container     - If nonempty, the name of the container in which the
#                        logger runs.  If empty the container runs natively.
#                        note that if the logger runs in a container, the
#                        daqrootdir and destination are relative to the filesystem
#                        seen in the container.
#     -  critical      - If true, the logger is critical (default)
#     -  partial       - If true it's a partial logger. (Default is false).
#     -  enabled       - If true the loggers starts enabled (defaults to true).
#
# @return integer      - id by which the logger can be referred to.
#
proc eventloggers::add {db daqrootdir ring host destination options} {
    # We cannot support duplicate destinations:
    
    if {[$db eval {SELECT COUNT(*) FROM logger WHERE destination = $destination}]} {
        error "There's already a logger putting data in $destination"
    }
    # Process the options
    
    #  Container name if supplied:
    
    set container -1
    if {[dict exists $options container]} {
        set containerName [dict get $options container]
        if {[::container::exists $db $containerName]} {
            set container [dict get \
                [::container::listDefinitions $db $containerName] id]
            
        } else {
            error "$containerName is not the name of an existing container definition."
        }
    }
    # Is  critical?
    
    set isCritical 1
    if {[dict exists $options critical]} {
        set isCritical [dict get $options critical]
    }
    
    # Partial?
    
    set partial 0
    if {[dict exists $options partial]} {
        set partial [dict get $options partial]
    }
    
    # enabled?
    
    set enabled 1
    if {[dict exists $options enabled]} {
        set enabled [dict get $options enabled]
    }
    
    #  Now we're ready to insert the record:
    
    $db eval {
        INSERT INTO logger
        (daqroot, ring, host, partial, destination, critical, enabled, container_id)
        VALUES ($daqrootdir, $ring, $host, $partial, $destination, $isCritical,
                $enabled, $container)
    }
    return [$db last_insert_rowid]
}
##
# eventloggers::enableLogger
#   Enable a single logger given its primary key.
#
# @param db   - database command.
# @param id   - id of the loggers (e.g. returned from add)
#
proc eventloggers::enableLogger {db id} {
    ::eventloggers::_requireId $db $id
    ::eventloggers::_setEnableFlag $db $id 1
    
}
##
# eventloggers::disableLogger
# @param db   - database command.
# @param id   - id of logger.
#
proc ::eventloggers::disableLogger {db id} {
    ::eventloggers::_requireId $db $id
    ::eventloggers::_setEnableFlag $db $id 0
}
##
# eventloggers::enable
#   Enable logging.  This allows loggers that are individually enabled
#   to log data to disk when the next run starts.
#
# @param db -database command.
#
proc eventloggers::enable {db} {
    $db eval {
        UPDATE recording SET state=1
    }
}
##
# eventloggers::disable
#   Same as above but disables.

proc eventloggers::disable {db} {
    $db eval {
        UPDATE recording SET state = 0
    }
}
##
# eventloggers::delete
#   Remove an event logger.
#
# @param db - database command.
# @param id - id of the logger.
#
proc eventloggers::delete {db id} {
    ::eventloggers::_requireId $db $id
    $db eval {
        DELETE FROM logger WHERE id= $id
    }
}
##
# eventloggers::list
#
# @param db - database command.
# @return list of dicts.  Each dict describes one logger with the following keys:
#     - id      - logger identifier.
#     - daqroot - data acquisition root.
#     - ring    - URI of ring.
#     - host    - Host in which the logger runs.
#     - partial  - If nonzero the logger is a partial logger.
#     - destination - The directory that is the target of the logged data.
#     - critical  - If true the logger is critical to the performance of the
#                   experimewnt and failure should result in shutdown.
#     - enabled  - Logger is enabled to run/record.
#     - container - Name of the container in which the logger runs or ""
#                   if there is none.
#     - container_id - id of the container or -1 if there is none.
#
#
proc ::eventloggers::list {db} {
    set result [list]
    $db eval {
        SELECT logger.id, daqroot, ring, host, partial, destination, critical,
            enabled, container_id, container.container
        LEFT JOIN container on container.id = logger.container_id
    }  values {
        lappend result [dict create                                         \
            id         $values(logger.id)       \
            daqroot    $values(daqroot)         \
            ring       $values(ring)            \
            host       $values(host)            \
            partial    $values(partial)         \
            destination $values(destiniation)   \
            critical    $values(critical)       \
            enabled     $values(enabled)        \
            container   $values(container.container) \
            container_id $alues(container_id)   \
        ]
    }
    return $result
}
##
# eventloggers::isEnabled
#   Determines if logging is enabled.
#
# @param db   - database command.
# @return bool - true if so.
#
proc eventloggers::isEnabled {db} {
    return [$db eval {
        SELECT state FROM recording
    }]
}

