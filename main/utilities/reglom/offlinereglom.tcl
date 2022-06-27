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
# @file offlinereglom.tcl
# @brief Tcl script to drive the offline reordering via unglom and reglom.
# @author Ron Fox <fox@nscl.msu.edu>
#

#  We assume that we're located in the same diretory as unglom and reglom.
 
set bindir [file dirname [info script]]


#  The tcllibs are  here:

set tcllibdir [file normalize [file join $bindir .. TclLibs]]
lappend auto_path $tcllibdir

##
#   This script is installed as a program and is used to fix event files that
#   come from the NSCL event builder but have out of order timestamps.
#
package require removetcllibpath
package require Tk
package require snit
package require DataSourceUI;          # for the DialogWrapper base class.
package require dialogwrapper

#-----------------------------------------------------------------------------
#  This is the setup UI.
#
#   It will be embedded into a dialog wrapper to prompt the user for the stuff
#   needed to run the unglom/reglom.
#
# Appearance:
#   +--------------------------------------------------------------------+
#   | Glom interval [  1 ]+/-    Output sourceid [  0 ] +/-              |
#   | Timestamp Policy <radiobutton set>                                 |
#   |                                                                    |
#   | [  Input run ] <not selected> choose one fie in a multisegment run |
#   | [ output file ] <not selected>                                     |
#   |                                                                    |
#   +--------------------------------------------------------------------+
#

