#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#
package provide runSequencer 1.0
package require Tk
package require Tktable
package require controller
package require ReadoutControl

puts "---------------------- Sequencer read in --------------"

#  The following are configurable items that can be overridden
#  via environment variables of the same name as the
#  config array index

set config(SEQCONFIGDIR)  [pwd]
set config(SEQCONFIGFILE) sequencer.config
set config(SEQACTIONFILE) sequencerActions.tcl


# Where we are in the run plan.

set ::runStep 1

# Columns and actions
# The columns are stored as lists in an array indexed by column number.
# The array is named columnDefinitions
# The list elements are:
#    column_label set_action initialize_action
# Where:
#  column_label    - is  label that appears at the top of the column.
#  set_action      - is a tcl command/script to invoke to set the item
#                    represented by a column to a value in a row.
#                   the set_action is invoked:  $set_action $column_label $value
# initialize_action- is a Tcl command/script to invoke to perform any initialization
#                    needed to initialize access to the item represented by the column.
#                    This is invoked:
#                           $initialize_action $column_label
#
set columnCount 0
#set columnInfo

#  Disable changes to the run plan:

proc disableChanges {table} {
    $table configure -state disabled
    set top [winfo toplevel $table]
    set menu $top.menu.file;		#  Maybe we could 'find this'.
    set last [$menu index end]
    for {set entry 0} {$entry <= $last} {incr entry} {
	catch {$menu entrycget $entry -label} label
	puts "Label: $label"
	if {$label eq "Clear..."} {
	    $menu entryconfigure $entry -state disabled
	}
    }
}


#  Enable changes to the run plan:

proc enableChanges {table} {
    $table configure -state normal
    set top [winfo toplevel $table]
    set menu $top.menu.file;		#  Maybe we could 'find this'.
    set last [$menu index end]
    for {set entry 0} {$entry <= $last} {incr entry} {
	catch {$menu entrycget $entry -label} label;   # could be separator.
	if {$label eq "Clear..."} {
	    $menu entryconfigure $entry -state normal
	}
    }
}

#  Read the configuration file.
#  The configuration file is just a file containing the lists to load into
#  columnInfo
#    blank lines and lines whose first non-whitespace character is #
#    are ignored...as are additional line field elements.
#
proc readConfiguration {} {
    global config
    global columnCount
    global columnInfo
    
    set configFile [file join $config(SEQCONFIGDIR) $config(SEQCONFIGFILE)]
    set fd [open $configFile "r"]
    while {![eof $fd]} {
        set line [gets $fd]
        set line [string trim $line]
        if {($line eq "") || ([string index $line 0] eq "#")} {
            # Ignore comment line.
        } else {
            set columnName [lindex $line 0]
            set setAction  [lindex $line 1]
            set initAction [lindex $line 2]
            if {$initAction ne ""} {
                $initAction $columnName
            }
            set columnInfo($columnCount) [list $columnName $setAction $initAction]
            incr columnCount
        }
    }
    close $fd
    
}


#
#   Adjust the configuration parameters based on
#   any existing env variables:
#
proc configDefaults {} {
    global config
    global env

    foreach item [array names config] {
        if {[array names env $item] ne ""} {
            set config($item) $env($item)
        }
    }
}

# Key binding function for moving to the next cell.
# next cell is the one to the right .. wrapping
# and then, if necessary creating a new row.
#
proc nextCell {table} {
    # the table index active command fails if no
    # cell is active. I don't see how that can be but
    # we'll be defensive.
    #
    if {[catch {$table index active} current]} {
        return
    }
    scan $current "%d,%d" row col
    incr col
    if {$col >= [$table cget -cols]} {
        set col 0
        incr row
        if {$row >= [$table cget -rows]} {
            $table insert rows end
        }
    }
    $table activate $row,$col
    $table see $row,$col
    $table reread
}
#
#  Key binding proc for moving to the previous
#  cell.  The prior cell is the one to the left
#  wrapping upwards if needed.  At the top,
#  bing the bell.
#
proc priorCell {table} {
    if {[catch {$table index active} current]} {
        return
    }
    scan $current "%d,%d" row col
    if {($row == 1) && ($col == 0)} {
        bell
        return
    }
    incr col -1
    if {$col < 0} {
        incr row -1
        set col [$table cget -cols]
        incr col -1
    }
    $table activate $row,$col
    $table see      $row,$col
    $table reread

}

# clearPlan table
#    Clear the current run plan from the table:
#
proc clearPlan table {
    global runPlan

    set rows [$table cget -rows]
    set cols [$table cget -cols]

    for {set row 1} {$row < $rows} {incr row} {
        for {set col 0} {$col < $cols} {incr col} {
            set runPlan($row,$col) ""
        }
    }
}

