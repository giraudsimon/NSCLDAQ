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

# @file  MissingSourceUI.tcl 
# @author Jeromy Tompkins 

#########################################################################################################
#
# INPUT PIPELINE CONFIGURATION DIALOGUE

package provide OfflineEVBMissingSourceUI 11.0

package require OfflineEVBInputPipeline
package require OfflineEVBHoistPipeline
package require snit
package require Tk
package require textprompter


## Overview of the OfflineEVBMissingSourcelineUI package
#
# @defgroup missingsourceui The MissingSourceUI Widget
#
# This is a widget that is designed with the Model-View-Presenter paradigm in
# mind.  There are two pieces to it: the MissingSourceConfigUIView and
# MissingSourceUIPresenter. The presenter object is responsible for both
# managing the view and the model. The view simply passes information to the
# presenter and the presenter determines what to do with it. The view is
# therefore just a simple widget that manages the actual widgets and also passes
# events to the presenter.
#
# The goal of this package is to provide a centralized place that a user can
# configure the parameters for the hoist pipeline. If the user knows that the
# data file being sent through the ringFragmentSource all have body headers,
# then it is possible for the user to opt out of providing the tstamp lib and
# id. 




## @brief The view for the MissingSourceConfigUI
# 
# Displays a simple widget that will hide configuration options if the user is
# not actually needing them. If the user knows that the tstamplib and id are not
# necessary, they simply are presented with a checkbutton and an explanation of
# that text button. However, if they know that the tstamp lib and id are needed,
# they can select the checkbutton and the data entry widgets to gather the
# needed information are made visible. This is for the purpose of presenting a
# very simple view to the user if they are treating a simple case.
#
# The user really should not instantiate this object directly unless they know
# what they are doing. This object has limited functionality unless it is
# attached to a presenter object. In fact, pressing buttons with no presenter
# will cause massive failures!
#
# 
snit::widget MissingSourceConfigUIView {

  option -tstamplib       -default ""  ;#< tstamp extraction library path
  option -id              -default 0   ;#< source id

  option -missing         -default 0  \
                          -configuremethod setMissingMode ;#< checkbutton state
  option -showbuttons     -default 1    ;#< Whether to show some buttons or not

  component m_presenter     ;#< the presenter to forward events to

  ## @brief Construct the megawidget
  #
  # @param presenter an Presenter object
  # @param args      a list of option-value pairs for setting the options
  #
  constructor {presenter args} {

    set m_presenter $presenter

    $self configurelist $args 

    # build the gui
    $self buildGUI    
  }

  ## Destroy this thing 
  #
  destructor {
    catch {destroy $win}
  }


  ## @brief Assemble the megawidget
  #
  # Because this is a snit::widget object, the hull is actually just a ttk::frame.
  # This fills that frame ($win) with a bunch of configuration widgets. These are 
  # simply some labels on text entries.
  #
  method buildGUI {} {

    ttk::checkbutton $win.missing -text "Missing body headers" \
                                  -variable [myvar options(-missing)]\
                                  -onvalue 1 -offvalue 0]
    trace add variable [myvar options(-missing)] write [mymethod onMissingChange]
    set top $win.descr
    ttk::frame $top
    if {"DescriptionFont" ni [font names]} {
      font create DescriptionFont -family Helvetica -size 10 -slant italic
    }
    tk::text $top.descrLbl -bg lightgray  -relief flat -wrap word -font DescriptionFont \
                           -height 3 -width 60
    set descText "Check this if some ring items lack body headers in the data, " 
    append descText "because additional information is required to send these "
    append descText "items through the event builder." 
    $top.descrLbl insert end $descText
    $top.descrLbl configure -state disabled

    grid $top.descrLbl -sticky nsew
    grid rowconfigure $top 0 -weight 1
    grid columnconfigure $top 0 -weight 1
          
    set top $win.params
    ttk::frame $top 

    #ttk::label $top.tstampLabel   -text "Timestamp extraction library"
    #ttk::entry $top.tstampEntry -textvariable [myvar options(-tstamplib)]
    textprompt $top.tstampEntry -text "Timestamp extraction library" \
      -textvariable [myvar options(-tstamplib)]
    ttk::button $top.tstampBrowse -text "Browse..." -command [mymethod _browseTstamp]
    #ttk::label $top.idLabel   -text "Source ID"
    #ttk::entry $top.idEntry -textvariable [myvar options(-id)]
    textprompt $top.idEntry -text "Source ID" -textvariable [myvar options(-id)]
    
    grid $top.tstampEntry $top.tstampBrowse \
                                              -sticky ew
    grid $top.idEntry     x  -sticky ew

    set spaceFrame $win.space
    ttk::frame $spaceFrame

    set buttonFrame $win.buttons
    ttk::frame $buttonFrame 
    ttk::button $buttonFrame.cancel  -text "Cancel" -command [mymethod onCancel]
    ttk::button $buttonFrame.apply   -text "Apply"  -command [mymethod onApply]
    grid $buttonFrame.cancel $buttonFrame.apply -sticky ew -padx 9 -pady 9
    grid columnconfigure $buttonFrame {0 1 2} -weight 1


    grid $win.missing -row 0 -sticky new
    if {$options(-missing)} {
      grid $win.params  -row 1 -sticky new
    } else {
      grid $win.descr  -row 1 -sticky new
    }
    grid $spaceFrame  -row 2 -sticky nsew

    if {$options(-showbuttons)} {
      grid $win.buttons -row 3 -sticky sew -padx 9 -pady 9
    }
    grid rowconfigure    $win 2 -weight 1
    grid columnconfigure $win 0 -weight 1
  }


  ## @brief Pass this a different presenter object 
  #
  # @param presenter an InputPipeConfigUIPresenter object
  #
  method setPresenter {presenter} {
    set m_presenter $presenter
  }


  ## @brief Return the value of $win for gridding the view
  #
  method getWindowName {} {return $win}

  ## @brief Forward the event to the presenter
  #
  method onApply {} {
    $m_presenter apply
  }

  ## @brief Forward the event to the presenter
  #
  method onCancel {} {
    $m_presenter cancel 
  }

  ## @brief Launch a file browser to find the user's .so
  #
  # By default, this will look for files that have extension .so but 
  # it is also possible for viewing all files. That way the user can make a 
  # file with a wacked out name and it will still work.  
  #
  method _browseTstamp {} {
    set types {
      {{Shared libraries} {.so}}
      {{All files}        {*}}
    }
    
    set file [tk_getOpenFile  -filetypes $types \
                              -title "Select a timestamp extraction library" \
                              -initialdir $::env(HOME)]

    $self configure -tstamplib $file
  }

  ## @brief The method that gets called when configure -missing is set.
  #
  # This method is responsible for changing what the user sees. If the 
  # the -missing option is 1, then the tstamplib and id entries are shownm but
  # if the -missing option is 0, the description text is displayed. Also, being
  # the configureMethod for the -missing option, this is responsible for
  # actually setting the new value.
  #
  # @param option   the name of the option being set (always -missing)
  # @param value    the value to set the option to
  #
  method setMissingMode {option value} {

    if {[string is true $value]} {
      grid remove $win.descr
      grid $win.params -row 1 -sticky new
      set options(-missing) 1 
    } else {
      grid remove $win.params
      grid $win.descr -row 1 -sticky new
      set options(-missing) 0
    }
  }

  ## @brief Handle a checkbutton press
  #
  # This method is registered to the value controlled by the checkbutton. If the
  # checkbutton is pressed, then this is called and it merely tries to configure
  # the -missing option. By doing so, the setMissingMode method is called.
  # 
  # @param name1  first name of the variable being set (option(-missing))
  # @param name2  second name (useful for arrays) but is unused
  # @param op     name of operation being performed...unused
  #
  method onMissingChange {name1 name2 op} {
    if {$options(-missing) == 1} {
      $self configure -missing 1
    } else {
      $self configure -missing 0
    }
  }

  method setSourceID {id} {
    $self configure -id $id
  }
}

