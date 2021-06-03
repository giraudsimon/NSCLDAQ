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
# @file   mg_evlogEditViews.tcl
# @brief  Views used by event log configuration editors.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide evLogEditViews  1.0
package require Tk
package require snit


##
# @class evLogEditLogger
#    Provides an editor for a single logger.
#
# OPTIONS
#    *  -containers   - Containers that can be chosen fromm.
#    *  -savelabel    - Label for the save command.
#    *  -command      - command script for the button.
#
# METHODS:
#    * load          - Load the values of the editor.
#    * get           - Get the values of the editor.
#    * selectContainer - Select a container from the containers list.
#
# PRESENTATION
#    entries for the root, ring, destination and host.
#    dropdown with containers and checkbuttons for critical, enabled, and partial
#
#   +-------------------------------------------------------------------+
#   |  DAQRoot: [ ] Browse...  Source [ ] Destination [ ] Browse...     |
#   |  Host:  [ ]    Container: [ ]                                     |
#   |  [ ] Partial [ ] Critical [ ] Enabled                             |
#   +-------------------------------------------------------------------+
#
snit::widgetadaptor evlogEditLogger {
    option -containers -default [list] -configuremethod _cfgContainers 
    option -savelabel -default Save
    option -command [list]
    
    #  Checkbutton variables:
    
    variable partial  0
    variable critical 1
    variable enabled  1
    
    constructor {args} {
        installhull using ttk::frame
        
        #  Top row:
        #    The root cluster of widgets:
        
        ttk::label $win.rootl -text "DAQRoot: "
        ttk::entry $win.root
        ttk::button $win.rootb -text Browse... \
            -command [mymethod _browseDir $win.root]
        
        #   Data source cluster of widgets
        
        ttk::label $win.srcl -text Source: 
        ttk::entry $win.source
        
        #     Destination cluster:
        
        ttk::label $win.destl  -text Dest:
        ttk::entry $win.dest
        ttk::button $win.destb -text Browse... \
            -command [mymethod _browseDir $win.dest]
        
        grid $win.rootl $win.root $win.rootb      \
             $win.srcl  $win.source  x            \
             $win.destl $win.dest $win.destb  -sticky w
        
        #  Second row
        #     Host cluster:
        
        ttk::label $win.hostl   -text Host:
        ttk::entry $win.host
        
        #    Container cluster:
        
        ttk::label $win.contl -text Container:
        ttk::combobox $win.cont
        
        grid $win.hostl $win.host x    \
             $win.contl $win.cont  -sticky w
        
        #   bottom row of checkboxes.
        
        ttk::checkbutton $win.partial -text Partial -variable [myvar partial]
        ttk::checkbutton $win.critical -text Critical -variable [myvar critical]
        ttk::checkbutton $win.enabled -text Enabled -variable [myvar enabled]
        grid $win.partial $win.critical $win.enabled
        
        #  The save button.
        
        ttk::button $win.save -textvariable [myvar options(-savelabel)] \
            -command [mymethod _dispatchCommand]

        grid $win.save
        
        $self configurelist $args
        
    }
    #--------------------------------------------------------------------------
    # Procs:
    #   _setEntry
    #      Set the contents of an entry.
    # @param entry - an entry widget spec.
    # @param value - the value.
    #
    proc _setEntry {entry value} {
        $entry delete 0 end
        $entry insert end $value
    }
    #---------------------------------------------------------------------------
    #  Configuration management
    
    ##
    #  _cfgContainers
    #    Configure the values in the container names combobox.
    #    Note that if the client wants to allow containerless specifiction,
    #    they should provide the first entry as an empty string.
    #
    # @param optname - name of the option being modified.
    # @param value   - List of allowed container names.
    #
    method _cfgContainers {optname value} {
        $win.cont configure -values $value
        set options($optname) $value
    }
    
    
    
    
    #-------------------------------------------------------------------------
    # Event handling.
    
    ##
    # _browseDir
    #   Browse for a directory that is loaded into the associated entry.
    #   Note the current directory, if there is one, is used for -initialdir.
    #
    #  Since we may have differing containers with differing mappings,
    #  we set -mustexist false on the dialog.
    #
    # @param entry - entry widget to load when a successful choice is made.
    #
    method _browseDir {entry} {
        set dir [$entry get]
        if {$dir eq "" } {
            set dir [pwd];              # use current if none yet.
        }
        set result [tk_chooseDirectory \
            -initialdir $dir -mustexist 0 -parent $win -title "Choose directory"
        ]
        if {$result ne ""} {
            _setEntry $entry $result
        }
    }
    ##
    # _dispatchCommand
    #   Dispatch to the user's script.  The user's script will have the
    #   widget name appended to it
    #
    method _dispatchCommand {} {
        set script $options(-command)
        if {$script ne ""} {
            lappend script $win
            uplevel #0 $script
        }
    }
    #------------------------------------------------------------------------
    # Public methods.
    
    ##
    # load
    #    Load the editor with an event logger definition.
    #
    # @param logger  The logger definition.  This in in the form of one of the
    #                dict elements that comes back from eventlog::listLoggers.
    #
    method load {logger} {
        #  This unloading of raw values from the dict ensures that if the dict
        # is improperly formatted the editor is not corrupted:
        
        set root [dict get $logger root]
        set ring [dict get $logger ring]
        set host [dict get $logger host]
        set ispartial [dict get $logger partial]
        set dest [dict get $logger destination]
        set iscritical [dict get $logger critical]
        set isenabled [dict get $logger enabled]
        set container [dict get $logger container]
        
        set cindex [lsearch -exact $options(-containers) $container]
        if {$cindex == -1} {
            error "The value of '$container' is not in the list of legal containers"
        }
        
        #  We could pull everything we need so load the editor:
        
        _setEntry $win.root $root
        _setEntry $win.source $ring
        _setEntry $win.dest $dest
        _setEntry $win.host $host
        $win.cont current $cindex
        set partial $ispartial
        set critical $iscritical
        set enabled $isenabled
        
    }
    ##
    # get
    #   Get the value of the editor.
    #
    # @return dict with the following key/value pairs:
    #        - daqroot - DAQ Root directory of the logger.
    #        - ring    - URI of the data source ring buffer.
    #        - host    - Name/IP address of the host in which the logger runs.
    #        - partial - Boolean indicating if  the logger is a partial logger.
    #        - destination - Destination directory to which events are logged.
    #        - critical - Boolean that's true if the logger is critical.
    #        - enabled  - True if the logger is currenly enabled.
    #        - container- Name of the container.
    #
    method get {} {
        return [dict create                                          \
            root [$win.root get]         ring [$win.source get]      \
            host [$win.host get]         partial $partial            \
            destination [$win.dest get]  critical $critical          \
            enabled $enabled             container [$win.cont get]  \
        ]
    }
    ##
    # selectContainer
    #    Choose one of the containers as the currently selected one.
    #    This is normally done after containers are described on a new
    #    widget so that a default container is selected.
    #
    # @param name - container name requested.
    #
    method selectContainer {name} {
        set index [lsearch -exact $options(-containers) $name]
        if {$index == -1} {
            error "There is no container named '$name'"
        } else {
            $win.cont current $index
        }
    } 
}