snit::widgetadaptor ParameterSelection {
    option -dt         -default   1 -configuremethod _updateSpinbox -cgetmethod _getSpinbox
    option -sourceid   -default   0 -configuremethod _updateSpinbox -cgetmethod _getSpinbox
    option -policy     -default   earliest
    option -inputfiles -default   ""  -configuremethod _updateInputFiles
    option -outputfile -default   ""
    
    # Array of widget names index by option they control.
    # note all are relative to $win which is not known yet.
    
    variable spinboxes -array [list                        \
        -dt dt -sourceid srcid                             \
    ]
    
    #
    #  Constructor
    #    Use a ttk::frame for the hull create the widgets
    #    and lay them out.
    #    Widgets will, where possible get bound to specific options an actions.
    #
    constructor args {
        installhull using ttk::frame
        
        ttk::label   $win.dtlabel -text {Glom interval}
        ttk::spinbox $win.dt      -from 0 -to 100000 -increment 1 \
                -validatecommand [mymethod _isInteger %P ]  \
                -validate key -width 5
        ttk::label   $win.srclabel -text {Output Source ID}
        ttk::spinbox $win.srcid   -from 0 -to 65535 -increment 1  \
            -validatecommand [mymethod _isInteger %P ] \
            -validate key -width 5
        
        ttk::label   $win.policylbl -text {Timestamp Policy} 
        ttk::radiobutton $win.earliest -text earliest -value earliest -variable [myvar options(-policy)]
        ttk::radiobutton $win.latest   -text latest   -value latest   -variable [myvar options(-policy)]
        ttk::radiobutton $win.average  -text average  -value average  -variable [myvar options(-policy)]
        
        ttk::button $win.chooseinfiles -text {Input Run} -command [mymethod _chooseInputFiles]
        ttk::label  $win.infiles     -text {<notchosen>}
        ttk::label  $win.infilenote  -text {For a multi segment run choose any segment of the run}
        
        ttk::button $win.chooseoutfile -text {Output File} -command [mymethod _chooseOutputFile]
        ttk::label  $win.outfiles      -textvariable [myvar options(-outputfile)]
        ttk::label  $win.outfilenote   -text {The output file will not be segmented}
        
        $self configurelist $args
        
        grid $win.dtlabel $win.dt $win.srclabel $win.srcid  -sticky w
        grid $win.policylbl $win.earliest $win.latest $win.average -sticky w
        
        #  This is two lines where the second line spans all cells (4).
        
        grid $win.chooseinfiles $win.infiles - - -sticky w
        grid $win.infilenote - - - -sticky w
        
        # similarly for the output file:
        
        grid $win.chooseoutfile $win.outfiles - - -sticky w
        grid $win.outfilenote - - - -sticky w
        
        # set initial spinbox values:
        
        foreach option [array names spinboxes] {
            $win.$spinboxes($option) set $options($option)
        }
    }
    #--------------------------------------------------------------------------
    
    ##
    # configuration management
    #
    
    ##
    # _updateSpinbox
    #   Invoked when a configuration option that controls a spinbox is
    #   updated.
    #    - look up the spinbox widget associated with the option.
    #    - update what that spinbox displays.
    #    - update the options array value.
    #
    # @param optname - option name (e.g. -dt).
    # @param optval  - new proposed option value.
    #
    #  @note - no validation is done.
    #
    method _updateSpinbox {optname optval} {
        set widget $spinboxes($optname)
        set widget $win.$widget
        
        scan $optval %i value
        $widget set $value
        
        set options($optname) $value
    }
    ##
    # _getSpinbox
    #    Gets the value of the specified spinbox.
    #
    # @param optname - name of the option
    #
    method _getSpinbox optname {
        set widget $spinboxes($optname)
        set widget $win.$widget
        
        return [$widget get]
    }
    ##
    # _updateInputFiles
    #    Called when -inputfiles is reconfigured.  In this case we must not only
    #    store the (potentially list) of input files but create and configure
    #    a suitable string to display in $win.infiles.
    #
    # @param optname - name of the configuration option (-inputfiles)
    # @param optval  - New value for that configuration option.
    #
    method _updateInputFiles {optname optval} {
        set options($optname) $optval
        
        # If there's only one file just show it:
        
        set text [file tail [lindex $optval 0]]
        
        # IF there's more then show first ... last:
        
        set nFiles [llength $optval]
        if {$nFiles > 1} {
            append text " ... " [file tail [lindex $optval end]]
        }
        $win.infiles configure -text $text
    }
    #-------------------------------------------------------------------------
    
    #  UI Event processing.  These methods are associated with events in the
    #  GUI:
    
    ##
    # _isInteger
    #   Invoked to validate a spinbox.  All spinboxes must have integers
    #   that are >= 0.
    #
    # @param newval - new proposed value.
    # @return bool - true if the value validates, false otherwise.
    #
    method _isInteger newval {
        
        if {![string is integer -strict $newval]} {
            return 0
        }
        
        if {$newval < 0} {
            return 0
        }
        return 1
    }
    ##
    # _chooseInputFiles
    #    Puts up a dialog that allows users to select an event file.
    #    The event file chosen  might be part of a multi segment run or not.
    #    If the event file is 'singular'  The file chosen is configured to be
    #    the -inputfiles (this triggers an update of the text shown).
    #
    #    If the file chosen is part of a segmented set, all segments are added
    #    to the list -inputfiles.
    #
    method _chooseInputFiles {} {
        set file [tk_getOpenFile -defaultextension .evt -parent $win    \
            -title {Choose run}  -filetypes [list                       \
            [list {Event files} {.evt}     ]                            \
            [list {All Files}    *         ]                            \
        ]]
        
        # Cancel sets file to an empty string:
        
        if {$file eq ""} return
        
        #  The prompter ensures the file exists.  If it fits the
        #  template of run-%dddd-%dd.whatever we'll see if there are
        #  any other segments...
        
        set fname [file tail $file]
        set dir   [file dirname $file]
        set nConverted [scan $fname "run-%04d-%02d.evt" run seg]
        
        # If we can't extract a run number and a segment from this template
        # We can't fine segments.k
        
        if {$nConverted != 2} {
            $self configure -inputfiles $file
            return
        }
        #  Construct a glob string to list the run segments.
        
        set elements [split $fname -.]
        if {[llength $elements] != 4} {
            error "Can't decode this '$file' to the form xxxx-rrrr-ss.yyyy"
        }
        set digits2 {[0-9][0-9]}
        set globstring [lindex $elements 0]-[lindex $elements 1]-$digits2.[lindex $elements 3]
        set globstring [file join $dir $globstring]
        set files [glob $globstring]
        
        $self configure -inputfiles $files
            
    }
    ##
    # _chooseOutputFile
    #    Selects the output file.
    #    A tk_getSaveFile dialog is used in this case.  Overwriting an existing
    #    file prompts for an are you sure.  On accepting the output file,
    #    the -outputfile is configured to update the selection.
    #
    method _chooseOutputFile {} {
        set file [tk_getSaveFile -defaultextension .evt -parent $win    \
            -title {Choose run}  -filetypes [list                       \
            [list {Event files} {.evt}     ]                            \
            [list {All Files}    *         ]                            \
        ]]
        if {$file ne ""} {
            $self configure -outputfile $file
        }
    }
}

