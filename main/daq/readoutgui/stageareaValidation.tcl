
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Jeromy Tompkins and Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file StageareaValidation.tcl
# @brief Procs for checking cleaning and checking stagearea for recording
# @author Ron Fox <fox@nscl.msu.edu>
# @author Jeromy Tompkins <tompkins@nscl.msu.edu> 

package provide stageareaValidation 1.0

package require ReadoutGUIPanel

namespace eval ::StageareaValidation {}

#
# correctFixableProblems
#
#  Some startup issues can be corrected:
# 1. experiment/current/.started exists
# 2. experiment/current/.exited exists
#
#  In this case these files are just deleted.
#
proc ::StageareaValidation::correctFixableProblems {} {
  # check if experiment/current/.started exists
  if {[::StageareaValidation::_dotStartedExists]} {
    ::StageareaValidation::deleteStartFile

  } 
  
  # check if experiment/current/.exited exists
  if {[::StageareaValidation::_dotExitedExists]} {
    ::StageareaValidation::deleteExitFile
  }
  
  #  If there are .evt links in current run dir get rid of them too.
  
  puts "Getting rid of old event file links in current"
  set dir [::ExpFileSystem::getCurrentRunDir]
  set evtfiles [glob -nocomplain [file join $dir run*.evt]]
  foreach file $evtfiles {
    if {[catch {file readlink $file} value] == 0} {
      puts "deleting link $file -> $value (just the link)"
      file delete -force $file
    }
  }
    
}

##
# deleteStartFile
#   Kill off the .started file.
#
proc ::StageareaValidation::deleteStartFile {} {
    set startFile [file join [::ExpFileSystem::getCurrentRunDir] .started]
    file delete -force $startFile 
    
}
##
# deleteExitFile
#   Delete the .exited file
#
proc ::StageareaValidation::deleteExitFile {} {
    
    set exitFile [file join [::ExpFileSystem::getCurrentRunDir] .exited]
    file delete -force $exitFile 
    
}

##
# ::StageareaValidation::_duplicateRun
#
#  @return string -   
#  @retval non empty  if the run we are about to write already exists.
#          we're going to define 'exists' as having a run directory in the
#          experiment view. 
# @retval empty if there is no sign that this run had already been recorded.
#
proc ::StageareaValidation::_duplicateRun {} {
    
    set run [::ReadoutGUIPanel::getRun]
    
    # Two possibilities;  If the run was properly ended, there will be a
    # run directory in the experimnent view
    
    set runDirPath [::ExpFileSystem::getRunDir $run]
    
    # If the run was improperly ended, there could be event segments in the
    # current directory.  We'll look for them with glob.
    #
    
    set checkGlob [::ExpFileSystem::getCurrentRunDir]
    set checkGlob [file join $checkGlob [::ExpFileSystem::genEventfileBasename $run]*.evt ]
    
    set eventSegments [llength [glob -nocomplain $checkGlob]]

    #  Figure out the return value:

    if {[file exists $runDirPath]} {
      set message "The final run directory '$runDirPath' already exists indicating this run may already have been recorded"
    } elseif {($eventSegments > 0)} {
      set message "[::ExpFileSystem::getCurrentRunDir] has event segments in it for this run ($run) indicating this run may already have been recorded but not finalized"
    } else {
      set message ""
    }
    return $message

}

##
#   Check whether or not the .started file lives in the 
#   experiment/current directory
#
proc ::StageareaValidation::_dotStartedExists {} {
  set currentPath [::ExpFileSystem::getCurrentRunDir]
  return [file exists [file join $currentPath .started]]
}

##
#   Check whether or not the .exited file lives in the 
#   experiment/current directory
#
proc ::StageareaValidation::_dotExitedExists {} {
  set currentPath [::ExpFileSystem::getCurrentRunDir]
  return [file exists [file join $currentPath .exited]]
}

##
#   Check whether or not .evt files exist in the 
#   experiment/current directory
#
#   @returns boolean indicating whether there are any files ending in .evt
#
proc ::StageareaValidation::_runFilesExistInCurrent {} {
  set currentPath [::ExpFileSystem::getCurrentRunDir]
  set evtFiles [glob -directory $currentPath -nocomplain *.evt]
  return [expr {[llength $evtFiles] > 0} ]
}

##
# ::StageareaValidation::listIdentifiableProblems
#
# Checks for a few things:
# 1. experiment/run# directory already exists
# 2. experiment/current/*.evt files exist
#
# @returns a list of error messages
proc ::StageareaValidation::listIdentifiableProblems {} {

  set errors [list]

  # check if run directory exist!
  set msg [StageareaValidation::_duplicateRun]
  if {$msg ne ""} {
    lappend errors $msg
  } 
  
  # check if experiment/current/*.evt files exist
  if {[::StageareaValidation::_runFilesExistInCurrent]} {
    set msg    "StageareaValidation error: the experiment/current directory contains run "
    append msg "segments and needs to be cleaned."
    lappend errors $msg
  }

  return $errors
}

proc ::StageareaValidation::correctAndValidate {} {
  if {[::ReadoutGUIPanel::recordData]} {
    ::StageareaValidation::correctFixableProblems;         # Some things can be fixed :-)
    set errorMessages [::StageareaValidation::listIdentifiableProblems]
    if {[llength $errorMessages]>0} {
      return -code error $errorMessages
    }
  }
}
