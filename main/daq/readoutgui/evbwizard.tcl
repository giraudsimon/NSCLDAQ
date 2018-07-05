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

lappend auto_path [file join $::env(DAQROOT) TclLibs]


##
# @file EVBWizard.tcl
# @brief Main flow of control for the event builder wizard.
# @author Ron Fox <fox@nscl.msu.edu>
#
package require evbsourceprompt
package require guisourceprompt
package require evbparameters

#  Entry point :
#   - withdraw .
#   - pop up a message box with some orientationtext.
#   - Setup the prompting/completion of GUI data sources.
#

after 50 wm iconify .

#  A bit of dirt here to relabel the Ok button on the message box:

after 100  {.__tk__messagebox.ok configure -text {Let's start}}
tk_messageBox -type ok -title "Event builder wizard" -icon info \
    -parent . \
-message "Welcome to the event builder wizard.  This program will \
help you set up an experiment that uses the NSCL Event builder \n\
Experiments that use the event builder have data sources.  Unfortunately \
this term is used in two contexts.  First, data sources are controlled \
by the Readout GUI.  Second data sources feed event data from \
ring buffers into the NSCL event builder which, in turn sorts data \
by timestamp and build events from coincidences \n\
This program will let you set up both the Readout GUI's data sources \
(writing part of the .settings.tcl file that needs to be copied to \
~/stagearea) and the event builder (writing a skeletal ReadoutCallouts.tcl \
which needs to be placed where the Readout Gui will use it). \n
To start defining your experiment, click on the button below"


#  Let's start up with the GUI data sources.
#
#
tk_messageBox -type ok -icon info -title "GUI Info" \
-message "We're going to start by defining the Readout programs that are \
controlled by the Readout GUI.  Each of these is defined in terms of a \
provider, which determines how the control is done, and provider dependent \
parameters.   In the following little UI use the 'New...' button to add a \
program and 'Done' when you've added all the necessary sources"

toplevel .prompt
GetDataSources .prompt.get -newcommand [list newSourceCommand .prompt.get] \
                -donecommand [list saveSources .prompt.get]
pack .prompt.get
wm title .prompt "Provide GUI data sources"
##
# saveSources 
#   - Gets the data sources and saves them in ::guiDataSources
#   - Destroys the top level that's responsible for prompting
#   - Starts the process of prompting for the event builder sources.
#
# @param w - full widget path to the prompter widget -- which is
#            assumed to live just down one level from a top level.
#
proc saveSources w {
    set ::guiDataSources [$w cget -sources]
    set toplevel [winfo toplevel $w]
    destroy $toplevel
    
    #  set up the prompter for Event builders.
    
    tk_messageBox -type ok -icon info -title {EVB Sources} \
        -message "Next we need to define the data sources the event builder \
will use.  Each of these event sources gets data from a ring buffer for \
one or more 'source ids'.  Ring buffers are identified by the host in which \
the ring buffer producer runs and the name of the ringbuffer in that host \n\
The sources have a description which is displayed \
on the event builder GUI. Fill in the form describing a data source and \
use 'Add' to add a new data source.  Use \
'Done' to finish adding event sources."
    
    toplevel .prompt
    wm title .prompt "Provide Event builder sources"
    GetEvbSources .prompt.get -addcommand [list addEvbSource .prompt.get] \
        -donecommand [list saveEvbSources .prompt.get]
    pack .prompt.get
    
}
##
# saveEvbSources
#   - Saves the event builder source information in ::evbSources
#   - Destroys the top level the prompter is in.
#   - Starts the process of prompting about the event builder
#   - parameters.
# @param w - the widget that runs the prompter.
#
proc saveEvbSources w {
    set ::evbSources [$w cget -sources]
    set top [winfo toplevel $w]
    destroy $top
    
    #  Now we'll prompt for event builder parameters.  We can use
    #  The event builder settings GUi for that..but we're not going
    #  support setting intermediate rings because for the most part,
    #  nobody uses that.
    #
    
    tk_messageBox -type ok -icon info -title {EVB Parameters} \
        -message "Now we'll get the global event builder parameters \
These include the coincidence interval that defines which fragments get \
built in to an event, the policy used to assign a timestamp to an event \
Whether or not event building is done or only ordering.   We must also \
set the event buider output parameters which include the destination ring \
specification and the data source id of events the event builder generates \
(this is used by any later stages of a hierarchically built event"

    toplevel .prompt
    GetEvbParameters .prompt.get -command [list saveParams .prompt.get]
    pack .prompt.get
    
}

##
# saveParams
#    Called when the GetEvbParameters prompter ok button is clicked
#   - pull the glom/ring parameters into ::evbParameters
#   - kill the dialog.
#   - write settings.tcl.
#   - write ReadoutCallouts.tcl
#   - Exit.
#
# @param w - the event builder parameters widget.
proc saveParams w {
    set ::evbParameters [dict create]
    foreach key [list policy build dt ring record sourceid] {
        dict set ::evbParameters $key [$w cget -$key]
    }
    
    set top [winfo toplevel $w]
    destroy $top
    
    
    writeSettings
    writeReadoutCallouts
    
    tk_messageBox -icon info -title {Done} -type ok \
    -message "We have written two files in your current working directory \
settings.tcl contains settings for the ReadoutShell.  Either copy or append it \
to ~/stagearea/.settings.tcl - This defines the programs ReadoutShell controls \
ReadoutCallouts.tcl is a starting point for your ReadoutShell customizations \
if you use it as is, it will start the event builder and manage the data \
sources you specified.  Copy it either to the directory in which you'll \
run ReaedoutShell or ~/stagearea/experiment/current  If you already have \
a ReadoutCallouts.tcl you'll need to merge the code here into that."
    
    exit
}

##
# writeSettings
#    Writes the bits of a .settings.tcl file needed to define data sources
#    to ReaodutShell.  In order to make it visible, we write it as
#    settings.tcl not .settings.tcl  Normally appending it to the
#    .settings.tcl should make the changes needed.
#
proc writeSettings {} {
    set fd [open settings.tcl w]
    puts $fd "set dataSources {$::guiDataSources}"
    close $fd
}
##
# writeReadoutCallouts
#   Writes a bare minimum ReadoutCallouts.tcl to start the event builder
#   as described by ::evbParameters and ::evbSources
#
#
#
proc writeReadoutCallouts {} {
    set fd [open ReadoutCallouts.tcl w]
    
    puts $fd "package require evbcallouts"
    
    puts $fd EVBC::useEventBuilder
    
    #  On start initializes the event builder.
    #   -gui true, -restart, true;
    #   -glomdt from config, -glombuild from config
    #   -glomid from config, -destring from config
    #   -setdestringasevtlogsource from config but note that in 11.3
    #    the output ring is unconditionally the logging ring unless
    #    the configuration manager is used to modify that.
    
    puts $fd "proc OnStart {} {"
    puts $fd "    EVBC::initialize -gui true -restart true   \\"
    puts $fd "        -glomdt [dict get $::evbParameters dt] \\"
    puts $fd "        -glombuild [dict get $::evbParameters build] \\"
    puts $fd "        -glomid [dict get $::evbParameters sourceid] \\"
    puts $fd "        -destring [dict get $::evbParameters ring]   \\"
    puts $fd "        -glomtspolicy [dict get $::evbParameters policy] \\"
    puts $fd "        -setdestringasevtlogsource [dict get $::evbParameters record] "
    
    puts $fd "}"
    
    
    #  Register the event sources:
    
    foreach source $::evbSources {
        set ringURI tcp://[dict get $source host]/[dict get $source ring]
        puts $fd "::EVBC::registerRingSource $ringURI {}    \\"
        puts $fd "    {[dict get $source sids]}              \\"
        puts $fd "    {[dict get $source description]} 1 1 10 0"
    }
    
    
    close $fd
}
