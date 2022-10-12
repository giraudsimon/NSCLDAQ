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
# @class container::BindingsList
#    List of bindings that can scroll both ways.
#    This can be used inboth container::Creator and container::Editor
#    below
#  The visual presentation is a listox with v and h scrollbars.
# OPTIONS:
#   -bindings - Bindings with which to populate the listbox.
#               This will have the form of the bindings dict key in
#               a container definition from container::listDefinitions
#   -selectscript - Script called if the selection changes.  The value of the
#               current selection is returned.
#
#
snit::widgetadaptor container::BindingsList {
    component bindings
    
    option -bindings -default [list] -configuremethod _update
    option -selectscript [list]
    
    delegate option * to bindings
    
    
    constructor args {
        installhull using ttk::frame
        install bindings using ::listbox $win.list \
            -yscrollcommand [list $win.vscroll set]   \
            -xscrollcommand [list $win.hscroll set] -selectmode single
        ttk::scrollbar $win.vscroll -orient vertical \
            -command [list $bindings yview]
        ttk::scrollbar $win.hscroll -orient horizontal \
            -command [list $bindings xview]
        
        bind $bindings <<ListboxSelect>> [mymethod _onSelect]
        
        grid $bindings $win.vscroll -sticky news
        grid $win.hscroll           -sticky ewn
        
        $self configurelist $args
        
        
    }
    #---------------------------------------------------------------------------
    #  Configuration methods.
    
    ##
    # _update
    #    Update the list box from a new set of binding definitions.
    # @param name  - option name (-bindings)
    # @param value - option values
    #
    method _update {name value} {
        set options($name) $value
        
        $bindings delete 0 end
        
        foreach binding $value {
            set text $binding
            if {[llength $binding] == 2} {
                set text "[lindex $binding 0] -> [lindex $binding 1]"
            }
            $bindings insert end $text
            
        }
        
    }
    #--------------------------------------------------------------------------
    #  Event handling
    #
    
    ##
    # _onSelect
    #   A selection was performed. If there is a -selectscript then
    #   invoke it with the selected binding passed as a parameter.
    #
    method _onSelect {} {
        set index [$bindings curselection]
        set script $options(-selectscript)
        if {($index ne "")  && ($script ne "")} {
            set binding [lindex $options(-bindings) $index]
            #puts "uplevel #0 $script [list $binding]"
            uplevel #0 $script [list $binding]
        }
    }
}

