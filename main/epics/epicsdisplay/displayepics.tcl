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

#
#   This gui is intended to be put in a TclServer connected to
#   controlpush;  Controlpush is responsible for maintaining
#   the following arrays:
#     EPICS_DATA (channelname)   - Value of a channel
#     EPICS_UNITS(channelname)   - Units of a channel
#     EPICS_UPDATED(channelname) - Time of last update.
#
#   We take as input a channels.txt file
#   Extract the channel names and 'same line comments',
#   Sort them alphabetically by channel and maintain them
#   in a tktable.
#

set scriptDir [file dirname [info script]]
set libdir    [file normalize [file join $scriptDir ..]]
lappend auto_path $libdir

package require removetcllibpath
package require Tktable
package require BWidget
package require rbc
package require Iwidgets
package require snit
package require Utils

namespace import ::rbc::stripchart
namespace import ::rbc::vector

set version 2.0

set tableWidget ""
set chartWidget ""
set paneWidget ""


set stripColors {black red green blue cyan magenta brown}
set stripStyles { "" {5 5} {2 2} {5 5 2 2} {5 2} { 5 5 2 5}}
set stripColorIndex 0
set stripStyleIndex 0
set vectorNumber 0
set time 0

set autotimeRange 0
set timeRange     2;          #Minutes.
set retainTime    {}

set chartChannels ""

set popupIndex 0

#   We don't want the vector to build up without bound in case
#   nobody ever hits the clear button.  The value below is the
#   maximum number of hours we will keep.
#
set maxKeepHours 4.0;			# Should be floating.

set here [file dirname [info script]]
set helpdir [file join $here .. data  epicsdisplay]

#------------------------------------------------------------------------------
# helpAbout:
#    Displays the message box that gives program version etc. info.
#
proc helpAbout {} {
    global version
    tk_messageBox -icon info -title About... \
	-message "displayepics Version $version\n\
Author:   Ron Fox\n \
(c) NSCL 2005 All rights reserved\n\
Licensed for use under the GPL\n\
See http://www.gnu.org/licenses/gpl.txt\n\
for terms and conditions."

}
#------------------------------------------------------------------------------
# helpTopics
#     If the hyperhelp widget is not yet displayed, pops it up and points it
#     at the overview topic.
#     We also make our best effort to make the hyperhelp widget visible.
#
proc helpTopics {} {
    global helpdir
    if {![winfo exists .help]} {
	::iwidgets::hyperhelp .help -topics {overview gui config} -helpdir $helpdir
	.help showtopic overview
    }
    focus .help
    wm deiconify .help
}
proc displayHelpTopic topic {
    helpTopics
    .help showtopic $topic
}
#------------------------------------------------------------------------------
# writeFile fd
#     Write a configuration file given the results of an open w  on it.
# Parameters:
#  fd   - The file descriptor returned from an open or an open like
#         command (e.g. socket is also ok).
#
proc writeFile fd {
    global tableWidget

    set rows [$tableWidget cget -rows]
    for {set row 1} {$row < $rows} {incr row} {
	set name    [$tableWidget get $row,0]
	set comment [$tableWidget get $row,3]
	puts $fd "$name  $comment"
    }
}
#------------------------------------------------------------------------------
# saveFile name
#     Save a configuration file given its name.
# Parameters:
#   path to the configuration file.
#
proc saveFile name {
    if {[catch {open $name w} fd]} {
	tk_messageBox -icon error -title "Open Failed" \
	    -message "Could not open $name for write: $fd"
	return
    }
    writeFile $fd
    close $fd
}
#------------------------------------------------------------------------------
# saveFailsafe
#    Save a failsafe file in the current directory.
#
proc saveFailsafe {} {
    saveFile failsafechannels.txt
}
#------------------------------------------------------------------------------
# fileSave
#     Responds to the File->Save menu entry. Using a tk_getSaveFile prompts
#     the user for a filename and then invokes [saveFile] to write a new channel
#     file.
#
proc fileSave {} {
    set name [tk_getSaveFile -title "Save File as"                   \
			     -defaultextension .txt                  \
			     -filetypes [list                        \
					 [list {Text File} .txt]     \
					 [list {Config file} .cfg]   \
					 [list {Channel file} .chan] \
					 [list {All files}   *]]]
    if {$name ne ""} {
	saveFile $name
    }
}
#------------------------------------------------------------------------------
# fileExit
#     Called in response to the File->Exit command.
#     If the user confirms
#     o Write the failsafe file.
#     o Exit.
#
proc fileExit {} {
    set answer [tk_messageBox -title exit -type question -type okcancel          \
		-message {Are you sure you want to exit? Click ok to confirm}]
    if {$answer eq "ok"} {
	saveFailsafe
	exit 0
    }
}

#------------------------------------------------------------------------------
# channelsAdd:
#     Add a channel to the display.
#
proc channelsAdd {} {
    getChannelName .new -helpcommand [list displayHelpTopic add]
    .new modal
    if {[winfo exists .new]} {
	set name [.new cget -channel]
	set plot [.new cget -plot]
	set name [string toupper $name]
	destroy .new
	if {$plot} {
	    set comment {chart added dynamically}
	} else {
	    set comment {added dynamically}
	}
	if {$name ne ""} {
	    addTableChannel $name $comment
	    if {$plot} {
		addPlotChannel $name
	    }
	    saveFailsafe
	    clearStripChart
	    restartControlPush failsafechannels.txt
	}
    }
}
#------------------------------------------------------------------------------
# flushOldData:
#     Called to determine if it is time to flush the old data
#     in the strip chart vector and, if so, does just that.
#
proc flushOldData {} {
    global timeVector
    global chartChannels
    global maxKeepHours

    set maxKeepSamples [expr round($maxKeepHours * 3600)]
    set samples [timeVector length]
    if {$samples > $maxKeepSamples} {
	set killcount [expr $samples - $maxKeepSamples/2]
	timeVector delete 0:$killcount
	for {set i 0} {$i < [llength $chartChannels]} {incr i} {
	    set name channel$i
	    $name delete 0:$killcount
	}

    }
}

#------------------------------------------------------------------------------
#
#  Select the style for a plot.
#
proc selectStyle {index} {
    global stripColors
    global stripStyles

    set color [lindex $stripColors \
                   [expr $index % [llength $stripColors]]]
    set style [lindex $stripStyles [expr $index / [llength $stripColors]]]

    return [list $color $style]

}
#------------------------------------------------------------------------------
#
#  compareFirst:
#    Simple sorting predicate to compare element 0 of two list
#    elements
#
proc compareFirst {el1 el2} {
    return [string compare $el1 $el2]
}
#------------------------------------------------------------------------------
#
#   Create a the Gui which now consists of
#   a table in a scrolled window.
#   The table starts with 1 row (a row of titles) and columns for:
#   channel name, channel value, channel units, channel comment.
#
proc makeGui {} {
    global paneWidget
    global tableWidget

    #  Set up the menubar

    menu .menubar
    menu .menubar.file -tearoff 0
    menu .menubar.channels -tearoff 0
    menu .menubar.help    -tearoff 0
    .menubar add cascade -label File     -menu .menubar.file
    .menubar add cascade -label Channels -menu .menubar.channels
    .menubar add cascade -label Help     -menu .menubar.help

    .menubar.file add command -label {Save...} -command fileSave
    .menubar.file add separator
    .menubar.file add command -label Exit      -command fileExit

    .menubar.channels add command -label {Add...} -command channelsAdd

    .menubar.help add command -label About  -command helpAbout
    .menubar.help add command -label Topics -command helpTopics

    . configure -menu .menubar

    # The body of the application:

    set paneWidget [frame .f -width 4i]
    set f $paneWidget

    #Table of channels...

    ScrolledWindow   $f.sw
    set twframe [$f.sw getframe]
    set tableWidget  [table $twframe.table -rows 1 -cols 4 -cache 1 \
                                  -titlerows 1  \
                                  -pady 2 -padx 2 -justify left]
    $tableWidget width 3 15
    $tableWidget set row 0,0 {Name Value Units Comment}
    $f.sw setwidget $tableWidget
    pack $f.sw -fill both -expand 1 -side top
    pack $paneWidget -side top -fill x -expand 1

    # Bind double clicks on the table to bring up the alarm configuration
    # menu for the corresponding channel:

    bind $tableWidget <Double-Button-1> [list configureChannelAlarm %x %y]
}
#------------------------------------------------------------------------------
#  setRowComment
#   Sets the comment value for the specified row.
# @param row
# @param comment  (note this is trimleft-ed before used)
#
proc setRowComment {row comment} {
    global tableWidget
    set comment [string trimleft $comment]

    $tableWidget configure -state normal
    $tableWidget set $row,3 $comment
    $tableWidget configure -state disable
    
}

