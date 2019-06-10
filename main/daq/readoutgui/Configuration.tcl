#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#

#
#  Configuration.tcl
#     This file contains code that manages configuration variables.
#     Configuration variables will live inthe configuration namespace.
#     ::configuration:: is not to be confused with ::Configuration::
#     which is where the procs in this package live.
#     They can be loaded from file, read from the environment and handed
#     explicit values.
#     Once given value, they can be retrieved, or modified, and written.
#     We do all this in a way that allows users to just to set commands to
#     set configuration values, rather than anything funky.
#
package provide Configuration 1.0

namespace eval configuration {};            # Config vars live here.

namespace eval Configuration {
    variable ConfigFile
};            # Procs live here.

# Configuration::readConfigFile   path
#     Read a configuration file.  The indicated configuration file
#     is read in at global level inside a namespace eval ::configuration.
# Parameters:
#    path           Filesystem path to the configuration file.
# Errors:
#    FileNotFound    - Nosuch file.
#    FileNotReadable - File found but could not be read.
#
proc Configuration::readConfigFile {path} {
    if {![file exists $path]} {
        error "Configuration::FileNotFound - Attempting to read configurationfile: $path"
    }
    if {![file readable $path]} {
        error "Configuration::FileNotReadable - $path was not readable by this user"
    }
    set Configuration::ConfigFile $path
    uplevel #0 {
        namespace eval ::configuration:: {
            source $Configuration::ConfigFile
        }
    }

}
# Configuration::readEnvironmet  confname envname default
#    Sets a configuration variable from an environment variable.
#    if the environment variable has not been set, a default is applied.
# Parameters:
#   confname  - Name of the configuration variable.
#   envname   - Name of the environment variable.
#   default   - Default value for the configuration variable.
#               This defaults to empty.
#
#
proc Configuration::readEnvironment {confname envname {default {}}} {
    global env

    if {[array names env $envname] == "" } {
        if {$default ne ""} {
	    set ::configuration::$confname $default
	}
    } else {
        set ::configuration::$confname $env($envname)
    }
}
# Configuration::Set confname value
#     Provides a value for a configuration variable,
#     creating it if needed.
# Parameters:
#   confname - Name of configuration valuel
#   value    - New value.
#
proc Configuration::Set {confname value} {
    set ::configuration::$confname $value
}
# Configuration::get  confname default
#    Retrieve the value of a configuration variable.
# Parameters:
#     confname    - name of configuration variable.
#     default     - Defautl value to return if not
#                   defined. Defaults to empty string.
#
# Returns:
#      Value of the config variable after all defaulting.
#
proc Configuration::get {confname {default {}}} {
    if {[info var ::configuration::$confname] != ""} {
        return [set ::configuration::$confname]
    } else {
        return $default
    }
}
# Configuration::writeConfigFile channel
#      Writes all the variables in the configuration namespace
#      to the file open on the channel.  The file is written
#      in such a way that it can be recovered as a configuration file.
#      This may seem trivial, but remember we have to get rid of
#      the namespace from the configuration variable names.
# Parameters:
#    channel   - Channel open on the output file.
#
proc Configuration::writeConfigFile channel {
    set names [info var ::configuration::*]
    foreach var $names {
        set value [set $var]
        set list [split $var :]
        set varname [lrange [split $var :] 4 end]
	set varname [join $varname ::]
        puts $channel [list set $varname $value]
    }
}