# End of MissingSourceConfigUIView code

#-------------------------------------------------------------------------------
################################################################################
################################################################################
################################################################################
################################################################################
################################################################################
#-------------------------------------------------------------------------------

## @brief The presenter for the MissingSourceConfigUI package
# 
# @ingroup missingsourceui
#
# This is the presenter portion of the package. It is responsible for
# manipulating the data in the model that it controls and responding to events
# from the view. Therefore, the view is just a slave of this and is actually
# created and owned by the presenter. It is intended that other software deals
# directly with the MissingSourceConfigUIPresenter rather than with its view.
# Being in charge of both the view and the model, this object is responsible for
# the logic behind synchronizing the two. There is never a partial
# synchronization that is accomplished because all sync operations affect all
# controlled aspects of the model. The synchronization can go both directions,
# either updating the view from the model or the model from the view.
#
# The model that is controlled by this is different from a job. Instead, it
# maintains on the parameters of a job that are associated with the input and
# hoist pipelines.  The model is actual a dict whose keys refer to those objects
# and the names of the keys are the same as those in a Job. The key-value pairs
# are as follows:
#
# key           value-type
# -inputparams  OfflineEVBInputPipeParams
# -hoistparams  OfflineEVBHoistPipeParams
#
snit::type MissingSourceConfigUIPresenter {

  option -widgetname -default "" ;#< name of the view
  option -ismaster   -default 0  ;#< whether this can destroy itself
  option -ownmodel   -default 1  ;#< whether this cna destroy the model

  component m_model     ;#< The model : dict of -inputparams and -hoistparams
  component m_view      ;#< The view, owned by this

  ## Construct the model, view, and synchronize view
  #
  # It is necessary that the user provides the name of the widget. If that is
  # not provided this cries, "Uncle!" The presenter is tightly bound to the view
  # and actually owns it. It passes the view itself on construction so that
  # there is never any confusion about who is in charge of it.
  #
  # @returns the name of this object
  # 
  # @throws error when -widgetname is not provided.
  #
  constructor {args} {
    $self configurelist $args
   
    if {$options(-widgetname) eq ""} {
      set msg    "MissingSourceConfigUIPresenter -widgetname option is mandatory "
      append msg "but was not provided!"
      return -code error $msg 
    }
    
    set options(-ownmodel) 1
    # Create the default model
    set m_model [dict create -inputparams [OfflineEVBInputPipeParams %AUTO%] \
                             -hoistparams [OfflineEVBHoistPipeParams %AUTO%] ]

    # Create the view and pass it the values of the model
    if {$options(-ismaster) eq ""} {
      set m_view   [MissingSourceConfigUIView $options(-widgetname) $self -showbuttons 1] 
    } else {
      set m_view   [MissingSourceConfigUIView $options(-widgetname) $self -showbuttons 0] 
    }
    $m_view setPresenter $self
    $self updateViewData $m_model

  }

  ## @brief Destroy the view 
  # 
  # Do not destroy the model because it is just a reference
  #
  destructor {
    if {$options(-ownmodel)} {
      dict for {key val} $m_model { $val destroy }
    }
    catch {$m_view destroy}
  }


  ## @brief Set the model to some user's model and synchronize
  #
  # If the previous model is owned by this, then this will destroy it.
  # Otherwise, it is forgotten and its destruction is left to be some other
  # object's responsiblity. 
  #
  # @param model  aParams object
  # @param own    whether this is able to destroy the new model
  #
  method setModel {model {own 0}} {

    # delete the model if we own it
    if {$options(-ownmodel)} {
      dict for {key val} $m_model { $val destroy }
    }
    
    set m_model $model
    set options(-ownmodel) $own

    $self updateViewData $m_model
  }

  ## @brief Retrieve the model
  #
  # @returns string
  # @retval the name of the OfflineEVBMissingSourceParams object this controls
  method getModel {} {
    return $m_model
  }
  
  ## @brief Synchronize the data displayed by the view with the model
  #
  # @param model  an OfflineEVBMissingSourceParams object
  #
  method updateViewData {model} {

    set tstamplib [[dict get $model -hoistparams] cget -tstamplib]
    set id        [[dict get $model -hoistparams] cget -id]
    set expectbh  [[dict get $model -hoistparams] cget -expectbheaders]

    $self synchronizeParams $tstamplib $id

    $m_view configure -tstamplib $tstamplib
    $m_view configure -id        $id
    $m_view configure -missing   [expr {!$expectbh}] 
  }

  ## @brief Set the values of the model to what are displayed
  #
  method commitViewDataToModel {} {

    set tstamplib [$m_view cget -tstamplib]
    set id        [$m_view cget -id]
    set missing   [$m_view cget -missing]

    [dict get $m_model -hoistparams] configure -expectbheaders [expr {!$missing}]
    $self synchronizeParams $tstamplib $id
  }  

  ## @brief Set params in model to the arguments
  #
  # This is just the portion of the synchronization that sets the value of the
  # model. 
  #
  # @param tstamplib  path to timestamp extraction library
  # @param id         source id
  #
  method synchronizeParams {tstamplib id} {

    [dict get $m_model -inputparams] configure -unglomid   $id

    [dict get $m_model -hoistparams] configure -tstamplib  $tstamplib 
    [dict get $m_model -hoistparams] configure -id         $id 
  }

  ## @brief Synchronize the model to the view data
  #
  # This is what is intended to be called when the user is done configuring 
  # values and is ready to apply the changes.
  #
  method apply {} {

    $self commitViewDataToModel
  }

  ## @brief Kill off the top level widget 
  #
  method cancel {} {

    if {$options(-ismaster)} {
      set widget [$m_view getWindowName]
      set top [winfo toplevel $widget]
      

    }
  }

  ## @brief Retrieve the view
  #
  # @returns string
  # @retval the name of an InputPipeConfigUIView object 
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