##
# UnglomProgress
#    This megawidget shows the progress of unglom.   It consists of
#    two listboxes.  The first listbox shows the files created by unglom
#    and their sizes.   The second listbox is a list of the messages
#    received from the unglom process.
#
#
snit::widgetadaptor UnglomProgress {
    
    
    ##
    # constructor
    #    Create the listboxes, their scroll bars and lay everything out.
    # @note there are no configuration parameters so we never configurelist.
    #
    constructor args {
        installhull using ttk::frame
        
        listbox $win.files -yscrollcommand [list $win.filescroll set]
        ttk::scrollbar $win.filescroll -orient vertical -command [list $win.files yview]
        
        listbox $win.messages -yscrollcommand [list $win.msgscroll set] -width 72
        ttk::scrollbar $win.msgscroll -orient vertical -command [list $win.messages yview]
        
        grid $win.files $win.filescroll -sticky nsew
        grid $win.messages $win.msgscroll -sticky nsew
        
    }
    ##
    # addMessage
    #    Adds  a message to the message scrolling listbox.
    # @param msg - the message to add.
    #
    method addMessage msg {
        $win.messages insert end $msg
    }
    ##
    # fileProgress
    #    Provides the current file progress.
    #
    # @param info - a list of two item lists.  The first item in each list is
    #               the name of a file.  The second item is the number of bytes
    #               in the file.
    #
    method fileProgress info {
        $win.files delete 0 end
        foreach file $info {
            set name [lindex $file 0]
            set bytes [lindex $file 1]
            set line [format "%-32s %d" $name $bytes]
            
            $win.files insert end $line
        }
    }
}

##
# writeConfigFile
#   Writes the default configuration for dt, sourceid, and policy
#
# @param dt - -dt default value.
# @param sid - -sourceid default value.
# @param pol - -policy default value.
#
proc writeConfigFile {dt sid pol} {
    set fd [open "~/.reglom" w]
    puts $fd "set dt $dt"
    puts $fd "set sourceid $sid"
    puts $fd "set policy $pol"
    close $fd
}

#------------------------------------------------------------------------------

#   Procs involved in unglomming:

##
# updateUnglomFiles
#    Periodically update the progress building output files for unglom.
#
# @param wid - widget that is assumed to be an UnglomProgress widget.
# @param ms  - milliseconds between updates
#
proc updateUnglomFiles {wid ms} {
    # This if makes us run down naturally when unglom is done:
    
    if {[winfo exists $wid]} {
        after $ms [list updateUnglomFiles $wid $ms];  # reschedule
        
        set filelist [glob -nocomplain sid-*]
        set fileinfo [list]
        foreach file $filelist {
            lappend fileinfo [list $file [file size $file]]
        }
        $wid fileProgress $fileinfo
    }
}

##
# createUnglomProgress
#
#    Pastes the unglom progress UI into the top level widget.
#    The resulting megawidget path is returned.
#    Also sets up an update of the file list of the box to happen.
#
proc createUnglomProgress {} {
    UnglomProgress .progress
    updateUnglomFiles .progress 1000
    
    return .progress
}

##
# unglomInput
#   Invoked when there's input on the glom fd.
#   If eof close the file and write the status to ::unglomdone
#   Otherwise, read a line from the file and report it to the
#   progress widget.
#
# @param fd - file descriptor open on the unglom stdout/stderr.
# @param wid - Widget that has the unglom progress.
#
proc unglomInput {fd wid} {
    puts "Unglom readable"
    if {![eof $fd]} {
        set line [gets $fd]
        puts "$line"
        if {[winfo exists $wid]} {
            $wid addMessage $line
        }
    } else {
        puts "EOF"
        set status [catch {close $fd} msg]
        set ::unglomdone $status
    }
}