# build a tab separated row from a table row:
# If the first cell is "" returns a blank
proc buildRowLine {table row} {
    set cols [$table cget -cols]
    set values [$table get $row,0 $row,$cols]

    if {[lindex $values 0] eq ""} {
        return [list]
    };

    set result ""
    foreach value $values {
        append result "$value\t"
    }
    return $result
}


# savePlan - Saves the current run plan to file.  the table contents are
#            saved (title row is prefixed with  a # to make it comment)
#            until the first blank cell in column 0.
#
proc savePlan table {
    set filename [tk_getSaveFile                   \
                    -title {Choose save filename}  \
                    -defaultextension .plan        \
                    -filetypes {{{Plan files}  .plan } \
                                {{Text files}  .txt}   \
                                {{All files}   *}}]
    if {$filename eq ""} return

    #  Have a filename, now attempt to open for write

    if {[catch {open $filename w} fd]} {
        tk_messageBox -icon error -title "Can't open file"  \
            -message "Unable to open $filename : $fd check that you have write access to the directory"
        return
    }
    # write out the title row:
    set line [buildRowLine $table 0]
    puts $fd "# $line"
    set rows [$table cget -rows]
    for {set r 1} {$r < $rows} {incr r} {
        set line [buildRowLine $table $r]
        if {$line ne ""} {
            puts $fd $line
        } else {
            break
        }
    }
    close $fd
}

