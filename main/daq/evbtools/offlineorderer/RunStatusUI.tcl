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

# @file  RunStatusUI.tcl 
# @author Jeromy Tompkins 

package provide OfflineEVBRunStatusUI 11.0

package require snit
package require Tk


## @brief The view for the RunStatusUI package
#
# This mainly just mmaintains a frame that displays a bunch of
# JobStatusDisplays. That is all that it does.
#
snit::widget RunStatusUIView {

    component m_jobDisplays   ;#< List of JobStatusDisplays
    component m_presenter     ;#< Presenter to pass events to
    
    ## @brief Construct the megawidget
    #
    # The user really should not
    #
    # @param presenter an RunStatusUIPresenter object
    # @param args      a list of option-value pairs for setting the options
    #
    constructor {presenter args} {
      
      set m_presenter $presenter

      # build the gui
      $self buildGUI    
    }

    ## @brief Destroy this thing 
    #
    destructor {
      catch {destroy $win}
    }

    ## @brief Add the argument to the list of gridded job displays
    #
    # @param jobDisplay   the widget name to display
    #
    method appendNewJobDisplay {jobDisplay} {
      grid $jobDisplay -sticky new
      set geometry [grid size $win]
      set row [expr {[lindex [grid size $win] 1]-1}]
      grid rowconfigure $win $row -weight 0
      grid columnconfigure $win 0 -weight 1
    }

    ## @brief Assemble the megawidget
    #
    # Because this is a snit::widget object, the hull is actually just a
    # ttk::frame.  This fills that frame ($win) with a bunch of configuration
    # widgets. These are 
    # simply some labels on text entries.
    #
    method buildGUI {} {

      set top $win.hdrFrame
      ttk::frame $top

      ttk::label $top.title -text "Run Progress" -style "H1.TLabel"
      ttk::label $top.nameLbl -text "Job Name" 
      ttk::label $top.statusLbl -text "Job Status" 
      ttk::separator $top.sep -orient horizontal 

      grid $top.title          -        -  -sticky new
      grid $top.nameLbl $top.statusLbl  x  -sticky sew -padx 9
      grid $top.sep            -        -  -sticky sew
      grid rowconfigure $top 0 -weight 1
      grid columnconfigure $top all -weight 1

      grid $win.hdrFrame -sticky new

      grid rowconfigure $win 0 -minsize 50
      grid columnconfigure $win 0 -weight 1 
    }

    ## @brief Pass this a different presenter object 
    #
    # @param presenter an RunStatusUIPresenter object
    #
    method setPresenter {presenter} {
      set m_presenter $presenter
    }


    ## @brief Return the value of $win for gridding the view
    #
    method getWindowName {} {return $win}

}

# End of RunStatusUIView code

#-------------------------------------------------------------------------------
################################################################################
################################################################################
################################################################################
################################################################################
################################################################################
#-------------------------------------------------------------------------------

