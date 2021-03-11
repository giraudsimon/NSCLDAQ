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
# APPEARANCE:
#
#   +----------------------------------------------+
#   | [rootdir] [browse] [destination] [browse]    |
#   | [container] [host]                           |
#   | [] critical [] partial [] enabled            |
#   +----------------------------------------------+
#
snit::widgetadaptor LoggerEntry {
    option -daqrootdir
    option -ring
    option -host
    option -destination
    option -container -cgetmethod _getContainer -configuremethod _cfgContainer
    option -critical -default 1
    option -partial -default 0
    option -enabled -default 1
    option -containers -default [list] \
        -configuremethod _cfgContainers -cgetmethod _cgetContainers
    
    constructor args {
        installhull using ttk::frame
        
        ttk::label $win.rootlbl -text "DAQROOT"
        ttk::entry $win.root    -textvariable [myvar options(-daqrootdir)]
        ttk::button $win.rootbrowse -text Browse... -command [mymethod _browseRoot]
        
        ttk::label $win.destlabel   -text "Destination"
        ttk::entry $win.destination -textvariable [myvar options(-destination)]
        ttk::button $win.destbrowse -text Browse... -command  [mymethod _browseDest]
        
        grid $win.rootlbl $win.root $win.rootbrowse \
             $win.destlabel $win.destination $win.destbrowse
        
        ttk::label $win.containerlbl -text Container
        ttk::combobox $win.container -values [list <None>] \
            -textvariable [myvar options(-container)]
        $win.container set <None>
        
        ttk::label $win.hostlabel -text Host
        ttk::entry $win.host      -textvariable [myvar options(-host)]
        
        grid $win.containerlbl $win.container $win.hostlabel $win.host
        
        ttk::checkbutton $win.critical -text Critical \
            -variable [myvar options(-critical)] -onvalue 1 -offvalue 0
        ttk::checkbutton $win.partial -text Partial \
            -variable [myvar options(-partial)] -onvalue 1 -offvalue 0
        ttk::checkbutton $win.enabled -text Enabled \
            -variable [myvar options(-enabled)] -onvalue 1 -offvalue 0
        
        grid $win.critical $win.partial $win.enabled
        
    }
}