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
# decodeBindings
#   Given a bindings value from the configuration file,
#   decode it into the expected list.
#
# @param bindings - raw bindings value from the file.
# @return list of bindings.
#
proc decodeBindings {bindings} {
    set result [list]
    set bindingsList [split $bindings ,];   # list of bindings
    foreach binding $bindingsList {
        lappend result [split $binding :]
    }
    return $result
}
##
#  processContainer
#     Process a container section.  This has mandatory keys:
#
#     - path - the host path to the container image.
#     - usropt - the path to the directory to be bound into /usr/opt relative to
#                native_tree in the [CONFIG] section
#
#  Additionally an optional section:
#
#    - bindings - can provide additonal bindings in the form recognized by
#      sigularity's --bind  option.
#
# @param ini - handle open on the ini file.
# @param name - container/section name.
# @return dict with the following keys:
#     -   name - section/container name.
#     -   path - container path.
#     -   usropt - path to /usr/opt.
#     -   bindings a possbly empty list of bindings 
# @note if manadtory key(s) are missing usage is called to abort the program.
#
proc processContainer {ini name} {
    #  Ensure all mandatory keys are present in the section:
    
    set missing [list]
    set mandatory [list path usropt]
    foreach key $mandatory {
        if {![ini::exists $ini $name $key]} {
            lappend missing $key
        }
    }
    if {[llength $missing] > 0} {
        usage "Config file [ini::filename $ini] definition of container $name is missing mandatory keys: '$missing'"
    }
    
    # Marshall the mandatory keys and name into the result:
    
    set result [dict create                   \
        name $name                            \
        path [ini::value $ini $name path]     \
        usropt [ini::value $ini $name usropt] \
    ]
    # If there's a bindings key get it and marshall it into a list of bindings:
    
    set bindings [list]
    if {[ini::exists $ini $name bindings]} {
        set rawBindings [ini::value $ini $name bindings]
        set bindings [decodeBindings $rawBindings]
    }
    

    dict set result bindings $bindings
    return $result
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
#  -   bindings - list of any additional bindings.  Each element is a one or
#       two element list - one element if the bindpoint is the same as the
#       host path, two elements if not and the second element is the bind point for
#       the first.
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
    set configContents [processConfigSection $ini]
    dict set result native_tree [lindex $configContents 0]
    dict set result container_tree [lindex $configContents 1]
    
    # Process the containers now:
    
    set containerSections [lreplace $keys $configIndex $configIndex]
    foreach container $containerSections {
        set containerDict [processContainer $ini $container]
        dict lappend result containers $containerDict
        
    }
    
    ini::close $ini
    
    return $result
}
#-----------------------------------------------------------------------------
#  User interface items:
#