##
# @class container::Creator
#    Create a container from an existing definition or from whole cloth.
#
#    +--------------------------------------------------+
#    |  Name: [      ]   Image [         ] [Browse...]  |
#    |                                                  |
#    | Bindings                                         |
#    |   From: [    ] To: [         ][Browse...]   [Add] [Remove]|
#    |   +---------------------------------+            |
#    |   |   Bindings list (scrollable)    |  [Remove]  |
#    |   +---------------------------------+            |
#    | InitScript [ filename ]   [Browse...]            |
#    +--------------------------------------------------+
#    |  [ Ok ]    [ Cancel ]                            |
#    +--------------------------------------------------+
#
#  @note - If creating from an existing container definition,
#          if there is an init script it is written to a file in /tmp and
#          that file is initially populated in the initscript entry.
#          This is necessary because the contents of the file are pulled in
#          not a reference to the file itself, in case it lives in a filesystem
#          that will not be visible to the container.
#  @note - users must take care, if they run this inside a container
#          that the From path of a binding is a host binding not
#          a mapped binding in the container (e.g.
#         /usr/opt/opt-buster not /usr/opt/).
#
#  OPTIONS
#    -name   - Container name.
#    -image  - image path.
#    -bindings - List of bindings (same form as container::listDefinitions gives).
#    -initscript - contents - current contents of an initscript.
#    -initfile   - Initialization file.
#    -okscript   - Script to execute on ok.
#    -cancelscript - Script to execute on cancel.
#    -enabletop   - Allows or disallows some editing
#
#
#          
snit::widgetadaptor container::Creator {
    option -name -default  [list]
    option -image -default [list]
    option -initscript -default [list] -configuremethod _configInitScript
    option -initfile -default [list] -readonly 1 -configuremethod _configDisallow
    option -okscript -default [list]
    option -cancelscript -default [list]
    option -enabletop -default 1 -configuremethod _setTopState
    
    component bindings
    delegate option -bindings to bindings
    
    constructor args {
        installhull using ttk::frame
        
        ttk::label $win.namelabel -text Name:
        ttk::entry $win.name -textvariable [myvar options(-name)]
        ttk::label $win.imagelabel -text Image:
        ttk::entry $win.image -textvariable [myvar options(-image)]
        ttk::button $win.imagebrowse -text Browse... -command [mymethod _browseImage]
        
        grid $win.namelabel $win.name $win.imagelabel $win.image $win.imagebrowse \
            -sticky news -padx 3
        
        ttk::label $win.bindingslabel -text Bindings
        grid $win.bindingslabel -sticky nsw
        
        ttk::label $win.fromlabel -text From:
        ttk::entry $win.from
        ttk::label $win.tolabel  -text To:
        ttk::entry $win.to
        ttk::button $win.tobrowser -text Browse... -command [mymethod _browseTo]
        ttk::button $win.addbinding -text Add -command [mymethod _addBinding]
        ttk::button $win.rmvbinding -text Remove -command [mymethod _removeBinding]
        
        grid $win.fromlabel $win.from \
            $win.tolabel $win.to $win.tobrowser \
            $win.addbinding $win.rmvbinding -sticky nsew -padx 3
        
        install bindings using container::BindingsList $win.bindings \
            -selectscript [mymethod _onBindingSelect] -width 50
        ttk::button $win.removebinding -text Remove -state disabled
        grid $bindings  -row 3 -column 0 -columnspan 5 -sticky nsew
        grid $win.removebinding -row 3 -column 5 -sticky e
        
        ttk::label $win.initscriptlabel -text {Init Script:}
        ttk::entry $win.initscript -textvariable [myvar options(-initfile)]
        ttk::button $win.browseinit -text Browse... -command [mymethod _browseInit]
        
        grid $win.initscriptlabel $win.initscript $win.browseinit -padx 3
        
        set actions [ttk::frame $win.actionframe -relief groove -borderwidth 4]
        ttk::button $actions.ok -text Ok -command [mymethod _dispatch -okscript]
        ttk::button $actions.cancel -text Cancel -command [mymethod _dispatch -cancelscript]
        
        grid $actions.ok $actions.cancel -padx 3
        grid $actions -sticky nsew -columnspan 7
        
        $self configurelist $args
    }
    #---------------------------------------------------------------------------
    #  Configuration handlers.
    
    ##
    # _setTopState
    #    Used to enable/disable the container name and image text entries.
    #
    method _setTopState {name value} {
        set options($name) $value
        if {$value} {
            set state normal
        } else {
            set state disabled
        }
        $win.name configure -state $state
        $win.image configure -state $state
        $win.imagebrowse configure -state $state
        $win.initscript configure -state $state
        $win.browseinit configure -state $state
    }
    
    ##
    # _configInitScript
    #    called to configure the initialization Script. This is done when
    #    loading an existing configuration.  The existing configuration
    #    contains, not the name of the init script but its contents.
    #    These are written to a /tmp file, that filename is loaded into
    #    -initfile.
    #  @param name option being configured -initscript
    #  @param value - value of that option - the script.
    #
    method _configInitScript {name value} {
        set options($name) $value
        set contents $value;               # Could be wrapped in {}
        set fd [file tempfile options(-initfile) /tmp/initscript]
        puts $fd $contents
        close $fd
    }
    ##
    # _configDisallow
    #     Called to indicate it's not allowed to set a specific option
    #    this is required for options that are -readonly but we don't
    #    want people to even be able to set them at construction time
    #   (e.g. -initfile)
    #
    # @param name - name of the option
    # @param value - Value proposed for the option.
    #
    method _configDisallow {name value} {
        error "Configuring option $name is not allowed container::Creator construction"
    }
    #---------------------------------------------------------------------------
    #  Event handlers.
    ##
    # _onBindingSelect
    #     Called when a user selects a binding in the bindings component.#
    #     The selected binding gets loaded into the From and if necessary To
    #     boxes
    #
    # @param binding - the binding selected. This is a one or two element list.
    #                  If one element, only the From was selected.  If two
    #                  From and two were both selected.
    # @note we (ab)use the fact that lindex off the end of a list returns an empty
    #       string
    #
    method _onBindingSelect {binding} {
        $win.from delete 0 end
        $win.from insert end [lindex $binding 0]
        
        $win.to delete 0 end
        $win.to insert end [lindex $binding 1]
        
    }
    ##
    # _browseInit
    #    Browses for an initialization file.  If a file is accepted:
    #    *   It's path is loaded into -initfile and
    #    *   It's contents loaded into -initscript.
    #
    method _browseInit {} {
        set filename [tk_getOpenFile -parent $win -title "Select init-script" \
            -defaultextension .sh -filetypes [list                         \
                [list {Shell scripts} .sh     ]                            \
                [list {Bash scripts}  .bash   ]                            \
                [list {All Files}     *       ]                            \
            ]
        ]
        #
        #  Filename is empty if e.g. cancel was clicked
        #
        if {$filename ne ""} {
            set  options(-initfile) $filename
            set fd [open $filename r]
            set options(-initscript) [read $fd]
            close $fd
        }
    }
    ##
    # _browseImage
    #    Browse for a container image file.  If one is selected it is loaded into
    #    -image.
    #
    # @note - This browser can be problematic as the filesystem location
    #         _must_ be one in the host system while this program itself is often
    #         run inside a container.  This makes me want to have symlinks from
    #         from /usr/opt-container-type -> /usr/opt/container even though
    #         they'll be broken links, they'll provide a 'correct' host
    #         image 'filename'.
    #
    method _browseImage {} {
        set filename [tk_getOpenFile -parent $win -title "Select container image" \
            -defaultextension .img                                \
            -filetypes [list                                     \
                [list "Image files"  .img]                       \
                [list "Singularity image files" .simg ]          \
                [list "All files"    *]                          \
            ]                                                    \
        ]
        if {$filename ne ""} {
            set options(-image) $filename
        }
    }
    ##
    # _browseTo
    #     Browse to find a mountpoint on which  a new binding will be mounted.
    #     If a directory is chosen, it is loaded into $win.to.  Note this
    #     directory should be a location in the container filesystem.  It
    #     is possible this directory will not actually exist and need to be
    #     typed in by hand.  If so, so be it.
    #
    method _browseTo {} {
        set dirname [tk_chooseDirectory                         \
            -parent $win -title {Bind to...} -mustexist 0       \
        ]
        if {$dirname ne ""} {
            $win.to delete 0 end
            $win.to insert end $dirname
        }
    }
    ##
    # _addBinding
    #   Adds a new binding.  While the from field must be non-empty and
    #   (should) refer to a directory in the host filesystem, the to directory
    #   can be empty indicating the host directory is mounted in the same place
    #   on the container filesystem.
    #
    method _addBinding {} {
        set from [$win.from get]
        set to   [$win.to get]
        if {$from eq ""} {
            tk_messageBox -icon error -parent $win -type ok  -title "Blank From" \
                -message "The 'from' field of a binding must not be empty"
        } else {
            set binding [list $from]
            if {$to ne ""} {
                lappend binding $to
            }
            #  Get the current set of bindings and append this one:
            
            set b [$bindings cget -bindings]
            lappend b $binding
            $bindings configure -bindings $b
            
            # Clear out the bindings entries:
            
            $win.from delete 0 end
            $win.to   delete 0 end
        }
    }
    ##
    # _removeBinding
    #    Removes the binding definition in from/to.  There are three possibilities:
    #    1.  There's no From - that's an error.
    #    2.  There's no match between the from/to and the -bindings, that too is an
    #        error.
    #    3. There's a match, the matching element is removed and $bindings
    #       reconfigured.
    #
    method _removeBinding {} {
        set from [$win.from get]
        set to   [$win.to get]
        set b    [$bindings cget -bindings]
        
        if {$from eq ""} {
            tk_messageBox -icon error -parent $win -type ok -title "Blank From" \
                -message "The 'from' field is empty so there's no binding to remove"
            
        } else {
            set deleteme [list $from]
            if {$to ne ""} {
                lappend deleteme $to
            }

            set found [lsearch -exact $b $deleteme]
            if {$found == -1} {
                tk_messageBox -icon error -parent $win -type ok -title "No match" \
                    -message "There's no matching binding to remove"
            } else {
                set b [lreplace $b $found $found]
                $bindings configure -bindings $b
            }
        }
        
        
    }
    ##
    # _dispatch
    #   Called to dispatch a script from either the Ok button or the Cancel button.
    #
    # $param name name of the option that holds the script to run.
    #
    method _dispatch {name} {
        if {$options(-initfile) ne ""} {
            set fd [open $options(-initfile) r]
            set options(-initscript) [read $fd]
            close $fd
        }
        parray options
        set script $options($name)
        if {$script ne ""} {
            uplevel #0 $script
        }
    }
}

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
    component currentimage;       # Label with current image name.
    component currentbindings ;   # listbox with current bindings list.
    component initscript;         # Button to popup current init script if one.
    
    constructor args {
        installhull using ttk::frame
        
        # Container list.
        
        install containerNames using ::listbox $win.names -selectmode single \
            -yscrollcommand [list $win.cscroll set]
        bind $containerNames <<ListboxSelect>> [mymethod _onSelect] 
        ttk::scrollbar $win.cscroll -orient vertical \
            -command [list $containerNames yview]
        
        grid $containerNames $win.cscroll -sticky nsew
        
        # Frame and contents for the selected image.
        #  - component currentimage    - currently selected image.
        #  - Component currentbindings - List of current bindings.
        #  - Component initscript      - Shows the current initialization script
        
        ttk::frame $win.current
        install currentimage using ttk::label $win.current.image \
            -text "Current image:       "
        
        install currentbindings using container::BindingsList $win.current.bindings 
        
        install initscript using ttk::button $win.current.script \
            -text "Init Script.." -command [mymethod _showCurrentScript] \
            -state disabled
        
        
        grid $currentimage -sticky nsew
        grid $currentbindings  -sticky nsew
        grid $initscript
        
        grid $win.current -row 0 -column 2 -sticky nsew
    
        #  Now the action buttons:
        
        set actions [ttk::frame $win.actionarea -relief groove -borderwidth 4]
        ttk::button $actions.new -text New... -command [mymethod _onNew]
        ttk::button $actions.edit -text Edit... -command [mymethod _onEdit]
        grid $actions.new $actions.edit
        grid $actions -sticky ew -columnspan 3
        
        # Process options
        
        $self configurelist $args
    }
    
    #--------------------------------------------------------------------------
    # Configuration:
    #
    
    method _update {optname optval} {
        set options($optname) $optval;   # now cget works.
        
        $containerNames delete 0 end
        foreach container $optval {
            $containerNames insert end [dict get $container name]
        }
        #  Select the first item in the list:
        
        $containerNames selection set 0;   # Forces _onSelect call.
        event generate $containerNames <<ListboxSelect>>
    }
    #---------------------------------------------------------------------------
    # Event handling
    #
    
    ##
    # _onSelect
    #   Called when a container name is selected from th elist box.
    #   we populate the rest of the UI based on which  was
    #   selected.
    #
    method _onSelect {} {
        set selected [$self _getCurrent]
        $self _emptySelected
        if {$selected ne ""} {
            $currentimage configure -text "Current Image: [dict get $selected image]"
            
            # Update the bindings list component:
            
            set bindingsList [list]
            if {[dict exists $selected bindings]} {
                set bindingsList [dict get $selected bindings]
            }
            $currentbindings configure -bindings $bindingsList
            
            #  set the stae of the initscript button accordingly:
            
            if {[dict exists $selected init]} {
                $initscript configure -state normal
            } else {
                $initscript configure -state disabled
            }
        }
    }
    ##
    # _showCurrentScript
    #    Called when the button that shows the current image's initscript
    #    is clicked.  We create a new toplevel named
    #    win.initscript - Destroying it if it exists already.
    #    The top level looks like this:
    #
    #    +---------------------------------------------+
    #    |   scrolling text widget with script         |
    #    +---------------------------------------------+
    #    |"Init script for container: name" [dismiss]  |
    #    +---------------------------------------------+
    #
    #  The top level title will also contain the same text as the label
    #  at the bottom.
    #  Dismiss's command will be destroy $win.initscript.
    #
    #
    method _showCurrentScript {} {
        set info [$self _getCurrent]
        if {$info ne ""} {
            if {![winfo exists $win.initscript]} {
                toplevel $win.initscript
                text $win.initscript.script -yscrollcommand [list $win.initscript.sb set]
                ttk::scrollbar $win.initscript.sb -command [list $win.initscript.script yview] \
                    -orient vertical
                ttk::label $win.initscript.name
                ttk::button $win.initscript.dismiss -text Dismiss -command [list destroy $win.initscript]
                
                grid $win.initscript.script -row 0 -column 0 -sticky nsew
                grid $win.initscript.sb     -row 0 -column 1 -sticky nsw
                grid $win.initscript.name   -row 1 -column 0 -sticky w
                grid $win.initscript.dismiss -row 1 -column 1 -sticky e
            }
            # We know the top level exists fill it in:
            
            set name [dict get $info name]
            set text [dict get $info init]
            set title "Init script for container: $name"
            wm title $win.initscript  $title
            $win.initscript.script delete 1.0 end
            $win.initscript.script insert end $text
            $win.initscript.name configure -text $title
        }
        
        
        
        
    }
    #-----------------------------------------------------------------
    # Editing
    #
    
    ##
    # _onNew
    #    If $win.containereditor  does not exist,
    #    bring one up and attach the Ok script to make the new
    #    container and the cancelscript to just kill the widget.
    #    We aso bind the destroy event.  While the container widget is alive,
    #    New... and Edit... are disabled.  This provides us with a
    #    non-modal editing dialog.
    #
    method _onNew {} {
        if {$options(-newcommand) ne ""} {
            uplevel #0 $options(-newcommand)
        }
        
    }
    ##
    # _onEdit
    #    Pretty much same as _onNew except:
    #    - The editor is loaded with the information from the currently selected
    #      container (If there isn't one that's an error).
    #    - The ok method dispatches to -replacecommand
    #
    method _onEdit {} {
        if {[winfo exists $win.containeredit] } {
            return
        }
        set selected [$self _getCurrent]
        if {$selected eq ""} {
            return
        }
        #  Now we can build the editor user interface:
        set bindings [list]
        if {[dict exists $selected bindings]} {
            set bindings [dict get $selected bindings]
        }
        
        toplevel $win.containereditor
        container::Creator $win.containereditor.editor \
            -okscript [mymethod _acceptContainer -replacecommand] \
            -cancelscript [mymethod _cancelEditor]                       \
            -name [dict get $selected name] -image [dict get $selected image] \
            -bindings $bindings
        
        if {[dict exists $selected init]} {
            $win.containereditor.editor configure \
                -initscript [dict get $selected init]
        }
        pack $win.containereditor.editor
        bind $win.containereditor <Destroy> [mymethod _editorKilled %W]
    }
    
    ##
    # _editorKilled
    #   Processes Destroy events in $inw.containereditor.
    #   If the toplevel is being destroyed, we can re-enable our new/edit
    #   buttons.
    #
    # @param w - the widget being destroyed.  Note we'll see events for each
    #            child of the top level, these must be ignored    
    #
    method _editorKilled {w} {
        if {$w eq "$win.containereditor"} {
            $win.actionarea.new  configure -state normal
            $win.actionarea.edit configure -state normal
        }
    }
    ##
    # _cancelEditor
    #   Bound to container::Creator's cancel button. We really just need to destroy
    #   the top level widgt.  _editorKilled will take care of the rest:
    #
    method _cancelEditor {} {
        destroy $win.containereditor
    }
    ##
    # _acceptContainer
    #   Called when container::Creator 's Ok button was clicked
    #   - Pull the information out of the editor about the container.
    #   - Invoke the appropriate script
    #
    # 
    method _acceptContainer {optname} {
        set script $options($optname)
        if {$script ne ""} {
            set name [$win.containereditor.editor  cget -name]
            set image [$win.containereditor.editor cget -image]
            set bindings [$win.containereditor.editor cget -bindings]
            set scriptfile [$win.containereditor.editor cget -initfile]
            
            set info [dict create name $name image $image bindings $bindings]
            if {$scriptfile ne ""} {
                dict set info scriptfile $scriptfile
                dict set info init  [$win.containereditor.editor cget -initscript]

            }
            # We must do the [list] below as uplevel unravles one level of listiness
            # (I think) and we want to keep the dict intact (dicts look like lists)
            #
            uplevel #0 $script [list $info]
            
            
        }
        destroy $win.containereditor
    }
    #------------------------------------------------------------------
    #  Utilities
    #
    ##
    # _getCurrent
    # @return dict - return the currently selected container definition dict.
    #                Empty string if there is no current selection.
    #
    method _getCurrent {} {
        set selection [$containerNames curselection]
        set result [dict create]
        if {$selection ne ""} {
            set result [lindex $options(-containers) $selection]
        }
        
        return $result
    }
    ##
    # _emptySelected
    #    Clear out the selection.
    #
    method _emptySelected {} {
        $currentimage configure -text "Current Image: "
        $initscript configure -state disabled
    }
}

