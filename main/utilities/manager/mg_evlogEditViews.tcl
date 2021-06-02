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
# @class evlogEditView
#    View used to edit the event loggers.  Supports:
#    - Adding new loggers,
#    - Removing old loggers.
#    - Modifying existing loggers.
# Normally this is intended to be used with a controller and model that
# handle the user interface requests and manage the data provided by the
# eventlog package.
#
# OPTIONS:
#    *  -data       - list of dicts that describe the loggers in the view.
#    *  -containers - List of known container names.
#    *  -addcommand - Script called to provide any default values to the
#                     new logger entry part of the presentation.
#    *  -delcommand - Script called when the user requests the deletion of
#                     an item.
#    *  -changecommand - Script called when the users requests the modification
#                      of an item.
#    *  -savecommand - Script called when the user wants to save edits in the
#                      editor section.
#    *  -savelabel   - Label on the save button.
# METHODS:
#    loadEditor   - Loads the editor section with values.
#    getEditor    - Retrieves the values of the editor section.
#    setStatus    - Sets the status line.
#
# PRESENTATION:
#    There are two sections of the display;  A treeview displays the current
#    set of loggers.
#    Below the treeview is:
#    -   An editor that allows entry or modification of a logger.
#    -   A save  button that invokes the -savecommand script.
#    -   A status line that indicates if we are adding or modifying an editor.
#