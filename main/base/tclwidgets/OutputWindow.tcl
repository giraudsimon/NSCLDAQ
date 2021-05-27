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
# @file   OutputWindow.tcl
# @brief  Provide output window and tabbed output window.
# @author Ron Fox <fox@nscl.msu.edu>
#

## Note this stuff has been extracted/factored from the ui.tcl  of the
#  old ReadoutGUi.tcl
#

package provide OutputWindow 1.0
package require Tk
package require snit

snit::macro getDefaultLogging {} {
    return  [dict create \
        output [list]           \
        log [list]              \
        error [list -background white -foreground red] \
        warning [list -foreground magenta]              \
    ]
}


##
# @class OutputWindow
#   Provides a widget that can be used to output lines of text. Two interesting
#   features of the output window are the history limit and logging classes.
#   The -history option allows the client to configure the number of lines of
#   text that will be retained on the widget.  This prevents the memory storage
#   of the widget from growing without bounds.
#
#  Logging classes are used with the log method.   The -logclasses option
#  defines a list of log item types that the log method will accept (an
#  error is thrown if log is called with a log type not in that list).
#  -showlog defines a list of log item types the log method will actually
#  display on the screen.  The initial set of log classes are:
#  *   output - intended to display output from some source (e.g. a data source program).
#  *   log    - intended to log some interesting event (e.g. the run started).
#  *   error  - intended to log some error condition.
#  *   warning- intended to log some condition that could be a problem.
#  *   debug  - intended for debugging output.
#
#  -showlog defines how or if items will be displayed.  It consists of a
#      list in the form accepted by array set.  The keys to the array are
#      log classes.  If a log class is not in the list it will not be displayed.
#      the elements of the array are lists of option/value pairs where each
#      option is an option accepted by the text widget tag configure operation.
#      As you might guess, log entries are given a tag that matches their logclass
#      Therefore changes to the -showlog are dynamic with the exception of the
#      addition/removal of classes from the list which only affect future log
#      operations. By default these are (note debug is not displayed):
#      *  output [list]
#      *  log    [list]
#      *  error  -foreground red -background white
#      *  warning -foreground yellow
#     @note a dict could be just as easily used as a list with keys the option names
#           and values the option values.
#
# LAYOUT:
#   +---------------------------------------------------------------+
#   | +---------------------------------------------+--+            |
#   | |  text widget                                |sb|            |
#   | +---------------------------------------------+--+            |
#   | |    scrollbar                                |               |
#   | +---------------------------------------------+
#   +---------------------------------------------------------------+
#
# OPTIONS:
#    -foreground - Widget foreground color
#    -background - Widget background color
#    -width      - Width of widget in characters.
#    -height     - Height of widget in characters.
#    -history    - Number of lines of historical text that are retained.
#    -logclasses - Defines the set of log classes accepted by the log method.
#    -showlog    - Defines which of the log classes will actually be displayed.
#    -monitorcmd - Defines a command to call when stuff is output to the window.
#                  the text of the output is appended to the command.
# METHODS:
#   puts         - puts data to the widget
#   clear        - clear the entire text and history.
#   log          - Make a timestamped log entry.
#   get          - Returns all of the characters in windowm, visible and historic
#   open         - Opens a log file for the widget.  The file is opened for
#                  append access. From this point on all data output will be logged
#                  to this file.
#   close        - closes the log file.
#
snit::widgetadaptor OutputWindow {
    component text
    
    delegate option -foreground to text
    delegate option -background to text
    delegate option -width      to text
    delegate option -height     to text

    option -history    -default 1000
    option -logclasses -default [list output log error warning debug]
    option -showlog    -default [getDefaultLogging] \
        -configuremethod _updateTagOptions
    option -monitorcmd [list]
    
    # If non empty, this is the log file fd.
    
    variable logfileFd  ""
    
    ##
    # constructor
    #   Builds the text widget along with horizontal and vertical scrollbars.
    #   the widget is set to -state disabled so that users can't type at it.
    #
    # @param args - The configuration options for the text widget.
    #
    constructor args {
        installhull using ttk::frame
        
        # Widget creation
        
        install text using text $win.text -xscrollcommand [list $win.xsb set] \
            -yscrollcommand [list $win.ysb set] -wrap word -background grey -state disabled
        ttk::scrollbar $win.ysb -orient vertical   -command [list $text yview]
        ttk::scrollbar $win.xsb -orient horizontal -command [list $text xview]
        
        # widget layout:
        
        grid $text $win.ysb -sticky nsew
        grid $win.xsb       -sticky new
        grid rowconfigure $win 0 -weight 1
        grid columnconfigure $win 0 -weight 1
        
        
        $self configurelist $args
        $self _updateTagOptions -showlog $options(-showlog)
    }
    ##
    # destructor
    #   Ensure we don't leak file descriptors:
    #
    destructor {
        if {$logfileFd ne ""} {
            close $logfileFd
        }
    }
    
    #---------------------------------------------------------------------------
    # Public methods:
    #
    
    ##
    # puts
    #   Puts simple text to the output window.
    #
    # @param args - This can be one or two parameters:
    #               *  text       - the text is output followed by a newline.
    #               * -nonewline  - The text is output with no trailing newline.
    #
    method puts  {args}       {

        if {[llength $args] == 1} {
            set line "[lindex $args 0]\n"
        } elseif {([llength $args] == 2) && ([lindex $args 0] eq "-nonewline")} {
            set line [lindex $args 1]
        } else {
            error "use puts ?-nonewline Text"
        }
        $self _output "__notag___" $line
    }
    ##
    # clear
    #    Clears the entire widget.
    #
    method clear {}  {
        $text config -state normal
        $text delete 0.0 end
        $text config -state disabled
    }
    ##
    # log
    #    Make a log like message.  A log message gets a timestamp and is
    #   associated with log class (-logclasses).  Further more, -showlog can
    #   associate rendering options with these messages.
    #
    # @param class - The log class associated with the message (must be in
    #                -logclasses)
    # @param msg   - The messagde to log.
    method log   {class msg} {
        set timestamp [clock format [clock seconds] -format {%D %T} ]
        if {$class ni $options(-logclasses)} {
            error "'$class' is not one of the known classes: {$options(-logclasses)}"
        }
        array set logRenditions $options(-showlog)
        if {$class in [array names logRenditions]} {
            $self _output $class "$timestamp : $class : $msg\n"
        }
    }
    ##
    # get
    #   Return the text in the widget.  No effor is made to return information
    #   about the rendering of the text... only the text itself is returned.
    #
    # @return - string containing all of the text in the output window.
    #
    method get   {}           {
        return [$text get 0.0 end]
    }
    ##
    # open
    #   Open a file and log subsequent output to it as well as to the window.
    #   *   If a file is already open it is closed.
    #   *   the file is opened for append access (a+).
    #   *   Output will be logged to the file until close is called or until
    #       a different file is opened.
    #
    # @param filename  - Name of the file to open.
    #
    # @throw if the file is not writable.
    #
    method open  {filename}   {
    
        #  Close any open file:
        
        if {$logfileFd ne ""} {
            $self close
        }
        #  Open the new file
        
        set logfileFd [open $filename "a+"]
    }
    ##
    # Close the log file.
    # @throw it is an error to close when no log file is active.
    #
    method close {}           {
        if {$logfileFd eq ""} {
            error "No logfile is open"
        } else {
            close $logfileFd
            set logfileFd ""
        }
    }
    
    #--------------------------------------------------------------------------
    #  Configuration processors.
    
    
    ##
    # _updateTagOptions
    #
    #  Processes a new set of -showlog values.  These get turned into tag
    #  configuration.
    #
    # @param optname  - The option being configured
    # @param value    - The new value for this option.
    #
    # @note see the class comments for more informationabout the value.
    #
    method _updateTagOptions {optname value} {
        #
        #  By putting the values into an array whose values are treated as dicts
        #  locally we
        #  - make it easy to iterate over the tags.
        #  - Syntax check the dictionary definition lists.
        
        array set tagInfo $value
        
        foreach tag [array names tagInfo] {
            set tagConfig $tagInfo($tag)
            if {([llength $tagConfig] % 2) == 1} {
                error "log option lists must be even but the one for '$tag' is not: {$tagConfig}"
            }
            dict for {tagopt tagoptValue} $tagConfig {
                $text tag configure $tag $tagopt $tagoptValue
            }
        }
        
        #
        # If we got here, everything is legal:
        #
        set options($optname) $value
    }
    #--------------------------------------------------------------------------
    # Private methods
    
    ##
    # _output
    #    Centralized output/file-logging/history-trimming output method:
    #
    # @param tag  - Tag to associate with the output.
    # @param text - Data to output.
    #
    method _output {tag data} {
        
        # Output the data to the widget:
        
	$text configure -state normal
        $text insert end $data  $tag
        $text yview -pickplace end
	$text configure -state disabled
        
        # Limit the number of lines that can appear.
        
        set lines [$text count -lines 0.0 end]
        if {$lines > $options(-history)} {
            $text delete 0.0 "end - $options(-history) lines"
        }
        # Log to file if we must:
        
        if {$logfileFd != ""} {
            puts -nonewline $logfileFd $data
            flush $logfileFd
        }
        # If there is a non null -monitorcmd hand it the text:
        
        set cmd $options(-monitorcmd)
        if {$cmd ne ""} {
            uplevel #0 $cmd "{$data}"
        }
    }
}
##
# @class OuputWindowSettings
#
#   Prompt the output window settings. This is normally wrapped
#   in a dialog.
#
# LAYOUT:
#   +----------------------------------------------------------------+
#   | Rows: [^V]   Columns [^V] History [^V] [ ] show debug messages |
#   +----------------------------------------------------------------+
#
#   * Plain text are labels.
#   * [^V] are spinboxes
#   * [ ] is a checkbutton.
#
# OPTIONS:
#   *  -rows    - Number of displayed rows of text.
#   *  -columns - Number of displayed text columns
#   *  -history - Number of lines of history text displayed.
#   *  -debug   - boolean on to display debug log messages.
#
#
snit::widgetadaptor OutputWindowSettings {
    option -rows    -configuremethod  _setSpinbox -cgetmethod _getSpinbox
    option -columns -configuremethod  _setSpinbox -cgetmethod _getSpinbox
    option -history -configuremethod _setSpinbox -cgetmethod _getSpinbox
    option -debug
    
    ##
    # constructor
    #    Create the widgets, bind them to the options and lay them out
    #    As shown in the LAYOUT comments section
    #
    # @args configuration options.
    #
    constructor args {
        installhull using ttk::frame
        
        # Widget creation:
        
        ttk::label $win.rowlabel -text {Rows: }
        ttk::spinbox $win.rows -from 10 -to 40 -increment 1 -width 3
        
        ttk::label $win.collabel -text {Columns: }
        ttk::spinbox $win.columns -from 40 -to 132 -increment 1 -width 4
        
        ttk::label $win.historylabel -text {History lines: }
        ttk::spinbox $win.history -from 500 -to 10000 -increment 100 -width 6
        
        ttk::checkbutton $win.debug -text {Show debugging output} \
            -variable [myvar options(-debug)] -onvalue 1 -offvalue 0
        
        # widget layout:
        
      #  grid $win.rowlabel $win.rows $win.collabel $win.columns \
      #      $win.historylabel $win.history $win.debug
	grid $win.historylabel $win.history $win.debug; # Lazy way to remove row/col.
 
        $self configurelist $args
        
    }
    #--------------------------------------------------------------------------
    #
    #  Configuration get/set operations.
    
    ##
    # _setSpinBox
    #
    #  Sets the spinbox associated with an option.  The spinbox must be
    #  named win.optionname where optionname is the option name with the -
    #  stripped.  e.g. for the -rows option the spinbox is named
    #  $win.rows
    #
    # @param optname - Name of the option.
    # @param value   - New value for the option.
    #
    method _setSpinbox {optname value} {
        set name [string range $optname 1 end];    # Strip off the leading$
        $win.$name set $value
    }
    ##
    # _getSpinbox
    #
    #   Gets the value of an option associated with a spinbox.  See
    #   _setSpinbox above for naming requirements
    #
    # @param optname - name of the option being queried.
    #
    method _getSpinbox optname {
        set name [string range $optname 1 end]
        return [$win.$name get]
    }
}
##
# @class TabbedOutput
#
#   This widget is a tabbed notebook containing output windows.  The idea is that
#   there is a main window which shows all puts and Logs that don't have a data  source.
#   As Logs are done with a data source, new tabs are created and the output is then segregated
#   into those tabs.  Tab names are:
#   Main - The window with non-sourced data.
#   Srcname - The tab for the data source Srcname
#  
#  Furthermore if output is directed to a windows that is not visible the number of outputs
#  is shown as a (n) after the tab name.  Displaying that tab eliminates the (n).
#
#
# OPTIONS
#    -foreground - The foreground color for all of the widgets.
#    -background - The background color for all of the widgets.
#    -width      - Width in characters of all widgets.
#    -height     - height of all widgets in lines of text.
#    -history    - number of lines of historical data each widget has.
#    -logclasses - Define the set of log classes each widget accepts.
#    -showlog    - Defines which log classes are displayed and how.
#    -monitorcmd - Defines the monitor command for all windows.
# METHODS
#  puts  - Outputs something to the main window.
#  clear - Clears display and history of all windows.
#  log   - Logs to the main window.
#  logFrom - logs from a specific data source.
#  open    - Turns on logging for all windows.
#  close   - Closes logging for all windows.
#  get     - returns a list of pairs.  The first element of each pair is the name of a source
#            ('main' or a data source name),  The second element of each pair is the contents of that window.
#
snit::widgetadaptor TabbedOutput {

    # wish we could formally delegate but we are a fanout.

    option -foreground  -configuremethod _RelayOption
    option -background  -configuremethod _RelayOption
    option -width       -configuremethod _RelayOption
    option -height      -configuremethod _RelayOption
    option -history     -configuremethod _RelayOption -default 1000
    option -logclasses  -configuremethod _RelayOption -default [list output log error warning debug]
    option -showlog     -configuremethod _RelayOption \
        -default [getDefaultLogging]
    option -monitorcmd  -configuremethod _RelayOption -default [list]
    option -errorclasses [list error warning]

    
    
    delegate option * to hull
    delegate method * to hull

    # Options that don't get fanned out:
    
    variable localOptions [list -errorclasses]

    #
    # List of Output widgets managed by us...this is in index order.
    # Index 0 is the Main widget.

    variable outputWindows [list]

    # Array of dicts indexed by widget namee each Dict containing:
    #      name - source name.
    #      lines- Unseen puts of text.
    #

    variable tabInfo       -array [list]
    
    # Used to uniquify output windows:
    
    variable outIndex     0
    
    # log file if open [list] if not...used to bring new windows into the log:
    
    variable logFile [list]

    ##
    #  constructor
    #
    #  *   Install a ttk::notebook as the hull.
    #  *   Create an output window $win.main as the main widget... in the notebook.
    #  *   Add the main output window to the outputWindows list.
    #  *   Populate -foreground, -background, -height, -width from the main window.
    #  *   Process the configuration options.
    #

    constructor args {
      installhull using ttk::notebook


        lappend outputWindows [OutputWindow $win.main]
        set tabInfo($win.main) [dict create name main lines 0]
        $hull add $win.main -text main
        $hull tab $win.main -sticky nsew
        set options(-foreground) [$win.main cget -foreground]
        set options(-background) [$win.main cget -background]
        set options(-width)      [$win.main cget -width]
        set options(-height)     [$win.main cget -height]

        $self configurelist $args

        grid rowconfigure $win 0 -weight 1
        grid columnconfigure $win 0 -weight 1

# When the selected tab has changed we need to update its tab to indicate
# It's lines have been read.

        bind $win <<NotebookTabChanged>> [mymethod _TabChanged]
    }

    #---------------------------------------------------------------------------------
    #
    #  Configuration methods


    ##
    # _RelayOption
    #     Called when a delegated option is modified.
    # @param opt - Name of the option.
    # @param val - New value.
    #
    method _RelayOption {opt value} {
	set options($opt) $value
	$self _DoAll [list configure $opt $value]
    }

    #---------------------------------------------------------------------------------
    # Public methods.
    #

    ##
    # puts
    #    Output a string to the main window.  If the main window is not current it gets a (n) with the number
    #    of outputs done since the last time it was current.
    #
    # @param args  -  Args see OutpuWindow puts.
    #
    method puts args {
	set widget [lindex $outputWindows 0];                   # Always main.
	$widget puts {*}$args

	$self _UpdateTabText $widget
    }
    ##
    # clear
    #    Clears the text/history from all windows.
    #    The tabs are set back to just the source names.
    #
    method clear {} {
        $self _DoAll clear
        foreach win $outputWindows {
            $hull tab $win -text [dict get $tabInfo($win) name]
        }
    }
    ##
    # log
    #    Logs to the main window.
    #    This basically delegates to the main windows log method.
    #
    # @param args - the parameters that would be passed to the log method normally.
    #
    # @note - The _UpdateTabText is called to ensure the tab title is updated if
    #         the window is not displated.
    #
    method log args {
        set widget [lindex $outputWindows 0]
        $self _LogToWidget $widget {*}$args
    }
    ##
    # logFrom
    #   Logs to a specific source window (creating it as needed).
    #
    # @param source - source of the log message.  If there is no output window
    #                 for this source, one is created.
    # @param args   - The stuff that goes into the source's window log method.
    #
    # @note - _UpdateTabText is invoked to ensure the tab title is updated if needed.
    #
    method logFrom {source args} {
        set widget [$self _GetSourceWindow $source]
        $self _LogToWidget $widget {*}$args
    }
    ##
    # open
    #   Open logging in all windows.  These go to a common log file.
    #
    # @param filename - Path of file to log to.
    #
    method open filename {
        $self _DoAll [list open $filename]
        set logFile $filename
    }
    ##
    # close
    #   Close the log file in all windows.
    #
    method close {} {
        $self _DoAll close
        set logFile [list]
    }
    ##
    # get
    #   Return the contents of all windows.
    #
    # @return list of pairs - each pair consists of a source name and the
    #                         data from the window associated with that source.
    #
    
    method get {} {
        set result [list]
        foreach widget $outputWindows {
            set name [dict get $tabInfo($widget) name]
            set contents [$widget get]
            lappend result [list $name $contents]
        }
        return $result
    }
    #--------------------------------------------------------------------------------
    #
    # Private utility methods.
    #

    ##
    #  _DoAll
    #    Do the same command in all windows.
    # cmd - List that is the command to perform.
    #
    method _DoAll cmd {
	foreach win $outputWindows {
	    $win {*}$cmd
	}
    }
    ##
    # _UpdateTabText
    #   If the widget is not the currently displayed one, increment its number of unseen
    #   lines and modify the tab text.
    #
    # @param widget - The widget to modify.
    # @param class  - The log class used
    #
    method _UpdateTabText {widget {class output}} {
	set windex [$hull index $widget]
	set cindex [$hull index current]

	if {$windex != $cindex} {
	    dict incr tabInfo($widget) lines
	    set name  [dict get $tabInfo($widget) name]
	    set lines [dict get $tabInfo($widget) lines]
	    set tabText [format "%s (%d)" $name $lines]
	    $hull tab $windex -text $tabText
            if {$class in $options(-errorclasses)} {
                $hull tab $windex -image output_error -compound left
            }

		      
	}
    }
    ##
    # _GetSourceWindow
    #    Returns the widget associated with a data source output window.
    #    If the data source does not have an output window, one is created
    #    and that widget is returned.
    #
    # @param source - Name of the source who's window we want
    #
    # @return window - Path to the wndow for that source.
    #
    method _GetSourceWindow {source} {
        foreach widget $outputWindows {
            if {[dict get $tabInfo($widget) name] eq $source} {
                return $widget
            }
        }
        #  Need to create a new one:
        
        set widget $win.source[incr outIndex]
        lappend outputWindows $widget
        set tabInfo($widget) [dict create name $source lines 0]
        $hull add [OutputWindow $widget] -text $source
        
        # Propagate the settings to the new window.
        
        foreach option [array names options] {
            if {$option ni $localOptions} {
                $widget configure $option $options($option)
            }
        }
        if {$logFile ne ""} {
            $widget open $logFile
        }
        
        # Return it as the output window:
        
        return $widget
    }
    
    ##
    # _LogToWidget
    #   Given an output widget log to it.
    #
    # @param widget - The widget to log to.
    # @param args   - log parameters
    #
    method _LogToWidget {widget args} {
        $widget log {*}$args
        set class [lindex $args 0];             #log class.
        $self _UpdateTabText $widget $class
    }
    ##
    # _TabChanged
    #   Called when the tab changes.  We must figure out the current widget
    #   and reset its tab name to just the source name (removing any
    #   unseen entry count.
    #
    #
    method _TabChanged {} {
        set idx [$hull index current]
        set widget [lindex $outputWindows $idx]
        set source [dict get $tabInfo($widget) name]
        $hull tab $idx -text $source -image [list]
        dict set tabInfo($widget) lines 0;    # No output is unseen.
    }
    
}
