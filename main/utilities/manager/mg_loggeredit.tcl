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
# @file   mg_loggeredit
# @brief  Edit the logger configuration for an experiment
# @author Ron Fox <fox@nscl.msu.edu>
#

set libdir [file normalize [file join [file dirname [info script]] .. TclLibs]]
lappend auto_path $libdir

package require Tk
package require snit
package require eventloggers
package require containers
package require sqlite3


#------------------------------------------------------------------------------
# Megawidgets

##
# @class LoggerEntry
#
#   Provides the widgets to describe a logger:
#
# OPTIONS
#    -daqrootdir   The NSCLDAQ root directory to use (determines which logger runs)
#    -ring         Ring buffer from which the data are recorded.
#    -host         Host on which the logger actuall runs.
#    -destination  Where the data goes (meaning depends on partial flag state).
#    -container    container - if not <NONE> the container in which the logger runs.
#    -critical     If 1 - critical if 0 not.
#    -partial      If 1 - partial logger (like multilog) otherwise not
#    -enabled      If 1 - enabled - will record when recording itself is enabled.
#
#    -containers - the allowed containers.
#
# METHDOS
#   clear - return the UI to the default state.
#
# APPEARANCE:
#
#   +----------------------------------------------+
#   | [rootdir] [browse] [destination] [browse]    |
#   | [ring] [container] [host]                    |
#   | [] critical [] partial [] enabled            |
#   +----------------------------------------------+
#

