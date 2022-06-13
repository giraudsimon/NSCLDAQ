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

# @file  OfflineOrdererUI.tcl 
# @author Jeromy Tompkins 
#!/usr/bin/env tclsh


package provide OfflineOrdererUI 11.0

package require Tk
package require snit

package require OfflineEVBRunProcessor
package require OfflineEVBJobBuilder
package require OfflineEVBRunStatusUI
package require OfflineEVBGlobalConfigUI
package require FrameSequencer

proc TabbedOutput {win args} {
}


## @brief Top-level widget for OfflineOrdererUI
#
# @defgroup offlineordererui The OfflineOrdererUI Package
#
# The OfflineOrdererUI package manages a good deal of the content for all of
# the root window in the OfflineOrderer GUI. It maintains the button at the
# bottom right of the screen and also a content frame that is filled with
# either the JobBuilderUI or the RunStatusUI. One might be led to think that it
# is actually also responsible for the GlobalConfigUI, but that is not quite
# correct. There is a FrameSequencer that manages all of the content in the root
# window which actually manages an instance of this. But the OfflineOrdererUI is
# probably best thought of as the real application. 
#
# When the user presses the button that this provides, it will transition
# between the JobBuilderUI and the RunStatusUI. When the user transitions into a
# processing mode, the OfflineOrderer launches the RunProcessor to enable the
# actually running of jobs.
#

#########################################################################################

## @brief View for the OfflineOrderer widget
#
# @ingroup offlineordererui
#
# This is really a very simple widget. It merely maintains a content window
# and a button that causes a transition between two states. The two states 
# are the configuration state and the processing state. The configuration state 
# is where the JobBuilderUI is displayed and the user can configure his/her 
# job parameters. During the run, the RunStatusUI is displayed so that the 
# user can see that something is happening. The RunStatusUI is not perfect
# because all of this is sequential at the moment besides the various pieces that
# are launched as pipelines. 
#
# The implementation of this is intended to be fairly empty and void of logic.
# It merely knows the widgets and how to display them. All events are directed
# to the presenter.
snit::widget OfflineOrdererUIView {
  
  option -mode     -default "config" -configuremethod setMode

  component presenter 

  variable m_viewMap
  variable m_runabortButton

  constructor {args} {

    $self configurelist $args

    # There are two views that can be shown in the content window
    set m_viewMap [dict create config "" run ""]

    # assemble the megawidget
    $self buildGUI

  }

  # we don't own the presenter and the widgets will be destoyed 
  # automatically when this frame is destroyed.
  destructor {
  }

  ## @brief Build the gui
  #
  method buildGUI {} {

    ttk::frame $win.frame

    set buttons $win.buttons
    set m_runabortButton $buttons.runabort
    ttk::frame $buttons 
    ttk::button $m_runabortButton -text "Process Jobs" -command [mymethod onPress]
    grid $m_runabortButton -sticky se ;# -padx 9 -pady 9
    grid columnconfigure $win.buttons 0 -weight 1

    # grid these things
    grid $win.frame  -padx 9 -pady 9 -sticky nsew
    grid $buttons    -padx 9 -pady 9 -sticky sew

    grid columnconfigure $win 0 -minsize 400 -weight 1
    grid rowconfigure    $win 0 -minsize 200 -weight 1

  }
  
  ## @brief The button was pressed.
  #
  method onPress {} {
    $presenter transition 
  }

  ## @brief Get the presenter that this responds to
  #
  # @returns the presenter
  method getPresenter {} {
    return $presenter
  }

  ## @brief Pass the view a new presenter
  #
  # @param newPresenter   a OfflineOrdererUIPresenter object
  #
  method setPresenter {newPresenter} {
    set presenter $newPresenter
  }

  ## @brief Retrieve which widget is currently gridded
  #
  # @returns name of gridded widget
  #
  method getCurrentWidget {} {
    return [grid slaves $win.frame] 
  }

  ## @brief  Set the dict of the view widgets
  # 
  # Currently there are only two different modes that are 
  # supported. The idea is that each mode has a corresponding
  # GUI to display. The dict has keys that are the mode names
  # and the associated widgets are the values.
  #
  # @param widgetDict   a dict of of {mode0 widget0 mode1 widget1}
  method setViewWidgets {widgetDict} {
    set m_viewMap $widgetDict
  }

  ## @brief Callback for configure -mode
  #
  # This is the configuremethod for the -mode option. The
  # main idea of this is to synchronize the displayed
  # widget with the mode.
  # 
  # @param option name of the option (in this case -mode)
  # @param mode   the new mode to set
  #
  method setMode {option mode} {
    variable m_runabortButton
    variable m_viewMap

    set children [grid slaves $win.frame]
    foreach child $children { grid forget $child }

    if {$mode eq "config"} {

      $m_runabortButton configure -text "Process Jobs" 
      set widget [dict get $m_viewMap config]
      if {$widget ne ""} {
        grid $widget -sticky nsew
        grid columnconfigure [$self getFrameWidget] 0 -weight 1
        grid rowconfigure    [$self getFrameWidget] 0 -weight 1
      }

      set options(-mode) $mode
    } elseif {$mode eq "run"} {
      $m_runabortButton configure -text "Back" 

      set widget [dict get $m_viewMap run]
      if {$widget ne ""} {
        grid $widget -sticky nsew
      }

      set options(-mode) $mode
    } else {
      set msg    "OfflineOrdererUIView::setMode passed \"$mode\" as "
      append msg "argument instead of \"config\" or \"run\"."
      return -code error $msg 
    }
  }


  ## @brief Getter for the content frame widget name
  #
  # @returns name of content frame
  method getFrameWidget {} {
    return $win.frame
  }

  ## @brief Retrieve the name of this megawidget
  #
  # @returns name of megawidget
  method getWindowName {} {
    return $win
  }
}



