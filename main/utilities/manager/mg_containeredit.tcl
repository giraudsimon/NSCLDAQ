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
# @file  mg_containeredit.tcl
# @brief provide the ability to edit container definitions.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide containeredit 1.0
package require Tk
package require snit
package require sqlite3
package require containers

namespace eval ::container {}
##
# @class container::Editor
#    Provides an editor for the container part of the database.
#    The top part contains a list of the containers - on the left side,
#    clicking a container will populate it's definition on the right side.
#    Double clicking a container will allow you to to edit it.
#    Clicking New... will allow you to edit a new container.
#    Note that when editing an existing container; if you change its name,
#    You can use it as a starting point for a new container.
#
# OPTIONS
#    -containers   - list of container definitions from, e.g. container::listDefinitions
#    -newcommand   - Command to execute if a new container is being created
#    -replacecommand - command to execute if a container is being modified.
#
# Normally -newcommand will invoke container::add and -replacecommand will
# first container::remove then container::add
# Both of those commands have appended to them the full container description.
#
snit::widgetadaptor container::Editor {
    option -containers -default [list] -configuremethod _update
    option -newcommand -default [list]
    option -replacecommand -default [list]
    
    component containerNames;     # list box with container names.
    
    constructor args {
        installhull using ttk::frame
        
        install containerNames using listbox $win.names -selectmode single
        grid $containerNames -sticky nesw
        
        $self configurelist $args
    }
    
    ###
    # Configuration:
    #
    
    method _update {optname optval} {
        set options($optname) $optval;   # now cget works.
        
        $containerNames delete 0 end
        foreach container $optval {
            $containerNames insert end [dict get $container name]
        }
    }
}


proc container::editorTest {} {
    catch [file delete containereditortest.db]
    exec [file join $::env(DAQBIN) mg_mkconfig] containereditortest.db
    sqlite3 db containereditortest.db
    container::add db a thing1 "" ""
    container::add db b thing2 "" ""
    container::add db test minimal.img "" ""
    
    container::Editor .e -containers [container::listDefinitions db]
    pack .e -fill both -expand 1
    
}