#---------------------------------------------------------------------------------
# clearAlarmSetup name row
#       Clears alarm information for  a channel.. This includes the
#       channel alarms as well as alarm description info in the
#       channel comment string.
# Parameters:
#    name   - Name of the channel.
#    row    - Row fo the channel in the table.
#
proc clearAlarmSetup {name row} {
    global tableWidget


    set comment [$tableWidget get $row,3]
    set nomindex [lsearch -exact $comment tolerance]
    if {$nomindex != -1} {
	set comment [lreplace $comment $nomindex [expr $nomindex + 1]]
    }
    set actindex [lsearch -exact $comment alarmaction]
    if {$actindex != -1} {
	set comment [lreplace $comment $actindex [expr $actindex +1]]
    }
    setRowComment $row $comment
    
}

#------------------------------------------------------------------------------
# setRowAlarmText row nominal range actions
#       Set the comment information for a row to include alarm descriptions
#       The assumption is that the alarm information has already been cleared
#       via a call to clearAlarmSetup.
#
# Parameters:
#    row    - Row number
#    nominal- Nomina value.
#    range  - Range specification relative to the nominal as gotten from
#             the alarmConfiguration dialog.
#    actions- The actions list from the alarm dialog,  except that color has
#             been turned in a color=value pair.
#
proc setRowAlarmText {row nominal range actions} {
    global tableWidget

    #  Translate the ok range into comment form:

    if {[llength $range] == 1 } {
	append spec $nominal : $range
    } else {
	append spec $nominal + [lindex $range 0] - [lindex $range 1]
    }
    # translate the actions.

    set alarmactions [join $actions ","]

    set comment [$tableWidget get $row,3]


    append comment " " tolerance " " $spec " " alarmaction " " $alarmactions
    setRowComment $row $comment
}

#------------------------------------------------------------------------------
#  modifyAlarmConfig  widget row
#        Modify the alarm configuration of a channel in the table as given
#        by an alarm configuration widget.
# Parameters:
#    widget   - The widget used to configure the alarm.
#    row      - Row of the table being configured.
#
proc modifyAlarmConfig {widget row} {
    global channelAlarms

    #  Get the channel name and clear the alarm configuration
    #  in the comment  of the table.
    #

    set name [$widget cget -channel]
    clearAlarmSetup $name $row

    # We only have work to do if the channel's alarm is
    # enabled.

    if {[$widget cget -enable]} {
	set units [.units$row cget -text]
	set value [.value$row cget -text]

	set nominal [$widget cget -nominal]
	set range   [$widget cget -range]
	set actions [$widget cget -actions]
	# some conversion on the actions needed for color.


	if {[llength $range] == 1} {
	    set delta [deltaToNumber $nominal $range]
	    set low   [expr $nominal - $delta]
	    set high  [expr $nominal + $delta]
	} else {
	    set dup   [deltaToNumber $nominal [lindex $range 0]]
	    set ddown [deltaToNumber $nominal [lindex $range 1]]
	    set low   [expr $nominal - $ddown]
	    set high  [expr $nominal + $dup]

	}
	if {[array names channelAlarms $name] ne ""} {
	    cancelAlarm $name $value [lindex $channelAlarms($name) 2]
	}
	createAlarm $name $low $high $actions $nominal
	setRowAlarmText $row $nominal $range $actions


	# If the channel is in a displayed alarm window the alarm
	# definition must be updated.
	# If the action is not page.. dismissal will be permanent even
	# if the alarm condition remains.

	if {[winfo exists .alarmpage]} {
	    if {[.alarmpage get $name] ne ""} {
		set spec $channelAlarms($name)
		.alarmpage add $name $low $high $value $units
	    }
	}

    } else {
	# If necessary remove the channelAlarms entry for this
	# channel

	if {[array names channelAlarms $name] ne ""} {
	    unset channelAlarms($name)
	}

	# If the channel is in a displayed alarm window it must be
	# removed.
	if {[winfo exists .alarmpage]} {
	    .alarmpage remove $name
	}
    }

    destroy $widget

    saveFailsafe
}
#
#------------------------------------------------------------------------------
# configureChannelAlarm x y
#        Responds to a double click in the table body.  The
#        channel is determined and an alarm configuration dialog is
#        displayed modally... the alarm configuration is then
#        grabbed from the widget and configured for use in alamr scanning.
# Parameters:
#    x    - The widget relative x position of the click.
#    y
proc configureChannelAlarm {x y} {
    global tableWidget
    global channelAlarms

    set rc [$tableWidget index @$x,$y]
    set rc [split $rc ","]
    set row [lindex $rc 0]
    #
    #  Can't configure the title row.
    #
    if {$row == 0} {
	return
    }
    set channel [$tableWidget get $row,0]
    set value   [.value$row cget -text]


    #  Can only set alarms on numerical channels

    if {![string is double -strict $value]} {
	return
    }

    alarmConfiguration .alarmconfig -channel $channel \
                                    -nominal $value  \
				    -cancelcommand [list destroy .alarmconfig] \
				    -command     [list modifyAlarmConfig .alarmconfig $row]

    if {[array names channelAlarms $channel] ne ""} {
	set low     [lindex $channelAlarms($channel) 0]
	set high    [lindex $channelAlarms($channel) 1]
	set actions [lindex $channelAlarms($channel) 2]
	set value   [lindex $channelAlarms($channel) 3]
	.alarmconfig configure -nominal $value
	set alarmactions [list]
	foreach act $actions {
	    if {[lindex $act 0] eq "color"} {
            append act = [lindex $act 1]
	    }
	    lappend alarmactions $act
	}
	.alarmconfig configure -actions $alarmactions -enable 1

	set plus  [expr $high - $value]
	set minus [expr $value - $low]
	if {$plus == $minus} {
	    .alarmconfig configure -asymmetric 0 -range $plus
	} else {
	    .alarmconfig configure -asymmetric 1 -range [list $plus $minus]
	}
    }

    .alarmconfig modal

}


