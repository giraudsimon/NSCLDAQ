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


# Handle initializing configurations of multiple components.
# The idea of this package is that a program may consist of an
# assemblage of several components, each with its own set of
# configuration variables and configuration files/environment
# variables to set them up.  Each configuration is represented
# by a package and a namespace in which at least the following
# 2 procs are defined
#
#   setDefaults   - Sets any default values for configuration
#                   variables managed by this package.
#   environmentOverrides - Reads the environment to override
#                   any defaults.
# Configuration files are assumed to override env variables.
#
#  We support the addition of packages, and configuration files
#  and finally the ordered configuration of all of these.
#
package provide InitializeConfiguration 1.0
package require Configuration

namespace eval InitializeConfiguration {
    variable packageList {}
    variable configurationFiles {}
}

# InitializeConfiguration::addSubsystem name
#     Adds a component (subsystem) to the list of configurable
#     packages.  The name must refer to a package in the
#     namespace $name that  defines procs like
#     setDefaults and environmentOverrides.
#
# Parameters:
#   name   - Name of the package.
# Errors:
#   NoSuchPackage if the package 'name' cannot be loaded.
#
proc InitializeConfiguration::addSubsystem name {
    if {[catch {package require $name}]} {
        error "InitializeConfiguration::NoSuchPackage - configuration package $name could not be found in tcl library path"
    }
    lappend InitializeConfiguration::packageList $name
}
# InitializeConfiguration::addConfigFile path
#    Adds a configuration file to the list of config files
#    sourced in at initialization time.  The file
#    must exist and be readable.
# Parameter:
#    path   - path to the configuration File
# Errors:
#   InitializeConfiguration::FileNotFound
#   InitializeConfiguration::FileNotReadable
#
proc InitializeConfiguration::addConfigFile path {
    if {![file exists $path]} {
        error "InitializeConfiguration::FileNotFound - Configuration file $path could not be found"
    }
    if {![file readable $path]} {
        error "InitializeConfiguration::FileNotReadable - Configuration file $path is not readable by this user"
    }
    lappend InitializeConfiguration::configurationFiles $path

}

# initialize
#    Initialize the configuration system for all packages and
#    configuration files.
#
proc InitializeConfiguration::initialize {} {


    # First bring the configuration namespaces online.
    # and set defaults:
    foreach pkg $InitializeConfiguration::packageList {
	package require $pkg
        set def ""
        append def $pkg :: setDefaults
        $def
    }



}

					   
# Process the configuration files for each pacakge:

proc InitializeConfiguration::processConfigFiles {} {
    # Read the configuration files.

    foreach f $InitializeConfiguration::configurationFiles {
	Configuration::readConfigFile $f
    }
}

#
#  Process the env variables for each configuration.
#
proc InitializeConfiguration::processEnv {} {
    # Finally process env vars.


    foreach pkg $InitializeConfiguration::packageList {

        set envo ""
        append envo $pkg :: environmentOverrides
        $envo
    }

}