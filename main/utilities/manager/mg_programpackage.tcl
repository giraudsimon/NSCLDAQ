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
# @file  mg_programpackage.tcl
# @brief Provide and API for programs
#
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide programs  1.0
package require containers
package require sqlite3

##
#  This package provides the api to the programs part fot the NSCLDAQ manager
#  subsystem.  Note that input/output monitoring requires that the host program
#  is running an event loop.
#  This package makes use of the containers API in that a program can optionally
#  be specified to run in a container.
#  It contains the following procs:
#
#  program::exists   - The named program has been defined.
#  program::add      - Adds a new program definition to the database.
#  program::remove   - Removes a program definition from the database.
#  program::run       - Runs a program in the database
#  program::listDefinitions    - List of dicts of program definitions.
#  program::getdef    - Get program definition by name.
#  program::isActive  - Check if a program is active.
#  program::activeContainers - List the containers we activated.
#  program::activePrograms - lists of programs that are running.
#  program::kill      - send a SIGINT to a program to try to kill it.


namespace eval program {
    variable activeContainers [list]; # list of container@host active containers.
    variable acivePrograms [list];    # List of still active programs.
    variable fds;                     # Array of file descriptors indexed by
    array set fds [list];             # Program name.
    
    variable containeredPrograms;        # array indexed by container@host name
    array set containerdPrograms [list]; # Whose elements are lists of programs
                                       ; # Running in that container on that host.
                                       
    ::container::_setup;             # Setup the tempdir.
}
#-------------------------------------------------------------------------------
#  Utilities:
#

##
# program::_typeId
#   @param db   - database command.
#   @param type - Program type string.
#   @return int - The id of the program type.
#   @retval -1  - No match.
#
proc program::_typeId {db type} {
    #
    #  The assumption is there will be 0 or 1 results:
    #
    set result [$db eval {                                   
        SELECT id FROM program_type WHERe type=$type
    }]
    if {$result eq ""} {
        return -1
    } else {
        return $result
    }
}
##
# program::_dictgetOrDefault
#  Get a value from a dict or a default if there is no key
#
# @param dict. - dictionary value.
# @param key   - Key to fetch.
# @param default - default value - defaults to empty string
#
proc program::_dictgetOrDefault {dict key {default {}}} {
    set result $default
    if {[dict exists $dict $key]} {
        set $result [dict get $dict $key]
    }
    return $result
}

#-------------------------------------------------------------------------------
##
# program::exists
#   @param db     - Database command name.
#   @param name   - Name of the program.
#   @return bool - true if name exists, false if not.
#
#
proc program::exists {db name} {
    return [expr \
        {[$db eval {SELECT COUNT(*) FROM program WHERE name=$name}] != 0} \
    ]
}
##
# program::add
#    Adds a new program definition to the database.
#    It  is an error if a program with this name already exists.
#
#  @param  db    - database command used to access the sqlite3 config db.
#  @param  name  - Name of the new program.
#  @param  path  - Path to the program. If the program is run in a
#                  container this path must be its path within the container.
#  @param  type - Type of program (Transient, Critical or Persistent)
#  @param  host  - Host in which the container is run.
#  @param  options - A dict that contains additional program options:
#                   *   container - If supplied this is the name of a container
#                                   and the program will be run inside that container.
#                   *   initscript- If supplied, this is a script of commands
#                                   that will be run prior to the program.
#                                   The contents of that script will be sucked
#                                   into the database.
#                   *   service   - If supplied, this is the daq port manager
#                                   name of a REST service that will be offered
#                                   by that program to participate in state transitions.
#                                   IF not provided but the program is part of the
#                                   state transition sequence old-stlye commands
#                                   will be sent to stdin instead.
#                   *   options   - list of program options of the form
#                                   [list [list optname optvalue]...].
#                   *   parameters - List of command line parameters.
#                   *   environment- list of environment variables to set in the
#                                    form [list [list name value]...].
#                   *   directory - working directory for the program if
#                                   it has specific wd requirements.
#                                   othwerwise the directory will be determined by
#                                   any container init script or be the  home.
#
#      
proc program::add {db name path type host options} {
    if {[program::exists $db $name]} {
        error "The program $name already has a definition"
    }
    # Get the type id corresponsing to the program.
    #
    set typdId [program::_typeId $db $type]
    if {$typeId == -1} {
        error "$type is not a valid program type."
    }
    #  If there's a container in the options get its id:
    
    set containerId ""
    set cname [::program::_dictGetOrDefault $options container]
    if {$cname ne ""} {
        set def [container::listDefinitions $db $cname]
        if {$defe eq ""} {
            error "There is no container named $cname"
        }
        set containerId [dict get $def id]
    }
    set intscript  [::program::_dictGetOrDefault $options initscript]
    set workingDir [::program::_dictGetOrDefault $options directory]
    set service    [::program::_dictGetOrDefault $options service]
    
    #  Now that we have everything we need to do the root record insertion,
    # we start the transaction and get to work shoving crap into the database.
    
    $db transaction {
        $db eval {
            INSERT INTO program
                (name, path, type_id, host, directory, container_id, initscript, service)
                VALUES ($name, $path, $typeId, $host, $workingDir,
                        $containerId, $initscript, $service
                )
        }
        set pgmid [$db last_insert_rowid]
        
        set options [::program::_dictGetOrDefault $options options]
        foreach $option $options {
            set name [lindex $option 0]
            set value [lindex $option 1]
            $db eval {
                INSERT INTO program_option (program_id, option, value),
                VALUES ($pgmid, $name, $value)
            }
        }
        set parameters [::program::_dictGetOrDefault $options parameters]
        foreach parameter $parameters {
            $db eval {
                INSERT INTO program_parameter (program_id, parameter)
                VALUES ($pgmid, $parameter)
            }
        }
        
        set env [::program::_dictGetOrDefault $options environment]
        foreach var $env {
            set name [lindex $env 0]
            set value [lndex $env 1]
            $db eval {
                INSERT INTO program_environment (program_id, name, value)
                VALUES ($pgmid, $name, $value)
            }
        }
    }
}
##
# program::remove
#   Remove an existing program from the database.
#   this requires a transaction as we're going to clean up the program_option,
#   program_parameter and program_environment tables too (Even though strictly
#   speaking, once the root is gone it's all gone as row ids are not recycled).
#
# @param db   - database command.
# @param name - Name of the program to remove.
# @note It is an error to remove a program that does not exist.
#
proc program::remove {db name} {
    if {![::program::exists $db $name]} {
        error "There is no program named $name"
    }
    
    
}
