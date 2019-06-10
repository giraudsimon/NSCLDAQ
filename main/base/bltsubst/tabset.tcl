#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

##
# @file tabset.tcl
# @brief - Subset implementation of blt::tabset in terms of ttk::notebook

package provide blt::tabset 1.0
package require Tk
package require snit


namespace eval blt {
namespace export tabset
}

snit::widgetadaptor blt::tabset {
    component notebook
    
    # Option delegation for the options that can be supported:
    
    delegate option -cursor      to notebook
    delegate option -pageheight  to notebook as -height
    delegate option -pagewidth   to notebook as -width
    
    delegate option -borderwidth to hull
    delegate option -relief      to hull
    
    # Unimplemented options:
    
    option -activebackground
    option -activeforeground
    option -anchor
    option -background
    option -dashes
    option -endimage
    option -font
    option -foreground
    option -gap
    option -gapleft
    option -highlightbackground
    option -highlightcolor
    option -highlightthickness
    option -outerpad
    option -perforationcommand
    option -rotate
    option -samewidth
    option -scrollcommand
    option -scrollincrement
    option -selectbackground
    option -selectcommand
    option -selectpad
    option -side
    option -slant
    option -startimage
    option -tabbackground
    option -tabrelief
    option -takefocus
    option -tearoff
    option -transient
    option -textside
    option -tiers
    option -tile
    option -width
    
    # Method delegations
    
    delegate method activate to notebook as select
    delegate method invoke   to notebook as select
    delegate method select   to notebook
    
    #
    # Tab options and their equivalents and mapping functions.
    # Each array element is indexed by the blt tab option
    # and contains either an empty list meaning it's ignored
    # or a single option name meaning that the option is
    # directly mapped to the target option, or a 2 element list
    # consisting of a target option and a mapping method that takes
    # as input source-option target-option source-value options
    # and returns a new options list.
    #
    variable tabOptions -array {
        -activebackground ""
        -activeforeground ""
        -anchor -sticky
        -background ""
        -bindtags   ""
        -command    ""
        -data       ""
        -fill       ""
        -font       ""
        -foreground ""
        -image      ""
        -leftimage  "-image _mapImage"
        -ipadx      ""
        -ipady      ""
        -padx       ""
        -pady       ""
        -rightimage "-image _mapImage"
        -selectbackground ""
        -shadow     ""
        -state      "-state"
        -stipple    ""
        -text       "-text"
        -underline  "-underline"
        -window     ""
        -windowheight ""
        -windowwidth  ""
        
    }
    #
    #  Tab names for each tab...indexed by tabname yielding ids.
    #
    variable tabNames -array {}
    variable autoNameIndex    0
    
    ##
    # constructor
    #    Create a ttk::frame hull with a ttk::notebook inside:
    #
    # @param args - option/value pairs:
    #
    constructor args {
        installhull using ttk::frame
        install notebook using ttk::notebook $win.notebook
        
        grid $notebook -sticky nsew
        
        $self configurelist $args
    }
    

    #-------------------------------------------------------------------------
    # Public methods
    #
    
    ##
    # bind
    #
    #  Binds an event to a tab.  Note that this really binds an event to the
    #  notebook widget that dispatches the binding if the event happened over the
    #  the specified tab.
    #
    # @param tagName One of the name of the tag to bind to or 'all' or 'Perforation'
    #                'all' or 'Performation' accept the binding on all tabs.
    # @param sequence An event sequence bound.
    # @param command  Scriptlet to bind.
    #
    method bind {tagName sequence command} {
        $self _killOldBinding $tagName $sequence $command
        
        bind $notebook $sequence +[mymethod _bindingFired %x %y $tag $command]
    }
    ##
    # delete
    #   Delete one or more tabs from the tabset specified by index
    #
    #  @param first - Index of the first one to kill off.
    #  @param ?last? - Index of the last tab to kill off, defaults to first.
    #
    method delete {first {last {}}} {
        
        # Default the last tab index
        
        if {$last eq ""} {
            set last $first
        }
        # Delete backwards so that indices are still valid:
        
        for {set i $last} {$last >= $first} {incr i -1} {
            
        }
    }
    ##
    # focus
    #   unimplemented for now.
    #
    method focus index {}
    
    ##
    # get
    #   returns the name of a tab given a valid  index value.
    #
    # @param index - index of the tab.
    #
    # @return tabname.
    # 
    method get index  {
        set tabNo [$self index -index $index]
        foreach name [array names tabNames] {
            if {$tabNames($name) == $tabNo} {
                return $name
            }
        }
        error "can't find tab with index $index"
    }
    ##
    # highlight
    #
    # @param index
    #
    #  unimplemented.
    #
    method highlight index {}
    
    ##
    # index
    #
    #  Given Given any of sevarl possible value returns the index of the tab.
    #
    # @param args - of the form ?flag? string
    #
    # @return Returns the node id of the tab specified by string. If flag is
    #        -name, then string is the name of a tab. If flag is -index, string
    #         is an index such as "active" or "focus". If flag is -both, string
    #         is either. If flag isn't specified, it defaults to -index.
    # 
    method index args {
        if {[llength $args] == 1} {
            set match -index
            set string $args
            
        } elseif {[llength $args] ==2} {
            set match [lindex $args 0]
            set string [lindex $args 1]
        } else {
            error "wrong number of args for $notebook get ?flag? string"
        }
        # Now do the indexing:
        
        if {$match eq "-index"} {
             return [$self _findIndex $string]
            
        } elseif {$match eq "-name"} {
            if {[array name exists tabNames $string]} {
                return $tabNames($string)
            } else {
                error "Can't find tab named \"$string\" in $win"
            }
            
        } elseif {$match eq "-both"} {
            if {[array names tabNames $string] ne ""} {
                return $tabNames($string)
            }
            return [$self _findIndex $string]
        } else {
            error "$notebook get flag must be -index | -name | -both was $match"
        }
        
    }


    ##
    # insert
    #     Inserts new tabs into the tabset. Tabs are inserted just before the
    #     tab given by position. Position may be either a number, indicating
    #     where in the list the new tab should be added, or end, indicating
    #     that the new tab is to be added the end of the list. Name is the
    #     symbolic name of the tab. Be careful not to use a number. Otherwise
    #     the tabset will confuse it with tab indices. Returns a list of indices
    #     for all the new tabs. If tab name is an empty string, or ends in
    #     #auto, it is generated using the given prefix. The list of created
    #     tab names is returned.
    #
    # @param position - where to insert the tab.
    # @param name     - Name of the new tab...see above.
    # @param args     - Tab options and values.
    #
    # @return The list of created tab names is returned.
    #
    method insert {position name args} {
        set opts [$self _constructOptions $args]
        set window  [$self _locateWindow $args]
        set name    [$self _generateTabName $name]
        
        # Adjust the position.. If $position >= number of items,
        # convert it to 'end'.
        #
        if {$position >= [llength [$notebook tabs]]} {
            set position end
        }
        $notebook insert $position $window  {*}$opts -text $name
        
        # Successful insertion...need to adjust following indices:
        
        if {$position eq "end"} {
            set tabs [$notebook tabs]
            set tabNo [expr {[llength $tabs] -1 }]
            set tabNames($name) $tabNo
        } else {
            foreach tname [array names tabNames] {
                if {$tabNames($tname) >= $position} {
                    incr tabNames($tname)
                }
            }
            set tabNames($name) $position
        }
        return $name
    }
    ##
    # move
    #   Moves a tab to a new position.
    #
    # @param index  - Index of the tab to move.
    # @param ba     - The text "before" or "after" indicating where
    #                 relative to the newindex to move the tab.
    # @param newIndex - Where to move the tab (see ba).
    #
    #  Not supported.. given time what would be needed is to save the set of
    #  options for each tab and then delete the tab and recreated it in the new
    #  position.
    method move {index ba newIndex} {
        
    }
    ##
    # nearest
    #   Returns the name of the tab nearest the specified x/y coordinates.
    #
    # @param x - X coordinate.
    # @param y - Y coordinate.
    # @param ?varname? - If the argument varName is present, this is a Tcl
    #                    variable that is set to either text, icon, icon2,
    #                    startimage, endimage, perforation or the empty string
    #                     depending what part of the tab the coordinate is over
    #
    # @return string - tab name or "" if there's no match.
    #
    # @note this implementation always sets the variable to 'text'.
    #
    method nearest {x y {varname {}}} {
        if {$varname ne ""} {
            uplevel $varname location
            set location text
        }
        set status [catch {
            $self index @$x,$y 
        } index]
        if {$status} {
            return ""
        }
        foreach tabName [array names tabNames] {
            if {$tabNames($tabName) eq $index} {
                return $tabName
            }
        }
        return ""
    }

    ##
    # perforation
    #
    #   blt::tabset supports perforated tabs.  ttk::notebook does not
    #   In BLT this "operation controls the perforation on the tab label."
    #
    method perforation args {}
    
    ##
    # scan
    #
    #  blt::tabset supports tab scanning.  ttk::notebook does not.
    #  in BLT: "This command implements scanning on tabsets."
    #
    method scan args {}
    
    ##
    # see
    #
    #  blt::tabset suipports tab scrolling.   ttk::notebook, if it does does
    #  not allow you to control the scrolling.
    #  In BLT this would scroll " the tabset so that the tab index is visible in the widget's window."
    #
    method see index {}
    
    ##
    # size
    #
    #   Returns the number of tabs in the tabset.
    #
    method size {} {
        return [$notebook index end]
    }
    ##
    # tab
    #
    #  Ensemble of commands that operate on tabs (some stubbed some not).
    #
    # @param operation - The operation to perform which is one of the following
    #     *  cget      - get the a configuration of an option.
    #     *  configure - Configure an option
    #     *  dockall   - Docks torn off tabs (stub).
    #     *  names     - Returns names of tabs matching a pattern.
    #     *  pageheight - (undocumented  stub)
    #     *  pagewidth  - (undocumented stub)
    #     *  select     - Selects a tab by name or index.
    #     *  tearoff    - Tears off a tab (undocumented stub).
    #
    #  @note
    #     For more information about the each of the operations and its parameters,
    #     see the comments at _tabOp where Op is the operation name e.g. _tabCget
    method tab {operation args} {
        
        
        
        # Ensure the operation is valid:
        
        set validops \
            [list cget configure dockall names pageheight pagewidth select tearoff]
        
        if {$operation ni $validops} {
            error "$operation should have been one of: $validops"
        }
        
        # compute the method name:
        
        append operationMethod _tab [string totitle $operation]
        
        $self $operationMethod {*}$args
    }
    
    ##
    # view
    #   Implements tab scrolling operations.  Since ttk::notebook
    #   does not support these this is a no-op.
    #
    method view args {}
    ##
    # 
    #-------------------------------------------------------------------------
    # Private methods:
    #
    ##
    #  _killOldBinding
    #
    #    IF a command script demands it, kill of an existing binding.
    #
    # @param tagName - name of the tag bound.
    # @param seq     - Sequence of operations bound.
    # @param command - Command bound.
    #
    method _killOldBinding {tagName seq command} {
        
        # If command does not start with a + then we have to locate and
        # remove any binding for that sequence.
        #  Each binding is of the form:
        #  snit leading junk _bindingFired x y tag command
        #
        if {[string range $command 0 0] ne "+"} {
            set existingBindings [bind $notebook $seq]
            
            bind $notebook $seq $tagName "";    # Erase the bindings.
            
            foreach binding $existingBindings {
                set i [lsearch -exact $binding _bindingFired]
                incr i 3;           # index of the tag.
                if {$tagName ne [lindex $binding $i]} {
                    bind $notebook $seq +$binding;     # reassert nonmatching binding
                }
            }
        }
        
    }
    ##
    # _bindingFired
    #
    #    Called when a binding fired on the notebook.  If appropriate
    #    the user script is called:
    #
    # @param x - Widget relative x position of the cursor on the event.
    # @param y - Widget relative y positino of the cursor on the event.
    # @param tag - Widget tag desired for the binding.
    # @param command  - Script associated with the binding.
    #
    method _bindingFired {x y tag command} {
        #
        #  IF x,y correspond to the tag, then invoke the command via uplevel #0.
        #  or if the tag is one of 'all' or 'Peforation'
        set tabNo [$notebook index -index @$x,$y]
        if {($tabNames($tag) == $tabNo) || ($tag in [list all Perforation])} {
            uplevel #0 $command
        }
    }
    ##
    # _findIndex
    #
    #   Locates a tab index using any of the index values of the ttk::notebook
    #   widget.
    #
    # @param index - lookup value.
    #
    method _findIndex index {
        return [$notebook index $index]
    }
    ##
    # _constructOptions
    #
    #   Construct the widget option list for insert from the BLT option list.
    #
    # @param options - list of option/value pairs.
    # @return  string - option value pairs.
    #
    method _constructOptions opts {
        set optionString [list]
        foreach {option value} $opts {
            if {[array names tabOptions $option] eq ""} {
                error "No such option: $option"
            }
            set mapping $tabOptions($option)
            
            # 1 element mapping just append the target option and value.
            
            if {[llength $mapping] == 1} {
                lappend optionString $mapping $value
            }
            # 2 element mapping requires the option value to be munged
            
            if {[llength $mapping] == 2} {
                set targetOption [lindex $mapping 0]
                set mappingMethod [lindex $mappgin 1]
                set optionString \
                    [$self $mappingMethod $option $targetOption $value $optionString]
                
            }
        }
        return $optionString
    }
    ##
    # _locateWindow
    #
    # Given a bunch of blt option/values determines the child window
    # by locating the -window option and returning its value.
    # an error is raised if no -window is present.
    #
    # @param options - The option/value list.
    # @return string - The window name.
    #
    method _locateWindow  opts {
        foreach {option value} $opts {
            if {$option eq "-window"} {
                return $value
            }
        }
        error "a -window option is required."
    }
    ##
    # _generateTabName
    #
    #   Given the tab name parameter generates the actual tab name.
    #
    # @param name - template name.
    # @return string -final name.. See the insert method for information
    #                 about how this is derived.
    #
    method _generateTabName name {
        if {$name eq ""} {
            set name #auto
        }
        if {[string range $name end-4 end] eq "#auto"} {
            set name [string replace $name end-4 end _$autoNameIndex]
            incr autoNameIndex
        }
        return $name
    }
    ##
    # _mapImage
    #
    #    Handles the -leftimage/-rightimage conversion to tabset options.
    #    notebook is restricted to a single image. Therefore:
    #    - If the option set already has a -compound switch throw an error.
    #    - Provide an -image option and
    #    - Determine and also add the appropriate -compound option.
    #
    # @note No validity checking on the image is done..ttk::notebook will
    #       presumably do that.
    #
    # @param sourceOption - The option we are handling
    # @param targetOption - The targe option (-image).
    # @param optionValue  - The value of the option (image name).
    # @param currentOptions - The current option set.
    #
    # @return list - output options.  We use list operations to get automated
    #                quoting and, after all, commands are lists.
    #
    method _mapImage {sourceOption targetOption optionValue currentOptinos} {
        if {[lsearch -exact $currentOptions -compound] ne -1} {
            error "blt::tabset only supports one of -leftimage right image not both"
        }
        lappend currentOptions -image $optionValue -compound
        if {$sourceOption eq "-leftimage"} {
            lappend currentOptions left
        } else {
            lappend currentOptions right
        }
        return $currentOptions
    }
    
    #--------------------------------------------------------------------------
    #
    #   tab subcommand processors:
    #     *  _tabCget      - get the a configuration of an option.
    #     *  _tabConfigure - Configure an option
    #     *  _tabDockall   - Docks torn off tabs (stub).
    #     *  _tabNames     - Returns names of tabs matching a pattern.
    #     *  _tabPageheight - (undocumented  stub)
    #     *  _tabPagewidth  - (undocumented stub)
    #     *  _tabSelect     - Selects a tab by name or index.
    #     *  _tabTearoff    - Tears off a tab (undocumented stub).
    
    ##
    # _tabCget
    #    Perform the tab cget method.  This allows users to retrieve an option
    #    value from a tab.  Note that BLT tab options are mapped to
    #    ttk::notebook tab options where there are corresponding options but
    #    "" is returned if there is no corresponding option.
    #
    #  @param nameOrIndex - Name or index of a tab (we're going to use
    #                       the index method to resolve this).
    #  @param option      - Name of BLT tab option.
    #
    method _tabCget {nameOrIndex option} {
        set id [$self index -both $nameOrIndex]
        if {$id eq ""} {
            error "No such tab $nameOrIndex"
        }
        # Ensure the option is valid too:
        
        if {[array names tabOptions $option] eq ""} {
            error "$option is not a legal blt::tabset tab option"
        }
        # Now map the option  we're only interested in two cases:
        # empty string - return the empty string.
        # nonempty string - use the first list element as the tab option for
        #                   ttk::notebook:
        
        set targetOption [lindex $tabOptions($option) 0]
        if {$targetOption eq ""} {
            return ""
        } else {
            return [$notebook tab $id $targetOption]
        }
    }
    ##
    # _tabConfigure
    #
    #    Implements a restricted version of the blt::tabset tab configure
    #    method.  Restrictions are:
    #    * only one nameOrIndex is allowed.
    #    * This cannot be used to query option values.
    #
    # @param nameOrIndex - Name or index of the tab, this is run through the index
    #                      method.
    # @param args        - option value pairs.
    #
    # @note - known defect: If there's already a -leftimage and you provide
    #         a -rightimage this is not detected as an error as for the
    #         initial widget creation operation, but replaces the current
    #         setting (similarly for existing -rightimage and provided -leftimage).
    #
    method _tabConfigure {nameOrIndex args} {
        set id [$self index -both $nameOrIndex]
        if {$id eq ""} {
            error "No such tab $nameOrIndex"
        }
        set opts [$self _constructOptions $args]
        $notebook tab $id {*}$opts
    }
    ##
    # _tabDockAll
    #   (stub) docks all torn off tabs.
    #
    method _tabDockAll {} {}
    
    ##
    # _tabNames
    #
    #   Returns the names of all tabs that match a pattern
    #
    #  @param pattern - glob pattern to match.
    #
    #  @return list - (possibly empty) of the names of tabs that match the
    #                  pattern parameter.
    #
    method _tabNames pattern {
        return [array names tabNames $pattern]
    }
    
    ##
    # _tabPageheight
    #
    #  Stub -- for that matter this is undocumented in BLT.
    #
    method _tabPageheight {} {}
    
    ##
    # _tabPagewidth
    #
    #  Stub -- for that matter this is undocumented in BLT.
    #
    method _tabPagewidth {} {}
    
    ##
    # _tabSelect
    #
    #   Same as the select method but selection is via valid input to the
    #   index method.
    #
    # @param nameOrIndex - valid input to the index method.
    #
    method _tabSelect nameOrIndex {
        set id [$self index -both $nameOrIndex]
        if {$id eq "" } {
            error "No such tab $nameOrIndex"
        }
        $self select $id
    }
    ##
    # _tabTearoff
    #
    #   Tears of a specified tab.  This is a no-op.
    #
    # @param index - Indeex of the tab to tearoff.
    # @param newName - If provided the name of the toplevel that parents the
    #                  new widgets are returned.
    #
    method _tabTearoff {index {newName {}}} {
        return ""
    }
}