#-------------------------------------------------------------------------------
################################################################################
################################################################################
################################################################################
################################################################################
################################################################################
#-------------------------------------------------------------------------------

## @brief The presenter object for the OfflineOrdererUI
# 
# @ingroup  offlineordererui
#
# This is manages the view but doesn't actually have much of a model to manage.
# When the view button is presenter, this passes the view the correct widget to
# display to the user and will possibly launch the offline orderer for
# processing.
#
snit::type OfflineOrdererUIPresenter {
  option -widgetname     -default "" ;#< name of view widget
  
  component m_view                   ;#< view object

  component runProgressPresenter     ;#< RunStatusUI presenter
  component jobBuilderPresenter      ;#< JobBuilderUI presenter
  component globalConfigPresenter    ;#< GlobalConfigUI

  component runProcessor             ;#< The RunProcessor

  ## @brief Constructor
  #
  # It is demanded that the user provide the name of the view widget through the
  # -widgetname option and an error is thrown if it is not provided. This
  # constructs all of the various presenters it owns (not the GlobalConfigUI).
  #
  # @params args  option-value pairs
  #
  # @returns name of this object
  # 
  # @throws an error if the -widgetname was missing
  constructor {args} {

    # allow the user to override the defaults
    $self configurelist $args

    if {$options(-widgetname) eq ""} {
      set msg    "OfflineOrdererUI -widgetname option is mandatory "
      append msg "but was not provided!"
      return -code error $msg 
    }

    
    # setup the view
    set m_view [OfflineOrdererUIView $options(-widgetname)]
    $m_view setPresenter $self

    
    # setup the running and configuration frames
    set fr [$m_view getFrameWidget]
    set runProgressPresenter [RunStatusUIPresenter %AUTO% -widgetname $fr.runUI]

    
    # this uses the singleton adapater for the JobBuilder
    set JobBuilder::widgetName $fr.configUI
    set jobBuilderPresenter [JobBuilder::getInstance] 

    
    # pass the names of the frames to view so that it knows what to show in
    # various modes.
    $m_view setViewWidgets [dict create run $fr.runUI \
                                        config $fr.configUI ]
    
    # create the run processor
    set runProcessor [RunProcessor %AUTO%]

    
    # connect the RunStatusUI to the RunProcessor as an observer. 
    $runProcessor addRunStatusObserver $runProgressPresenter
    
    $runProgressPresenter configure -runprocessor $runProcessor

    # set the display mode
    $m_view configure -mode config

  }

  ## @brief Destroy the view and also the run processor
  #
  destructor {
    catch {destroy $m_view}
    catch {$runProcessor destroy}
  }

  ## @brief Access the JobBuilderUIPresenter that this knows about
  #
  # @returns  the JobBuilderUIPresenter object
  method getJobBuilderPresenter {} {
    return $jobBuilderPresenter
  }

  ## @brief Set the view for this object
  #
  # The new view merely displaces the ownership of the previous view. It is
  # assumed that some other object might want to take over the view.
  #
  # @param theview  the new OfflineOrdererUIView object
  #
  # @return the name of the previous view object
  method setView {theview} {
    set prevView $m_view
    set m_view $theview
    return $prevView
  }


  ## @brief Respond to a press of the button
  #
  # This attempts to transition from one display to the other.  If moving to a
  # running mode, then the run method is invoked to start the pipelines.
  #
  method transition {} {
    set state [$m_view cget -mode]

    if {$state eq "config"} {
      # button pressed to enter run mode

      # remove the menu
      $Globals::menu delete 0 1

      # change to the run status display
      $m_view configure -mode run 

      # run
      $self run
    } else {
      set cmd0 [list $::Globals::sequencer select config ]
      set cmd1 [list $::Globals::menu delete 0 1 ]
      set cmd  [list $cmd0 $cmd1]
      $::Globals::menu add command -label "Config" -command [join $cmd "; "] 

      $m_view configure -mode config
    }
  }

  ## @brief Pass the list of jobs from the JobBuilderUI to the RunProcessor
  #
  # It is not necessary to check the quality of the jobs at this point because
  # they have already been vetted by the JobBuilderUI.
  #
  method run {} { 
    variable jobBuilderPresenter
    variable runProcessor 

    set masterJobList [$jobBuilderPresenter getJobsList]
    if {[dict size $masterJobList]>0} {
      $runProcessor configure -jobs $masterJobList
      set status [$runProcessor run]
    };

  } 

} ;# end of OfflineOrderer

#------------------------------------------------------------------------------
###############################################################################
###############################################################################
###############################################################################
###############################################################################
###############################################################################
#------------------------------------------------------------------------------

## @brief A Global namespace that makes it fairly easy to find the global menu item and
#   also the name of the FrameSequencer
#
#
namespace eval Globals {
  variable menu ".m"        ;#< Name of the menu in the root window
  variable sequencer ".seq" ;#< Name of the FrameSequencer controlling the root
                            ;#  window
}