#------------------------------------------------------------------------------
#
#  Reads a setupfile.  The setup file is read and
#  returned as a  list of pairs:
#  channelname comment
#  sorted by channel name.
proc readSetupFile {filename} {
    set fd [open $filename r]
    set file [read $fd]
    close $fd
    set lines [split $file "\n"]
    foreach line $lines {
        # Skip comment lines.
        if {[string index $line 0] != "#"} {
	    set listLine [split $line " \t"]
	    set channel  [lindex $listLine 0]
	    set channelLength [string length $channel]
	    set comment [string range $line $channelLength end]
            lappend channelList [list $channel $comment]
        }
    }
    return $channelList
}
#------------------------------------------------------------------------------
#
#   Processes a channel list returned by readSetupFile e.g.
#   Creates a list of channels to be strip charted.
#   Strip charted channels have comments with the first
#   word "chart"
#
proc getStripchartChannels {channels} {
    set result ""
    foreach channel $channels {
        set comment [split [lindex $channel 1] " \t"]
        if {[lsearch -exact $comment chart] != -1} {
            lappend result [lindex $channel 0]
        }
    }
    return $result
}
#------------------------------------------------------------------------------
# addTableChannel name comment
#       Adds a channel line to the end of the table.
# Parameters:
#   name    - The name of the channel
#   comment - the channel comment.
#
proc addTableChannel {name comment} {
    global tableWidget
    $tableWidget configure -state normal

    $tableWidget insert row end
    set row [$tableWidget cget -rows]
    incr row -1
    $tableWidget set $row,0 $name
    $tableWidget set $row,3 $comment
    label .value$row -textvariable EPICS_DATA($name)
    label .units$row -textvariable EPICS_UNITS($name)

    $tableWidget window configure $row,1 -window .value$row -sticky e
    $tableWidget window configure $row,2 -window .units$row -sticky e

    $tableWidget configure -state disable
}
#------------------------------------------------------------------------------
#
#   Fills the gui table with a list of the form created by
#   readSetupFile.
#    For each row we put:
#       col0: name of channel.
#       col1: A label widget that is linked to EPICS_DATA(channelname)
#       col2: A label widget that is linked to EPICS_UNITS(channelname)
#       col3: A comment.
# list:     The list to use to fill the table.
proc fillTable {list} {
    global tableWidget
    foreach channel $list {
        set name    [lindex $channel 0]
	set comment [lindex $channel 1]
	if {$name ne ""} {
	    addTableChannel $name $comment
	}


    }
    $tableWidget configure -colstretchmode last
}
#------------------------------------------------------------------------------
#
#   Update the strip chart from the chartChannels channels.
#
proc updateStripChart {secs} {
    global chartChannels
    global timeVector
    global time
    global EPICS_DATA
    set ms [expr $secs*1000]

    timeVector append $time
    incr time

    for {set vectorNumber 0} {$vectorNumber < [llength $chartChannels]} {incr vectorNumber} {
        set vectorname channel$vectorNumber
        global $vectorname
        set name [lindex $chartChannels $vectorNumber]
        set value $EPICS_DATA($name)
        if {[string is double -strict $value]} {
            $vectorname append  $value
        } else {
	    $vectorname append 0.0
	}
    }
    flushOldData
    after $ms "updateStripChart $secs"
}
#------------------------------------------------------------------------------
#
#  Clear the strip chart:  The time is set back to zero.
#  The vectors are cleared of data:
#
proc clearStripChart {} {
    global timeVector
    global time
    global chartChannels

    set time 0
    timeVector length 0
    for {set vectorNumber 0} {$vectorNumber < [llength $chartChannels]} {incr vectorNumber} {
	set vectorname channel$vectorNumber
	global $vectorname
	$vectorname length 0
    }

}
#------------------------------------------------------------------------------
#
#   Sets the strip chart range.
#   Two globals determine what we should do:
#   if autotimeRange is non-zero, the time range will be set
#   to compress the timescale of the chart as needed to fit all
#   the points.  If not, then timeRange should be a
#   number of minutes of time range (the axes will be in seconds note).
#   As needed, the axis will shift by 10% of range.
proc setStripchartRange {} {
    global autotimeRange
    global timeRange
    global chartWidget
    global time
    global retainTime
    global maxKeepHours

    if {$autotimeRange} {
        $chartWidget axis configure x -autorange $time
        $chartWidget axis configure x -autorange 0

    } else {
        if {[string is integer -strict $timeRange]} {
            set seconds [expr $timeRange * 60]
            $chartWidget axis configure x \
                -autorange $seconds      \
                -shiftby   [expr $seconds/10]
        } else {
            tk_messageBox -icon error \
               -message "Time range must be an integer number of minutes" \
               -title "Bad int" -type ok
        }
    }
    # Set the maxkeep hours:

    if {[string is double -strict $retainTime]} {
	set maxKeepHours $retainTime
    } else {
	tk_messageBox -icon error \
	    -message "Retension time must be a floating point number of hours" \
	    -title "Bad doulbe" -type ok
    }
}
#------------------------------------------------------------------------------
#
# Setup the strip chart control panel.  We have a button
# to reset time to 0/clear vectors.
# We have a checkbutton that puts the chart in/or out of
# autoscale and an entry for the time range when the chart
# is not in autoscale.  We also havea 'set' button to make
# the time scale entries take effect.
#
proc setupStripControls {} {
    global maxKeepHours
    global chartChannels

#    if {[llength $chartChannels] != 0} {

	frame .stripcontrols -bd 2 -relief groove
	button .stripcontrols.clear -text Clear  -command clearStripChart

	frame .stripcontrols.time -bd 2 -relief groove
	checkbutton .stripcontrols.time.auto -variable autotimeRange -text "Auto Range"
	label       .stripcontrols.time.rlabel -text {  Range (min) :}
	entry       .stripcontrols.time.range -textvariable timeRange
	label       .stripcontrols.time.retainl -text {Retain data (hrs)}
	entry       .stripcontrols.time.retain -textvariable retainTime
	button      .stripcontrols.time.set   -text Set -command setStripchartRange

	.stripcontrols.time.retain insert end $maxKeepHours

	pack .stripcontrols.time.auto \
	    .stripcontrols.time.rlabel  .stripcontrols.time.range \
	    .stripcontrols.time.retainl .stripcontrols.time.retain \
	    .stripcontrols.time.set -side left
	pack .stripcontrols.clear  -side left
	pack .stripcontrols.time   -side right
	pack .stripcontrols -side top -fill x -expand 1
 #   }
}
#------------------------------------------------------------------------------
# addPlotChannel name
#      Adds a channel to the stripchart.
# Parameters:
#   name  - The paramter name.
#
proc addPlotChannel name {
    global vectorNumber
    global chartWidget
    global chartChannels
    global channel$vectorNumber

    makeStripChart

    set vect [vector create channel$vectorNumber]

    set style [selectStyle $vectorNumber]
    incr vectorNumber
    $chartWidget element create $name \
	    -xdata timeVector -ydata $vect \
	    -smooth linear -color [lindex $style 0] \
	    -symbol ""
    lappend chartChannels $name
}
#------------------------------------------------------------------------------
# makeStripChart
#     If the stripchart does not exist yet it is created.
#
proc makeStripChart {} {
    global chartWidget
    global paneWidget
    global timeVector


    if {![winfo exists $chartWidget]} {
	set f [frame .chart -height 4i ]
        set chartWidget [stripchart $f.stripchart -height 3i -width 4i]
	$chartWidget grid configure -hide 0
	$chartWidget legend configure -position top -anchor w

        vector create timeVector

	$chartWidget axis configure x -autorange 120 \
				    -shiftby 12 -title seconds
	$chartWidget axis configure y -autorange 0
        pack $chartWidget -fill both -expand 1
	pack $f -side top -fill x -expand 1
	setupStripControls
	setStripchartRange
	updateStripChart 1
    }
}
#------------------------------------------------------------------------------
#
#   Setup the stripchart given the list of channels to chart.
#
proc setupStripChart {channels} {
    global paneWidget
    global chartWidget
    global timeVector
    global chartChannels

    set channels [getStripchartChannels $channels]
    set vectorNumber 0
    foreach channel $channels {
	addPlotChannel $channel
    }
    if {[llength $channels] != 0} {
	clearStripChart
    }
}
#------------------------------------------------------------------------------
# getTableLine name
#      Returns the line of the table widget that contains a name.
# Returns:
#    The row of the table in which the name occurs or  -1 if it does not.
#    Note that the first used row of the table is 1 as row 0 has titles.
#
proc getTableLine name {
    global chans
    set row 1
    foreach entry $chans {
	if {$name == [lindex $entry 0]} {
	    return $row
	}
	incr  row
    }
    return -1
}
#------------------------------------------------------------------------------
# dimsissAlarmPopup widget name
#     Called to dismiss an alarm popup for a channel.  Note the popup will
#     only stay dismissed if the alarm condition is gone.
# Parameters:
#  widget   - The widget to destroy.
#  name     - The name of the alarming channel
#
proc dismissAlarmPopup {widget name} {
    global channelPopups
    destroy $widget

    unset channelPopups($name)
}
#------------------------------------------------------------------------------
# fireAlarms name value actions
#     Reports an alarm condition for the channel. ALl the necessary actions
#     are performed.
# Parameters:
#     name     - name of the channel
#     value    - Current value of the channel
#     actions  - Set of actions to report.
#
proc fireAlarm {name value actions} {
    global tableWidget
    global channelAlarmState
    global channelAlarms
    global channelPopups
    global popupIndex


    # If this is a transition, create the channelAlarmState array element

    if {[array names channelAlarmState $name] eq ""} {
	set channelAlarmState($name) [clock seconds]
	set transition 1
    } else {
	set transition 0
    }
    # Now do action dependent stuff:

    foreach action $actions {
        set request [lindex $action 0]
        switch -exact -- $request {
            color {
                set line [getTableLine $name]
                if {$line > 0} {
                    set color [$tableWidget tag cget $name -background]
                    $tableWidget tag row $name $line
                    .value$line configure -background $color
                    .units$line configure -background $color
                }
            }
            page {
                if {![winfo exists .alarmpage]} {
                    alarmPage .alarmpage -windowdismisscommand [list destroy .alarmpage]
                }
                if {[.alarmpage get $name] eq ""} {
                    set line [getTableLine $name]
                    set units [.units$line cget -text]
                    set spec $channelAlarms($name)
                    set low [lindex $spec 0]
                    set high [lindex $spec 1]
                    .alarmpage add $name $low $high $value $units
                }
                .alarmpage update $name $value
            }
            popup {
                #  Create the popup if it does not exist:
        
                if {[array names channelPopups $name] eq "" || ![winfo exists $channelPopups($name)]} {
                    set channelPopups($name) [alarmPopup .popup$popupIndex -name $name \
                                             -command [list dismissAlarmPopup %W $name] \
                                     -initialtime $channelAlarmState($name)]
    
                }
                $channelPopups($name) configure -value $value
                $channelPopups($name) updateDuration
                if {$transition} {
                    $channelPopups($name) configure -initialtime $channelAlarmState($name)
                }
            }
            beep {
                bell -nice
            }
        }
    }
}
#-----------------------------------------------------------------------------
# cancelAlarm name value actions
#      Takes the necessary actions to indicate that an alarm condition does
#      not exist on a channel anymore.
# Parameters:
#     name     - name of the channel
#     value    - Current value of the channel
#     actions  - Set of actions to report.
#
proc cancelAlarm {name value actions} {
    global channelAlarmState
    global tableWidget

    if {[array names channelAlarmState $name] ne ""} {
	unset channelAlarmState($name)
    }

    # Do action dependent stuff:

    foreach action $actions {
        set request [lindex $action 0]
        switch -exact -- $request {
            color {
                set line [getTableLine $name]
                if {$line > 0} {
                    set bgcolor [$tableWidget cget -background]
                    $tableWidget clear tags $line,0 $line,3
                    .value$line configure -background $bgcolor
                    .units$line configure -background $bgcolor
                }
            }
            page {
                if {[winfo exists .alarmpage]} {
                    if {[.alarmpage get $name] ne ""} {
                    .alarmpage update $name $value
                    }
                }
            }
            popup {
    
            }
            beep {
            }
        }
    }

    # unset the variable's channelAlarmState element if it exists..

    if {[array names channelAlarmState $name] ne ""} {
	unset channelAlarmState($name)
    }
}