##
# @class evlogListView
#    Provides a view for listing event loggers and performing actions
#    on them.
#
# OPTIONS
#    -data - List of event log definitions such as you might get from
#            e.g. eventlog::listLoggers.  Note that the id and container_id
#            are ignored and also not needed.
#    -contextmenu - context menu definition.  This consists of
#             text elements that are put in a context menu.
#    -command - Script executed when a context menu entry is selected.
#             The script receives the menu command text and definition of the
#             item the menu was invoked on.
# METHODS:
#    highlight - Renders the specified item in reverse video coloring.
#    unhighlight - Removes the highlight from the a specified item.
#    unhighlightall - removes the highlight from all items.
# PRESENTATION:
#    The data are presented as a tree with the following columns:
#    *  DAQRoot - the DAQ installation version used.
#    *  source  - URL of the ring buffer data is taken from.
#    *  host    - The Host on which the logger runs.
#    *  destination - the destination in whichthe data are logged.
#    * container- Name of the container (or blank if none) in which the
#                  logger run.s
#    *  partial - X if the logger is partial
#    *  critical- X if the logger is critical
#    *  enabled - X if the logger is enabled.
#
snit::widgetadaptor evlogListView {
    option -data -default [list] -configuremethod _cfgData
    option -contextmenu -default [list] -configuremethod _cfgMenu
    option -command -default [list]
    
    variable contextMenu "";       # If not empty the context menu.
    
    constructor args {
        installhull using ttk::treeview
        
        set columns [list root source host destination container partial critical embedded]
        set titles   [list DAQRoot source host destination container P C E]
        
        ttk::treeview $win.t -yscrollcommand [list $win.vsb set]    \
            -columns $columns -show headings -selectmode none
        
        foreach c $columns h $titles {
            $win.t heading $c -text $h
            if {[string length $h] == 1} {
                $win.t column $c -width 15 -stretch 0
            }
        }
        
        ttk::scrollbar $win.vsb -command [list $win.t yview] -orient vertical
        
        #  Make an item tag and bind the button-3 event to post any 
        #  context menu:
        
        $win.t tag add item [list];         # All items.
        $win.t tag bind item <ButtonPress-3> [mymethod _postContextMenu %X %Y %x %y]
        $win.t tag add highlighted [list];  # Highlighted items.
        
        grid $win.t $win.vsb -sticky nsew
        grid columnconfigure $win 0 -weight 1
        grid columnconfigure $win 1 -weight 0
        
        $self configurelist $args
    }
    #----------------------------------------------------------------------
    #  Utility methods/procs
    #
    
    ##
    # _boolValue
    #   Takes a dict and a key and returns "" if the value is boolean false and
    #   X otherwise.
    #
    # @param d   - dictionary value.
    # @param k   - Key name.
    # @return string - "" or "X"
    proc _boolValue {d k} {
        set value [dict get $d $k]
        
        if {$value} {
            return "X"
        } else {
            return " "
        }
    }
    ##
    # _textToBool
    #    Takes a text value and turns it into a bool
    #
    # @param value
    # @return bool
    #
    proc _textToBool {value} {
        if {$value eq "X"} {
            return 1
        } else {
            return 0
        }
    }
    
    ##
    # _addLogger
    #   Adds a single logger to the end of the list of loggers.
    #  The logger is tagged with the item ta so that it's got the right click
    #  bound to it.
    #
    # @param logger  - The dict containing the logger definition.
    #
    method _addLogger {logger} {
        
        set values [list                                             \
            [dict get $logger daqroot]  [dict get $logger ring]      \
            [dict get $logger host]     \
            [dict get $logger destination] [dict get $logger container] \
        ]
        lappend values [_boolValue $logger partial]
        lappend values [_boolValue $logger critical]
        lappend values [_boolValue $logger enabled]
        
        $win.t insert {} end -values $values -tags [list item]
    }
    ##
    # _createContextMenuIfNecessary
    #    If the conextMenu variable is an empty string and
    #    options(-contextmenu) is a nonempty list, create the new context
    #    menu.  The elements of the option(-contextmenu) list are labels for
    #    command entries in that menu and all commands route to _dispatchCommand.
    #    which will dispatch to the user's -command script.
    #
    # @param x - window coordinates of pointer.
    # @param y - window coordinates of pointer.
    # @note the window coordinates are passed to the menu action method so that
    #       the element the context menu was created under can be retrieved and passed
    #       to the user's script.
    #
    method _createContextMenuIfNecessary {x y} {
        if {($contextMenu eq "") && ([llength $options(-contextmenu)] > 0) } {
            set contextMenu [menu $win.context -tearoff 0]
            foreach item $options(-contextmenu) {
                $contextMenu add command \
                    -label $item -command [mymethod _dispatchMenuItem $item $x $y]
            }
        }
    }
    
    #----------------------------------------------------------------------
    # Configuration handling
    #
    
    ##
    # _cfgData
    #   Stock the treeview with the data in the list.
    # @param optname - name of the option being modified.
    # @param value   - new value.
    #
    method _cfgData {optname value} {
        $win.t delete [$win.t children {}]
        foreach logger $value {
            $self _addLogger $logger
        }
        
        set options($optname) $value
    }
    ##
    # _cfgMenu
    #   Configure the menu contents.. This just involves destroying
    #   and emptying out the contextMenu string so that the next
    #   posting will regenerate the menu:
    #
    # @param optname - name of the option being modified.
    # @param value   - new value.
    #
    method _cfgMenu {optname value } {
        set options($optname) $value
        destroy $contextMenu
        set contextMenu ""
    }
    
    #-----------------------------------------------------------------------
    # Event handling.
    #
    
    ##
    # _postContextMenu
    #    If there's an existing context menu post it at the cursor position.
    #    If there isn't a context menu, generate it and post it.
    #
    # @param x  - Root window x coordinate of pointer.
    # @param y  - Root window y coordinate of pointer.
    # @param wx - Window coordinates of pointer.
    # @param wy - Window coordinates of pointer.'
    #
    #
    method _postContextMenu {x y wx wy} {
        $self _createContextMenuIfNecessary $wx $wy
        if {$contextMenu ne ""} {
            $contextMenu post $x $y
        }
    }
    ##
    # _dispatchMenuItem
    #     Called when a context menu item is clicked. If the user has a
    #     -command script the menu item label is lappended to it along with the
    #     dict describing the item the pointer was over when the context menu
    #     was posted and the script is called at global level.
    #
    # @param item  - the text of the label of the menu item clicked.k
    # @param x     - X window Position of the item the menu was posted on.
    # @param y     - Y Window position of the item the menu was posted on.
    #
    method _dispatchMenuItem {item x y} {
        set command $options(-command)
        if {$command ne ""} {
            set el [$win.t identify item $x $y]
            set data [$win.t item $el -values]
            set def [dict create                                          \
                daqroot [lindex $data 0]      ring [lindex $data 1] \
                host [lindex $data 2]     \
                destination [lindex $data 3]    container [lindex $data 4] \
                partial [_textToBool [lindex $data 5]]                     \
                critical [_textToBool [lindex $data 6]]                     \
                enabled [_textToBool [lindex $data 7]]                     \
             ]
            lappend command $item $def
            uplevel #0 $command
        }
    }
}   