## @brief Defines the logic for the RunStatusUIView
#
# This is actually quite simple in that it responds to events from the view
# and manages the model. It is allowed to edit the model and is thus responsible
# for synchronizing the state of the model with the displayed values when an 
# apply command is given it.
#
# @important This does not own the model! Rather it just has a reference to the
#            model and the power to manipulate it.
#
snit::type RunStatusUIPresenter {

  option -widgetname -default ""
  option -runprocessor -default ""

  component m_model     ;#< The model : OfflineEVBInputPipeParams
  component m_view      ;#< The view, owned by this

  variable m_jobDisplayNames

  ## Construct the model, view, and synchronize view
  #
  # It is necessary that the user provides the name of the widget. If that is
  # not provided this cries, "Uncle!" The presenter is tightly bound to the view
  # and actually owns it. It passes the view itself on construction so that
  # there is never any confusion about who is in charge of it.
  #
  # @returns the name of this object
  # 
  # Exceptional returns
  # - Error if -widgetname is not provided.
  #
  constructor {args} {
    $self configurelist $args
   
    if {$options(-widgetname) eq ""} {
      set msg    "RunStatusUIPresenter -widgetname option is mandatory "
      append msg "but was not provided!"
      return -code error $msg 
    }
    
    # Create the default model
    set m_model [dict create processing "" aborted [list] queued [list] completed [list]] 

    # Create the view and pass it the values of the model
    set m_view   [RunStatusUIView $options(-widgetname) $self] 
    $m_view setPresenter $self
    $self updateViewData $m_model

    set m_jobDisplayNames [dict create]
  }

  ## @brief Destroy the view 
  # 
  # Do not destroy the model because it is just a reference
  #
  destructor {
    if {[catch {destroy $m_view} msg]} {
      puts $msg
    }
  }


  ## @brief Set the model to some user's model and synchronize
  #
  #
  # @param model  an OfflineEVBInputPipeParams object
  #
  method set {model} {
    $self setModel $model
    $self updateViewData $m_model
  }

  ## @brief Set the model without synchronizing
  #
  # @param model  a dict with keys {completed processing queued}
  method setModel {model} {
    set m_model $model
  }

  ## @brief Retrieve the model
  #
  # @returns string
  # @retval the name of the OfflineEVBInputPipeParams object this controls
  method getModel {} {
    return $m_model
  }
  

  ## @brief Transition the current to completed, and pop a queued event into 
  #         processing
  #
  # This handles the logic for when a job successfully completes. What happens
  # is that the name of the job that was processing is appended to the completed
  # list and then the next queued job is transitioned to the current 
  # processing.
  #
  # @param jobName  name of the job to make current
  #
  #
  method transitionToNextJob {jobName} {
    # mark the current job as completed
    $self transitionCurrent completed
    
    # find the new job in the list of queued  and make it current
    set queuedJobs [dict get $m_model queued]
    set newJobIndex [lsearch $queuedJobs $jobName]
    if {$newJobIndex>=0} {
      dict set m_model processing [lindex $queuedJobs $newJobIndex]

      # remove the current job from queued list
      set queuedJobs [lreplace $queuedJobs $newJobIndex $newJobIndex]
      dict set m_model queued $queuedJobs
    }

    # synchronize the view with the model
    $self updateViewData $m_model
  }

  ## @brief Transition the current to a new key
  #
  # @param key  the name of key to transition current job to 
  #
  method transitionCurrent {key} {

    set current [dict get $m_model processing]
    if {$current ne ""} {
      dict lappend m_model $key $current
      dict set m_model processing ""
    }
  }

  ## @brief Handles the logic for completing a list of jobs
  #
  # Really all this needs to do is transition the the currently
  # processing job to completed and update what the user sees.
  #
  method finish {} {
    $self transitionCurrent completed
    $self updateViewData $m_model 
  }
 
  ## @brief Handles the logic for updating the display caused by an abort
  #
  # At the moment, this merely updates the view data and nothing else
  #
  method abort {} {
    $self transitionCurrent aborted
    $self updateViewData $m_model 
  }

  ## @brief Synchronize the data displayed by the view with the model
  #
  # @param model  an OfflineEVBInputPipeParams object
  #
  method updateViewData {model} {

    $self updateDisplayDataForStatusType completed 

    $self updateDisplayDataForStatusType aborted

    $self updateDisplayDataForStatusType processing

    $self updateDisplayDataForStatusType queued

  }

  ## @brief Logic for synching the displayed data for a set jobs in a similar 
  #         state
  #
  # If the job exists, this finds the displayed object and updates its state.
  # However, it is also possible that a new job has never been added to the 
  # view. In this case, append a job status display to the view for the 
  # state.
  #
  method updateDisplayDataForStatusType {status} {

    # get the list of jobs for the given status
    set jobList [dict get $m_model $status]

    # handle each of the statuses
    foreach job $jobList {
      if {[$self isDisplayedJob $job]} {
        [dict get $m_jobDisplayNames $job] configure -status $status 
      } else {
        # the job doesn't exist as a display object
        # make a new one and then pass it the view to display
        set widgetName [$self createNewJobDisplay $status $job]

        $m_view appendNewJobDisplay $widgetName
        dict set m_jobDisplayNames $job $widgetName 
      }
    }

  }

  ## @brief Creates a new JobStatusDisplay with unique name
  #
  # @param status   the status to give to the new display
  # @param job      the name to give to the new display
  #
  # @returns name of the widget created for displaying
  #
  method createNewJobDisplay {status job} {
    set widgetName [$self findUniqueName]
    JobStatusDisplay $widgetName -status $status -jobname $job
    $widgetName setRunProcessor [$self cget -runprocessor]
    return $widgetName
  }

  ## @brief Retrieve the list of controlled display jobs
  #
  method getDisplayedJobs {} {
    return $m_jobDisplayNames
  }

  ##
  # (Primarily for testing)
  #
  method setDisplayedJobs {jobs} {
    set m_jobDisplayNames $jobs
  }

  ## @brief Checks whether the job name has a controlled display
  #
  # @brief job  the name of the job
  #
  # @returns boolean
  # @retval 0 - job is not found
  # @retval 1 - job is found
  #
  method isDisplayedJob {job} {
    return [dict exists $m_jobDisplayNames $job]
  }

  ## @brief Generate a unique widget that will not fail
  #
  # The name of each jobDisplay are simply (view).jobDisplay<num>, where
  # (view) is the widget name of the view managed by this presenter.
  # This produces a new display that is unique by increment <num> until
  # a name is find that doesn't match a key in m_jobDisplayNames.
  #
  # @important It is noteworthy that this doesn't check to see if the name is 
  # already associated with a widget known to the window manager but is not 
  # known to this object instance.
  #
  # @returns the unique name
  #
  method findUniqueName {} {

    # scan until find a name that doesn't exist in the list 
    set w "$options(-widgetname).jobDisplay"
    set index 0
    set name [format "%s%d" $w $index]

    # get the current list of names
    set existingNames [dict values $m_jobDisplayNames]

    # keep incrementing the index used to form the name
    # until the name produced is not found in the existingNames
    while {[lsearch $existingNames $name]>=0} {
      incr index
      set name [format "%s%d" $w $index]
    }

    # a unique name was found, so return it.
    return $name
  }

  ## @brief Retrieve the view
  #
  # @returns string
  # @retval the name of an RunStatusUIView object 
  method getViewObj {} {
    return $m_view
  }

  ## @brief Retrieve the name of the widget this is ultimately connected to
  #
  # The user actually should already have access to this information because 
  # they provided the info via the -widgetname option. However, it is trivial
  # to let them ask for the name again.
  #
  # @return string
  # @retval the widget name (i.e. .widget)
  method getViewWidget {} {
    return [$m_view getWindowName]
  }

}


