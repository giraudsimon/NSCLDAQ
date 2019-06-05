#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file DataSourceUI.tcl
# @brief GUI for selecting data sources for the experiment.
# @author Ron Fox <fox@nscl.msu.edu>

package require snit
package require Tk
package provide DataSourceUI 1.0

##
# @class ChooseProvider
#
#   A Graphical user interface that allows users to choose from amongst a list
#   of data source providers.
#
#
# LAYOUT:
#   +---------------------------------------------+
#   |  +----------------------------------+-0+    |
#   |  |   list box of provider names     |sb|    |
#   ... ...                               ...   ...
#   |  |                                  |  |    |
#   |  +----------------------------------+--+    |
#   +---------------------------------------------+
#
# OPTIONS:
#   -command   - Script invoked when a provider is chosen via double click.
#   -providers - list of provider names.
#
# METHODS:
#   selected   - Returns the name of the provider that is currently highlighted
#               (empty string if none is).
#
# SUBSTITUTIONS:
#   %W    - The name of the widget that is firing the callback.
#   %P    - The selected  provider.
#
snit::widgetadaptor ChooseProvider {
    option -command [list]
    option -providers -configuremethod _setProviderList
    ##
    # constructor
    #   Get the list of data source providers and make a chooser widget.
    #   the widget is policy free (e.g. not a dialog).   It is up to the client
    #   to embed this in a dialog or not as they see fit.
    #
    # @param args - The set of option/values that configure the widget.
    #
    constructor args {
       installhull using ttk::frame
       
       listbox  $win.providers -yscrollcommand [list $win.sb set] \
            -selectmode single
       ttk::scrollbar $win.sb -orient vertical -command [list $win.providers yview]
       
       grid $win.providers $win.sb -sticky nsew
       
       $self configurelist $args
       
       bind $win.providers <<ListboxSelect>> [mymethod _onSelect]
    }
    #---------------------------------------------------------------------------
    #  Public methods.
    #
    
    ##
    # selected
    #
    # @return the selected item or [list] if nothing is selected.
    #
    method selected {} {
        set index [$win.providers index anchor]
        return [$win.providers get $index]
    }
    
    #--------------------------------------------------------------------------
    # Private methods.
    #
    
    ##
    # _onSelect
    #
    #   Called in response to the <<ListboxSelect>> virtual event.  This indicates
    #   a change in the selection.  If there's a non-null -command script
    #   perform the substitutions and invoke it at the global call level.
    #
    method _onSelect {} {
        set command $options(-command)
        if {$command ne ""} {
            set command [string map [list %W $win %P [$self selected]] $command]
            uplevel #0 $command
        }
    }
    
    ##
    # _setProviderList
    #    configuremethod for the -providers option.  The list box is emptied
    #    and filled with the list provided.  The listbox is filled in the order
    #    specified by the providers parameter.
    #
    # @param optname   - Name of the option being configured (-providers)
    # @param providers - List of providers to stuff in the listbox.
    #
    method _setProviderList {optname providers} {
        $win.providers delete 0 end
        foreach provider $providers {
            $win.providers insert end $provider
        }
        set options($optname) $providers
    }
}

##
# @class PromptDataSource
#
#  Provides a form that prompts for the data source parameters.
#  At present this is a very simple form consisting of a two column grid where
#  the left column is the descriptive name of a parameter and the right hand
#  column the parameter value.  In the future, I think parameters maybe should have
#  either types or specify the widget to use to prompt for them.
#
# OPTIONS
#   -parameters - The dict that describes the data source parameters.
# METHODS
#   getForm     - Returns a dict that descdribes the parameters with their
#                 responses integrated.
#
snit::widgetadaptor PromptDataSource {
    option -parameters -default [list] -configuremethod _layoutForm
    
    
    ##
    # @var formDescripton  - This will contain a dict that is keyed by
    #                        the parameter name from the -parameter option
    #                        and whose entry values are a 2 element list containing
    #                        *  - The prompt string
    #                        *  - The promting widget path.
    #
    variable formDescription [dict create]
    
    ##
    # constructor
    #   Creates the widget and processes the configuration.  Note that it is
    #   the configuration itself that creates the form contents.
    #   Note as well, it's probably best from the point of view of window and
    #   geometry management to set the -parameters option before managing the
    #   window.
    #
    # @param args - The list of -option value pairs.
    constructor args {
        installhull using ttk::frame
        
        $self configurelist $args
    }
    #--------------------------------------------------------------------------
    #   Public methods.
    #
    
    ##
    # getForm
    #
    #  Gets the current values of the form.
    #
    # @return  - a dict where the keys are short parameter names.  The contents
    #            of each key are a list consisting of in order
    #            *   The prompt string for the parameter.
    #            *   The path to the prompting widget for the parameter.
    #            *   The value of the widget (entry).
    #
    method getForm {} {
        set result $formDescription
        dict for {param values} $formDescription {
            set entry [lindex $values 1]
            dict lappend result $param [$entry get]
        }
        return $result
    }
    
    #---------------------------------------------------------------------------
    #  Configuration methods:
    #
    
    ##
    # _layoutForm
    #
    #   Called when the -parameters option changes.   The existing contents of
    #   the widget are destroyed and rebuilt in accordance with the description.
    #
    # @param optname     - the name of the option being configured (-parameters).
    # @param description - the new value of the option.   This is a dict of the
    #                      sort that is returned from the parameterization method
    #                      of a data source provider.  Specifically; keys are
    #                      parameter keywords and values are the prompt string.
    #
    method _layoutForm {optname description} {
        
        #  Destroy the existing widgets, and empty the formDescription
        #  variable.
        
        foreach win [winfo children $win] {
            destroy $win
        }
        set formDescription [dict create]
        
        #  Create the prompt labels and entries and lay them out while
        #  rebuilding the new formDescription dict.
        #
        set fieldno 0
        dict for {param prompt} $description {
            ttk::label $win.label$fieldno -text [lindex $prompt 0]
            ttk::entry $win.field$fieldno
            grid $win.label$fieldno -row $fieldno -column 0 -sticky e
            grid $win.field$fieldno -row $fieldno -column 1 -sticky w
            
            dict set formDescription $param [list $prompt $win.field$fieldno]
            
            incr fieldno
        }
        
        
        #  Set the option variable:
        
        set options($optname) $description
    }
    
}