##
# unglom
#   Unglom the input files to sid-n files.
#   - Construct the unglom command (catting the source files
#     to a - data source).
#   - Exec the file capturing its stdin/stderr.
#   - Set  a file event on the stdin/stderr
#   - Create a gui to track the progress and in which to report errors.
#   - vwait for the ::unglomdone to change.
#   That variable will be 0 for success and -1 for errors.
#
# @param  eventfiles - the event files to unglom.
#
proc unglom eventfiles {
    
    #  Create the command as a cat eventfile |unglom | capture stdout/err.
    
    set command "cat $eventfiles | $::bindir/Unglom - |& cat"
    
    set fd [open "| $command" r]
    
    wm deiconify .
    set widget [createUnglomProgress]
    pack $widget
    wm title . {Unglom progress}
    
    fconfigure $fd -buffering line
    fileevent $fd readable [list unglomInput $fd $widget]
    
    
    puts "Waiting on unglomdone"
    vwait ::unglomdone
    puts "Unglom status $::unglomdone"
    destroy $widget
    
}
#------------------------------------------------------------------------------
# Procs involved in reglomming.

##
#  ReglomUi
#    User interface megawidget for reglom.
#
#  +-------------------------------------------+
#  | outputfile     nbytes                     |
#  |   +-----------------------------+         |
#  |   |   output line list          |         |
#  |   +-----------------------------+         |
#  +-------------------------------------------+
#
# OPTIONS:
#   -file       what's displayed in outfile.
#   -size       Number of bytes (displayed as nbytes).
#
# PUBLIC METHODS:
#    addLine  - Add a text line to the list of lines.
#
snit::widgetadaptor ReglomUI {
    option -file
    option -size
    
    ##
    # constructor
    #    Install a  ttk::frame as the hull and build the GUI inside of it.
    #
    # @param args - any command  line parameters on construction - treated as options.
    #
    constructor args {
        installhull using ttk::frame
        
        ttk::label $win.filename -textvariable [myvar options(-file)]
        ttk::label $win.nbytes   -textvariable [myvar options(-size)]
        
        listbox $win.outputs -width 80 -yscrollcommand [list $win.sb set]
        ttk::scrollbar $win.sb -command [list $win.outputs yview]
        
        # Lay out -- the outputs will fill the left two columns;  so the
        # two labels get west justified:
        
        grid $win.filename $win.nbytes -sticky w
        grid $win.outputs - $win.sb -sticky nsew
        
        
        $self configurelist $args
    }
    ##
    # addLIne
    #   adds a line of text to the list box:
    #
    # @param line - the line
    #
    method addLine line {
        $win.outputs insert end $line
    }
}


##
# filetoURI
#   Turns a filename into a URI.
#   - normalize the file path.
#   - prepend file://
#
# @param file - name of the file to URI-ize.
#
proc filetoURI file {
    set file [file normalize $file]
    return file://$file
}