snit::widgetadaptor LoggerEntry {
    option -daqrootdir
    option -ring
    option -host
    option -destination
    option -container -cgetmethod _cgetContainer -configuremethod _cfgContainer
    option -critical -default 1
    option -partial -default 0
    option -enabled -default 1
    option -containers -default [list] \
        -configuremethod _cfgContainers
    
    variable lastRoot 
    variable lastDest $::env(HOME);         # Good starting point.
    
    constructor args {
        set lastRoot [file normalize [file join [file dirname [info script]] ..]]
        installhull using ttk::frame
        
        ttk::label $win.rootlbl -text "DAQROOT"
        ttk::entry $win.root    -textvariable [myvar options(-daqrootdir)]
        ttk::button $win.rootbrowse -text Browse... \
            -command [mymethod _browseRoot]
        
        ttk::label $win.destlabel   -text "Destination"
        ttk::entry $win.destination -textvariable [myvar options(-destination)]
        ttk::button $win.destbrowse -text Browse...  \
            -command  [mymethod _browseDest]
        
        grid $win.rootlbl $win.root $win.rootbrowse \
             $win.destlabel $win.destination $win.destbrowse
        
        ttk::label $win.ringlabel -text {Ring URI}
        ttk::entry $win.ring      -textvariable [myvar options(-ring)]
        
        ttk::label $win.containerlbl -text Container
        ttk::combobox $win.container -values [list <None>] \
            -textvariable [myvar options(-container)]
        
        ttk::label $win.hostlabel -text Host
        ttk::entry $win.host      -textvariable [myvar options(-host)]
        
        grid $win.ringlabel $win.ring \
            $win.containerlbl $win.container $win.hostlabel $win.host
        
        ttk::checkbutton $win.critical -text Critical \
            -variable [myvar options(-critical)] -onvalue 1 -offvalue 0
        ttk::checkbutton $win.partial -text Partial \
            -variable [myvar options(-partial)] -onvalue 1 -offvalue 0
        ttk::checkbutton $win.enabled -text Enabled \
            -variable [myvar options(-enabled)] -onvalue 1 -offvalue 0
        
        grid $win.critical $win.partial $win.enabled
        $self clear
        $self configurelist $args
        
    }
    #--------------------------------------------------------------------------
    # Public methods
    
    ##
    # clear
    #    Return the UI to its default state.
    #
    method clear {} {
        set options(-daqrootdir) ""
        set options(-ring) ""
        set options(-host) ""
        set options(-destination) ""
        $self configure -container <None>
        set options(-partial)
        set options(-critical) 1
        set options(-partial) 0
        set options(-enabled) 1
        
    }
    #----------------------------------------------------------------------------
    # Configuration management.
    
    ##
    # _cfgContainer
    #   Set the container value
    #
    # @param optname -name of the option.
    # @param value   -new container name.
    #
    method _cfgContainer {optname value} {
        $win.container set $value
    }
    ##
    # _cgetContainer
    #    Returns the current container value.
    # @param optname -name of the option.
    # @return string
    #
    method _cgetContainer {optname} {
        return [$win.container get]
    }
    
    ##
    # _cfgContainers
    #   Given a set of container definitions - sets the list of containers
    #
    # @param optname  option name.
    # @param value    list of container dict definitions.
    #
    method  _cfgContainers {optname value} {
        # Create the list:
        
        set containerList <None>
        foreach c $value {
            lappend containerList [dict get $c name]
        }
        $win.container configure -values $containerList
        set options(-optname) $value
    }
    #--------------------------------------------------------------------------
    # Event handling.
    
    ##
    # _browseRoot
    #    Browse for a DAQROOT directory to use for the logger being edited.
    #
    method _browseRoot {} {
        set directory [tk_chooseDirectory -initialdir $lastRoot -parent $win \
            -title {DAQROOT} ]
        if {$directory ne ""} {
            set options(-daqrootdir) $directory
            set lastRoot $directory
        }
    }
    ##
    # _browseDest
    #   Browse for a destination directory.
    #
    method _browseDest {} {
        set directory [tk_chooseDirectory -initialdir $lastDest -parent $win \
            -title {Destination} ]
        if {$directory ne "" } {
            set options(-destination) $directory
            set lastDest $directory
        }
    }
}
##
# @class LoggerList
#    Lists the set of loggers that have been defined.
#
# OPTIONS
#   -loggers - list of logger definition dicts.
#   -ondouble - Script to call when a logger is double-clicked.
#
#   This displays as a treeview with the hidden, liast, column "definition"
#   that contains enough of the definition dict of a logger to reconstruct it
#   in the database.
#
snit::widgetadaptor LoggerList {
    component tree
    
    option -loggers   -default [list] \
        -configuremethod _cfgLoggers -cgetmethod _cgetLoggers
    option -ondouble [list]
    
    constructor args {
        installhull using ttk::frame
        
        install tree using ttk::treeview $win.tree \
            -yscrollcommand [list $win.ysb set]    \
            -columns [list   \
                daqroot ring host container destination enabled critical partial \
                definition] \
            -show headings -displaycolumns [list                                 \
                daqroot ring host container destination enabled critical partial]           \
            -selectmode browse
        foreach col [list daqroot ring host container destination enabled critical partial] \
            heading [list Root "Ring URI" Host "Container" Destination "E" "C" "P"] {
            $tree heading $col -text $heading
        }
        foreach col [list enabled critical partial] {
            $tree column $col -width 20
        }
        
        ttk::scrollbar $win.ysb -command [list $tree yview]
        
        grid $win.tree $win.ysb -sticky nsew
            
        bind $tree <Double-1> [mymethod _onDoubleClick]
        
        $self configurelist $args
    }
    #---------------------------------------------------------------------------
    #  Configuration handling.
    
    ##
    # _cfgLoggers
    #   Configures a new set of loggers.
    #   - Clears the tree.
    #   - Adds each logger to the tree view
    #
    # @param optname - the option name (-loggers - ignored).
    # @param value   - list of logger definition dicts e.g. from
    #                 ::eventloggers::listLoggers
    # @note Since we have a cget method we don't need to store the results in
    #       options($optname).  We'll fish them programmatically from the
    #       tree.
    # @note We don't care about the id nor container_id keys in the dict.
    #
    method _cfgLoggers {optname value} {
        
        #  Clear the tree:
        
        set items [$tree children {}]
        $tree delete $items
        
        # Now populate it:
        
        foreach item $value {
            #  The enabled, critical and partial item get turned into X and blank
            
            set enabled  [_Xmark $item enabled]
            set critical [_Xmark $item critical]
            set partial  [_Xmark $item partial]
            
            $tree insert {} end -values [list                               \
                [dict get $item daqroot]  [dict get $item ring]             \
                [dict get $item host]     [dict get $item container]        \
                [dict get $item destination]                                \
                $enabled $critical $partial $item                           \
            ]
        }
        
        
    }
    ##
    # _cgetLoggers
    #   Returns the list of dicts from -loggers. This is done by building up a list
    #   of the values of the definition hidden column (column # 8 from 0)
    #
    # @param optname - -loggers - ignored.
    # @return list of dicts  see above.
    #
    method _cgetLoggers {optname} {
        set result [list]
        set items [$tree children {}]
        foreach item $items {
            lappend result [lindex [$tree item $item -values] 8]
        }

        return $result
    }
    #--------------------------------------------------------------------------
    #  Event handling methods.
    
    ##
    # _onDoubleClick
    #
    #    Called in response to a double-click in the tree.
    #    If an item is selected, and the -ondouble script is defined,
    #    it is called with the definition of the item selected.
    #
    method _onDoubleClick {} {
        set selection [$tree selection]
        set script $options(-ondouble)
        if {($selection ne "") && ($script ne "")} {
            set def [lindex [$tree item $selection -values] 8]
            lappend $script $def
            uplevel #0 $script
        }
    }
    
    #---------------------------------------------------------------------------
    # Private procs:
    
    ##
    # _Xmark
    #   Given a dict takes a key value, interprets it as a bool and returns
    #   an appropriate indicator
    #
    # @param value - the dict.
    # @param key   - the key in the dict.
    # @return string - indicator X if true and " " if not.
    #
    proc _Xmark {value key} {
        if {[dict get $value $key]} {
            set result X
        } else {
            set result " "
        }
        return $result
    }
}

