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
# @file  mg_programedit.tcl
# @brief GUI elements to configure programs 
# @author Ron Fox <fox@nscl.msu.edu>
#

##
#  Contains editor GUI elements for programs.  Programs can be
#  displayed and edited
    


package provide programedit 1.0
package require Tk
package require snit

package require programs;           # If nothing else defines the ::program:: NS.
##
# @class ::program::View
#    Provides a view of a program.  This is a snit megawidget that
#    looks like this:
#
#  +---------------------------------------------------------------------+
#  | Name: [name] Image [image] [browse...]  container [      ] [Browse] |
#  | options    parameters    env           Host [host]                   |
#  | |slist|   |slist    |   | slist  |                                  |
#  | +-----+   +---------+   +--------+                                  |
#  | [name]    [value]        [name]                                     |
#  | [value]                  [value]                                    |
#  | [New]     [New]          [ New]                                     |
#  +---------------------------------------------------------------------+
#
# OPTIONS
#    -name   - Value of the name entry box.
#    -image  - Value of the image entry box.
#    -container - Value of the container name box.
#    -browsecontainers - Script that is supposed to browse the
#               existing containers and return either an empty string
#               if the user does not select a container or the name of
#               a selected container
#   -host   - Host in which the program is to be run.
#   -options - List of option value pairs.
#   -parameters - List of parameter values.
#   -environment - list of name value environment variable pairs.
# EVENTS:
#   Double clicking in a list box, removes the item from the list and
#   loads it into the entries associated with the box.  This supports both
#   editing and removal.
#
snit::widgetadaptor program::View {
    option -name
    option -image
    option -container
    option -browsecontainers
    option -host
    option -parameters
    option -environment
    
    # Components for program options
    
    component optionslist;       # list of options
    component optionname;        # Proposed option name.
    component optionvalue;       # Proposed value
    
    # Program parameter components:
    
    component parameterlist;    # List of parameters
    component parametervalue;   # proposed new parameter value.
    
    # program environment components
    
    component envlist;         # list box of parameters
    component envname;         # Name entry
    component envvalue;        # Value entry
    
    constructor args {
        installhull using ttk::frame
        
        set base [ttk::frame $win.base -relief groove -borderwidth 4];        # Frame for basic parameters-
        ttk::label $base.namelabel -text Name:
        ttk::entry $base.name -textvariable [myvar options(-name)]
        
        ttk::label $base.imagelabel -text Image:
        ttk::entry $base.image      -textvariable [myvar options(-image)]
        ttk::button $base.imbrowse  -text "Browse..." \
            -command [mymethod _browseImage]
        
        ttk::label $base.hostlabel -text "Host"
        ttk::entry $base.host -textvariable [myvar options(-host)]
        
        ttk::label $base.containerlabel -text "Container"
        ttk::entry $base.container -textvariable [myvar options(-container)]
        ttk::button $base.contbrowse -text "Browse..."  \
            -command [mymethod _browseContainer]
        
        grid $base.namelabel $base.name \
            $base.imagelabel $base.image $base.imbrowse -padx 3 -sticky nswe
        grid $base.hostlabel $base.host \
            $base.containerlabel $base.container $base.contbrowse \
                -padx 3  -sticky nsew
        grid $base -sticky nswe -columnspan 6
        
        
        # program options widgets:
        
        set lists [ttk::frame $win.lists -relief groove -borderwidth 4]    ; # Frame for the list boxes.
        set optionsframe [ttk::frame $lists.options]; # Frame for options. lisbox/sb
        
        ttk::label $lists.optionlabel -text "Program Options:"
        install optionslist using listbox $optionsframe.list -selectmode single \
            -yscrollcommand [list $optionsframe.sb set] -width 32
        ttk::scrollbar $optionsframe.sb -orient vertical \
            -command [list $optionslist yview]
        grid $optionslist $optionsframe.sb -sticky nsew
        
        ttk::label $lists.optionnamelabel -text "Name:"
        install optionname using ttk::entry $lists.optionname
        ttk::label $lists.optionvaluelabel -text "Value:"
        install optionvalue using ttk::entry $lists.optionvalue
        ttk::button $lists.newoption -command [mymethod _newOption] -text New
        
        grid $lists.optionlabel -columnspan 2 -row 0 -column 0 -sticky w
        grid $optionsframe -columnspan 2 -row 1 -column 0 -sticky nsew
        grid $lists.optionnamelabel  -row 2 -column 0 -sticky w
        grid $optionname -row 2 -column  1 -sticky w  -padx 5
        grid $lists.optionvaluelabel -row 3 -column 0 -sticky w
        grid $optionvalue      -row 3 -column 1 -sticky w -padx 5
        grid $lists.newoption        -row 4 -column 0 -sticky w
        
        # Program environment variables  widgets.
        
        set parameterframe [ttk::frame $lists.params]
        
        ttk::label $lists.parameterlabel -text "Program Parameters"
        install parameterlist using listbox $parameterframe.list -selectmode single \
            -yscrollcommand [list $parameterframe.sb set] -width 32
        ttk::scrollbar $parameterframe.sb -orient vertical \
            -command [list $parameterlist yview]
        grid $parameterlist $parameterframe.sb -sticky nsew
        
        ttk::label $lists.paramlabel -text "Value:"
        install parametervalue using ttk::entry $lists.paramvalue
        ttk::button $lists.newparam -text "New" -command [mymethod _newParam]
        
        grid $lists.parameterlabel -row 0 -column 2 -columnspan 2 -sticky w
        grid $parameterframe       -row 1 -column 2 -columnspan 2 -sticky nsew
        grid $lists.paramlabel      -row 2 -column 2 -sticky w
        grid $parametervalue       -row 2 -column 3 -sticky w -padx 5
        grid $lists.newparam       -row 3 -column 2 -sticky w
        
        
        # Environment variable widgets.
        
        set envframe [ttk::frame $lists.env]
        ttk::label $lists.envlabel -text "Program Environment"
        install envlist using listbox $envframe.list -selectmode single \
            -yscrollcommand [list $envframe.sb set] -width 32
        ttk::scrollbar $envframe.sb -orient vertical \
            -command [list $envlist yview]
        grid $envlist $envframe.sb -sticky nsew
        
        ttk::label $lists.envnamelabel -text "Name:"
        install envname using ttk::entry $lists.envname
        ttk::label $lists.envvaluelabel -text "Value:"
        install envvalue using ttk::entry $lists.envvalue
        ttk::button $lists.newenv -text New -command [mymethod _newEnv]
        
        grid $lists.envlabel -row 0 -column 4 -columnspan 2 -sticky w
        grid $envframe       -row 1 -column 4 -columnspan 2 -sticky nsew
        grid $lists.envnamelabel -row 2 -column 4 -sticky w
        grid $envname            -row 2 -column 5 -sticky w
        grid $lists.envvaluelabel -row 3 -column 4 -sticky w
        grid $envvalue            -row 3 -column 5 -sticky w
        grid $lists.newenv         -row 4 -column 4 -sticky w
        
        
        #  Grid the entire outer frame.
        
        grid $lists -row 1 -column 0 -columnspan 6 -sticky nsew
        
        
        
        
    }
}