#-------------------------------------------------------------------------------
#  Dialogs:  These take the forms above and integrate them with a dialog
#            that has Ok/Cancel buttons.
#            Each dialog is a pair:  A snit megawidget that displays the dialog
#            and a convenience routine that returns the output of the dialog.
#

##
# DialogWrapper
#   Wraps a widget in a dialog container this includes:
#   *  OK/Cancel buttons located below the widget.
#   *  Method to become application modal.
#   *  Delegation of all unrecognized options and methods to the contained
#      widget<methodsynopsis>
# OPTIONS
#  -form    - Sets the widget that will appear in the control area.
#  -showcancel - Determines if the widget displays the cancel button.  If
#                false, only the Ok button is displayed.
# METHODS
#  controlarea - Returns the parent that should be used when creating the
#                -form widget.
#  modal    - grabs events and blocks until either OK or Cancel is clicked.
#             Returns either Ok, Cancel or Destroyed to indicate what
#             caused the exit from modality.
#
snit::widgetadaptor DialogWrapper {
    component controlarea;                 # The wrapped widget.

    option -form -default ""  -configuremethod _setControlArea
    option -showcancel -default true   -configuremethod _showCancelConfig
    delegate option * to controlarea
    delegate method * to controlarea
    
    variable action "";               # Will contain the event that ended modality.
    
    ##
    # constructor
    #   lays out the generic shape of the dialog and fills in the
    #   action area.  The dialog is of the form:
    #
    #  +-----------------------------------+
    #  |  frame into which controlarea     | (control area)
    #  |  is put                           |
    #  +-----------------------------------+
    #  | [OK]   [Cancel]                   | (action area).
    #  +-----------------------------------+
    #
    constructor args {
        installhull using ttk::frame
        
        ttk::frame $win.controlframe -relief groove -borderwidth 2
        ttk::frame $win.actionframe
        ttk::button $win.actionframe.ok     -text Ok
        ttk::button $win.actionframe.cancel -text Cancel
        
        grid $win.actionframe.ok $win.actionframe.cancel
        grid $win.controlframe -sticky nsew
        grid $win.actionframe

        grid rowconfigure $win 0 -weight 1       
        grid columnconfigure $win 0 -weight 1       
        grid rowconfigure $win.controlframe 0 -weight 1       
        grid columnconfigure $win.controlframe 0 -weight 1       
 
        $self configurelist $args
    }
    ##
    # destructor
    #   Ensure there are no callback handlers left:
    #
    destructor {
        catch {$win.actionframe.ok     configure -command [list]}
        catch {$win.actionframe.cancel configure -command [list]}
        catch {$win bind <Destroy> [list]}
        
    }
    #-------------------------------------------------------------------------de
    #  Public methods
    
    ##
    # controlarea
    #
    # @return widget path - the parent of any -form widget.
    #
    method controlarea {} {
        return $win.controlframe
    }
    ##
    # modal
    #   Enters the modal state:
    #   *   Adds handlers for the buttons and destroy event.
    #   *   grabs events to $win
    #   *   vwaits on the action var.
    #   *   When the vwait finishes, kills off the handlers.
    #   *   Returns the contents of the action variable which will have been
    #       set by the action handler that fired.
    #
    # @return string the action that ended the wait
    #        * Ok - the Ok button was clicked.
    #        * Cancel - the Cancel button was clicked.
    #        * Destroy - The widget is being destroyed.
    #
    method modal {} {
        $win.actionframe.ok configure      -command [mymethod _setAction Ok]
        $win.actionframe.cancel configure -command  [mymethod _setAction Cancel]
        bind $win <Destroy>                         [mymethod _setAction Destroyed]
        
        # Here's the modal section.
        
        grab set $win
        vwait [myvar action]
        catch {grab release $win};         # catch in case we're being destroyed.
        
        #  Catches here in case the windows being configured have been
        #  destroyed...
        
        catch {$win.actionframe.ok configure -command [list]}
        catch {$win.actionframe.cancle configure -command [list]}
        catch {bind $win <Destroy> [list]}
        
        if {[info exists action]} {
            return $action
        } else {
            return "Destroyed";                # Destruction too far in process
        }
    }
    
    #-------------------------------------------------------------------------
    # Configuration handlers:
    
    ##
    # _setControlArea
    #
    #   Option handler for the -form configuration option.  This sets the
    #   widget that will be contained in the control area.
    #
    # @param optname - Name of the option being configured (-form)
    # @param widget  - Widget path to the object that should be pasted into the form.
    #
    # @note - The form is installed as the controlarea component so that options
    #         and methods can be delegated to it.
    #
    method _setControlArea {optname widget} {
        install controlarea using set widget
        grid $widget -in $win.controlframe -sticky nsew
        
        set options($optname) $widget
    }
    ##
    # _showCancelConfig
    #    Modify the value of the -showcancel option.
    #
    # @param optname - option name.
    # @param value   - new value.
    #
    method _showCancelConfig {optname value} {
        set old $options($optname)
        set options($optname $value)
        
        if {$old != $value} {
            if {$value} {
                grid $win.actionframe.cancel -row 0 -column 1
            } else {
                grid forget $win.actionframe.cancel
            }
        }
    }
    #---------------------------------------------------------------------------
    #  Private action handlers.
    
    ##
    # _setAction
    #   Set the action variable to a specific value.
    #
    method _setAction value {
        set action $value
    }
}
namespace eval DataSourceUI {}
##
#  DataSourceUI::getProvider
#
#   Pop up a dialog that queries the user for a provider.
#   The method returns either the name of the provider chosen or an empty
#   string if nothing was chosen.
#
# @param providers - list of providers amongst which to choose.
# @return string   - Provider or "" if no provider was chosen or the dialog was
#                    Cancelled or Destroyed.
#
proc DataSourceUI::getProvider providers {
    toplevel .datasourceprompter
    DialogWrapper .datasourceprompter.dialog
    .datasourceprompter.dialog configure \
        -form [ChooseProvider            \
            [.datasourceprompter.dialog controlarea].f -providers $providers]
    
    pack .datasourceprompter.dialog
    
    set action [.datasourceprompter.dialog modal]
    
    set result ""
    if {$action eq "Ok"} {
        set result [[.datasourceprompter.dialog controlarea].f selected]
    }
    destroy .datasourceprompter
    
    return $result
}
##
# DataSourceUI::getParameters
#
#  Popup Dialog that queries the user for the parameters associated with a
#  provider.    If the provider has a prompter of its own (package $provider_Prompter).
#  that package is loaded and ${provider}::promptParameters is called instead of
#  using our own default dialog.  This separation of packages allows the provider to
#  be free of Tk and hence testable with tcltest in a noninteractive shell (e.g.
#  Jenkins) at the cost of a bit more complexity.
#
# @param provider   - The provider name.
# @param parameters - The parameter description dict from the data source
#                     provider.
# @return dict      - If Ok is clicked, the form contents dict from PromptDataSource
#                     is returned else an empty dict.
#
proc DataSourceUI::getParameters {provider parameters} {
    
    set providerPrompter "${provider}_Prompter"

    if {[lsearch -exact [package names] $providerPrompter] != -1} {
        #
        #  The provider has a prompter of its own!
        
        package require $providerPrompter
        set params [${provider}::promptParameters]
        # puts "Dialog gave me: $params"
        return $params
    } else {
	#
	#  Special case. if there are no parameters just return
	#  an empty dict.

	if {[llength $parameters] == 0} {
	    return [dict create no-parameters [list] ]
	}
	
        #  Use the generic ugly prompter.
        #
        toplevel .parameterprompter
        set w [DialogWrapper .parameterprompter.dialog]
        $w configure \
            -form [PromptDataSource [$w controlarea].f -parameters $parameters]
        
        pack $w
        set action [$w modal]
        
        set result ""
        if {$action eq "Ok"} {
            set result [[$w controlarea].f getForm]
        }
        destroy .parameterprompter
        
        return $result
    }
}
