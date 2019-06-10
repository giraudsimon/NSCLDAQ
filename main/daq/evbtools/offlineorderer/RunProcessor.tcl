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

# @file  RunProcessor.tcl 
# @author Jeromy Tompkins 
#!/usr/bin/env tclsh


package provide OfflineEVBRunProcessor 11.0

package require snit
package require OfflineEVBJobProcessor
package require OfflineEVBOutputPipeline
package require evbcallouts
package require RunstateMachine 

package require Thread

## @class RunProcessor
#
# @brief Manages the jobs in the list of jobs to be processed
#
# This maintains a JobProcessor instance that does the real heavy lifting.
# The purpose of this class really is to manage the list of jobs that need
# to be processed and iterate through them. For each job, the RunProcessor
# will configure the JobProcessor and set it into motion. At the same time,
# it maintains a list of observers that receive information about the 
# status of the job processing. These observers are alerted when a new
# run is begun, when a new job is started, when the run completes, and when
# run fails. 
#
# The RunProcessor handles a list of jobs in the form of a dict. The dict is 
# composed of keys reflecting the name of each job and those keys map to another
# dict whose keys and values are :
#   --inputparams         OfflineEVBInputPipeParams
#   --hoistparams         OfflineEVBHoistPipeParams
#   --evbparams           EVBC::AppOptions 
#   --outputparams        OfflineEVBOutputPipeParams
#
# The RunProcessor only knows how to start up jobs and has no idea how to determine
# whether the job has finished. It is actually the responsibility of the JobProcessor
# to tell the RunProcessor whether the job was completed. It then tells the RunProcessor
# to transition to the next job.
# 
# The JobProcessor actually runs on a separate thread of execution owned by this
# this. The management of that JobProcessor is made by sending it commands
# to the job processor via the thread::send command. There is no shared data
# between the main thread that the run processor lives in and the worker thread
# in which the JobProcessor runs.
#
snit::type RunProcessor {

  option -jobs  ;#< List of jobs that are to be processes. (it is really a 
                 #  dict whose keys are the job name and the values are the
                 # parameters)

  variable m_runObservers ;#< A list of observers for the job transitions
  variable m_completionStatus; #< Completion status
  
  component processor ;#< The JobProcessor that will do the work

  variable m_workerThread;

  ## @brief Construct the data members and configure options
  #
  constructor {args} {
    variable m_runObservers

    $self configurelist $args

#    set processor [JobProcessor %AUTO% -runprocessor $self]
    set m_workerThread [thread::create -joinable]
    thread::send $m_workerThread "lappend auto_path $::auto_path"
    thread::send $m_workerThread {package require OfflineEVBJobProcessor}
    thread::send $m_workerThread {wm withdraw .}
    set processor [thread::send $m_workerThread {set processor [JobProcessor %AUTO%]}]
    thread::send $m_workerThread "set _parentThread [thread::id]"
    thread::send $m_workerThread [list $processor configure -runprocessor $self]

    set m_runObservers [list]
  }

  ## Destructor
  #
  destructor {
    thread::join $m_workerThread
  }


  ## @brief Entry point for the actual processing
  # 
  # The caller of this function sets in motion the processing of all jobs in the
  # job list. Because Jobs take a while to process, this sets up the start of the
  # first job in the dict (if it exists).
  method run {} {

    # Tell the observers that a new run is beginning
    $self observeNewRun 

    # Start the first job in the list
    $self runNext

  }

  ## @brief Start processing the next job 
  #
  # If there is another job to process, then its parameters are passed to the JobProcessor
  # and the JobProcessor is set in motion. However, if the job processor
  #
  method runNext {} {

    # get the list of jobs
    set jobs [dict keys $options(-jobs)]

    # if we have a job still, then configure it and fire it off
    if {[llength $jobs]>0} {

      # get the first job name
      set job [lindex $jobs 0]

      $self observeNewJob $job

      # copy the job 
      set iparams [dict get $options(-jobs) $job -inputparams]
      if {[catch {set run [$self guessRunNumber [$iparams cget -file]]} msg]} {
        set resp [tk_messageBox -icon error -message "RunProcessor::run failed to identify the run number"]
        $self observeAbort
        return
      }
      thread::send $m_workerThread [list ReadoutGUIPanel::setRun $run ]


      $self configureJobProcessor [dict get $options(-jobs) $job]

#      # launch this thing but stop if it was aborted.
#      if {[catch {$processor run} msg]} {
#        tk_messageBox -icon error -message $msg
#        $self observeAbort
#        return
#      }
      thread::send -async $m_workerThread [list $processor run ] \
                        [myvar m_completionStatus]
      vwait [myvar m_completionStatus]

      if {$m_completionStatus ne "OK"} {
        $self observeAbort
        return
      }

      # remove the current processing job
      set options(-jobs) [dict remove $options(-jobs) $job]

      # call the next run
      $self runNext
    } else {
      $self observeCompleted
    }
  }

  ## @brief Access the job processor that this owns
  # 
  # @returns the job processor
  method getJobProcessor {} {
    return $processor
  }

  method getWorkerThread {} {
    return $m_workerThread
  }

  method pickle {snitobj} {
    set opts [$snitobj info options]

    set state [dict create]
    foreach opt $opts {
      set value [$snitobj cget $opt]
      dict set state $opt $value 
    }
    return $state
  }

  method abortCurrent {} {
    thread::send $m_workerThread [list $processor abortRun]
  }

  ## @brief Configure the job processor with the job parameters
  #
  # @param dict with standard job parameter keys
  #
  method configureJobProcessor {params} {
    set iparams [dict get $params -inputparams]
    set opts [$self pickle $iparams]
    set iparams [thread::send $m_workerThread [list OfflineEVBInputPipeParams %AUTO% {*}$opts]]
    thread::send $m_workerThread [list $processor configure -inputparams $iparams]

    set hparams [dict get $params -hoistparams]
    set opts [$self pickle $hparams]
    set iparams [thread::send $m_workerThread [list OfflineEVBHoistPipeParams %AUTO% {*}$opts]]
    thread::send $m_workerThread [list $processor configure -hoistparams $iparams]

    set eparams [dict get $params -evbparams]
    set opts [$self pickle $eparams]
    set iparams [thread::send $m_workerThread [list EVBC::AppOptions %AUTO% {*}$opts]]
    thread::send $m_workerThread [list $processor configure -evbparams $iparams]

    set oparams [dict get $params -outputparams]
    set opts [$self pickle $oparams]
    set iparams [thread::send $m_workerThread [list OfflineEVBOutputPipeParams %AUTO% {*}$opts]]
    thread::send $m_workerThread [list $processor configure -outputparams $iparams]

#    $processor configure -inputparams [dict get $params -inputparams] 
#    $processor configure -hoistparams [dict get $params -hoistparams]
#    $processor configure -evbparams   [dict get $params -evbparams]
#    $processor configure -outputparams [dict get $params -outputparams]
  }
 
  ## @brief Try to guess what the run number is from the run 
  # 
  # This expects that the run is called prefix-xxxx-yy.evt
  # and then extracts the xxxx piece. It discards all of the leading
  # zeroes before returning it. 
  #
  # @param files  a list of files that should have the same run number
  #
  # @returns the run number extracted from the first file name
  #
  method guessRunNumber {files} {
    set file [file tail [lindex $files 0]]
    set pattern {^(\w+)-(\d+)-(\d+).evt$}
    if {[regexp $pattern $file match prefix run segment]} {
      # the number is treated as octal if it is padded with 0 on the left
      # so we need to trim those off
      set run [string trimleft $run "0"]
      # but if the run number was 0, then we are left with an empty string.
      # set it to 0.
      if {$run eq {}} {set run 0}
      return $run 
    } else {
      return -code error "RunProcessor::guessRunNumber unable to parse run number from file list"
    }
  }
  
  ## @brief Appends an observer to the list of observers
  # 
  # This assumes that the user has passed in a command ensemble
  # name that implements the proper observer interface. Those are:
  #
  #   setModel {dict}
  #   abort
  #   finish
  #   transitionToNextJob {name}
  # 
  # At the moment it is not checked whether the observer actually implements
  # this.
  #
  # @param observer   the observer to receive callbacks from 
  method addRunStatusObserver {observer} {

    lappend m_runObservers $observer
  }


  ## @brief Access the list of observers
  # 
  # @return list of observers
  method listRunStatusObservers {} {
    return $m_runObservers
  }

  ## @brief Observe a new order of jobs
  #
  # A new run causes all of the observers to receive the new list of
  # jobs
  #
  method observeNewRun {} {

    # The new status object has all jobs in the queued position.
    set newdict [dict create queued     [dict keys $options(-jobs)] \
                             aborted   [list] \
                             processing "" \
                             completed  [list]]

    # Pass the list to all of the observers.
    foreach observer $m_runObservers {
      $observer set $newdict
    }
  }

  ## @brief Observe the transition to a new job
  #
  method observeNewJob {jobName} {
    foreach observer $m_runObservers {
      $observer transitionToNextJob $jobName
    }
  }

  ## @brief Observe the completion of the job list
  method observeCompleted {} {
    foreach observer $m_runObservers {
      $observer finish 
    }
  }

  ## @brief Observe the abortion of a process.
  method observeAbort {} {
    foreach observer $m_runObservers {
      $observer abort
    }

  }

}

