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
# @file mg_container_wizard.tcl
# @brief Wizard that defines containers in the nscldaq experiment manager.
# @author Ron Fox <fox@nscl.msu.edu>
#
#   Add the DAQTCLLibs to the auto_path

if {[array names env DAQTCLLIBS] ne ""}  {
    lappend auto_path $env(DAQTCLLIBS)
}

package require Tk
package require inifile
package require containers
package require sqlite3
package require snit
package require containeredit



#----------------------------------------------------------------------------
#  Uses a contain description file (see 5daq Container Description File for
#  the format) to make the definition of containers in the manager system
#  easy(ier).
#
#

# List of locations checked for the container description file in the
# order in which they are checked.  Note that if the user supplies a path
# we use that instead.

set DescriptionFilename containers.ini
set DescriptionSearchPaths [list  \
    [pwd] ~ /usr/opt  /non-container
]
  # prepend from the evironment if its defined:
  
if {[array names env CONTAINER_CONFIG] ne ""} {
    set DescriptionSearchPaths \
        [linsert $DescriptionSearchPaths 0 {*}$env(CONTAINER_CONFIG)]
}

## program usage:
#

proc usage {msg} {
    puts stderr "$msg\n"
    puts stderr "Usage:"
    puts stderr "   mg_container_wizard  configdb"
    puts stderr "Where:"
    puts stderr "   configdb is an experiment configuration database"
    puts stderr "Note:"
    puts stderr "  The environmentvariable CONTAINER_CONFIG can be defined to"
    puts stderr "  a properly formatted Tcl list that will be prepended to the"
    puts stderr "  list of directories searched for the containers.ini file"
        
    exit -1
}


##
# locateIniFile
#   Locate containers.ini in the search path provided and return the first match.
#   if not found, usage is called to report the error and exit.
#
# @return filename - path to containers.ini
#
proc locateIniFile { } {
    foreach dir $::DescriptionSearchPaths {
        set path [file join $dir $::DescriptionFilename]
        if {[file readable $path]} {
            return $path
        }
    }
    usage "Unable to find a containers.ini in search path: '::$DescriptionSearchPaths'"
}


#------------------------------------------------------------------------------
#  Ini file processing.

##
# processConfigSection
#    Processes the [CONFIG] section of the init file. This must have two keys:
#    *    native_key - where the /usr/opt's are of each container when
#        not in a container
#    *   container_tree - same but when running a container
#
#  Yes, this requires some consistency in how containers are activated by interactive
#  users
#
# @param ini - ini file handler from ini::open.
# @return list - element 0 is the native tree value and element 1 the container tree
#                value
# @note If one (or both for that matter) of the keys is missing we exit by invoking
#       usage.
#
proc processConfigSection {ini} {
    set missing [list]
    set mandatory [list native_tree container_tree]
    foreach key $mandatory {
        if {![ini::exists $ini CONFIG $key]} {
            lappend missing $key
        }
    }
    if {[llength $missing] > 0} {
        usage "Config file [ini::filename $ini] is missing mandatory keys '$missing' from [CONFIG]"
    }
    
    return [list [ini::value $ini CONFIG native_tree] [ini::value $ini CONFIG container_tree]]
        
    
}

    

##
#  processIniFile
# process the containers.ini file into a dict internal representation.
#
#   The ini file is represented as a dict with the following keys:
#
#  -   native_tree - path to container file systems in native system.
#  -   container_tree - path to container filesystems in containzerized systems.
#  -   containers    - container definitions as a list of dicts each with keys:
#
#  -   name  - container name (section name).
#  -   path  - path to the container image.
#  -   usropt - Path to usropt within the specified tree.
#  -   bindings - list of any additional bindings.
#
# @param path - path to the ini file.
# @return ini file dict as describeda above.
#
proc processIniFile {path} {
    set ini [ini::open $path]
    set result [dict create]
    #  There must be a config section - process it to get the
    #  native_tree and container_tree values:
    #
    
    set keys [ini::sections $ini]
    set configIndex [lsearch -exact $keys CONFIG]
    
    if {$configIndex == -1} {
        usage "Config file: $path is mising mandatory CONFIG sections"
    }
    set containerSections [lreplace $keys $configIndex $configIndex]
    set configContents [processConfigSection $ini]
    dict set result native_tree [lindex $configContents 0]
    dict set result container_tree [lindex $configContents 1]
    
    
    ini::close $ini
    
    return $result
}


#-------------------------------------------------------------------------------
# Entry point
#   - Ensure the user provided a database file and open it.
#   - Locate and read the ini file.
#   - Create an internal representation of the containers described in  the
#     configuration file
#   - Create and populate the GUI.
#

if {[llength $argv] != 1} {
    usage "Invalid number of command line arguments"
}

sqlite3 db $argv

set inifile [locateIniFile]
set containers [processIniFile $inifile]

puts $containers

db close

    
