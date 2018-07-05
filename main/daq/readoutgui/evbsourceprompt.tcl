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
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file evbsourceprompt
# @brief Event builder source  prompter.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide evbsourceprompt 1.0
package require Tk
package require snit

##
#  @class EvbSourcePrompt
#     Form that prompts for event builder data sources. For each source we need:
#    -  Ring buffer host, name.
#    -  List of source ids.
#    -  Textual description of the source.
#  Everything else we provide 'sensible defaults'.   We ignore the possibility
#  of time offsets.  The user can manually add that to the generated file.
#
# Options:
#    -host        - host of ring buffer we're getting data from.
#    -ring        - Name of ringbuffer in that host.
#    -sids        - list of source ids.
#    -description - Description of the source.
#
snit::widgetadaptor EvbSourcePrompt {
    option -host
    option -ring
    option -sids -configuremethod _setSids -cgetmethod _getSids
    option -description
    
    #
    #  Create and layout the widgets.
    constructor args {
        installhull using ttk::frame
        
        # At the top is the stuff managing source ids:
        # The binding to double 1 removes an id from the list box
        # if it is double clicked.
        
        ttk::label $win.idstitle -text {Event source ids: }
        entry      $win.nextid -width 6
        ttk::button $win.add    -text {Add Id} -command [mymethod _addNextId]
        listbox    $win.idlist -yscrollcommand [list $win.idscroll set]
        ttk::scrollbar $win.idscroll -command [list $win.idlist yview]
        bind $win.idlist <Double-1> [mymethod _removeSelectedId]
        bind $win.nextid <Return>   [mymethod _addNextId]
        
        #  Next are the specifications of the scalar configuration items.
        #  of the ring.
        
        ttk::label $win.hosttitle -text Host:
        entry      $win.host      -textvariable [myvar options(-host)] \
                -width 20
        ttk::label $win.ringtitle -text Ring:
        entry      $win.ring      -textvariable [myvar options(-ring)] \
                -width 10
        ttk::label $win.desctitle -text Description:
        entry      $win.descrip   -textvariable [myvar options(-description)] \
                -width 20
        
        # Layout:
        
        # Top row. Individual gridding because of differences in stickiness.
        
        grid $win.idstitle -row 0 -column 0 -sticky w
        grid $win.nextid   -row 0 -column 1 -sticky e
        grid $win.add      -row 0 -column 2 -sticky ew
        grid $win.idlist   -row 0 -column 3 -sticky ew
        grid $win.idscroll -row 0 -column 4 -sticky nsw
        
        # Second row.
        
        grid $win.hosttitle -row 1 -column 0 -sticky e
        grid $win.host      -row 1 -column 1 -sticky w
        grid $win.ringtitle -row 1 -column 2 -sticky e
        grid $win.ring      -row 1 -column 3 -sticky w
        grid $win.desctitle -row 1 -column 4 -sticky e
        grid $win.descrip   -row 1 -column 5 -sticky w
        
        
        # Process options:
        
        $self configurelist $args
    }
    #----------------------------------------------------------------
    #  Event handling methods
    
    ##
    # _addNextId
    #    Adds the id in the $win.netxid text box to the listbox of ids.
    #    We require that the id is an integer (could be any supported base).
    #
    #  On successful add the text box is cleared.
    #
    method _addNextId {} {
        set idText [$win.nextid get]
        if {[string is integer -strict $idText]} {
            $win.idlist insert end $idText
            $win.nextid delete 0 end
        } else {
            tk_messageBox -icon error -title "Bad id" -type ok \
                -message {Ids must be integers}
            return
        }
    }
    ##
    # _removeSelectedId
    #    Remove the selected Id from the list of ids.
    #
    method _removeSelectedId {} {
        set i [$win.idlist index active];   # The active one is what was clicked.
        $win.idlist delete $i
        
    }
    #-----------------------------------------------------------------
    # configuration methods.
    
    ##
    # _setSids
    #   Called to configure -sids.  We must stock the list box.
    #
    # @param optname - name of the option being configured (-sids)
    # @param val     - New value.   list of items.
    # @note the user is expected to have checked the items for legality.
    #
    method _setSids {optname val} {
        $win.idlist delete 0 end;        # clear existing items.
        $win.idlist insert end {*}$val
        
    }
    ##
    # _getSids
    #   Returns a list of the ids currently in the list box.
    #
    #  @param optname -name of the option (-sids).
    #  @return list - possibly empty of ids in the listbox.
    #
    method _getSids optname {
        return [$win.idlist get 0 end]
    }
}
##
# EvbSourceList
#    Megwidget that lists event builder sources. A treeview is used
#    for the display - it's the hull for crying out loud.
#
# OPTIONS:
#    
#   * -sources Maintains the current set of sources.  This is a dict
#      consisting of
#       - host - host name of the ring the data is in.
#       - ring - Name of the ringbuffer the data is in.
#       - description - description of the data source.
#       - sids - list of source ids present in the ring.
#
#  These dict items are also the treeview headers.
#
snit::widgetadaptor EvbSourceList {
    option -sources -configuremethod _updateDisplay
    delegate option * to hull
    delegate method * to hull

    #  Constructor
    #   Create the hull as a treeview and set it up.
    #
    constructor args {
        set columnNames [list host ring description "Source Ids"]
        installhull using ttk::treeview  \
            -columns $columnNames        \
            -displaycolumns $columnNames \
            -selectmode none              \
            -show  headings
        
        foreach  heading $columnNames {
            $hull heading $heading -text $heading  \
                -anchor w
        }
        $self configurelist $args
    }
    #-----------------------------------------------------------
    # internal methods:
    #
    
    ##
    # _clear
    #    Remove all entries from the table.
    #
    method _clear {} {
        set ids [$hull children {}]
        foreach id $ids {
            $hull delete $id
        }
    }
    
    ##
    # _append
    #    Given a source  dict, append it to the table.
    #
    # @param source - Dictionary describing the source.
    #
    method _append source {
        #  Pull out the id list  and the first sid
        #  - the first id will be on the top level, any
        #     additionals will be children.
        
        set sids [dict get $source sids]
        set firstId [lindex $sids 0]
        
        # Now the top line:
        
        set id [$hull insert {} end \
            -values [list [dict get $source host] \
                     [dict get $source ring]      \
                     [dict get $source description] \
                     $firstId]                     \
            -open 1]
        #  Now add any other source ids as children:
        
        foreach sid [lrange $sids 1 end] {
            $hull insert $id end \
                -values [list "" "" "" $sid]
        }
        
        
    }
    ##
    # _updateDisplay
    #    Called when -sources is configured.
    #
    # @param opt   - Name of the option being configured.
    # @param value - new value.
    #
    method _updateDisplay {opt value} {
        $self _clear
        foreach source $value {
            $self _append $source
        }
        set options($opt) $value
    }
}
##
# GetEvbSources
#    Form that requests Event builder sources.
#    This is an EvbSourcePrompt with an Add and Done button at the bottom.
#    For the most part we're just dispatching add and done clicks
#
# OPTIONS
#   * -sources Maintains the current set of sources.  This is a dict
#      consisting of
#       - host - host name of the ring the data is in.
#       - ring - Name of the ringbuffer the data is in.
#       - description - description of the data source.
#       - sids - list of source ids present in the ring.
#   * -addcommand - command called by the add button.
#   * -donecommand - command called by the done command.
# PUBLIC METHODS:
#   *  clear clears the form.
#
snit::widgetadaptor GetEvbSources {
    option -addcommand [list]
    option -donecommand [list]
    
    component form
    component list
    delegate option -sources to list
    delegate option * to form
    
    constructor args {
        installhull using ttk::frame
        
        install list using EvbSourceList  $win.list
        install form using EvbSourcePrompt $win.form
        
        ttk::button $win.add -text Add... \
            -command [mymethod _dispatch -addcommand]
        ttk::button $win.done -text Done  \
            -command [mymethod _dispatch -donecommand]
        
        grid $list -columnspan 4 -sticky nsew
        grid $form -columnspan 4 -sticky nsew
        grid $win.add  -row 2 -column 0
        grid $win.done -row 2 -column 3
        
        $self configurelist $args
    }
    #---------------------------------------------------------------
    # Public methods
    
    ##
    # clear
    #    Clears the form.
    #
    method clear {} {
        $form configure -host "" -ring "" -description "" -sids ""
    }
    #------------------------------------------------------------------
    # Event handling
    #
    
    
    ##
    # _dispatch
    #   Dispatch to a level 0 script command.
    #
    # @param optname - name of the option  holding the script.
    #
    method _dispatch optname {
        uplevel #0 $options($optname)
    }
}

##
# addEvbSource
#    Can be attached to the Add.. button of GetEvbSources above.
#    Accepts the data from the form, wraps it into a dictionary and
#    appends it to the -sources value of the widget it's passed.
#
# @param w -  A GetEvbSources (or derived) widget that's calling us.
#
proc addEvbSource w {
    # Marshall a dict of the option/vaules
    
    set newSource [dict create]
    foreach opt [list host ring description sids] {
        set value [$w cget -$opt]
        if {$value eq "" } {
            tk_messageBox -icon error -type ok -title {Missing opt} \
                -message "Missing value for $opt"
            return
        }
        dict set newSource $opt [$w cget -$opt]
    }
    set sources [$w cget -sources]
    lappend sources $newSource
    $w configure -sources $sources
    $w clear
}