#------------------------------------------------------------------------------
# checkAlarm name value spec
#     Checks to see if a channel is in the alarm state.
#     If so, the alarm actions fire. Note that if the value
#     is not a strict floating point value, the channel is
#     assumed either not to be alarmable or alternatively
#     not yet given up data and therefore no alarm will be
#     fired.
#
# Parameters:
#    name   - The name of the channel being checked.
#    value  - The value of the channel
#    spec   - The alarm specification [list low high actions]
#
proc checkAlarm {name value spec} {

    #  only check numeric values:

    if {[string is double -strict $value]} {
	set low     [lindex $spec 0]
	set high    [lindex $spec 1]
	set actions [lindex $spec 2]
	if {($value < $low) || ($value > $high)} {
	    fireAlarm $name $value $actions
	} else {
	    cancelAlarm $name $value $actions
	}
    }
}
#------------------------------------------------------------------------------
# processAlarms seconds
#      Self rescheduled procedure to check for and process alarms.
# Parameters:
#   seconds  - Number of seconds between reschedules.
#
proc processAlarms seconds {
    global channelAlarms
    global EPICS_DATA

    foreach channel [array names channelAlarms] {
	set value     $EPICS_DATA($channel)
	set alarmSpec $channelAlarms($channel)
	checkAlarm $channel $value $alarmSpec
    }

    after [expr {$seconds*1000}] [list processAlarms $seconds]
}
#------------------------------------------------------------------------------
# deltaToNumber nominal delta
#     Converts a delta specification to an actual number.
#      delta specifications are floating point values that
#      may be followed by a %.  If followed by a % they represent
#      a percentage of the nominal, otherwise they are an absolute value.
# Parameters:
#    nominal   - The nominal value that the delta expresses a difference to.
#    delta     - The delta specification
proc deltaToNumber {nominal delta} {
    if {[string last "%" $delta] != -1} {
	# % difference.

	scan $delta "%f" pct
	set value [expr {abs($nominal * $pct/ 100.0)}]
	return $value
    } else {
	# Absolute difference.

	return $delta
    }
}
#------------------------------------------------------------------------------
# getNominalRange rangeSpec
#    Parses an alarm range specification into a low and high
#    limiit that defines the nominal range of the channel.
# Parameters:
#    rangeSpec   - The range specification. See setupAlarms for more  information
#                  about the form this can take.
# Returns
#   A two element list that consists of the low and high limits within which the
#   parameter must lie in order to be considered not in alarm.
#
proc getNominalRange rangeSpec {

    set numberlist [string map [list : " " + " " - " "] $rangeSpec]
    scan $rangeSpec %f nominal


    # figure out if likely symmetric or what...

    if {[llength $numberlist] == 2} {
	# Likely symmetric... for now we accept any sep char.

	set size [deltaToNumber $nominal [lindex $numberlist 1]]
	set low  [expr {$nominal - $size}]
	set high [expr {$nominal + $size}]
	return [list $low $high $nominal]

    } else {
	# Likely asymmetric...need to figure out which is + and which -.

	set num1 [deltaToNumber $nominal [lindex $numberlist 1]]
	set num2 [deltaToNumber $nominal [lindex $numberlist 2]]


	set plusIndex  [string first "+" $rangeSpec]
	set minusIndex [string first "-" $rangeSpec]
	if {$plusIndex > $minusIndex} {
	    # -num1+num2
	    set low   [expr {$nominal - $num1}]
	    set high  [expr {$nominal + $num2}]
	} else {
	    #+num1-num2
	    set low   [expr {$nominal + $num1}]
	    set high  [expr {$nominal - $num2}]
	}
	return [list $low $high $nominal]
    }


}
#------------------------------------------------------------------------------
# getActions comment
#     Processes a comment string for alarm actions.  The alarm actions
#     are returned as a list of actions.  No attempt to parse or validate the
#     actions is performed.
#
# Parameters:
#    comment - The channel comment which has an action actions element.
# Returns:
#    The list of actions.
proc getActions comment {
    set actionIndex [lsearch -exact $comment alarmaction]
    if {$actionIndex != -1} {
	incr  actionIndex
	set actions [lindex $comment $actionIndex]
	return [split $actions ","]
    } else {
	return [list]
    }
}
#------------------------------------------------------------------------------
# createAlarm name low high actions
#     Create an entry in the channelAlarms array describing a channel alarm
# Parameters:
#  name    - The channel name.
#  low     - The low limit of the nominal range.
#  high    - The high limit of the nominal range.
#  actions - List of alarm actions.
#
proc createAlarm {name low high actions nominal} {
    global channelAlarms
    global channelPopups
    global tableWidget

    # Actions is processed to ensure that
    #  - all actions are recognized
    #  - The color actions result in an appropriate tag
    #    in the table widget.
    #

    set actionList [list]
    foreach action $actions {
	switch -glob -- $action {
	    page {
	    }
	    popup {
	    }
	    beep {
	    }
	    color* {
		set action [split $action "="]
		set color [lindex $action 1]
		$tableWidget tag configure $name  -background $color
	    }
	    default {
		error "Alarm action specification for $name has unrecognized alarm action: $action"
	    }
	}
	lappend actionList $action
    }
    set channelAlarms($name) [list $low $high $actionList $nominal]
}

#------------------------------------------------------------------------------
# setupAlarms
#     Processes the channel definitions for alarms. Alarms are indicated in the
#     comment fields by a pair of keywords and parameters to those keywords:
#      tolerance - Defines the channel nominal value and a tolerance.
#                  The parameter is of one of the following forms:
#                 nominal:tolerance
#                 nominal+high-low
#                 nominal-low+high
#                 Where tolerance is a floating point number followed by an optional %
#                 as are high and low.
#
#                 The first form specifies a symmetric acceptance region around
#                 nominal.  If  tolerance has a %, it must be in the range [0,100]
#                 and represents a percentage of nominal.  Otherwise it is an
#                 absolute tolerance (size of the region.
#
#                 The second and third form represnt an asymmetric acceptance interval
#                 about nominal, and low and high can have optional %'s on them
#                 as for tolerance above.
#     action  - Specifies the list of actions to take when the channel is out of the
#               tolerated limits.  The parameter is a comma separated list of actions
#               that are drawn from:
#                page   - Add the alarm to the alarm display page.
#                popup  - Popup a widget on alarm.
#                beep   - Beep the speaker on alarm.
#                color=value - color the table widget line containing the
#                         channel as described by value.  value can be any
#                         color specification acceptable to Tk.
#      The following global data are built to describe the alarms
#      channelAlarms - an array indexed by channel name.
#                      The contents of each element is a 3 element list:
#                      lowlimit highlimit actions
#                         actions is a Tcl formatted list of the actions to take when the
#                         channel transitions into alarm state.
#      channelPopups - An array indexed by channel name.
#                      The contents of each element that exists is the widget path for
#                      an alarm popup that is open for that channel.
#      channelAlarmTime
#                    - An array indexed by channel name.  Each element that exists represents
#                      a channel that is currently in the alarmed state.  The contents of the array
#                      element represent the time in [clock seconds] form at which the transition from
#                      Ok to alarm occured.
# Parameters:
#    chans  - The channels in the definition file.  Each channel is a 2 element list
#             containing the channel name and the comment.
#
proc setupAlarms {chans} {
    foreach channel $chans {
	set name    [lindex $channel 0]
	set comment [split [lindex $channel 1]  " "]

	# Since actions are useless without tolerances  look first for them

	set tolindex [lsearch -exact $comment "tolerance"]
	if {$tolindex != -1} {
	    incr tolindex;             #The tolerance descriptor.
	    set range [getNominalRange [lindex $comment $tolindex]]
	    set low  [lindex $range 0]
	    set high [lindex $range 1]
	    set nominal [lindex $range 2]
	    set actions [getActions $comment]
	    createAlarm $name $low $high $actions $nominal
	}
    }
    processAlarms 1
}

