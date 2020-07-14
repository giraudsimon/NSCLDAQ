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
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file utility.tcl
# @brief misc. EVB utility methods/snidgets etc.
#


#-----------------------------------------------------------------------------
# Package stuff
#

package provide EVBUtilities 1.0

package require Tk
package require snit
package require Iwidgets

#------------------------------------------------------------------------------
#
#  Establishing namespaces
#
#
namespace eval EVB {
    namespace eval utility {}
    namespace eval test    {}
}


#------------------------------------------------------------------------------
# Useful snidgets


##
# @class EVB::utility::sortedWidget
#
#  Incomplete class that allows you to produce a sorted list of widgets.
#  widgets are displayed next to an identifying value (integer), and the
#  widgets remain sorted by id.
#
#  This class is incomplete in that you must supply a pair of callback
#  scripts.
#
# OPTIONS
#   -title       - Title of the frame in which the list is built
#                  (ttk::labelframe)
#   -lefttitle   - Title of the left column of the widget.
#   -rightttitle - Title of the widget column.
#   -create      - Mandatory Script called to create a new widget.  The
#                  requested widget id is passed in.
#   -update      - Mandatory Script called to update an existing widget.
#                  this is passed the widget id and the dict passed to
#                  the setValue method.
#   -destroy     - Optinonal Script called when a widget is being destroyed.
#                  the script is passed the widget name and the id
#                  the widget has not yet been destroyed at this time but
#                  will be when the script returns.
#
# METHODS
#   listids       - Returns the list of known ids.
#   getWidget id  - Get the widget corresponding to the specified id.
#   update id valuelist - Requests an update. valuelist ia passed
#                         without interpretation to the -update callback
#   reset         - Destroy all ids/widgets.
#
# NOTE:
#   The -create and -update callbacks are required.  If they would be called but
#   none have been set, and error is thrown.
#
snit::widgetadaptor EVB::utility::sortedWidget {
    component lefttitle
    component righttitle
    component container
    
    delegate option -title to  hull as      -text
    delegate option -lefttitle to lefttitle as   -text
    delegate option -righttitle to righttitle as -text

    
    option -create  [list]
    option -update  [list]
    option -destroy [list]
    
    delegate option * to container
    
    
    variable idlist [list];     # Sorted list of ids.
    
    ##
    # constructor
    #
    #  - Install the hull as a ttk frame.
    #  - Build the top-level widgets.
    #  - Lay them out in the hull.
    #
    # @param args - The option/value pairs that are used for the initial
    #               configuration.
    #
    constructor args {
        installhull using ttk::labelframe
        
        install lefttitle  using ttk::label $win.lefttitle -width 10 
        install righttitle using ttk::label $win.righttitle -width 10
        install container  using ::iwidgets::scrolledframe $win.container \
            -hscrollmode none -vscrollmode dynamic
        
        # Layout the widgets inside the hull frame.
        
        
        grid $win.lefttitle $win.righttitle -sticky ew -padx 5 -pady 5
        grid $win.container -columnspan 2 -sticky nsew -padx 5 -pady 5

        grid columnconfigure $win {0 1} -weight 1
        grid rowconfigure    $win 1     -weight 1

        $self configurelist $args
    }

    ##
    # listids
    #   List the known ids for the widget sets.
    #
    # @return list - list of ids that are known to the widget
    #
    method listids {} {
        return $idlist
    }
    ##
    # getWidget id
    #
    #   Return the widget associated with the specified id.  It is an error
    #   to request the widget of an id that is not in the idlist
    #
    # @param id  - id to get the widget name for.
    # @return string - widget name.
    #
    method getWidget id {
        if {$id in $idlist} {
            return [$self _WidgetId $id]
        } else {
            error "$id does not have a corresponding widget"
        }
    }
    ##
    # update id client-data
    #
    #  Requests an update of the widget corresponding to $id.
    #  - If the widget does not yet exist, the -create callback is called
    #    and with the name of the desired widget, and the list both
    #    internal and display are adjusted appropriately.
    #  - The -update callback is given both the widget name and the client-data
    #    and is expected to make appropriate alterations to the widget appearance.
    #
    # @param id          - The id of the widget to update.
    # @param client-data - data passwd without interpretation to the update
    #                      script.
    #
    method update {id clientData} {
        
        # If needed create the new widget and modify the layout:
        
        if {$id ni $idlist} {
            $self _CreateNewWidget $id
            $self _LayoutWidgets
        }
        set widgetName [$self _WidgetId $id]
        $self _UpdateWidget $widgetName $clientData
    }
    ##
    # reset
    #
    # Destroys all widgets and empties the idlist.
    # If a -destroy script is defined, it is called once for each id.
    #
    method reset {} {
        set destroyscript $options(-destroy)
        foreach id $idlist {
            set widget [$self _WidgetId $id]
            
            # If there's a destroy script invoke it for this id.
            
            if {$destroyscript ne ""} {
                uplevel #0 $destroyscript $widget $id
            }
            
            # Destroy the wiget pair.
            
            destroy [$container childsite].id$id
            destroy [$self _WidgetId $id]
        }
        set idlist [list]
    }
    
    #---------------------------------------------------------------------------
    #
    # Private methods
    
    ##
    # _WidgetId id
    #
    #  Produces the name of a widget given the id that 'controls it.
    #
    # @param id - the id that is the key for the widget.
    method _WidgetId id  {
        return [$container childsite].widget$id
    }
    ##
    # _CreateNewWidget id
    #
    #  Creates a widget that corresonds to the id passed in.  To do this
    #  the -create script is called.  The id is then put into the idlist
    #  in the correct/sorted position.
    #
    # @param id - The id that 'selects' the widget.
    #
    # @throw - It is an error not to have a -create script.
    #
    method _CreateNewWidget id {
        
        # Error out if there's no script to create widgets.
        
        if {$options(-create) eq ""} {
            error "A -create script must be supplied by the time ids are added"
        }
        # Create the id widget and the user's widget:
        
        label [$container childsite].id$id -text $id 
        set script $options(-create)
        uplevel #0 $script [$self _WidgetId $id]
        
        # Insert the id in the correct stop of the id list...for now
        # just lappend and re-sort the list.  The assumptions are that:
        # - There will be a relayout
        # - Relayout does  a layout of all widgets in the list.
        #
        
        lappend idlist $id
        set idlist [lsort -integer $idlist]
    }
    ##
    # _LayoutWidgets
    #
    #  Unmanages all widgets in the container then re-manages them in
    #  the appropriate order.
    #
    method _LayoutWidgets {} {
        
        # Unmanage the current set of slaves
        
        set currentSlaves [grid slaves [$container childsite]]
        if {[llength $currentSlaves]} {
            grid forget {*}$currentSlaves
        }
        
        # Manage the widgets in the correct order:
        
        set row 0
        foreach id $idlist {
            set idWidget [$container childsite].id$id
            set userWidget [$self _WidgetId $id]
            grid $idWidget   -row $row -sticky nsw -column 0
            grid $userWidget -row $row -sticky nse -column 1
            incr row
        }
    }
    ##
    # _UpdateWidget name data
    #
    #  Updates the specified user widget.  This is done by invoking the
    #  -update script.  It is an error for this method to be called prior to the
    #  establishment of an update script to handle it.
    #
    # @param name - Name of the widget to update.
    # @param data - Application specific data to be used by -update to determine
    #               what changes need to be made to the widget.
    #
    #  @throw - if the -update script is empty.
    #
    method  _UpdateWidget {name data} {
        
        # Toss an error if the -update script is empty:
        
        if {$options(-update) eq ""} {
            error "An -update script must have been supplyed to change the widgets"
        }
        
        #  Get the script and execute it at the global level:
        
        set script $options(-update)
        uplevel #0 $script $name [list $data]
        
    }
}



