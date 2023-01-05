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
# @file mg_programserver.tcl
# @brief REST server for /Program domain.
# @author Ron Fox <fox@nscl.msu.edu>
#
lappend auto_path $::env(DAQTCLLIBS)
package require restutils
package require  json::write
package require  sqlite3
package require  programs
package require  containers
    

Url_PrefixInstall /Programs [list programHandler]

##
# _makeProgramsArray
#    Creates the list of JSON objects representing programs.
#
# @param db - databse object command.
# @return list of JSON objects.
#
proc _makeProgramsArray {db} {
    set result [list]
    set defs [::program::listDefinitions $db]
    
    foreach def $defs {
        set name [dict get $def name]
        set path [dict get $def path]
        set type [dict get $def type]
        set host [dict get $def host]
        set cont ""
        if {[dict exists $def container_name]} {
            set cont [dict get $def container_name]
        }
        set active 0
        if {[::program::isActive $name]} {
            set active 1
        }
        lappend result [json::write object                          \
            name [json::write string $name]                         \
            path [json::write string $path]                         \
            type [json::write string $type]                         \
            host [json::write string $host]                         \
            container [json::write string $cont]                    \
            active $active                                          \
        ]
    }
    
    return $result
}
##
# _transformBindings
#    Take the bindings list and turn it into a list of elements of the
#    form in the --bind option to singularity e.g.
#   -  hostfs
#   - hostfs:targetfs
#
# @param raw   - bindings list as it comes from container::list.
# @return list - list of bindings as described above.
#
proc _transformBindings {raw} {
    set result [list]
    
    foreach binding $raw {
        if {[llength $binding] == 1} {
            lappend result [json::write string $binding]
        } else {
            lappend result [json::write string            \
                [lindex $binding 0]:[lindex $binding 1]   \
            ]
        }
    }
    
    return $result
}

##
# _listActivations
#
#  @param name - container name.
#  @return list - list of hosts on which the container has been activated.
#
proc _listActivations {name} {
    set result [list]
    set activations [::program::activeContainers]
    
    foreach activation $activations {
        set cname [lindex $activation 0]
        set host [lindex $activation 1]
        if {$name eq $cname} {
            lappend result [json::write string $host]
        }
    }
    
    return $result
}

##
# _makeContainersArray
#    Make the list of JSON container objects.
#
# @param db   - database parameter.
# @return list - list of container objects that will be rendered as an array.
#
proc _makeContainersArray {db} {
    set result [list]
    set defs [container::listDefinitions $db]
    foreach def $defs {
        set name [dict get $def name]
        set image [dict get $def image]
        set bindings [_transformBindings [dict get $def bindings]]
        set activations [_listActivations $name]
        
        lappend result [json::write object                          \
            name [json::write string $name]                         \
            image [json::write string $image]                       \
            bindings [json::write array {*}$bindings]               \
            activations [json::write array {*}$activations]         \
        ]
    }
    return $result
}


##
# programHandler
#    Handles domain requests in the /Programs domain.
#    presently only the /status subdomain is recognized.
#
# @param sock - socket object the request is along.
# @param suffix - URL suffix (e.g. /status).
#
proc programHandler {sock suffix} {
    if {$suffix eq "/status"} {
        
        set programs [_makeProgramsArray db]
        set containers [_makeContainersArray db]
        
        Httpd_ReturnData $sock application/json [json::write object     \
            status [json::write string OK]                              \
            message [json::write string ""]                             \
            containers [json::write array {*}$containers]               \
            programs   [json::write array {*}$programs]                 \
        ]
    } else {
        ErrorReturn $sock "$suffix subcommand not implemented"
    }
}
#-------------------------------------------------------------------------------
#  Initialization:
#   Get the containers and hosts associated with all programs.
#   Use them to shutdown all containers that might be running.
#

set opened 0
if {[info commands db] eq ""} {
    sqlite3 db $::env(DAQ_EXPCONFIG)
    set opened 1
}
set programs  [program::listDefinitions db]

# Indexed by  host - list of containers  that can be in that host.
#
array set containerArray [list]

foreach program $programs {
    if {[dict exists $program container_name]} {
        lappend containerArray([dict get $program host]) [dict get $program container_name]
    }
}
#  for each  host that could have a container, uniquify the container names
#  and stop the containers in that host using deactivate:

foreach host [array names containerArray] {
    array set containers [list]
    foreach container $containerArray($host) {
        set containers($container) ""
    }
    set containerList [array names containers]
    
    # Deactivate the containers:
    
    foreach c $containerList {
        container::deactivate $host $c
    }
}
    

if {$opened} {
    db close
}

    