#------------------------------------------------------------------------------
#
#  getChannelName
#       This is a modal snit widget that prompts for an epics channel name.
#       unfortunately, there's no way (I know of), to enumerate the channels
#       so we just put up a warning message that they have to type the channel
#       correctly.
# The widget layout is:
#
#     +---------------------------------------------------+
#     |  The warning message I spoke of above......       |
#     |   Epics Channel: <channel name>                   |
#     |                  [ ] Plot channel                 |
#     +---------------------------------------------------+
#     |   [ Ok ]     [ Cancel ]               [Help]      |
#     +---------------------------------------------------+
#
#  Options:
#      -command         Script called on ok.
#      -cancelcommand   Script called on cancel.
#      -helpcommand     Script invoked on help button.
#      -plot            Boolean value of the plot checkbutton.
#      -channel         Value of the channel entry widget.
#
snit::widget getChannelName {
    hulltype toplevel

    option   -command         {}
    option   -cancelcommand   {}
    option   -helpcommand     {}
    option   -plot
    option   -channel

    variable plotting     0
    variable channelName  {}
    variable hiddenWidget {}
    
    constructor args {
	$self configurelist $args

	frame    $win.warning
	message  $win.warning.message -text \
                 {Warning, this application has no way to know if you are
feeding it real EPICS channel names or just garbage.
If you don't see a value for the channel chances are
good you fed it garbage}

	set work [frame $win.work]
	label $work.chanlabel -text {EPICS Channel: }
	entry $work.channel    -textvariable [myvar channelName]
	checkbutton $work.plot -variable [myvar plotting] -text {Add to plot}

	set action [frame $win.action -relief groove -borderwidth 4]
	button $action.ok     -text Ok      -command [mymethod onOk]
	button $action.cancel -text Cancel  -command [mymethod onCancel]
	button $action.help   -text Help    -command [mymethod onHelp]

	# Layout the geometry:

	pack $win.warning.message -fill both -expand 1
	pack $win.warning         -fill x    -expand 1
	grid $work.chanlabel $work.channel
	grid     x           $work.plot
	pack $work     -side top -fill x -expand 1
	grid $action.ok $action.cancel $action.help
	pack $action   -side top -fill x -expand 1

    }
    # cget -plot

    oncget -plot {
	return $plotting
    }

    # configure -plot

    onconfigure -plot value {
	set plotting $value
    }

    # cget -channel

    oncget -channel {
	return $channelName
    }

    # configure -channel

    onconfigure -channel value {
	set channelName $value
    }
    # dispatch script
    #     Dispatches a widget script.
    #     The following substitutions are done:
    #      %W    - The widget name.
    #      %C    - The current channel name.
    #      %P    - 1 if plotting is on 0 otherwise.
    #
    method dispatch script {
	if {$script ne "" } {
	    if {$plotting eq ""} {
		set plot "{}"
	    } else {
		set plot $plotting
	    }
	    regsub -all "%W" $script $win         script
	    regsub -all "%C" $script $channelName script
	    regsub -all "%P" $script $plot        script

	    eval $script
	}
    }
    # endModality
    #      If the variable hiddenWidget represents a widget,
    #      destroy it to end the modality of the dialog.
    #
    method endModality {} {
	if {[winfo exists $hiddenWidget]} {
	    destroy $hiddenWidget
	    set hiddenWidget {}
	}
    }
    # onHelp
    #      Called when the help button is clicked.  The
    #      -helpcommand is dispatched.
    #
    method onHelp {} {
	$self dispatch $options(-helpcommand)
    }
    # onOk
    #     Called when the ok button is clicked.  Dispatch
    #     to the -command script and end the widget modality
    #     if necessary.
    method onOk {} {
	$self dispatch $options(-command)
	$self endModality
    }
    # onCancel
    #    Called when the cancel button is clicked.. sets
    #    the channel to empty, invokes the -cancelcommand
    #    script and ends modality.
    #
    method onCancel {} {
	set channelName {}
	$self dispatch $options(-cancelcommand)
	$self endModality
    }
    # modal
    #    If the widget is not yet modal, make it modal.
    #
    method modal {} {
	if {![winfo exists $hiddenWidget]} {
	    set hiddenWidget [frame $win.hidden]
	    focus $win
	    wm deiconify $win
	    grab $win
	    tkwait win $hiddenWidget
	    grab release $win
	}
    }

}
#------------------------------------------------------------------------------
#
# alarmPopup
#        This megawidget implements the alarm popup megawidget.
#        It can be created for channels that specify the popup action.
#        Layout is roughly:
#               +-----------------------------------------+
#               | Alarm on Channel:       name            |
#               | Alarmed at      :      timestamp        |
#               | Duration        :      elapsed time     |
#               | Current value   :      channel value    |
#               +-----------------------------------------+
#               |               [ Dismiss ]               |
#               +-----------------------------------------+
# Options:
#      -name           Provides the channel name.
#      -initialtime    Provides the [clock seconds] at which the alarm started.
#                      If not provided, this becomes the [clock seconds] of the
#                      construction of the widget.
#      -value          Provides a value for the channel.
#      -command        Provides a script called when the dismiss button is clicked.
#                      %W is substituted with the name of this widget.
#
#
# Methods:
#     updateDuration   Computes the difference between [clock seconds] and the time in
#                      the alarmed at field and displays it in the Duration field.
#
snit::widget alarmPopup {
    hulltype toplevel

    option -name         {}
    option -initialtime  {}
    option -value        {}
    option -command      {}

    constructor args {
	#  Create the GUI

	set data [frame $win.data -relief groove -borderwidth 4]
	label $data.chanlabel -text {Alarm on Channel} -anchor w
	label $data.channel   -anchor w
	label $data.initlabel -text {Alarmed at} -anchor w
	label $data.initial   -anchor w
	label $data.durlabel  -text {Duration} -anchor w
	label $data.duration  -anchor w
	label $data.vallabel  -text {Current value} -anchor w
	label $data.value     -anchor w

	grid $data.chanlabel  $data.channel   -sticky ew
	grid $data.initlabel  $data.initial   -sticky ew
	grid $data.durlabel   $data.duration  -sticky ew
	grid $data.vallabel   $data.value     -sticky ew

	pack $data -side top -fill x -expand 1

	frame  $win.action
	button $win.action.dismiss -text Dismiss -command [mymethod onDismiss]
	grid   x $win.action.dismiss   x
	pack  $win.action -side top -fill x -expand 1


	#  Process the configuration options:

	$self configurelist $args

	#  If there's no initial time set it to now:

	if {$options(-initialtime) eq ""} {
	    $self configure -initialtime [clock seconds]
	}
    }
    # name configure -name channelname
    #     Configure the channel name.  This is saved in options(-name)
    #     and configured as the text of the $win.data.channel label.
    #
    onconfigure -name channelname {
	set options(-name) $channelname
	$win.data.channel configure -text $channelname
    }
    # name configure -initialtime seconds
    #     Configures the start time to be seconds.
    #     this is set in options(-initialtime) and, formatted
    #     via [clock format] as the value of the win.data.initial label.
    #
    onconfigure -initialtime seconds {
	set options(-initialtime) $seconds
	$win.data.initial configure -text [clock format $seconds]
    }
    # name configure -value  value
    #     Configures the value of the channel.   The value is saved in
    #     options(-value) and configured into the $win.data.value widget.
    #
    onconfigure -value value {
        set options(-value) $value
        $win.data.value configure -text $value
    }
    # updateDuration
    #      Computes the difference between >now< and initialtime and displays that in
    #      $win.data.duration
    #
    method updateDuration {} {
        set now [clock seconds]
        set delta [expr {$now - $options(-initialtime)}]
        set display [Utils::formatDeltaTime $delta]

    
        $win.data.duration configure -text $display

    }
    # onDismiss
    #     Called when the dismiss button is clicked.  If there is a -command script
    #     it has instances of %W regsubbed for $self... and then invoked.
    #
    method onDismiss {} {
	set script $options(-command)
	if {$script ne ""} {
	    regsub "%W" $script $win finalscript
	    eval $finalscript
	}
    }

}
#------------------------------------------------------------------------------
# alarmPage
#     The alarm page megawidget.  This displays a table of the alarms which
#     have the action "page"  Alarms are displayed along with the following
#     information:
#
#     Name, Value, Time Alarmed, Duration of most recent alarm.
#     The foreground text of each line is color coded as follows:
#        red    - indicates a transition from unalarmed to alarmed that has not
#                 yet been acklowledged
#       yellow  - indicates a channel in the alarmed state that has been
#                 acknowledged (the user selected the channel and clicked
#                 the acknowledge button
#       black   - indicates the channel is no longer in the alarmed state.
#
#  Layout
#          +-------------------------------------------------------------------+
#          | +-------------------------------------------------------+         |
#          | | Name       Value  Units  Alarm time   Alarm duration  |         |
#          | |                                                       |  ^      |
#          | |                                                       |  V      |
#          | +-------------------------------------------------------+         |
#          |          [Dismiss alarm]        [Acknowledge]                     |
#          +-------------------------------------------------------------------+
#          |                       [Dismiss window]                            |
#          +-------------------------------------------------------------------+
#
#  Options
#       -windowdismisscommand  - Command to invoke when window is dismissed.
#       -alarmdismisscommand   - Command to invoke when an alarm is dismissed.
#       -alarmackcommand       - Command to invoke when an alarm is acknowledged
# The scripts support the following substitutions:
#    %W  The name of the object.
#    %A  The name of the alarm being acked or dismissed for
#        -alarmdismisscommand and -alarmackcommand.
#
#
#  Methods
#     add      name low high value units
#                                    Adds a channel to the alarm list (If name
#                                    is already in the table, updates its info.
#     update   name value            Updates the value of a name in the alarm
#                                    list.  It is an error to update a name
#                                    that is not present.
#     get      name                  Returns the channels array element
#                                    associated with name or [list] if
#                                    name is not in the alarm list.
#     remove   name                  Remove the name from the table.
#
#
# Other:
#     In order for the alarm duration to update, it is necessary
#     to update the channel periodically in this implementation.
#
snit::widget alarmPage {
    hulltype toplevel

    option -windowdismisscommand {}
    option -alarmdismisscommand  {}
    option -alarmackcommand      {}

    #  Color controls:

    variable newAlarmColor       red
    variable ackedAlarmColor     orange

    #   The variable below is an array indexed by channel name containing a
    #   list that has:
    #       value, units, low-limit, high-limit, initial-alarm-time, table-row
    #   Where most of these are as you might expect, and initial-alarm-time is
    #   {} if the channel is not now in alarm state.
    #
    variable channels

    #   The variable below contains the selected row.. or is {} if none are
    #   selected.

    variable selectedRow {}

    constructor args {
	$self configurelist $args;            # nothing needs widgets to exist.

	# Basic widgets and layout:

	set infoframe  [frame $win.infoframe -relief groove -borderwidth 4]
	ScrolledWindow $infoframe.tablewindow -auto both
	set tableframe [$infoframe.tablewindow getframe]
	table $tableframe.table -cols 5 -cache 1 -titlerows 1 -colwidth 18 \
	                        -rows 1
	$infoframe.tablewindow setwidget $tableframe.table
	ArrowButton $infoframe.up   -dir top    -command [mymethod onUp]
	ArrowButton $infoframe.down -dir bottom -command [mymethod onDown]
	button      $infoframe.dismiss -text {Dismiss Selected}     \
				       -command [mymethod dismissAlarm]
	button      $infoframe.ack     -text {Acknowledge Selected} \
				       -command [mymethod ackAlarm]

	set actionframe [frame $win.action]
	button $actionframe.dismiss -text {Dismiss Window} \
					-command [mymethod onDismiss]

	grid $infoframe.tablewindow     -              x
	grid          ^                 ^              $infoframe.up
	grid          ^                 ^              $infoframe.down
	grid $infoframe.dismiss         $infoframe.ack x

	grid     x                      $actionframe.dismiss x

	pack $infoframe   -side top -fill x -expand 1
	pack $actionframe -side top -fill x -expand 1

	# Title the table, and create tags for the alarms states:

	$tableframe.table set row 0,0 [list Name Value Units {Alarm Time} {Alarm Duration}]
	$tableframe.table tag configure newalarm   -foreground $newAlarmColor
	$tableframe.table tag configure ackedalarm -foreground $ackedAlarmColor
	$tableframe.table tag configure sel        -background blue
	$tableframe.table tag raise     sel

	# Event bindings... need to be able to select a row of the table.

	bind $tableframe.table <Button-1> [mymethod onClick %x %y $tableframe.table]
    }
    # swapRows row1 row2
    #      Swap the contents of two rows of the table.  Tags applied to the
    #      row move along with the contents (at least the newalarm or ackedalarm
    #      tags do.
    # Parameters:
    #    row1, row2   - The two rows to swap
    #
    method swapRows {row1 row2} {

	# Figure out the tags currently on each row

	set row1tag {}
	set row2tag {}
	set table $win.infoframe.tablewindow.table

	if {[$table tag includes newalarm $row1,0]} {
	    set row1tag newalarm
	}
	if {[$table tag includes ackedalarm $row1,0]} {
	    set row1tag ackedalarm
	}
	if {[$table tag includes newalarm $row2,0]} {
	    set row2tag newalarm
	}
	if {[$table tag includes ackedalarm $row2,0]} {
	    set row2tag ackedalarm
	}
	# get the contents of each row:

	set row1contents [$table get $row1,0 $row1,4]
	set row2contents [$table get $row2,0 $row2,4]

	#  Swap contents of the row.

	$table set row $row1,0 $row2contents
	$table set row $row2,0 $row1contents

	# Swap tags:

	$table tag row $row2tag $row1
	$table tag row $row1tag $row2

	# Update the channels database too.

	set channel1 [lindex $row1contents 0]
	set channel2 [lindex $row2contents 0]

	set channels($channel1) [lreplace $channels($channel1) 5 5 $row2]
	set channels($channel2) [lreplace $channels($channel2) 5 5 $row1]

    }
    # onUp
    #      Called when the up arrow button is clicked.  This button means
    #      that you want to re-order the currently selected row upwards,
    #      swapping it with the one above.  The selection sticks with
    #      the cell contents (moving upwards as well).  If the selected
    #      row is row 1, no movement is allowed and the bell is rung.
    #
    method onUp {} {
	if {($selectedRow eq [list]) || ($selectedRow <= 1) } {
	    bell
	    return
	}
	# Move is allowed.

	set otherRow $selectedRow
	incr otherRow -1
	$self swapRows $selectedRow $otherRow

	set selectedRow $otherRow
	setSelection $win.infoframe.tablewindow.table $selectedRow


    }

    # onDown
    #     Called when the down arrow button is clicked.  This button means
    #     the selected row should be swapped with the one below.
    #     The selection moves down as well.  If the selected row
    #     is the last table row, no movement is allowed, and the bell
    #     is rung.
    #
    method onDown {} {
	set rows [$win.infoframe.tablewindow.table cget -rows]
	if {($selectedRow eq [list]) || ($selectedRow >= ($rows-1))} {
	    bell
	    return
	} else {
	    # Move allowed.
	    set otherRow $selectedRow
	    incr otherRow
	    $self swapRows $selectedRow $otherRow

	    set selectedRow $otherRow
	    setSelection $win.infoframe.tablewindow.table $selectedRow
	}
    }

    # onClick x y widget
    #       Called on a mouse click.  The row under the pointer becomes
    #       the selected row.  If it is already selected, then it is
    #       unselected.
    # Parameters:
    #   x,y    - Pointer coordinates in widget origin relative pixels.
    #   widget - The table widget path.
    #
    method onClick {x y widget} {

	set index [$widget index @$x,$y]
	scan $index "%d,%d" row column
	$widget selection clear 0,0 end
	if {$row ==  $selectedRow || ($row == 0)} {\
	    set selectedRow {}
	} else {
	    set selectedRow $row
	    setSelection $widget $selectedRow
	}
    }

    # newRow name low high value units
    #       Called to unconditionally add a new row to the table.
    #       all the other rows are shifted down by one, and a new
    #       entry in the channels array is made.  If the
    #       current value is outside the nominal window, the new row
    #       is tagged newalarm.
    # Parameters:
    #    name   - Channel name.
    #    low    - Low limit on nominal window.
    #    high   - High limit on nominal window.
    #    value  - Current channel value.
    #    units  - Units of measure.
    #
    method newRow {name low high value units} {
	$win.infoframe.tablewindow.table insert rows 0
	$win.infoframe.tablewindow.table set row 1,0 [list $name $value $units]
	set alarmtime {}
	set alarmduration 0

	if {($value < $low) || ($value > $high) } {
	    $win.infoframe.tablewindow.table tag row newalarm 1
	    set alarmtime [clock seconds]
	    $win.infoframe.tablewindow.table set row 1,3 [list [clock format $alarmtime] $alarmduration]
	}
	foreach chan [array names channels] {
	    set line [lindex $channels($chan) 5]
	    incr line
	    set channels($chan) [lreplace $channels($chan) 5 5 $line]

	}
	set channels($name) [list $value $units $low $high $alarmtime 1]

    }
    #  updateDuration name
    #         Updates the alarm duration of a channel.  The assumption is
    #         that the channel exists and is, in fact, in alarm.
    # Parameters:
    #   name   - Name of the channel
    #
    method updateDuration name {
        set now [clock seconds]
        set alarmtime [lindex $channels($name) 4]
        set delta [expr {$now - $alarmtime}]
        set deltastamp [Utils::formatDeltaTime $delta]
        
        set tableline [lindex $channels($name) 5]
        $win.infoframe.tablewindow.table set $tableline,4 $deltastamp
    }

    # add name low high value units
    #       Adds a channel to the alarm list.. it is perfectly legal although
    #       perhaps bizzarre to add a channel that is not in the alarm state.
    #       The channel is inserted at the top of the table.  If the
    #       channel is now in the alarm state, then it is tagged with
    #       the newalarm tag.
    #          Note that if the channel is already in the table;
    #       its information in the channel array is updated, its value
    #       is updated, and the tag on the table adjusted accordingly.
    # Parameters:
    #   name   - Name of the channel
    #   low    - Low alarm limit of the channel
    #   high   - Low alarm limit on the channel
    #   value  - Current value of the channel.
    #   units  - Units of measure of the channel
    #
    method add {name low high value units} {
	if {[array name channels $name] ne "" } {
	    set oldvalue [lindex $channels($name) 0]
	    set oldunits    [lindex $channels($name) 1]
	    set lowlimit [lindex $channels($name) 2]
	    set hilimit  [lindex $channels($name) 3]
	    set alarmtime [lindex $channels($name) 4]
	    set tablerow  [lindex $channels($name) 5]
	    set channels($name) [list $oldvalue $units $low $high $alarmtime $tablerow]
	    if {$oldunits ne $units} {
		$win.infoframe.tablewindow.table set $tablerow,2 $units
	    }
	} else {
	    $self newRow $name $low $high $value $units
	}
	$self update $name $value

    }
    #  update name value
    #         Updates the value of a channel that's in the display. It is an
    #         error to do this for channels that have not yet been added.
    #         The value is updated in the display, and in the channels
    #         array.  If a transition is made from non-alarmed -> Alarmed,
    #         the line is tagged with the newalarm tag.  If a transition is
    #         made from alarmed -> non-alarmed, tags are removed from the line.
    # Parameters:
    #    name    - Name of the channel being updated.
    #    value   - The new value of the channel.
    #
    method update {name value} {
	if {[array names channels $name] eq ""} {
	    error "$win update - $name has not been added to the alarm list"
	}
	set tableLine [lindex $channels($name) 5]
	$win.infoframe.tablewindow.table set $tableLine,1 $value

	# If the channel is not in alarm, we can just remove tags.
	# If it's in alarm, we can only tag if this is a transition
	# since it might have been acknowledged

	set low  [lindex $channels($name) 2]
	set high [lindex $channels($name) 3]

	if {($value < $low) || ($value > $high) } {
	    set oldvalue [lindex $channels($name) 0]
	    if {($oldvalue >= $low) && ($oldvalue <= $high)} {
		# Transition:
		$win.infoframe.tablewindow.table tag row newalarm $tableLine
		set alarmTime [clock seconds]
		set channels($name) [lreplace $channels($name) 4 4 $alarmTime]
		$win.infoframe.tablewindow.table set row $tableLine,3 [list [clock format $alarmTime] 0]
	    } else {
		# Continued alarm state:
		$self updateDuration $name
	    }
	} else {
	    $win.infoframe.tablewindow.table tag row {} $tableLine
	    $win.infoframe.tablewindow.table set row $tableLine,3 [list {} {}]
	}
	# Update the channel value.

	set channels($name) [lreplace $channels($name) 0 0 $value]

    }
    # get name
    #        Gets the channel array information about the requested channel.
    #        If the channel is not in the channel array [list] is returned
    #        (an empty list).
    # Parameters:
    #    name   - Name of the channel to get information about.
    #
    method get name {
	if {[array names channels $name] ne ""} {
	    return $channels($name)
	} else {
	    return [list]
	}
    }
    # remove name
    #        Removes an alarm from the table.
    #        If the alarm is in the table, this is done by moving it to the bottom of the
    #        table and deleting its row... the channels array element is also unset.
    #        It is a no-op to remove a channel that is not in the table.
    # Parameters:
    #    name  - Name of channel to remove.
    method remove name {
	if {[array names channels $name] ne ""} {
	    set row [lindex $channels($name) 5]
	    $self moveToBottom $row
	    $win.infoframe.tablewindow.table delete rows end
	    unset channels($name)
	}
    }
    # dispatchAlarmScript option alarm
    #       Dispatches an alarm processing script
    #       The script is first processed by
    #       substituting $win for %W and
    #       the alarm name for %A
    # Parameters:
    #    option  - the option that names the script e.g. -alarmackcommand
    #    alarm   - The channel that was being processed.
    #
    method dispatchAlarmScript {option alarm} {
	set script $options($option)
	if {$script ne ""} {
	    set script [regsub -all "%W" $script $win]
	    set script [regsub -all "%A" $script $alarm]
	    eval $script
	}
    }
    # moveToBottom row
    #       Move a row of the table to the bottom.
    #  Parameters:
    #    row   - The row to move.
    #
    method moveToBottom row {
	set table $win.infoframe.tablewindow.table
	set thisrow $row
	set rows [$table cget -rows]
	incr rows -1;                   # Number of last row.
	for {set nextrow [expr $row + 1]} {$thisrow != $rows} {incr nextrow} {
	    $self swapRows $thisrow $nextrow
	    set thisrow $nextrow
	}


    }

    # ackAlarm
    #       Called in response to the acknowlege alarm button.
    #       unless the selected row is in the newalarm state,
    #       this does nothing.  If the selected alarm is in the
    #       new alarm state:
    #       - It is tagged with ackedalarm to indicate the
    #         alarm is acknowledged.
    #       - It is moved down to the bottom row to get it out of
    #         the way.
    # NOTE:
    #    1. Movement is done just by repeated swapping.   Not efficient,
    #       but pretty well gaurenteed to work
    #    2. The selection does not travel down with the acked row.
    #
    method ackAlarm {} {
	#  No selected row

	if {$selectedRow eq {}} {
	    return
	}
	set table $win.infoframe.tablewindow.table
	set channel [$table get $selectedRow,0]

	# Only do anything if the selected row is tagged as a new
	# alarm.
	#
	if {[$table tag includes newalarm $selectedRow,0]} {
	    $table tag row ackedalarm $selectedRow
	    $self moveToBottom $selectedRow
	    $self dispatchAlarmScript -alarmackcommand $channel
	}

    }
    # dismissAlarm
    #      Method to dismiss an alarm.  The selected alarm is removed from
    #      the table and fromt he channels array.  Note that it will just
    #      be back with a new start time if it's really still alarmed and
    #      the calling program knows what it's doing.
    #
    method dismissAlarm {} {
	if {$selectedRow eq ""} {
	    return
	}
	set table $win.infoframe.tablewindow.table
	set channel [$table get $selectedRow,0]

	unset channels($channel)
	$table delete rows $selectedRow 1
	$table reread
	set selectedRow [list]
	$self dispatchAlarmScript -alarmdismisscommand $channel
    }
    # onDismiss
    #      Called whenever the window dismiss button is clicked.
    #      we just call the -windowdismisscommand since it's
    #      responsible for doing any deletion.
    #      NOTE:
    #         This is a bit dirty...just call dispatchAlarmScript
    #         with an empty alarm name.
    #
    method onDismiss {} {
	$self dispatchAlarmScript -windowdismisscommand [list]
    }

    #
    # setSelection widget row
    #      Select 'row' in the widget.
    #
    proc setSelection {widget row} {
	$widget selection clear 0,0 end

	# Don't remove this after... evidently on the
	# click release, my selection gets released
	# so delaying some ms before setting the selection
	# appears to work around this and is not too visible.
	#
	after 5 [list $widget selection set $row,0 $row,4]
    }
}
#------------------------------------------------------------------------------
# alarmConfiguration
#        This snit megawidget dialog (potentially modal) allows the user to
#        set the alarm conditions for a channel.
# Layout is:
#        +------------------------------------------------------+
#        |     Alarm configuration for:   channel               |
#        |     [ ]  Enable Alarms                               |
#        |     [ ]  Asymmetric acceptable range                 |
#        | Nominal: <      > + <        > [ ]% - <        > [ ]%|
#        |                                                      |
#        | Alarm Actions:      [ ] Add to alarm page            |
#        |                     [ ] Pop up alarm window          |
#        |                     [ ] Emit periodic beeps          |
#        |                     [ ] Color in table ...           |
#        +------------------------------------------------------+
#        |       [ Ok ]       [Cancel]                          |
#        +------------------------------------------------------+
#
# Options:
#      -channel     - Name of channel to be configured (in label)
#      -enable      - Boolean true to enable the alarms
#      -asymmetric  - Boolean true for asymmetric range
#      -nominal     - Nominal value of channel
#      -range       - One or two elements of range information
#                     %after an element that represents % of nominal
#                     single element implies a symmetric range and will
#                     turn off -asymmetric.
#      -actions     - list of actions:
#                       page     - lights add to alarm page.
#                       popup    - Lights popup alarm window.
#                       beep     - Lights periodic beep
#                       color=x  - lights Color in table.. and sets its fg to x
#      -command     - Script invoked on Ok click
#      -cancelcommand
#                   - Script invoked on Cancel click.
# script substitutions
#       %W          - The widget.
#       %C          - the channel name.
# Widget commands:
#      modal        - Makes the dialog modal.
#
snit::widget alarmConfiguration {
    hulltype toplevel
    option   -channel         {}
    option   -enable          0
    option   -asymmetric      0
    option   -nominal
    option   -range
    option   -actions        {}
    option   -command        {}
    option   -cancelcommand  {}

    variable range1
    variable range1IsPct
    variable range2
    variable range2IsPct

    variable page             0
    variable popup            0
    variable beep             0
    variable color            0
    variable colorvalue       {red}

    constructor args {

	# Build the gui and lay it out:

	set work [frame $win.work -relief groove -borderwidth 3]

	label $work.label        -text {Alarm configuration for: }
	checkbutton $work.enable -text {Enable Alarms} -variable [myvar options(-enable)]
	checkbutton $work.asym   -text {Asymmetric acceptable range} \
	                         -variable [myvar options(-asymmetric)] \
				 -command [mymethod onAsymmetricToggle]
	label       $work.nominallbl -text {Nominal:}
	entry       $work.nominal    -textvariable [myvar options(-nominal)]
	label       $work.plus       -text {+}
	entry       $work.range1     -textvariable [myvar range1]
	checkbutton $work.pct1       -variable [myvar range1IsPct] -text {%}
	label       $work.minus      -text {-}
	entry       $work.range2     -textvariable [myvar range2]
	checkbutton $work.pct2       -variable [myvar range2IsPct] -text {%}

	label       $work.actions    -text {Alarm Actions:}
	checkbutton $work.page       -text {Add to alarm page}  \
	                             -variable [myvar page]
	checkbutton $work.popup      -text {Pop up alarm window} \
	                             -variable [myvar popup]
	checkbutton $work.beep       -text {Emit periodic beeps} \
	                             -variable [myvar beep]
	checkbutton $work.color      -text {Color in table...} \
	                             -variable [myvar color] \
				     -command  [mymethod onChooseColor]

	set action [frame $win.action]
	button $action.ok     -text Ok     -command [mymethod onOk]
	button $action.cancel -text Cancel -command [mymethod onCancel]

	grid $work.label    -  - - - - - -
	grid x x $work.enable    - - - - -  -sticky w
	grid x x $work.asym      - - - - -  -sticky w
	grid $work.nominallbl $work.nominal $work.plus    $work.range1  \
	     $work.pct1       $work.minus   $work.range2  $work.pct2
	grid $work.actions  -  $work.page  - - -   -sticky w
	grid     x x $work.popup - - -   -sticky w
	grid     x x $work.beep  - - -   -sticky w
	grid     x x $work.color - - -   -sticky w

	pack $work -side top -fill x -expand 1

	grid  x $action.ok  x $action.cancel
	pack $action -side top -fill x -expand 1
	# Configure to match the args:

	$self configure -asymmetric 0
	$self configurelist $args
    }
    ##### Configuration handlers.

    # configure -channel name
    #       Provides a channel name for the
    #       top label
    #
    onconfigure -channel name {
	set options(-channel) $name
	$win.work.label configure -text [list Alarm configuration for $name:]
    }
    #  configure -asymmetric bool
    #       If bool is true, the range is configured assymetric.
    #       Otherwise, the second range and percent are ghosted,
    #       plus is turned into +/- and minus is turned into blank.
    #
    onconfigure -asymmetric value {
	set options(-asymmetric) $value
	    $win.work.plus       configure -text {+}
	    $win.work.minus      configure -text {-}
	    $win.work.range2     configure -state normal
	    $win.work.pct2       configure -state normal
	if {$value} {
	} else {
	    $win.work.plus        configure -text {+/-}
	    $win.work.minus       configure -text {}
	    $win.work.range2      configure -state disable
	    $win.work.pct2        configure -state disable
	}
    }
    #   configure -range range
    #         configures the range of the nominal.. this will
    #         also automatically do the appropriate configure -assymetric.
    #         range can be a one or 2 element list. One element implies
    #         symmetric while 2 elements asymmetric.  Each element can be
    #         a pure number or a number followed by a % indicating it
    #         means a percentage of the nominal value.
    #
    onconfigure -range range {

	$self setRangeElement 1 [lindex $range 0]
	if {[llength $range] == 2} {
	    $self configure -asymmetric 1
	    $self setRangeElement 2 [lindex $range 1]

	} else {
	    $self configure -asymmetric 0
	}
    }
    # configure -actions list
    #       Configures the -actions.  The actions are a list
    #       see the options part of the comment header.
    # Parameters:
    #   list   - The action list.
    #
    onconfigure -actions list {
	# First reset all the actions.
	# Then set the ones specified

	foreach var {page popup beep color} {
	    set $var 0
	}


	$win.work.color configure -background [$win.work cget -background]
	foreach item $list {
	    switch -glob -- $item {
		popup {
		    set popup 1
		}
		color* {
		    set color 1
		    set info [split $item =]
		    set colorvalue [lindex $info 1]
		    $win.work.color configure -background $colorvalue
		}
		page {
		    set page 1
		}
		beep {
		    set beep 1
		}
	    }
	}
    }
    ######
    #  cget handlers.
    #

    # cget -range
    #     Return the current range specification.
    #     This will be one element if the range is symmetric and
    #     Two if asymmetric.  The values in the list could end with
    #     a % charcter indicating that they represente percentages
    #     of the nominal value rather than +/- values.
    #
    oncget -range {
	if {$range1IsPct} {
	    set pct {%}
	} else {
	    set pct {}
	}
	append value1 $range1 $pct
	if {$options(-asymmetric)} {
	    if {$range2IsPct} {
		set pct {%}
	    } else {
		set pct {}
	    }
	    append value2 $range2 $pct
	    return [list $value1 $value2]
	} else {
	    return [list $value1]
	}
    }
    # cget -actions
    #       Retrieve the set of alarm actions currently authorized for
    #       this channel.  This will be a list in the same form as could
    #       be supplied to configure -actions, and is derived from the
    #       state of the action checkbuttons as well as the color of
    #       the color one if applicable.
    #
    oncget -actions {
	set result [list]
	if {$page} {
	    lappend result {page}
	}
	if {$popup} {
	    lappend result {popup}
	}
	if {$beep} {
	    lappend result {beep}
	}
	if {$color} {
	    lappend result "color=$colorvalue"
	}
	return $result
    }
    ######
    #  Button event handlers?
    #

    # onAsymmetricToggle
    #     Called when the asymmetric checkbutton is toggled.
    #     We just need to configure ourself to the correct state.
    #
    method onAsymmetricToggle {} {
	$self configure -asymmetric $options(-asymmetric)
    }
    # onChooseColor
    #     Called when the color alarm action toggle button is clicked.
    #     if color is enabled, the color chooser dialog is brought up
    #     to allow the user to choose the color.
    #
    method onChooseColor {} {
	if {!$color} {
	    return
	}
	set newcolor [tk_chooseColor -initialcolor $colorvalue \
			             -parent       $win        \
			             -title {Choose alarm color}]
	if {$newcolor ne ""} {
	    set colorvalue $newcolor
	    $win.work.color configure -background $newcolor
	}
    }
    # onCancel
    #     Called in response to the Cancel button being clicked.
    #     If there'a a -cancelcommand it gets dispatched.
    #     If the dialog is modal, the hidden window the modality
    #     is waiting on is destroyed.
    #
    #
    method onCancel {} {
	$self dispatchScript -cancelcommand
    }
    # onOk
    #     Called in response to the Ok button being clicked.
    #     If there's a -command script it gets dispatched.
    #     If the dialog is modal the hidden window is destroyed.
    #
    method onOk {} {
	$self dispatchScript -command
    }

    ######
    #  regular methods.

    # dispatchScript selector
    #     Dispatches a callback script.  Substitutions are done
    #     prior to the dispatch... See the header comments
    # Parameters:
    #  selector   - name of the option used to establish the
    #               callback script.
    #
    method dispatchScript selector {
	set script $options($selector)
	if {$script ne ""} {
	    regsub -all {%W} $script $win script
	    regsub -all {%C} $script $options(-channel) script
	    eval $script
	}
    }

    # setRangeElement item value
    #       Configures gui elements for a range element.
    # Parameters:
    #   item   - Range element number 1 or 2
    #   value  - Range value could be value  or value%
    #
    method setRangeElement {item value} {
	append pctname range $item IsPct
	if {[string index $value end] eq "%"} {
	    set range$item [string range $value 0 end-1]
	    set $pctname 1

	} else {
	    set $pctname 0
	    set range$item $value
	}
    }
    # modal
    #     Turn the dialog into a modal one.
    #
    method modal {} {
	wm deiconify $win
	frame $win.hidden
	focus $win
	after 100 [list grab  $win]

	tkwait window $win.hidden
    }

}
