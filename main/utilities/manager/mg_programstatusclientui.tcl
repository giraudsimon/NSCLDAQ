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
# @file mg_pgmstatusclientui.tcl
# @brief Provides megawidgets to display program status information.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide pgmstatusclientui 1.0

package require Tk
package require snit

##
# @class ContainerStatusList
#    Provides a list of containers and their activations.
#    This is presented as a ttk::treeview with the following columns:
#    *  name - name of the container.
#    *  image - path to containe image
#    *  binding - filesystem binding (see below).
#    *  host    - activation host (see below).
#
#   Each container is represented as a top level item in the tree with
#   two sub elements called "bindings" and "activations"
#   The bindings subelement has subelements for each binding and those
#   subelements populate the binding column.
#   The activations subelement has subelelements for each activation of the
#   container and those populate the host column.
#
# Naturally this megawidget provides a vertical scroll bar.
#
# OPTIONS:
#   -containers   - list of container definition dicts gotten from e.g.
#                   a call to ProgramClient::status on an instance.
# METHOD:
#   addActivation - Adds an activation to a container.
#   removeActivation - Removes an activation from a container.
#
#  The relative paucity of methods is because the things that are most likely
#  to vary dynamically are the activations of a container and we want to
#  be able to update those without the user seeing the entire tree close as it
#  would if we re-generated the tree from scratch.
#
snit::widgetadaptor ContainerStatusList {
    option -containers -default [list] -configuremethod _cfgContainers
    component tree
    constructor {args} {
        installhull using ttk::frame
        
        set colnames [list  image binding host]
        set coltitles [list Image Binding Host]
        
        install tree using ttk::treeview $win.tree                 \
            -columns $colnames                                     \
            -displaycolumns $colnames -show [list tree headings]    \
            -selectmode none -yscrollcommand [list $win.ysb set]
        foreach col $colnames title $coltitles {
            $tree heading $col -text $title
        }
        $tree heading #0 -text Container;   # Label the tree header.
        
        ttk::scrollbar $win.ysb -orient vertical -command [list $tree yview]
        
        grid $tree $win.ysb -sticky nsew
        grid columnconfigure $win 0 -weight 1;   # Allow the treeview to grow
        grid columnconfigure $win 1 -weight 0;   # But not the scrollbar.
        
        $self configurelist $args
        
    }
    #------------------------------------------------------------------------
    # Private utility methods.
    #
    
    ##
    # _addContainer
    #    Add a container to the end of the set of things in the tree.#
    #      Top level has name, image, bindings where name is the tree.
    #      second level is bindings and activations in the tree level
    #      etc.
    #
    # @param item - dictionary item that describes the container to add.
    #
    method _addContainer {item} {
        set name [dict get $item name]
        set image [dict get $item image]
        
        set toplevel [$tree insert {} end -text $name -values $image -tags container]
        
        #  The bindings:
        
        set bindings [$tree insert $toplevel end -text bindings -tags bindcontainer]
        foreach binding [dict get $item bindings] {
            $tree insert $bindings end -values [list "" $binding] -tags binding
        }
        # Any activiations:
        
        set activations [$tree insert $toplevel end -text activations -tags activationcontainer]
        foreach host [dict get $item activations] {
            $tree insert $activations end -values [list "" "" $host] -tags activation
        }  
    }
    ##
    # _findContainer
    #    Find the entry that corresponds to the named container.
    #
    #  @param name - name of the container.
    #  @return entry - entry id of the container top level.
    #  @note it's an error for there not to be a match.
    #
    method _findContainer {name} {
        foreach item [$tree children {}] {
            if {[$tree item $item -text] eq $name} {
                return $item
            }
        }
        error "No such container: $name"
    }
    ##
    # _findActiviations
    #    Find the subelement of a container that holds the activations.
    #
    # @param c  - container item (e.g. from _findContainer).
    # @return entry - activations entry.
    # @note it's an error for there not to be an activation entry.
    #
    method _findActivations {c} {
        foreach sub [$tree children $c] {
            if {[$tree item $sub -text] eq "activations"} {
                return $sub
            }
        }
        error "The container [$tree item $c -text] does not have an activations subtree"
    }
    ##
    # _addOptionActivation
    #
    #  Searches the option list for the named container and
    #  lappends an activation record to it.
    #
    # @param name   - container name.
    # @param host   - Host the container has been activated on.
    # @note it is an error 'name' does not exist.
    #
    method _addOptionActivation {name host} {
        set containers $options(-containers)
        set index 0
        foreach container $containers {
            if {[dict get $container name] eq $name} {
                dict lappend container activations $host
                lset containers $index $container
                set options(-containers) $containers
                return 
            }
            incr index
        }
        error "The -containers option has no match to $name"
    }
    
    ##
    # _addTreeActivation
    #
    #    Searchs the tree for matching named container and adds
    #    an activation record to it.
    #
    # @param name - name of the container.
    # @param host - Host of new activation.
    # @note it is an error if does not exist.
    #
    method _addTreeActivation {name host} {
        set child [$self _findContainer $name]
        set subtree [$self _findActivations $child]
        $tree insert $subtree end  -values [list "" "" $host] -tags activation    
        return
    }
    
    ##
    # _removeOptionActivation
    #   Removes an activation entry from the options.
    #
    # @param name   - name of the container.
    # @param host   - host in which it's being deactivated.
    # @note name must be an existing container active on host.
    #
    method _removeOptionActivation {name host} {
        set containers $options(-containers)
        set index 0
        foreach container $containers {
            if {[dict get $container name] eq $name} {
                set activations [dict get $container activations]
                
                set which [lsearch -exact $activations $host]
                if {$which == -1} {
                    error '$name is no active on $host
                }
                
                set activations [lreplace $activations $which $which]
                dict set container activations $activations
                lset containers $index $container
                
                set options(-containers) $containers
                
                return
            }
            incr index
        }
        error "There is no container named $name in -containers"
    }
    ##
    # _removeTreeActivation
    #   Removes an activation entry from a container in the tree.
    #
    # @param name - container name.
    # @param host - host in which it's being deactivated.
    #
    method _removeTreeActivation {name host} {
        set child [$self _findContainer $name]
        set sub   [$self _findActivations $child]
    
        foreach a [$tree children $sub] {
            set h [lindex [$tree item $a -values] 2]
            if {$h eq $host} {
                $tree delete $a
                return
            }
        }
        #  No activation:
        
        error "Tree has no activation on $host of $name"

}
    
    
    #------------------------------------------------------------------------
    #  Configuration handling
    
    ##
    # _cfgContainers
    #    Called when -containers is configured.
    #    - Clear the tree
    #    - Repopulate it using the new value.
    # @param optname - option name.
    # @param optval  - new option value.
    # @note  options(optname) is set so we don't need a cget operation.
    #
    method _cfgContainers {optname optval} {
        foreach child [$tree children {}] {
            $tree delete $child;    # Deletes the subierarchy.
        }
        
        foreach item $optval {
            $self _addContainer $item
        }
        # If there are no errors, we can update:
        
        set options($optname) $optval
    }
    #----------------------------------------------------------------------
    #  Public methods
    
    ##
    # addActivation
    #    adds an activation to an existing container
    #
    # @param name   - Name of the container
    # @param host   - Host to add to the activations.
    # @note nothing is done to detect/reject duplicates.
    # @note it is an error to specify a nonexisting container name.
    # @note The corresponding item in the -container option list is
    #       modified. while the tree is modified in place so that
    #       nothing prematurely closes.
    #
    method addActivation {name host} {
        $self _addOptionActivation $name $host
        $self _addTreeActivation $name $host
    }
    ##
    # removeActivation
    #
    #     Removes activation from an existing container.
    #
    # @param name   - Name of the container.
    # @param host   - Host on which that container is no longer active.
    # @note nonexistent containers and nonexistent active hosts in the
    #       container activation list are errors.
    # @note as with addActivation, the -containers option value is updated
    #       to be accurate.
    #
    method removeActivation {name host} {
        $self _removeOptionActivation $name $host
        $self _removeTreeActivation $name $host
    }
     
}
    
