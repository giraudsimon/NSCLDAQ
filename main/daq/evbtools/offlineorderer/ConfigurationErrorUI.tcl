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

# @file  ConfigurationErrorUI.tcl 
# @author Jeromy Tompkins 

package provide ConfigErrorUI 11.0

package require Tk
package require snit


## @brief A Package to display a set of error messages to the user
#
# @defgroup configerrorui The ConfigErrorUI Package
#
# The package provided here allows the user to display a clean overview of 
# a set of errors to the user that are organized into context areas. This
# expects a dict whose keys point to dicts whose keys reference lists of error
# messages. You can think of this as a tree. In the OfflineEVB, the dict
# provided has the following form:
# 
# @verbatim
#   job0
#   |-- inputparams
#   |   |-- list of errors
#   |-- hoistparams
#   |   |-- list of errors
#   |-- evbparams
#   |   |-- list of errors
#   |-- outputparams
#   |   |-- list of errors
#   job1
#   |-- inputparams
#   |   |-- list of errors
#    and so on...
# @endverbatim
# 
# The dict is transformed into a ttk::treeview object for easy navigation of the
# errors.
#
# This like many other UI components in the offline orderer is designed with the
# Model-View-Presenter paradigm. It is composed of two separate classes:
# ConfigErrorView and ConfigErrorPresenter. SOftware that interacts with this
# package should deal with the ConfigErrorPresenter as it is the piece that will
# generate a ConfigErrorView. 

## @brief The view for the ConfigErrorUI package
#
# @ingroup configerrorui
#
# This provides a megawidget that consists of a simple label and treeview
# widget. It doesn't provide any input widget for the user to provide
# information and instead exists for displaying information. Other software
# should not instantiate one of these without an associated
# ConfigErrorPresenter. In fact, the ConfigErrorPresenter will create one of
# these in its constructor.
#
snit::widget ConfigErrorView {
  option -errors  ;#< A dict as described above in the package overview

  component m_presenter   ;#< name of the presenter that owns this
  component m_errorTree   ;#< name of the treeview widget


  ## @brief Constructor
  #
  # @param presenter  the name of the presenter that will control this
  # @param args       option-value pairs
  #
  # @returns the name of the new instance create by this
  constructor {presenter args} {

    set m_presenter $presenter

    $self configurelist $args
    $self buildGUI
  }


  ## @brief Assemble the megawidget
  #
  method buildGUI {} {

    # install the treeview
    install m_errorTree using ttk::treeview $win.errors 

    set top $win.buttons
    ttk::frame $top
    ttk::button $top.okay -text "Okay" -command [mymethod onOkay]
    grid $top.okay -sticky se -padx 9 -pady 9
    grid columnconfigure $top 0 -weight 1 
    
    grid $win.errors -sticky nsew -padx 9 -pady 9
    grid $win.buttons -sticky sew -padx 9 -pady 9
    grid rowconfigure $win 0 -weight 1
    grid columnconfigure $win 0 -weight 1

  }

  ## @brief Forward the press of "okay" button to presenter
  #
  method onOkay {} {
    $m_presenter okay
  }

  ## @brief Insert a job entry into the treeview
  #
  # The -error option should provide a dict whose keys are job. This will handle
  # the mapping of the details of a job. 
  #
  # @param name       name of job (a key of -errors option)
  # @param errorDict  dictionary of errors (value associated with name key)
  #
  method addJobEntry {name errorDict} {
    $m_errorTree insert {} end -id $name -text $name 
    dict for {key errors} $errorDict {
      if {[llength $errors]>0} {
        $m_errorTree insert $name end -id "$name$key" -text $key
        foreach err $errors { 
          $m_errorTree insert "$name$key" end -text $err
        }
      }
    } 
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
}

#------------------------------------------------------------------------------
###############################################################################
###############################################################################
###############################################################################
###############################################################################
###############################################################################
#------------------------------------------------------------------------------


## @brief The presenter for the ConfigErrorUI package
#
# @ingroup configerrorui
#
# The ConfigErrorPresenter is in charge of handling the logic for the
# ConfigError package. It doesn't handle much logic and really just determines
# how to manage an click of the "Okay" button. The model that this controls is
# the dictionary of job errors as described in the overview of the package at
# the top of the page.
#
snit::type ConfigErrorPresenter {

  option -widgetname -default ""  ;#< name of the view widget

  component m_model   ;#<  dictionary of error messages
  component m_view    ;#<  the view widget

  ## @brief Constructor
  #
  # Create the view and synchronize.
  #
  # @param args   option-value pairs (-widgetname is the only supported)
  #
  # @throws error if the -widgetname is not passed.
  constructor {args} {
    $self configurelist $args
   
    if {$options(-widgetname) eq ""} {
      set msg    "ConfigErrorPresenter -widgetname option is mandatory "
      append msg "but was not provided!"
      return -code error $msg 
    }
    
    # Create the default model (an empty dict)
    set m_model [dict create] 

    # Create the view and pass it the values of the model
    set m_view   [ConfigErrorView $options(-widgetname) $self] 
    $m_view setPresenter $self
    $self updateViewData $m_model

  }


  ## @brief Destroy the view 
  # 
  # Do not destroy the model because it is just a reference
  #
  destructor {
    catch {$m_view destroy}
  }

  ## @brief Set the model to some user's model and synchronize
  #
  #
  # @param model  a dict of error messages
  #
  method setModel {model} {
    set m_model $model
    $self updateViewData $m_model
  }

  ## @brief Retrieve the model
  #
  # @returns string
  # @retval the dictionary of error messages  
  method getModel {} {
    return $m_model
  }
  
  ## @brief Synchronize the data displayed by the view with the model
  #
  # @param model  a dict of error messages 
  #
  method updateViewData {model} {
    dict for {job errDict} $model {
      $m_view addJobEntry $job $errDict
    }
  }

  ## @brief Set the values of the model to what are displayed
  #
  method commitViewDataToModel {} {
    # the user cannot alter the model through the view so this is a noop
  }

  ## @brief  Respond to a press of the view's "okay" button
  #
  # This just closes the window.
  #
  method okay {} {
    set top [winfo toplevel [$m_view getWindowName]]
    destroy $top
  }

  ## @brief Retrieve the view
  #
  # @returns string
  # @retval the name of the ConfigErrorView object controlled by this 
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


