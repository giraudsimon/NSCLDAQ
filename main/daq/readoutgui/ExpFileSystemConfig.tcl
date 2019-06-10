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

#  This file contains the package that maintains the configuration of the
# experimental file system.
# The filesystem hangs off two roots:
#     Root       ENV        Default
#     StageArea  EVENTS     ~/stagearea
#     Experiment EXPDIR     ~/experiment
# Since it is critical for this configuration to be correct
# and consistent, we don't support programmatic mid-stream
# changes.

package provide ExpFileSystemConfig 1.0
package require Configuration
package require StateManager

namespace eval  ExpFileSystemConfig {

}
# ExpFileSystemConfig::setDefaults
#    Sets the defaulst for the configuration variables.
#    See the file comment header for more information.
#
proc ExpFileSystemConfig::setDefaults {} {
    global env
    set home $env(HOME)
    Configuration::Set StageArea [file join $home stagearea]
    Configuration::Set Experiment [file join $home experiment]
}
# ExpFileSystemConfig::environmentOverrides
#    Override the default settings with environment vars as
#    described in the file header comments.
#
proc ExpFileSystemConfig::environmentOverrides {} {
    global env
    set home $env(HOME)
    Configuration::readEnvironment StageArea   EVENTS [file join $home stagearea]
    Configuration::readEnvironment Experiment  EXPDIR [file join $home experiment]
}
# ExpFileSystemConfig::getStageArea
#   Return the value for the stagearea config param.
#
proc ExpFileSystemConfig::getStageArea {} {
    return [Configuration::get StageArea]
}
# ExpFileSystemConfig::getExperimentDirectory
#   Get the experiment directory configuration parameter.
#
proc ExpFileSystemConfig::getExperimentDirectory {} {
    return [Configuration::get Experiment]
}

#
#   Loading the package initializes it:
#
ExpFileSystemConfig::setDefaults
ExpFileSystemConfig::environmentOverrides


set sm [StateManagerSingleton %AUTO%]
$sm addStateVariable StageArea Configuration::get Configuration::Set

