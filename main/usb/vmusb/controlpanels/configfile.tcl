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
package provide gdgconfigfile 1.0


set moduleList [list]

# Provides procedures to process module configuration files.
# At this point we are only concerned with modules that are
#  jtec/Wiener gate and delay generators and their names.
# however the code is written to be a bit more general.

# Processes the module command. we are only interested in 
#  Module create <type> <name>
# If we find one, we append the type/name to the global
# moduleList

proc Module args {
    global moduleList


    set subcommand [lindex $args 0]
    if {$subcommand ne "create"} {
	return ""
    }
    set type [lindex $args 1]
    set name [lindex $args 2]

    lappend moduleList [list $type $name]
}


# process a configuration file. 
# This just sources the file and returns the moduleList.
#
proc processConfig filename {
    global moduleList
    source $filename
    return $moduleList

}