##
# @class ProgramStatusList
#    Provides a status display widget suitable for programs.
#    this is just a flat list in a ttk::treeview containing the following
#    columns:
#
#     -  name   - name of the program.
#     -  path   - filesystem path to the program.
#     -  host   - host on which the program runs.
#     -  container - if the program runs in a container the container's name.
#     -  active  -  X if the container is active.
#
# OPTIONS:
#   -programs - list of program dicts as from ProgramClient
snit::widgetadaptor ProgramStatusList {
    option -programs -default -list -configuremethod _cfgPrograms
    component tree
    constructor {args} {
        installhull using ttk::frame
        
        set columns [list name path host container active]
        set headings [list Name {Image Path} Host {Container Name} Active]
        
        install tree using ttk::treeview $win.tree \
            -columns [list name path host container active] \
            -displaycolumns $columns -show headings         \
            -yscrollcommand [list $win.ysb set]
        foreach col $columns head $headings {
            $tree heading $col -text $head
        }
        
        ttk::scrollbar $win.ysb -orient vertical -command [list $tree yview]
        
        grid $tree $win.ysb -sticky nsew
        grid columnconfigure $win 0 -weight 1
        grid columnconfigure $win 1 -weight 0
        
        $self configurelist $args
        
    }
    #-------------------------------------------------------------------------
    # Configuration handling.
    
    ##
    # _cfgPrograms
    #    Called when -programs is configured to repopulate the
    #    tree.
    #    Since the tree is flat, there's no harm in a complete repopulation.
    #
    # @param optname - name of the option being set -programs.
    # @param value   - list of program status dicts
    #
    method _cfgPrograms {optname value} {
        
        $tree delete [$tree children {}];                # clears the tree.
        
        foreach program $value {
            set status [dict get $program active]
            set statusIndicator [expr {($status)? "X" : " "}]
            $tree insert {} end -values  [list                    \
                [dict get $program name]                          \
                [dict get $program path]                          \
                [dict get $program host]                          \
                [dict get $program container]                     \
                $statusIndicator
            ]
        }
        
        set options($optname) $value;            # Makes cget work.
    }
}
##
#
#  test
#    Excercises the UI and client API.
#
# @param user - user that runs the server.
# @param host - Host the server is running on.
proc test {user host} {
    lappend ::auto_path $::env(DAQTCLLIBS)
    package require programstatusclient
    
    
    
    ##
    # definitionsChanged
    #    Determine if the change in container definitions make it
    #    worth just refreshing the whole display:
    #    -  Number of containers differ.
    #    -  Change in container names.
    #    -  Change in definitions of like named containers other than
    #       activations which can be independently updated.
    #
    # @param old     - existing definitions.
    # @param new     - New definitions.
    # @return bool   - True if the best thing is to just reconfigure the whole
    #                  thing.
    #
    proc definitionsChanged {old new} {
        if {[llength $old] != [llength $new]} {
            return 1
        }
        # Load new into an array indexed by names:
        
        array set hash [list]
        foreach item $new {
            set hash([dict get $item name]) $item
        }
        set newNames [array names hash]
        foreach item $old {
            if {[dict get $item name] ni $newNames} {
                return 1;       # at least one name change.
            }
            set newItem $hash([dict get $item name])
            if {[dict get $item image] ne [dict get $newItem image]} {
                return 1;          # Image changed
            }
            set oldBindings [lsort [dict get $item bindings]]
            set newBindings [lsort [dict get $newItem bindings]]
            if {$oldBindings ne $newBindings} {
                return 1;         # Bindings changed.
            }
        }
        
        return 0;                # Subtle changes at best.
    }
    ##
    # activationsChanged
    #    Determine if activations for a container need to be updated.
    #    This is the case if the sorted set of hosts in each activation
    #    are not identical.
    #
    # @param old     - prior definition
    # @param new     - New definitions.
    # @return bool   - true if activations changed.
    #
    #
    proc activationsChanged {old new} {
        set oldAct [dict get $old activations]
        set newAct [dict get $new activations]
     
        if {[llength $oldAct] != [llength $newAct]} {
            return 1;                  # not even same length.
        }
        
        set oldAct [lsort $oldAct]
        set newAct [lsort $newAct]
        return [expr {$oldAct ne $newAct}]
    }
    ##
    # updateActivations
    #   Update the activations on a container.
    #
    # @param w   - the widget.
    # @param old - the activations in the ui
    # @param new - Activations currently know about.
    #
    proc updateActivations {w old new} {
        set name [dict get $old name];
        set oldacts [dict get $old activations]
        set newacts [dict get $new activations]
        
        #  Remove the ones that are gone:
        
        foreach act $oldacts {
            if {$act ni $newacts} {
                $w removeActivation $name $act
            }
        }
        # Add in the new ones:
        
        foreach act $newacts {
            if {$act ni $oldacts} {
                $w addActivation $name $act
            }
        }
    }
    ##
    # updateContainers
    #    update the containers tab:
    #  @param w - the container widget.
    #  @param defs - current container definitions
    #
    proc updateContainers {w defs} {
        set current [$w cget -containers]
        
        # IF any definitions (not activation lists) changed, just re-configure
        
        if {[definitionsChanged $current $defs]} {
            $w configure -containers $defs
            return
        }
        # Only activations changed:
        #  - Toss then current defs into an array indexed by name.
        #  - For each item in the current list, if activations changed,
        #    then update those.
        #
        array set hash [list]
        foreach def $defs {
            set hash([dict get $def name]) $def
        }
        #  We know there are array elements matching each current item because
        #  otherwise we would have reconfigured.
            
        foreach item $current {
            set def $hash([dict get $item name])
            if {[activationsChanged $item $def]}  {
                updateActivations $w $item $def
            }
        }
    }
    ##
    # updateUi
    #   Self re-scheduling proc that updates the UI display.
    # @param ms - number of ms between updates.
    #
    proc updateUi {ms} {
        
        set info [client status]
        set programs [dict get $info programs]
        set containers [dict get $info containers]
        
        # Programs just configure:
        
        .n.p configure -programs $programs
        
        #  Containers are tricky enough (with the ability to add/remove
        #  activations they're worth another proc
        
        updateContainers .n.c $containers
        
        after $ms updateUi $ms
    }
    
    set ::client  [ProgramClient %AUTO% -user $user -host $host]
    
    ttk::notebook .n
    ProgramStatusList .n.p
    ContainerStatusList .n.c
    .n add .n.p -text Programs
    .n add .n.c -text Containers    
    pack .n -expand 1 -fill both
    
    ProgramClient client -user $user -host $host
    
    updateUi 1000
    
}