##
# @class container::listbox
#    Provides a list of containers given their definitions.  This is actually
#    a ttk::treeview with the name and image as columns.
#    double clicking an item fires off a command.
#
# OPTIONS:
#   -containers - container definitions from e.g. container::listDefinitions.
#   -command    - Command invoked on a container double click
#
snit::widgetadaptor container::listbox {
    option -containers -configuremethod _loadTree
    option -command -default [list]
    
    constructor {args} {
        installhull using ttk::frame
        ttk::treeview $win.list -yscrollcommand [list $win.sb set] \
            -columns [list name image definition]            \
            -displaycolumns [list name image]  -show headings \
            -selectmode extended
        $win.list heading name -text Name
        $win.list heading image -text Image
        bind $win.list  <<TreeviewSelect>> [mymethod _dispatchCommand]
        
        ttk::scrollbar $win.sb -orient vertical -command [list $win.list yview]
        
        grid $win.list $win.sb -sticky nsew
        
        $self configurelist $args
    }
    #-------------------------------------------------------------------------
    #  Configuration handling:
    
    ##
    # _loadTree
    #    Called when the -containers option is configured:
    #    - Empty the tree.
    #    - Fill it with new stuff.
    #    - set options(-containers) to the list so cget works.
    #
    # @param name - option name being set.
    # @param value - Proposed new value.
    #
    method _loadTree {name value} {
        set options($name) $value
        
        set items [$win.list children {}]    
        $win.list delete $items
        
        foreach container $value {
            set name [dict get $container name]
            set image [dict get $container image]
            $win.list insert {} end -values [list $name $image $container] \
                -tags [list container]
        }
    }
    #------------------------------------------------------------------------
    # Event handling.
    
    ##
    # _dispatchCommand
    #    Called on a double click in a container in the list.
    #    If there is a -command script:
    #    *  If there is a selection:
    #       - Get the definition of the selection.
    #       - Remove the selection.
    #       - Invoke the -command script with the definition as a parameter.
    #    * If there is no selection:
    #       - Invoke the command with an empty list as a parameter.
    #
    method _dispatchCommand {} {
        set script $options(-command)
        if {$script ne ""} {
            set selection [$win.list selection]
            if {$selection ne ""} {
    
                set values [$win.list item $selection -values]
                set def [lindex $values end]
            } else {
                set def [list]
            }
            uplevel #0 $script [list $def]
        }
        
    }

}
##
#  This can be called to test the code.
#
proc container::editorTest {} {
    catch [file delete containereditortest.db]
    exec [file join $::env(DAQBIN) mg_mkconfig] containereditortest.db
    sqlite3 db containereditortest.db
    container::add db a thing1 ~/.bashrc a
    container::add db b thing2 "" \
        [list [list a] [list b /usr/opt/daq/12.0-pre3] [list d]]
    container::add db test minimal.img "~/.profile" ""
    
    container::Editor .e -containers [container::listDefinitions db]
    file delete containereditortest.db      
    
    pack .e -fill both -expand 1
    
}