##
# @class EVB::utility::sortedPair
#
#  This class maintains  a sorted pair of numbers. The left column is sorted
#  ascending while the right column is assumed to be some associated data
#  typical use is as a set of counters with some identifier (e.g. source id or
#  fragment type) as the sort key.
#
# OPTIONS
# - -title     -   Overall Title.
# - -lefttitle -  Title of the left column
# - -righttitle - Title of the right column
#
# METHODS
#   - setItem key value sets a key/value pair.  If necessary the key's widgets are
#     created.
#   - clear value=0 All of the values are set to the new value.
#   - reinit All key/value pairs are deleted.
#   - idlist Return all the ids.
#
# LAYOUT
#
#  +---------------------------------------+
#  |             top title                 |
#  |   Left title        Right Title       |
#  |         id                value      ^|
#  |      ...                   ...       V|
#  +---------------------------------------+
#
snit::widgetadaptor EVB::utility::sortedPair {
    delegate option -title to hull
    delegate option -lefttitle to hull
    delegate option -righttitle to hull

    delegate option * to hull
    
    delegate method reinit to hull as reset
    delegate method idlist to hull as listids
    delegate method setItem to hull as update
    


    ##
    # constructor
    #
    #  This is where the main work is done.  The hull is installed as a
    #  EVB::utility::sortedWidget and the -create -update scripts installed
    #  to meet our needs.
    #
    # @param args - option/value pairs.
    #
    constructor args {
        installhull using EVB::utility::sortedWidget \
            -create [mymethod _CreateLabel] -update [mymethod _UpdateLabel]
        
        $self configurelist $args
    }

    ##
    # clear ?value = 0?
    #
    #  Clears all widgets.  An optional value (defaulting to 0) may be specified.
    #  This iterates over all ids and calls the hull update method.
    #
    # @param value - (optional default 0) new value for all widgets.
    #
    method clear {{value 0}} {
        set ids [$hull listids]
        
        foreach id $ids {
            $hull update $id $value
        }
    }
    #---------------------------------------------------------------------------
    #
    # Private methods.
    #
    
    ##
    # _CreateLabel widget
    #
    #   Create a label named 'widget'.
    #
    # @param widget - full path of widget to create.
    #
    method _CreateLabel widget {
        label $widget -width 12
    }
    ##
    # _UpdateLabel
    #
    #  Set the text in a label as directed.
    #
    #  @param widget - Widget to modify.
    #  @param text   - Text to set.
    #
    method _UpdateLabel {widget text} {
        $widget configure -text $text
    }
    
}
##
# EVB::utility _formatUnsigned
#
#   Factored common code from various _unsignedOption methods
proc EVB::utility::_formatUnsigned {value} {
    if {[string is integer -strict $value]} {
            return  [format %u $value]
        } else {
            return  $value
        }
}

##
# EVB::utility::_dispatch
#  Dispatches a command at level 0:
#
# @param cmd   - The base command which may have substitution strings.
# @param map   - substitution map.  This is a list of the form
#                expected by [string map] it defines substitution strings
#                and their substitution values.
# @return - value of the command
# @note - there's no need to ensure that cmd is nonempty if cmd is an empty string
#         this method is a no-op. In that case an empty string is returned.
#
proc EVB::utility::_dispatch {cmd map} {
    if {$cmd ne ""} {
        set cmd [string map $map $cmd]
        return [uplevel #0 $cmd]
    } else {
        return ""
    }
}
##
# EVB::utility::_setStateFromBool
#
#  Given a boolean value set the state of a widget to normal (true) or
#  disabled (false)
#
# @param w - the widget.
# @param b - the boolean
#
proc EVB::utility::_setStateFromBool {w b} {
    if {$b} {
        set state normal
    } else {
        set state disabled
    }
    $w configure -state $state

}