##
# @class container::WizardChooser
# Select container and NSCLDAQ version:
#
#  +----------------------------------------------+
#  |  container                  nscldaq          |
#  | +----------------+   +--------------------+  |
#  | |  container list|   | DAQ version list   |  |
#  | |  ...           |   |                    |  |
#  | +----------------+   +--------------------+  |
#  |  selected-container   selected-daq-version   |
#  +----------------------------------------------+
#
# Options:
# *    -model - contains the data used to drive the widget.  This is a
#             list of pairs.  The first item in each pair is the name of a container
#             The second item in the list is a list of the NSCLDAQ versions found
#             for the usr opt that container has.
#      -container - readonly retreives the selected container.
#      -daqversion - reaodonly retrieves the selected daq version.
#
#  Events:
#   *  <Double-1> on the container list:
#       -   Loads the selected container
#       -   Clears the selected-daq-verison
#       -   Fills the versions listbox from the versions the model selected supports.
#
#   *  <Double-1> on the daq version list box; loads the selection in to
#      selected-daq-version.
# 
# Note:
#   Normally this widget is part of some larger thing that, when the user is
#   happy with what they have goes on to allow them make changes to the container.
#
snit::widgetadaptor container::WizardChooser {
    option -container -readonly 1 -cgetmethod _getvalue
    option -daqversion -readonly 1 -cgetmethod _getvalue
    option -model -default list -configuremethod _stockcontainer
    constructor {args} {
        installhull using ttk::frame
        
        ttk::label $win.containers -text Containers:
        ttk::label $win.daqversions -text {NSCLDAQ versions:}
        
        ::listbox $win.containerlist  \
            -yscrollcommand [list $win.containerscroll set] \
            -selectmode single
            
        ttk::scrollbar $win.containerscroll -orient vertical -command [list $win.containerlist yview]
        ::listbox $win.versionlist \
            -yscrollcommand [list $win.versionscroll set] \
            -selectmode single
        ttk::scrollbar $win.versionscroll -orient vertical -command [list $win.versionlist yview]
        
        ttk::label $win.selcontainer -text {}
        ttk::label $win.seldaq       -text {}
        
        grid $win.containers -row 0 -column 0
        grid $win.daqversions -row 0 -column 2
        
        grid $win.containerlist $win.containerscroll \
            $win.versionlist $win.versionscroll \
                    -sticky nsew -padx 2
        
        grid $win.selcontainer -row 2 -column 0
        grid $win.seldaq  -row 2 -column 2
        
        $self configurelist $args
        
        #  Bind double click on the two list boxes:
        
        bind $win.containerlist <Double-1> [mymethod _selectContainer]
        bind $win.versionlist   <Double-1> [mymethod _selectVersion]
        
    }
    ##
    #  cgetters:
    #
    
    ##
    # _getvalue
    #  @param cname -configuration  name- must be one of -container or -daqversion.
    # @return value in the specified text field.
    #
    method _getvalue {cname} {
        if {$cname eq "-container"} {
            return [$win.selcontainer cget -text]
        } elseif {$cname eq "-daqversion"} {
            return [$win.seldaq cget -text]
        } else {
            error "invalid cget in _getvalue"
        }
    }
    ##
    # csetters:
    
    ##
    # _stockcontainer
    #   Called when the model is reconfigured.  Clear both list boxes
    #   and both selection.  Stock $win.containerlist with the list of
    #   containers in the model.
    #
    # @param cname - "-model"
    # @param value  - new model.
    #
    method _stockcontainer {cname value} {
        set options($cname) $value ;     # So cget works inter-alia.
        
        # Clear the UI:
        
        $win.containerlist delete 0 end
        $win.versionlist delete 0 end
        $win.selcontainer configure -text {}
        $win.seldaq configure -text {}
        
        #  Stock containers:
        
        foreach item $value {
            set container [lindex $item 0]
            $win.containerlist insert end $container
            
        }
    }
    ##
    #  event handling.
    
    
    ##
    # _selectContainer
    #    Processes a double click on an element of the container list.
    #
    method _selectContainer {} {
        set selection [$win.containerlist curselection]
        if {[llength $selection] > 0} {
            set selectedContainer [$win.containerlist get $selection]
            $win.selcontainer configure -text $selectedContainer
            $win.seldaq configure -text {}
            
            set versions [$self _getVersions $selectedContainer]
            $win.versionlist delete 0  end
            foreach version $versions {
                $win.versionlist insert end $version
            }
        }
    }
    
    ##
    # Utilties:
    
    ##
    # _getVersions
    #   @param name   - container name.
    #   @return list - versions of NSCLDAQ supporte according to the model.
    #
    method _getVersions name {
        
        set index [lsearch -exact -index 0 $options(-model) $name]
        
        if {$index == -1} {
            error "BUG - no container match in model for $name"
        }
        return [lindex [lindex $options(-model) $index] 1]
    }
}
 
##
# probeDaqVersions
#    Determine the set of versions of NSCLDAQ a container's /usr/opt has
# @param hosttree  - tree containing the /usr/opt dirs in native system.
# @param containertree - where hosttree is bound into the running container.
# @param path     - Path inside either hosttree or container_tree that has daq.
# @return list of daq version directories.
# @note we don't probe subdirectories that contain experimental DAQ systems.
#
proc probeDaqVersions {hosttree containertree path} {
    return [list]
}
    
 
##
# createChooserModel
#   Given the configuration dict, create the model that describes the
#   containers and their DAQ systems for the WizardChooser.
#
# @param config  - the configuration dict.
# @return list   - see WizardChooser for the shape of this list.
#
proc createChooserModel {config} {
   set hosttree [dict get $config native_tree]
   set ctree    [dict get $config container_tree]
   
   set result [list]
   
   foreach container [dict get $config containers] {
        set name [dict get $container name]
        set path [dict get $container path]
        set versions [probeDaqVersions $hosttree $ctree $path]
        
        lappend result [list $name $versions]
   }
   
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


#  Make the model for the wiaard chooser:

set model [createChooserModel $containers]

container::WizardChooser .chooser -model $model
pack .chooser


    
