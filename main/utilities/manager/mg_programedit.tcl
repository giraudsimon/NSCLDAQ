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
#  | WOrking dir [name] [Browse...]
#  | options    parameters    env           Host [host]                  |
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
#   -directory - default directory.
#   -type      - program type (Critical, )
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
    option -type Transitory
    option -directory
    option -options     -cgetmethod _getOptions -configuremethod _setOptions
    option -parameters  -cgetmethod _getParameters -configuremethod _setParameters
    option -environment -cgetmethod _getEnvironment -configuremethod _setEnvironment
    
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
        
        ttk::label $base.imagelabel -text "Program file:"
        ttk::entry $base.image      -textvariable [myvar options(-image)]
        ttk::button $base.imbrowse  -text "Browse..." \
            -command [mymethod _browseImage]
        
        ttk::label $base.hostlabel -text "Host"
        ttk::entry $base.host -textvariable [myvar options(-host)]
        
        ttk::label $base.containerlabel -text "Container"
        ttk::entry $base.container -textvariable [myvar options(-container)]
        ttk::button $base.contbrowse -text "Browse..."  \
            -command [mymethod _browseContainer]
        
        ttk::label $base.wdlabel -text "Working Directory"
        ttk::entry $base.wd -textvariable [myvar options(-directory)]
        ttk::button $base.browsewd -text Browse... -command [mymethod _browseWd]

        set typeframe [ttk::labelframe $base.type -text Type:]
        ttk::radiobutton $typeframe.critical -text Critical -value Critical \
            -variable [myvar options(-type)]
        ttk::radiobutton $typeframe.persistent -text Persistent -value Persistent \
            -variable [myvar options(-type)]
        ttk::radiobutton $typeframe.transient -text Transitory -value Transitory \
            -variable [myvar options(-type)]
        grid $typeframe.critical $typeframe.persistent $typeframe.transient
        
        
        grid $base.namelabel $base.name \
            $base.imagelabel $base.image $base.imbrowse -padx 3 -sticky nswe
        grid $base.hostlabel $base.host \
            $base.containerlabel $base.container $base.contbrowse \
                -padx 3  -sticky nsew
        grid $base.wdlabel $base.wd $base.browsewd -padx 3 -sticky nsew
        grid $typeframe -row 2 -column 3 -columnspan 2 -rowspan 2
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
        
        
        #  Add selection events to the list boxes that
        # load the value into the entries and remove the from the list:
        
        bind $optionslist <<ListboxSelect>> [mymethod _selectOption]
        bind $parameterlist <<ListboxSelect>> [mymethod _selectParameter]
        bind $envlist <<ListboxSelect>> [mymethod _selectEnvVar]
        
        $self configurelist $args
    }
    #------------------------------------------------------------------------
    #  Option handlers.
    
    ##
    # _getOptions
    #   Retrieve program options from the list box.  The options are stored inthe
    #   list box as option=value  What we return is a list of {option value} pairs
    #   or, if there is no value, just {option}
    #
    # @param optname - name of the option which will always be -options.
    #
    method _getOptions {optname} {
        set rawValues [$optionslist get 0 end]
        set result [list]
        foreach value $rawValues {
            set valueList [split $value =]
            set name [lindex $valueList 0]
            set value [lindex $valueList 1]
            if {$value ne ""} {
                lappend result [list $name $value]
            } else {
                lappend result [list $name]
            }
        }
        
        return $result
    }
    ##
    # _setOptions
    #   Sets the options in the list box from a new configuration of -options.
    #   The configured value is a list of option value pairs, or option if there is
    #   no value for the option.
    #
    # @param optname - name of the option.
    # @param value   - proposed new value.
    #
    method _setOptions {optname value} {
        $optionslist delete 0 end;             # Empty out current values.
        set items [list];                      # We'll build a list of optname=value
        foreach entry $value {
            if {[llength $entry] == 2} {
                lappend items [lindex $entry 0]=[lindex $entry 1]
            } else {
                lappend items [lindex $entry 0]
            }
        }
        $optionslist insert end {*}$items
    }
    
    ##
    # _getParameters
    #   Retrieve the -parameters option values from the list of parameters.
    #
    # @param optname - name of the option being cgotten (-parameters)
    #
    method _getParameters {optname} {
        return [$parameterlist get 0 end]
    }
    ##
    # _setParameters
    #    process the configure -parameters operation.  The parameters passed
    #    in are loaded into the listgbox.
    #
    # @param optname - option name (-parameters).
    # @param value   - list of parameters.
    #
    method _setParameters {optname value} {
        $parameterlist delete 0 end
        $parameterlist insert end {*}$value
    }
    
    ##
    # _getEnvironment
    #   process cget -environment  - environment variable s are stored in the
    #   list box as name=value.
    #   We return a list of name value pairs.  We assume that every environment
    #   variable has a value...even if it's just an empty string.
    # @param optname - name of the option (-environment).
    #
    method _getEnvironment {optname} {
        set rawItems [$envlist get 0 end]
        set result [list]
        foreach item $rawItems {
            set itemList [split $item =]
            lappend result [list [lindex $itemList 0] [lindex $itemList 1]]
        }
        return $result
    }
    ##
    # _setEnvironment
    #    Process configure -environment  - convert list of name/value pairs into
    #    name=value strings and store the result in the list box.
    #
    # @param optname  - option name (-environment)
    # @param value    - proposed option value.
    #
    method _setEnvironment {optname value} {
        $envlist delete 0 end
        set list [list]
        foreach item $value {
            lappend list [lindex $item 0]=[lindex $item 1]
        }
        $envlist insert end {*}$list
    }
    #------------------------------------------------------------------------
    #  Event handlers.
    
    ##
    # _browseImage
    #   Browse for the program image file.  Note that if the program is run
    #   containerized, this program image must be in the filesystem of the
    #   activated container.  This can cause confusion...especially when selecting
    #   e.g. NSCLDAQ programs.
    #
    method _browseImage {} {
        set filepath [tk_getOpenFile -title {Choose program file} -parent $win \
            -filetypes [list [list {All files } *]]
        ]
        if {$filepath ne ""} {
            set options(-image) $filepath
        }
    }
    ##
    # _browseContainer
    #   Called when the Browse.. button was clicked to select a container.
    #   Containers are an orthogonal functionality to this UI and we don't want
    #   the user interface to be concerned with the available containers.
    #   Therefore, it's up to the user to establish a script using the
    #   -browsecontainers options.
    #   * If this script has been established we call it and it's supposed to return
    #     the name of the container the user selected. Or an empty string
    #     if no container was selected.  If a non-empty string is returned,
    #     it is loaded into options(-container).  Otherwise that's left alone.
    #   * If no script has been established, we pop up a message box letting
    #     the user know that browsing is not available.
    #
    # @todo - in the future add a handler to the -browsecontainers configuration
    #         which simply disables the Browse button if there is no script.
    #         and enables it when there is one.
    #
    method _browseContainer {} {
        set script $options(-browsecontainers)
        if {$script ne ""} {
            set result [uplevel #0 $script]
            if {$result ne ""} {
                set options(-container) $result
            }
        } else {
            tk_messageBox -type ok -icon info -parent $win -title "Browsing unavailable" \
                -message {Container browsing is not available at this time}    
        }
    }
    ##
    # _browseWd
    #   Browse for the working directory  in which the program runs.
    #   Note this is somewhat problematic as if the program is running in a container,
    #   the wd must be in the filesystem of the active container. This can be
    #   ameliorated by:
    #   * Either defining the program while running the container and/or
    #   * Where possible making the bindings  map to the same target as their source
    #
    method _browseWd {} {
        set dir [tk_chooseDirectory -parent . -title {Choose working dir}]
        if {$dir ne ""} {
            set options(-directory) $dir
        }
    }
    ##
    # _newNameValue
    #   Accepts a new name/value from entry widgets and appends them to the
    #   list widget.  This is common code for _newOption and _newEnv both of which
    #   result in name=value strings.
    #
    # @param namew   - Name entry widget.
    # @param valuew  - Value entry widget.
    # @param listw   - Listbox widget.
    # @param valueRequired - true if both a name and a value are required.
    #
    method _newNameValue {namew valuew listw {valueRequired 0}} {
        set name [$namew get]
        set value [$valuew get]
        if {[string trim $name ] eq ""} {
            return;        # Must at least have a name.
        }
        if {$valueRequired && ([string trim $value] eq "")} {
            return
        }
        if {[string trim $value ] ne ""} {
            $listw insert end "$name=$value"
        } else {
            $listw insert end $name
        }
    }
    ##
    # _newOption
    #    Takes the values of the option name and option value entries, constructs
    #    a new option string (either --option=value or --option if the value is
    #    an empty string) and appends it to the list box.
    #
    #
    method _newOption {} {
        $self _newNameValue $optionname $optionvalue $optionslist
    }
    ##
    # _newParam
    #   Adds a new parameter to the list of parameters in the listbox.
    #   Since parameters are often positional, there's a bit of a quandary
    #   about how to do thins.  Here's my solution and I'm sticking with it;
    #   *  If there's a selection, the new parameter is inserted just prior
    #      to that selected item.
    #   *  If there's no selection the new parameter is inserted at the
    #      end of the list.
    #   * After insertion, the selection is immediately cancelled.
    #   * An empty parameter deletes any selected parameter or does nothing
    #     if there is no selection
    # @todo - maybe add a Delete button that's active when there's a selection
    #         and not when there isn't so deletion isn't so clunky and error
    #         prone.
    #
    method _newParam {}  {
        set value [$parametervalue get]
        if {[string trim $value] ne ""} {
            # insertion:
            set sel [$parameterlist curselection]
            if {$sel eq ""} {
                set sel end
            }
            $parameterlist insert $sel $value
        } else {
            # Potentially delete:
            set sel [$parameterlist curselection]
            if {$sel ne ""} {
                $parameterlist delete $sel
            }
        }
        # Regardless when done, clear the selection:
        
        $parameterlist selection clear 0 end
    }
    ##
    # _newEnv
    #   Accept a new environment variable.
    #   JUst call _newNameValue:
    #
    method _newEnv {} {
        $self _newNameValue $envname $envvalue $envlist 1
    }
    
    ##
    # _selectNameValue
    #    Common code between selecting an option or an environment variable
    #    The only differences are the widgets in use:
    #
    # @param listbox - listbox widget with the selection.
    # @param namew   - Name entry widget.
    # @param valuew  - Value entry widget.
    #
    method _selectNameValue {listbox namew valuew} {
        set index [$listbox curselection]
        if {$index ne "" } {
            set item [$listbox get $index]
            set item [split $item =];     # Split the item into name value
            
            $namew delete 0 end
            $namew insert  end [lindex $item 0]
            
            $valuew delete 0 end
            $valuew insert end [lindex $item 1]
            
            $listbox delete $index
        }
    }
    
    ##
    # _selectOption
    #   Option values are not postional so this bit of code
    #   1. Decodes the selection and loads it into the optionname and optionvalue
    #     entries.
    #   2. Deletes the selected entry.
    #  Naturally if there is no selection (I believe this event/method fires
    #  on changes in selection which includes removing all items from a selection)
    #
    method _selectOption  {} {
        $self _selectNameValue $optionslist $optionname $optionvalue
        
    }
    ##
    # _selectParameter
    #   A bit of juggling is needed here to ensure that we can retain the
    #   order of things if all that's being done is an edit:
    #   1. Get the selected item and load it into the parameter entry.
    #   2. Delete the parameter entry.
    #   3. Set the selection to:
    #      a)  The new item at that index if the selected item was not the last
    #      b)   Empty if the selection was the last.
    #
    method _selectParameter {} {
        set index [$parameterlist curselection]
        if {$index ne ""} {
            $parametervalue delete 0 end
            $parametervalue insert end [$parameterlist get $index]
            
            $parameterlist delete $index
            if {$index < [$parameterlist size]} {
                $parameterlist selection set $index
            } else {
                $parameterlist selection clear 0 end
            }
        }
    }
    ##
    # _selectEnvVar
    #    Process a selection of an environment variable.
    #
    method _selectEnvVar {} {
        $self _selectNameValue $envlist $envname $envvalue
    }
}