##
# createReglomUI
#
#   Create a ReglomUI megawidget and pack it into the main window.
#
# @param filename -name of the file being created by reglom.
# @return widget  - name of the widget.
#
proc createReglomUI filename {
    ReglomUI .reglomprogress -file $filename
    
    return .reglomprogress
}
##
# updateReglomProgress
#    updates the reglom progress window with the file size.
#
# @param widget - The widget to update.
# @param file   - name of the file to update for.
# @param ms     - ms to next update.
#
# @note if widget has been destroyed we don't reschedule.
#
proc updateReglomProgress {widget file ms} {
    if {[winfo exists $widget]} {
        if {[file exists $file]} {
            $widget configure -size [file size $file]
        }
        after $ms [list updateReglomProgress $widget $file $ms]
    }
}
##
# readReglomPipe
#    Called when stdout/stderr for reglom becomes readable.
#    - IF EOF, close the fd and signal ::reglomdone
#    - Otherwise read the line and pass it to the progress widget.
#
# @param fd  - File descriptor.
# @param widget - progress widget.
#
proc readReglomPipe {fd widget} {
    if {![eof $fd]} {
        set line [gets $fd]
        $widget addLine $line
    } else {
        set status [catch {close $fd}]
        set ::reglomdone $status
    }
}
##
# reglom
#   Runs the reglom of source event files:
#   - turns the event files into file urls
#   - Creates the reglom progress window in .
#   - Starts up the reglom process capturing stdout/stderr.
#   - Sits back vwaiting for the reglom to finish.
#
# @param files   - The set of files to glom (each a data source).
# @param dt      - Timestamp difference that defines the coincidence window for
#                  glomming.
# @param sid     - output sourceid.
# @param tsPol   - Policy to assign timestamps to output events.
# @param outfile - File to which to write the final built events.
#
proc reglom {files dt sid tspol outfile} {
    
    # Convert the files to URIS.
    
    set uris [list]
    foreach file $files {
        lappend uris [filetoURI $file]
    }
    # Construct the reglom command.
    
    set command "$::bindir/reglom --dt $dt --sourceid $sid --timestamp-policy $tspol "
    append command " --output $outfile "
    append command $uris
    append command " |& cat ";        # Capture stdout and stderr.
    
    #  Fire  up the update UI - and file progress updates.

    set widget [createReglomUI $outfile]
    pack $widget
    updateReglomProgress $widget $outfile 1000;
    wm title . {Reglom progress}
    after 100 [list wm geometry . ""]
    
    #  Fire up the subprocess with output monitoring.
    
    set fd [open "| $command" r]
    fconfigure $fd -buffering line
    fileevent $fd readable [list readReglomPipe $fd $widget]
    
    #  Wait for the process to complete:
    
    vwait ::reglomdone
    puts "Reglom status $::reglomdone"
    destroy $widget;            # Eventually runs down reglom process.
}

#------------------------------------------------------------------------------
# Entry point
#



wm withdraw .

#  DAQBIN is necessary to locate glom:

if {[array names env DAQBIN] eq ""} {
    tk_messageBox -icon error -type ok -title {DAQBIN not defined} \
	-message {The DAQBIN environment variable must be defined to use this command source daqsetup.bash from the top level directory of the NSCLDAQ installation this comes from.}
    exit -1
}

# Clean up other unglommed files:

set files [glob -nocomplain sid-*]
foreach file $files {
    file delete $file
}

#  Figure out the default values for dt, sourceid, policy:

set dt      1
set sourceid 0
set policy  earliest
 
if {[file exists ~/.reglom]} {
    source ~/.reglom;                       # overrides the settings above.
}

#  Now prompt for the full set of settings:

toplevel .prompt
DialogWrapper .prompt.settings
set controlarea [.prompt.settings controlarea]
set prompter    [ParameterSelection $controlarea.prompt -dt $dt -sourceid $sourceid -policy $policy]
.prompt.settings configure -form $prompter
pack .prompt.settings

set result [.prompt.settings modal]

if {$result eq "Ok"} {
    set dt       [$prompter cget -dt]
    set sourceid [$prompter cget -sourceid]
    set policy   [$prompter cget -policy]
    writeConfigFile $dt $sourceid $policy
    
    set eventFiles [$prompter cget -inputfiles]
    set outfile    [$prompter cget -outputfile]
    
    if {[llength $eventFiles] == 0} {
        puts stderr "Input event file(s) have not been selected"
        exit -1
    }
    if {$outfile eq ""} {
        puts stderr "No outputEvent file has been selected"
    }
    destroy .prompt
    
    #  Split the event files apart by sourceid.
    
    unglom $eventFiles
    set sourceFiles [glob -nocomplain sid-*];     # list of files created by unglom.
    puts $sourceFiles
    
    reglom $sourceFiles $dt $sourceid $policy $outfile
    
    # Clean up my temp files.
    
    foreach file $sourceFiles {
        file delete $file
    }
}



exit 0