# readPlan table
#   Read a plan file into the table.
#   Note that once the user has accepted a plan file we clear the table
#   before doing the read:
#
proc readPlan table {
    global runPlan
    set filename [tk_getOpenFile -title "Select plan file" \
                    -defaultextension .plan                 \
                    -filetypes {{{Plan Files} .plan}        \
                                {{Text Files}  .txt}        \
                                {{All Files}     *}}]
    if {$filename eq ""} {
        return
    }
    set toplevel [winfo toplevel $table]
    wm title $toplevel $filename
    # Ensure we can open the file:

    if {[catch {open $filename r} fd]} {
        tk_messageBox -title {Can't open file for read}  \
            -icon error                                 \
            -message "Can't open $filename for read : $fd - be sure the file exists and is readable"
        return
    }
    #
    # Clear the old plan and read the new:
    #
    clearPlan $table
    set plan [read $fd]
    close $fd

    set planLines [split $plan "\n"]
    #
    # A good approximation to the table size required is the the # of lines
    # in the plan file... note that this will overestimate as it will count
    # comments.
    #
    set rows [$table cget -rows]
    incr rows -1;               # Can't count the title row.
    set fileLines [llength $planLines]
    if {$fileLines >= $rows} {
        set diff [expr $fileLines - $rows + 1]
        $table insert rows end $diff
    }
    # Now that the table is big enough we can start
    # processing lines from the file:

    set row 1
    foreach line $planLines {
        set firstele [lindex $line 0]
        #
        #  Only process non comment lines:
        #
        if {[string range $firstele 0 0] ne "#"} {
            set col 0
            foreach item $line {
                set runPlan($row,$col) $item
                incr col
            }
            incr row
        }
    }

}
#------------------------------------------------------------------------
#
#  Start  the next plan step.
#
proc nextStep {button table} {
    global runStep
    global columnCount
    global columnInfo


    set priorStep $runStep
    incr runStep
    set rows [$table cget -rows]
    set cols [$table cget -cols]

    if {$runStep >= $rows}  {
        $table tag delete current
        $button configure -text "Execute Plan" \
            -command [list executePlan $button $table]
        enableChanges $table
        tk_messageBox -icon info                        \
            -message {Run plan has ended}               \
            -title   {plan status}                      \
            -type    ok
        
        set ::seqController::plannedRun 0;#  runs started are not planned.
        return
    }
    # Extract the run settings... and also exit if the first
    # one is empty
    #
    set settings [$table get $runStep,0 $runStep,$cols]
    if {[lindex $settings 0] eq ""} {
        $table tag delete current
        $button configure -text "Execute Plan" \
            -command [list executePlan $button $table]
        enableChanges $table
        tk_messageBox -icon info                        \
            -message {Run plan has ended}               \
            -title   {plan status}                      \
            -type    ok
        set ::seqController::plannedRun 0;# runs started are not planned.
        return
    }
    # Ok remove the highlight from the prior row, and highlight
    # the current row:

    $table clear tags  $priorStep,0 $priorStep,$cols
    $table tag row current $runStep

    # now do the actions needed to setup for the run:
    
    for {set i 0} {$i < $columnCount} {incr i} {
        set setting [lindex $settings $i]
        set name    [lindex $columnInfo($i) 0]
        set action  [lindex $columnInfo($i) 1]
        if {$action ne ""} {
            if {[$action $name $setting]} {
                # 
                # Action returning nonzero is an error
                #
                planAborted
                $table tag delete current
        
                enableChanges $table
                $button configure -text "Execute Plan" \
                    -command [list executePlan $button $table]		
                return
            }
        }
    }
    # Run might have gotten aborted:

    if {$runStep < $rows} {
        startPlannedRun  $button $table
    } else {
        $table tag delete current
        enableChanges $table
        $button configure -text "Execute Plan" \
            -command [list executePlan $button $table]

    }
}
#
# Execute the run plan
#
proc executePlan {button table} {
    global runStep
    set runStep 0

    set fg [$table cget -foreground]
    set bg [$table cget -background]

    # Current step is in reverse video.

    $table tag configure current -foreground $bg -background $fg

    # Give the application some control here:

    startingPlan

    # Start the step.


    $button configure -text "Abort Plan" \
        -command [list abortPlan $button $table]
    disableChanges $table
    nextStep $button $table
}

# Stop execution of a run plan:
#
proc abortPlan {button table {stop 1}} {
    global runStep
    set runStep 10000000;   # so that when/if nextStep is called its over.
    if {$stop && ([ReadoutControl::getState] eq "Active")} {
        stopPlannedRun
    }
    planAborted

    $table tag delete current
    
    enableChanges $table

    $button configure -text "Execute Plan" \
        -command [list executePlan $button $table]

    set ::seqController::plannedRun 0
}
#-------------------------------------------------------------------------
#
#  Create the main gui.  This is just a simple menu with File and
#  Help pull downs a tktable that takes up most of the tube and
#  a button that starts the execution of the run sequence.
#
#  Note that the top parameter must be a toplevel widget.
proc setupGui {{top .}} {
    global tcl_platform
    global runPlan
    global columnInfo
    global columnCount

    if {$top eq "."} {
        set mtop ""
    } else {
        set mtop $top
    }


    # Menu structure:

    menu $mtop.menu -tearoff 0
    set fileMenu [menu $mtop.menu.file -tearoff 0]
   # set helpMenu [menu $mtop.menu.help -tearoff 0]
    $mtop.menu add cascade -label File -menu $mtop.menu.file
    $mtop.menu add cascade -label Help -menu $mtop.menu.help

    $top configure -menu $mtop.menu

    $fileMenu add command -label "Open..." \
        -command [list readPlan $mtop.tblframe.tbl]
    $fileMenu add command -label "Save..." \
        -command [list savePlan $mtop.tblframe.tbl]
    $fileMenu add separator
    $fileMenu add command -label "Clear..." \
        -command [list clearPlan $mtop.tblframe.tbl]

   # $helpMenu add command -label "About..."
   # $helpMenu add command -label "Topics..."

    # The top part of the widget is a table in a frame with a scrollbar.

    frame $mtop.tblframe -relief groove -borderwidth 3 \
                -takefocus 0
    set table [table $mtop.tblframe.tbl \
                    -cols $columnCount  \
                    -rows 40           \
                    -variable runPlan \
                    -cache 1           \
                    -height 15         \
                    -sparsearray 0     \
                    -titlerows   1     \
                    -colstretchmode all \
                    -colwidth     15    \
                    -yscrollcommand [list $mtop.tblframe.sb set]
                    ]

    set sb [scrollbar $mtop.tblframe.sb -orient vertical \
                    -command [list $table yview] -takefocus 0]
    grid $table -row 0 -column 0 -sticky nesw
    grid $sb    -row 0 -column 1 -sticky nse
    pack $mtop.tblframe -fill both -expand 1

    # fill in the header:
    #
    for {set col 0} {$col < $columnCount} {incr col} {
        set runPlan(0,$col) [lindex $columnInfo($col) 0]
    }

    # Bindings on the table so that <Return> and <Tab> go to the next
    # table cell and Shift-Tab go to the previous cell (arrows are
    # already part of the default bindings:
    #  Note that left-tab is something I'm only sure I know how to do on
    # linux.
    #

    bind $table <Return> [list nextCell $table]
    bind $table <Tab>    [list nextCell $table]
    if {$tcl_platform(os) eq "Linux"} {
        bind $table <ISO_Left_Tab> [list priorCell $table]
    }


    # the control buttons:

    button $mtop.run -text {Execute Plan} -takefocus 0 -command [list executePlan $mtop.run $table]
    pack   $mtop.run -side bottom -anchor s
}

# Set up our configuration.

configDefaults
source [file join $config(SEQCONFIGDIR) $config(SEQACTIONFILE)]

readConfiguration


toplevel .sequencer
setupGui .sequencer