##
# @class  program::SelectProgram
#
#    This widget provides a list of programs.  A selection can be made from
#    these programs and a script fired off when that happens.
#
#   The visual looks like this:
#
#   +----------------------------------+
#   | +-----------------------------+  |
#   | | List of programs            |  |
#   | +-----------------------------+  |
#   +----------------------------------+
#
#  The list is a treeview with the following columns:
#  *   Program name
#  *   Program image
#  *   Program host
#  *   Container name (if specified empty column if not).
#
#  In addition, a hidden column:  definition contains the full program
#  definition dict.
#
#  Options:
#     -programs  - List of programs from e.g. ::program::listDefinitions
#     -command   - Script called when an entry is selected.  The script
#                  is passed the definition over which was clicked.
#
snit::widgetadaptor program::SelectProgram {
    option -programs -configuremethod  _fillListbox
    option -command [list]
    
    constructor args {
        installhull using ttk::frame
        
        ttk::treeview $win.list -yscrollcommand [list $win.sb set] \
            -columns [list name image host container definition]  \
            -displaycolumns [list name image host container]      \
            -show headings -selectmode extended
        $win.list heading name  -text "Name"
        $win.list heading image -text "Program file"
        $win.list heading host  -text "Host"
        $win.list heading container -text "Container"
        
        ttk::scrollbar $win.sb -orient vertical -command [list $win.list yview]
        
        grid $win.list $win.sb -sticky nsew
        
        $self configurelist $args
        
        #  Bind the Double-1 event to the program tag such that the -command
        #  will be invoked.
        
        $win.list tag bind program <Double-1> [mymethod _dispatchCommand]
    }
    #-------------------------------------------------------------------------
    #  Configuration management
    
    ##
    #  _fillListBox
    #    Called on to process configure -programs
    #    First clears the tree and then loads it with the program definitions
    #    in its value
    #
    # @param optname - option (always -programs)
    # @param values  - Proposed value.
    method _fillListbox {optname value} {
    
        set items [$win.list children {}]
        $win.list delete $items
      
        foreach item $value {
            set name [dict get $item name]
            set image [dict get $item path]
            set host  [dict get $item host]
            set container ""
            if {[dict exists $item container_name]} {
                set container [dict get $item container_name]
            }
    
            $win.list insert {} end -values [list $name $image $host $container $item] \
                -tags program
        }
        
        set options($optname) $value
    }
    #--------------------------------------------------------------------------
    # Event handling.
    
    ##
    # _dispatchCommand
    #     Method called when an item is double clicked.
    #
    method _dispatchCommand {} {
        set script $options(-command)
        set selection [$win.list selection]
        if {($script ne "") && ($selection ne "")} {
            
            # Clear the selection now because the script can change
            # The contents of the tree:
            
            $win.list selection remove $selection
            
            # Get the definition:
            
            set items [$win.list item $selection -values]
            set def [lindex $items end]
            
            uplevel $script [list $def]
            
            
        }
    }
    
}

