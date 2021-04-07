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
lappend auto_path $::env(DAQTCLLIBS)
package require programstatusclient
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
snit::widgetadaptor ContainerStatusList {
    option -containers -default [list] -configuremethod _cfgContainers
    component tree
    constructor {args} {
        installhull using ttk::frame
        
        set colnames [list name image binding host]
        set coltitles [list Name Image Binding Host]
        
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
}
    