#----------------------------------------------------------------------------
#  Unbound utility procs:

##
# usage
#    Describes the usage of the command to stderr and then exits.
#
# @param msg - message that prepends the usage.
#
proc usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "    \$DAQBIN/mg_loggeredit configuration-database"
    puts stderr "Where:"
    puts stderr "   configuration-database - is the configuration database file"
    puts stderr "\nConfigures the set of event loggers that will run\n"
    
    exit -1
     
}
##
# _needNonBlank
#    Called to pop up an error that an entry field was left blank that should not
#    have been.
#
# @param field -name of field.
#
proc _needNonBlank {field} {
    set fullMessage "The field '$field' cannot be empty"
    tk_messageBox -parent . -title "Blank field" -icon error -type ok \
        -message $fullMessage
}
##
# _addLogger
#    Adds the logger described in the LoggerEntry passed in.
#
# @param e - a LoggerEntry widget that contains the definition to load.
# @param loggers -the event logger list widget.
# @note A container of <None> means there's no container.
# @note We require all elements be fillled in.
#
proc _addLogger {e loggers} {
    set root [$e cget -daqrootdir]
    set ring [$e cget -ring]
    set dest [$e cget -destination]
    set host [$e cget -host]
    
    set cont [$e cget -container]
    if {$cont ne "<None>"} {
        set container $cont    
    } else  {
        set container ""
    }
    set crit [$e cget -critical]
    set part [$e cget -partial]
    set en [$e cget -enabled]
    
    if {$root eq ""} {
        _needNonBlank "DAQ root"
        return
    }
    if {$ring eq ""} {
        _needNonBlank "Ring URI" 
        return
    }
    if {$dest eq ""} {
        _needNonBlank "Destination"
    }
    #  Create the dict describing the logger and lappend it to the existing
    #  loggers in the UI - wew don't save anything to the database:
    
    set newItem [dict create                                                \
        daqroot $root ring $ring host $host partial $part                   \
        destination $dest critical $crit enabled $en container $container   \
    ]
    set loggerList [$loggers cget -loggers]
    lappend loggerList $newItem
    $loggers configure -loggers $loggerList
}
#-----------------------------------------------------------------------------
#
#  Entry point:

if {[llength $argv] != 1} {
    usage {Incorrect command line parameter count}
}

set fname [lindex $argv 0]
if  {![file writable $fname]} {
    usage "$fname must exist and must be writable."
}

##
# Build the user interface:
#  At the top a logger list.
#  At the middle a logger entry with Add and Delete buttons.
#  At the bottom a Save/Quit button pair.
#

sqlite db $fname
set loggers [::eventloggers::listLoggers db]
LoggerList .loggers -loggers $loggers

ttk::frame .entry
set Entry [LoggerEntry .entry.loggers  -containers [::container::listDefinitions db]]
ttk::button .entry.add -text Add -command [list _addLogger  $Entry .loggers]
ttk::button .entry.delete -text Delete
ttk::button .entry.clear  -text Clear -command [list $Entry clear]

grid .entry.loggers -row 0 -column 0 -rowspan 2
grid .entry.add -row 0 -column 1
grid .entry.clear -row 1 -column 1
grid .entry.delete -row 1 -column 2


ttk::frame .action
ttk::button  .action.save -text Save
ttk::button  .action.quit -text Quit
grid .action.save .action.quit

grid .loggers -sticky nsew
grid .entry   -sticky nsew
grid .action  -sticky nsew
