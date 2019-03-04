#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#      NSCLDAQ Development Group
#
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

# This software is Copyright by the Board of Trustees of Michigan
# State University (c) Copyright 2297884. 

# @file  OutputPipeline.tcl 
# @author Jeromy Tompkins 

package provide OfflineEVBOutputPipeline 11.0

package require snit
package require InstallRoot 
package require ring
package require DAQParameters 
package require eventLogBundle
package require ui

## @brief Parameters to configure the eventLogBundle
# 
# This is a very simple object that consists mainly of a bunch of options and
# some validation methods. The parameters control the different settings that
# can be specified to the eventLogBundle state machine callouts package. It is
# also the same set of parameters that can be used to configure an output
# pipeline that consists of just an eventlog process running as a pipeline. 
#
# One unique aspect of this is that the eventLogBundle expects that the ring
# name is a complete URI rather than just the name of the ring.
snit::type OfflineEVBOutputPipeParams {
  ## @brief URI for the ring to attach to
  option -ringname     -default "tcp://localhost/OfflineEVBOut"

  ## @brief prefix of the run file that will be written to disk
  option -prefix       -default "reordered"
  
  ## @brief path to the directory to treat as a stagearea
  option -stagearea    -default [file join $::env(HOME) offlinestagearea]

  option -nsources     -default "2" ;#< number of sources in data stream
  option -logger       -default ""  ;#< path to the eventlog binary

  ## @brief whether to provide --number-of-sources switch
  option -usensrcs     -default 1  

  ## @brief whether to set override the run number with a new one
  option -forcerun     -default 0

  ## @brief whether to compute an sha512 checksum while writing to disk
  option -usechecksum  -default 1 


  ## @brief The constructor
  # 
  # Sets all of the option values.
  #
  # @param args   option-value pairs
  constructor {args} {
    set options(-logger) [DAQParameters::getEventLogger]
    $self configurelist $args   

  }

  ## @brief Check to see if the the various options are okay
  #
  # This passes a list around to a number of validation methods and those fill
  # that list with any error message they find. The final list contains all of
  # the error conditions that were observed.
  #
  # @returns a list of error messages
  method validate {} {
    set errors [list]
    $self validateRing errors
    $self validatePrefix errors
    $self validateStagearea errors
    $self validateNSources errors
    $self validateLogger errors

    return $errors 
  }

  ## @brief Check that ring name is a valid URI
  #
  # A good ring name will follow the tcp://hostname/ring format. IF the ring is
  # good, then it is important that the ring also exists.
  # 
  # @param errors_  the variable name of a list to append errors to
  #
  method validateRing {errors_} {
    upvar $errors_ errors
    set pattern {^(\w+://)([/]*[\w\.]+)(/[\w\.]+)*$}
    if {[regexp $pattern $options(-ringname) match proto host ring]} {
      if {![info exists ring]} {
        lappend errors "Ring name not specified as a valid proto://host/ring"
      }

      set ring [string trimleft $ring "/"]
      if {![file exists [file join /dev shm $ring]]} {
        lappend errors "Ring \"$ring\" does not exist on localhost." 
      }
    } else {
      lappend errors "Ring name \"$options(-ringname)\" is not formed as proto://host/ring"
    }
  }

  ## @brief Check the user supplied an prefix
  #
  # This only cares that the user supplied something other than an empty string.
  #
  # @param errors_  the variable name of a list to append errors to
  #
  method validatePrefix {errors_} {
    upvar $errors_ errors
      if {$options(-prefix) eq ""} {
      lappend errors "Run file prefix must be a string of nonzero length."
    }
  }


  ## @brief Ensure the stagearea has the proper format
  #
  # This cares the proper directory structure exists and that the proper
  # ownership is attributed to the directories.
  #
  # @param errors_  the variable name of a list to append errors to
  #
  method validateStagearea {errors_} {
    upvar $errors_ errors
    set dir $options(-stagearea)
    if {![file exists $dir]} {
      lappend errors "Stagearea must be a directory that exists."
    } else {
      set subdir [file join $dir experiment]
      if { (![file exists $subdir]) || (![file owned $subdir]) } {
        lappend errors "Stagearea must contain writable experiment directory"
      } 
      set subdir [file join $dir experiment current]
      if { (![file exists $subdir]) || (![file writable $subdir]) } {
        lappend errors "Stagearea must contain writable experiment/current directory"
      } 
      set subdir [file join $dir complete]
      if { (![file exists $subdir]) || (![file owned $subdir]) } {
        lappend errors "Stagearea must contain writable complete directory"
      }
    }
  }



  ## @brief Ensure that -nsources is non-negative
  #
  # @param errors_  the variable name of a list to append errors to
  # 
  method validateNSources {errors_} {
    upvar $errors_ errors
    if {$options(-nsources) < 0} {
      lappend errors "Number of sources for eventlog must be greater than or equal zero."
    }
  }

  ## @brief Ensure that the file exists
  #
  # It doesn't check anything more than file existence.
  #
  # @param errors_  the variable name of a list to append errors to
  #
  method validateLogger {errors_} {
    upvar $errors_ errors
    if {![file exists $options(-logger)]} {
      lappend errors "No eventlog program exists at \"$options(-logger)\""
    }
  }


  ## @brief Create a copy of this object
  #
  # @returns a new OfflineEVBOutputPipeParams object with the same option values
  method clone {} {
    
    # get all of the options and their values and make a dict of them
    set state [dict create]
    foreach opt [$self info options] {
      set value [$self cget $opt]
      dict set state $opt $value 
    }

    # return a new snit object with the same params
    return [[$self info type] %AUTO% {*}$state]
     
  }


}

# --------------------------------------------------------------
# Redefine some of the ReadoutGUIPanel information

namespace eval ReadoutGUIPanel {
  variable run

  proc recordData {} { 
    return 1
  }

  proc setRun {number} {
    variable run
    set run $number
  }

  proc getRun {} {
    variable run 
    return $run 
  }

  proc incrRun {} {
  }

  proc normalColors {} {}

  proc isRecording {} {}

  proc getTitle {} {return "Offline run"}

  proc Log {source class msg} {
    puts "$source $class $msg"
  }

  proc outputText {msg} {puts $msg}
}


# --------------------------------------------------------------
# Make sure that the StatusBar instance exists because if it doesn't
# all will break!
#
#set msg ""
#set ret [catch {::StatusBar::getInstance .statBar} msg]
#puts "$ret : $msg"

namespace eval ::StatusBar {
  variable theInstance
}

StatusArea .statBar
set ::StatusBar::theInstance .statBar

#-------------------------------------------------------------------------------
################################################################################
################################################################################
################################################################################
################################################################################
################################################################################
#-------------------------------------------------------------------------------


## @brief A class to manage a pipeline running the eventlog program
#
# This is not used in the Offline Orderer but it is still a functionaly piece of
# code and is therefore not thrown out. It is able to launch a pipeline and
# output the data read from it to stdout.
#
snit::type OfflineEVBOutputPipeline {
  option -daqroot   -default {::InstallRoot::Where}

  variable fd -1         ;#< the file descriptor
  variable running false ;#< whether the program is still running

  ## @brief Constructor
  constructor {args} {
    $self configurelist $args
  }

  ## @brief Shutdown the pipeline
  #
  destructor {
    if {[$self getPipeFD]>0} {
      catch {close [$self getPipeFD]} msg 
      $self setPipeFD -1
      $self setRunning false
    }
  }

  ## @brief Launch the pipeline using the parameters passed in
  #
  method launch params {
    if {[catch {ringbuffer usage [$params cget -ringname]} msg]} {
      # failed to find the ring
      ringbuffer create [$params cget -ringname]
    }

    set command [$self createPipelineCommand $params]
      
    $self setPipeFD [open $command r]
    set theFD [$self getPipeFD]
    # update the running state
    $self setRunning true

    chan configure $theFD -blocking 0
    chan configure $theFD -buffering line 
    chan event $theFD readable [mymethod _handleReadable $theFD]
  }


  ## @brief Set the file descriptor handle
  #
  # @param newfd  a file descriptor handle
  method setPipeFD {newfd} {
    set fd $newfd
  }

  ## @brief Retrieve the file descriptor handle
  #
  #  @returns the file descriptor handle
  method getPipeFD {} {
    return $fd
  }

  ## @brief Set the flag to indicate that this is running
  # 
  # @param onoff  boolean to indicate whether this is running
  method setRunning {onoff} {
    set running $onoff 
  }

  ## @brief Check whether the pipeline is still running
  #
  # @return boolean value indicating if the pipeline is still running
  #
  method getRunning {} {
    return $running 
  }

  ## @brief Form the command that will be executed
  #
  method createPipelineCommand params {
    set ringname  [$params cget -ringname]
    set prefix    [$params cget -prefix]
    set dir       [$params cget -stagearea]
    set nsources  [$params cget -nsources]

    set daqbin [file join [InstallRoot::Where] bin]
    set command    "| $daqbin/eventlog --source=$ringname "
    append command "--prefix=$prefix "
    append command "--path=$dir "
    append command "--oneshot --number-of-sources=$nsources "
    append command "|& cat"

    return $command
  }

  ## @brief Callback for when the file descriptor is readable
  #
  method _handleReadable {fid} {

    if {[catch {gets $fid line} len] || [chan eof $fid]} {
      puts "Input pipeline has exited"
      chan event $fid readable "" 
      catch {close $fid}
      $self setPipeFD   -1
      $self setRunning false
    } elseif {$len>0} {
      puts $line
    }
  }

}