################################################################################
################################################################################

## @class JobStatusDisplay
#
# @brief a Megawidget that provides visual feedback for the processing status 
#        of a job
#
# This knows about 3 three states: queued, processing, and completed. 
# The queued and completed states merely show the job name and the text indicating
# they are either queued or completed. However, the processing status show a 
# progress bar with indeterminate progress. It simply moves back and forth while
# active.
#
#
snit::widget JobStatusDisplay {

  option -jobname
  option -status -configuremethod setStatus

  variable runProcessor 

  ## @brief Construct a new widget
  # 
  constructor {args} {
    $self buildGUI
    $self configurelist $args

    set runProcessor {}
  }

  ## @brief Build the widgets that form the whole
  #
  method buildGUI {} {
    ttk::label $win.jobLbl  -textvariable [myvar options(-jobname)]
    ttk::label $win.jobStatusLbl -textvariable [myvar options(-status)]
    ttk::progressbar $win.jobProgress -orient horizontal -mode indeterminate
    ttk::button $win.abort -text "Abort" -command [mymethod abortCurrent]

#    grid $win.jobLbl $win.jobStatusLbl $win.jobProgress -sticky ew -padx 9 -pady 9
#    grid $win.jobLbl $win.jobStatusLbl $win.abort -sticky ew -padx 9 -pady 9
    grid $win.jobLbl $win.jobProgress $win.abort -sticky ew -padx 9 -pady 9

#    grid columnconfigure $win {0 1 2} -weight 1 -minsize 81
    grid columnconfigure $win {0 1} -weight 1 -minsize 81
  }


  method abortCurrent {} {
    $runProcessor abortCurrent
  }

  method setRunProcessor processor {
    set runProcessor $processor
  }

  ## @brief Transition the visible components for the new state 
  #
  # If the new state value is not completed, processing, or queued this will
  # do nothing.
  #
  # @param option the option being set (always -status in this context)
  # @param value  the new status value
  #
  method setStatus {option value} {
    if {$value eq "processing"} {
      grid remove $win.jobStatusLbl
      grid $win.jobProgress -column 1 -row 0 -sticky ew
      $win.jobProgress start 20
    } else {
      $win.jobProgress stop
      grid remove $win.jobProgress
      grid configure $win.jobStatusLbl -column 1 -row 0 -sticky ew
    }
    set options(-status) $value

 } 


}
