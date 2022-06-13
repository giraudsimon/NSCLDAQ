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
# @file ReadoutGui.tcl
# @brief Ties the new readout GUI together into a neat bundle.
# @author Ron Fox <fox@nscl.msu.edu>

package provide ReadoutGui 1.0
package require Tk
package require snit
package require ui
package require RunstateMachine
package require eventLogBundle
package require DataSourceManager
package require DataSourceMonitor
package require DataSourceUI
package require ExpFileSystem
package require rdoCalloutsBundle;     # Auto registers.
package require ExpFileSystemConfig
package require Diagnostics
package require StateManager
package require dialogwrapper

##
# @class ProviderList
#
#  Produces a multi-column list of data source providers that are currently
#  running in a modal dialog.  The list is displayed in a ttk::treeview
#  that is used as a table with the following columns:
#  *   source id   - Id of the data source.
#  *   source type - provider name.
#  *   parameter   - Name of a parameter in the source parameterization.
#  *   value       - Value of the parameterization.
#
#  information about a source will take several lines, the first line has the
#  sourceid and type,  the remaining lines the parameterization e.g.:
#
# +-------------------------------------------------------------------+
# | source id | source type | parameter   |  value                    |
# |         1 | SSHPipe     |             |                           |
# |           |             | host        | localhost                 |
# |           |             | path        | /user/fox/readout/Readout |
# |           |             | parameters  |                           |
#  ...
#
# OPTIONS
#   -sources - the list of source dicts from the data source manager.
#
# METHODS
#   getSelected - Returns the source id of the selected data source.

snit::widgetadaptor ProviderList {
    option -sources -default [list] -configuremethod _updateDisplay
    component table

    variable columns [list {Source Id} {Source Type} Parameter Value]
    variable selectedId ""
     
    ##
    # constructor
    #   construct the object.
    #    * hull is instantiated as a ttk::frame to get the colors right
    #    * The table component is instantiated as an appropriately
    #      configured ttk::treeview.
    #    * configuration options are processed which, if -sources is specified,
    #      adds the set of sourcdes to the treeview.
    #
    constructor args {
        installhull using ttk::frame
        set f $win
        install table using ttk::treeview $f.table  \
            -xscrollcommand [list $f.hsb set] -yscrollcommand [list $f.vsb set] \
            -columns [concat hiddenid $columns] -display $columns -show headings
        foreach col $columns {
            $table heading $col -text $col
        }
        $table tag bind items <Button-1> [mymethod _selectItem %x %y]
        
        ttk::scrollbar $f.vsb -command [list $table yview] -orient vertical
        ttk::scrollbar $f.hsb -command [list $table xview] -orient horizontal
        
        grid $table $f.vsb -sticky nsew
        grid $f.hsb -sticky nsew
#       JRT postfix
#        grid $f  -sticky nsew
        
        grid rowconfigure $win 0 -weight 1
        grid rowconfigure $win 1 -weight 0
        grid columnconfigure $win 0 -weight 1
        grid columnconfigure $win 1 -weight 0

        #configure whether cols in treeview stretch on resize
        $table column #1 -stretch off -width 100
        $table column #2 -stretch off -width 100
        $table column #3 -stretch off -width 100
        $table column #4 -stretch on -width 500
        
        $self configurelist $args
    }
    #-------------------------------------------------------------------------
    # Public methods
    
    ##
    #  getSelected
    #
    # Get the selected data source.
    #
    method getSelected {} {
        return $selectedId
    }
    #-------------------------------------------------------------------------
    # Private methods
    #
    
    ##
    # _selectItem
    
    method _selectItem {x y} {
        set item [$table identify item $x $y]
        if {$item ne ""} {
            set selectedId [lindex [$table item $item -values] 0]
        } else {
            set selectedId "";    # Click not on element.
        }
    }
    #-------------------------------------------------------------------------
    # Configuration handling
    #
    
    ##
    # _updateDisplay
    #   Updates the contents of the table with current values.
    # @param optname - Name of the option that holds the configuration.
    # @param value   - new value.
    #
    method _updateDisplay {optname value} {
        set options($optname) $value
        
        $table delete [$table children {}]

        foreach source $value {
            set id   [dict get $source sourceid]
            set type [dict get $source provider]
            dict unset source sourceid
            dict unset source provider
            
            # This leaves only the parameterization in source
            
            set parent [$table insert {} end \
                -values [list $id $id $type] -tags items]
            dict for {param value} $source {
                $table insert {} end \
                    -values [list $id "" "" $param $value] -tags items
            }
            
        }
    }
}

##
# @class ProviderListDialog
#   Produces a dialog list wrapped in a dialog.
#
snit::widgetadaptor ProviderListDialog {
    
    component table
    component dialog
    
    delegate option -sources to table
    delegate method * to dialog
    
   ##
    #  constructor
    #     Construct and layout the dialog
    # @param args - configuration parameters.
    #
    constructor args {
        installhull using toplevel
        
        install dialog using DialogWrapper $win.dialog -showcancel 0
        set tableContainer [$dialog controlarea]
        install table using ProviderList $tableContainer.t
        $dialog configure -form $table
        
        grid $dialog -sticky nsew
        grid rowconfigure $win 0 -weight 1
        grid columnconfigure $win 0 -weight 1
        grid rowconfigure $dialog 0 -weight 1
        grid columnconfigure $dialog 0 -weight 1
        
        $self configurelist $args
        

    }

}
##
# @class ProviderSelectDialog
#   This is similar to ProviderList dialog however we also expose the
#   ability of the ProviderList to keep track of which source id is
#   selected.  We also make visible the cancel button which allows
#   the user to change their mind about selecting a data source.
#   Furthermore  the client must make use of the modal method directly
#   rather than the constructor calling it.
#
snit::widgetadaptor ProviderSelectDialog {
    
    component table
    component dialog
    
    delegate option -sources to table
    delegate method  getSelected to table
    delegate method * to dialog
    
   ##
    #  constructor
    #     Construct and layout the dialog
    # @param args - configuration parameters.
    #
    constructor args {
        installhull using toplevel
        
        install dialog using DialogWrapper $win.dialog -showcancel 1
        set tableContainer [$dialog controlarea]
        install table using ProviderList $tableContainer.t
        $dialog configure -form $table
        
        grid $dialog -sticky nsew
        grid rowconfigure $win 0 -weight 1
        grid columnconfigure $win 0 -weight 1
        
        $self configurelist $args
        
        
    }

}


#-----------------------------------------------------------------------------#
#
# Init button

#
# The megawidget that handles the calling of initialization routines for 
# specific data source providers.
#
snit::widget InitProvider {
  option -targets -default [list all] -configuremethod setTargets

  variable selection ;#< the target

  constructor {args} {
    $self initThemes
    $self buildGui

    $self configurelist $args

    set selection [lindex $options(-targets) 0]

  }

  method buildGui {} {
    ttk::label $win.title -text "Initialize" -anchor s \
                          -style "H1.TLabel"
    ttk::label $win.selectLabel -text "Target" -padding {8 0 8 0} \
                                -anchor e
    ttk::combobox $win.combo -height 1 -values $options(-targets) \
                             -width 12 -textvariable [myvar selection]
#    ttk::button $win.initButton -text "Init" -command [mymethod onInit]

    grid $win.title - -sticky nsew 
    grid $win.selectLabel $win.combo -sticky nsew -pady {4 4} -padx {4 4}
#    grid $win.initButton - -sticky nsew -pady {4 4} -padx {4 4}

    grid rowconfigure    $win 0 -weight 0 -minsize 20
    grid rowconfigure    $win 1 -weight 0 -minsize 20
    grid columnconfigure $win {0} -weight 0 -minsize 65
    grid columnconfigure $win {1} -weight 1 -minsize 65
  }
  
  method initThemes {} {
    ttk::style configure H1.TLabel -font {Helvetica 14}
  }

  method setTargets {opt vallist} {
    $win.combo configure -values $vallist
    $win.combo configure -height [llength $vallist]
    set options($opt) $vallist
  }

  method getSelection {} {
    return $selection
  }

} 

snit::widgetadaptor InitProviderDialog {

  component dialog
  component form 

  delegate option -targets to form 
  delegate method getSelection to form
  delegate method * to dialog

  constructor {args} {
    installhull using toplevel

    install dialog using DialogWrapper $win.dialog -showcancel 1
    set container [$dialog controlarea]

    install form using InitProvider $container.f
    $dialog configure -form $form

    grid $dialog -sticky nsew
    grid rowconfigure $win 0 -weight 1
    grid columnconfigure $win 0 -weight 1

    $self configurelist $args

  }
}


##
# @class ReadoutGuiApp
#
#   This snit::type is the application class.  It interacts with the various
#   components to create a harmoniously integrated whole (or so we hope).
#   Really what this chunk of code mostly needs to do is work with the
#   RunstateMachine singleton to ensure that the following pieces and parts
#   of the system are configured and tied in to state transitions:
#   *  Event logger.
#   *  Data Source Manager
#   *  Readout GUI megawidget.
#   *  Event file monitor (Status bar showing event file segments for current run)
#
#  We also make some adjustments to the user interface:
#
#   * Add a Data Source menu.
#   * Add Data Source->Add...
#   * Add Data Source->Delete...
#   * Add Data Source->List...
#
snit::type ReadoutGuiApp {
    component dataSources
    component readoutGui
    component stateMachine
    component dataSourceMenu
    component settingsMenu

    option -settingsfile -default ".settings.tcl"
    
    ##
    # constructor
    #    Construct the object:
    #    *  Make the ReadoutGUI megawidget
    #    *  Connect the data source manager to the run state machine
    #    *  Connect the eventLogger to the run state machine
    #    *  Make the menus for the data source manager.
    #    *  Register the things we need to save.
    #
    # @param args - not used as we have no options to be concerned with.
    #
    constructor args {
        $self configurelist $args

        install stateMachine using RunstateMachineSingleton %AUTO%
        install dataSources  using DataSourcemanagerSingleton %AUTO%
        install readoutGUI using ReadoutGUI .gui
        grid .gui -sticky nsew
        grid rowconfigure . 0 -weight 1
        grid columnconfigure . 0 -weight 1
        grid configure .gui -padx 5 -pady 5
        
        $self _createDataSourceMenu
        $self _createSettingsMenu
        $self _checkFilesystem

        
        ::EventLog::register
        ::DataSourceMonitor::register
        
        # State is saved to the stagearea root in the file .settings.tcl
        # this is a hidden file from the user's standpoint.
        
        set savedFile [file join [ExpFileSystem::getStageArea] [$self cget -settingsfile]]
        set state [StateManagerSingleton %AUTO% -file $savedFile]
        
        # Set up Run state saving.
        
        $state addStateVariable run       [mymethod _getRun]       [mymethod _setRun]
        $state addStateVariable title     [mymethod _getTitle]     [mymethod _setTitle]
        $state addStateVariable recording [mymethod _getRecording] [mymethod _setRecording]
        $state addStateVariable timedRun  [mymethod _getTimedRun]  [mymethod _setTimedRun]
        $state addStateVariable duration  [mymethod _getDuration]  [mymethod _setDuration]
        $state addStateVariable dataSources [mymethod _getSources] [mymethod _setSources]
        $state addStateVariable segmentsize [mymethod _getSegmentSize] [mymethod _setSegmentSize]
        
        
        # If the file exists restore it now (other parts of the app  have had
        # their chance by now to register state variable handlers)
        
        if {[file readable $savedFile]} {
            $state restore
        }
        
        $state destroy
        
        # Arrange for the state to be saved on transitions to Active and Halted
        # (after the run number increments e.g.).
        
        $stateMachine addCalloutBundle ReadoutGUIStateManagement
        
        # The output window can be a bit wider than default:
        
        set ow [Output::getInstance]
        $ow configure -width 90
        grid columnconfigure $ow 0 -weight 1 
        grid rowconfigure $ow 0 -weight 1
        ::DataSourceMgr::register
    }
    ##
    # slave
    #    Put the user interface into remote control (slave) mode.
    #    This just means disabling the run control GUI.
    #
    method slave {} {
    }
    ##
    # master
    #   Take the user interface out of remote control mode.
    #
    method master {} {
    }


    #--------------------------------------------------------------------------
    #
    #  Private methods:
    #
    
    ##
    # _setSegmentSie
    #   Set the run segment size during a restor
    # @param name -state variable name
    # @param value - parameter value.
    #
    method _setSegmentSize {name value} {
        DAQParameters::setEventLoggerFileSegmentSize $value
    }
    ##
    # _getSegmentSize
    #  @return the file segment size for the statemanager to store.
    #
    method _getSegmentSize {name} {
        return [DAQParameters::getEventLoggerFileSegmentSize]
    }
    
    ##
    # _getRun
    #
    #    Retrieves the current run number for the state saver.
    #
    # @param name - The state variable that will be used (run).
    # @return integer current run number.
    #
    method _getRun {name} {
        return [::ReadoutGUIPanel::getRun]
    }
    ##
    # _setRun
    #    set the run number durig a state restore:
    #
    # @param name - name of the state variable (ignored).
    # @param value - Value (new run number).
    #
    method _setRun {name value} {
        ::ReadoutGUIPanel::setRun $value
    }
    ##
    # _getTitle
    #
    # @param name - state variable name.
    # @return string - Current title
    #
    method _getTitle {name} {
        return [::ReadoutGUIPanel::getTitle]
    }
    ##
    # _setTitle
    #
    # @param name - state variable name.
    # @param value - New value for the title string.
    #
    method _setTitle {name value} {
        ::ReadoutGUIPanel::setTitle $value
    }
    ##
    # _getRecording
    #
    # @param name - state variable name for the recording flag.
    # @return boolean - stateu of the recording flag.
    #
    method _getRecording {name} {
        return [::ReadoutGUIPanel::recordData]
    }
    ##
    # _setRecording
    #
    # @param name - state variable name
    # @param value - Desired state of recording flag.
    #
    method _setRecording {name value} {
        if {$value} {
            ::ReadoutGUIPanel::recordOn
        } else {
            ::ReadoutGUIPanel::recordOff
        }
    }
    ##
    # _getTimedRun
    #
    # @param name state variable name.
    # @return bool - Flag to indicate if a run is timed.
    #
    method _getTimedRun {name} {
        return [::ReadoutGUIPanel::isTimed]
    }
    ##
    # _setTimedRun
    #
    # @param name - name of state variable.
    # @param value - Value of state variable.
    #
    method _setTimedRun {name value} {
        ::ReadoutGUIPanel::setTimed $value
    }
    ##
    # _getDuration
    #
    # @param name- name of state variable
    # @return integer - number of seconds for requested run time.
    #
    method _getDuration {name} {
        return [::ReadoutGUIPanel::getRequestedRunTime]
    }
    ##
    # _setDuration
    #
    # @param name - name of state variable.
    # @param value - number of seconds in requested run time.
    #
    method _setDuration {name value} {
        ::ReadoutGUIPanel::setRequestedRunTime $value
    }
    ##
    # _getSources
    #
    #   Returns the set of data sources.  These are just a list
    #   dicts.
    #
    # @param name - Name of state variable.
    # @return list of dicts see above.
    #
    method _getSources {name} {
        return [$dataSources sources]
    }
    ##
    # _setSources
    #    Sets the current bunch of data sources to match those described by
    #   the value parameter.
    #
    # @param name - state variable name.
    # @param value - List of dicts as returned from $dataSources sources
    #
    method _setSources {name value} {

        # create a dict that maps the source id to the index in the list
        set order [dict create]
        set index 0
        foreach sourceDict $value {
          set sid [dict get $sourceDict sourceid]
          dict set order $sid $index
          incr index
        }

        # loop through the sorted ids and look up the list index
        # for each id...THis ensures that sources are added in the order
        # of their source ids.
        set orderedIds [lsort -integer -increasing [dict keys $order]]
        foreach id $orderedIds {
            set sourceDict [lindex $value [dict get $order $id]]
            set provider   [dict get $sourceDict provider]
            
            # Remove extraneous dicts to forma pure parameterization dict.
            
            dict unset sourceDict provider
            dict unset sourceDict sourceid
            
            catch {$dataSources load $provider};   #Make sure the provider's loaded
            $dataSources addSource $provider $sourceDict
        }
	      $self _setPausability

    }
    ##
    # _createDataSourceMenu
    #
    #   Create the data source menu for the data source manger and attach it
    #   to the appropriate callbacks.
    #
    method _createDataSourceMenu {} {
        install dataSourceMenu using ::ReadoutGUIPanel::addUserMenu dataSource {Data Source}
        
        $dataSourceMenu add command -label "Add..." -command [mymethod _addProvider]
        $dataSourceMenu add command -label "Delete..." -command [mymethod _deleteProvider]
        $dataSourceMenu add separator
        $dataSourceMenu add command -label "List" -command [mymethod _listDataProviders]
        $dataSourceMenu add separator 
        $dataSourceMenu add command -label "Init" -command [mymethod _initProviders]
        
    }
    ##
    # _createSettingsMenu
    #
    #   Populate the settingsm enu which consists of the following:
    #
    #   * Event Recording... - Event logger settings.
    #
    method _createSettingsMenu {} {
        install settingsMenu using ::ReadoutGUIPanel::addUserMenu Settings Settings
        $settingsMenu add command -label  {Event Recording...} -command [mymethod _eventLoggerSettings]
        $settingsMenu add command -label {Log window...}  -command [mymethod _logWindowSettings]
    }
    #--------------------------------------------------------------------------
    # Data source menu handlers:
    #
    
    ##
    # _addProvider
    #   Work with the DataSourceUI package to prompt for a  data source and
    #   its  parameters...when one is selected and parameterized, it is added
    #   to the set of managed data sources
    #
    #   Legal only if the state is in {NotReady, Halted} and forces the state
    #   to not ready.  
    #
    method _addProvider {} {
        if {[$stateMachine getState] ni [list NotReady Halted]} {
            Diagnostics::Info "You can only add data sources when no run is ongoing"
            return
        }
        
        set provider [DataSourceUI::getProvider \
            [DataSourceManager enumerateProviders]]
        #
        # Prompt for the data source parameters only if a data source was selected:
        #
        if {$provider ne ""} {
            $dataSources stopAll
            $stateMachine transition NotReady
            
            catch {$dataSources load $provider};    # May be loaded.
            set requiredParams [$dataSources parameters $provider]
            set parameters [DataSourceUI::getParameters $provider $requiredParams]
            #
            #  Probably a cancellation on the dialg
            #
            if {$parameters eq ""} {
                return
            }
            set parameters [convertPromptedValues $parameters]
           
            $dataSources addSource $provider $parameters
            
            # If the collective system cannot pause, we need to turn off'
            # the pause button:
            
	    $self _setPausability

        }
    }
    ##
    # _listProviders
    #
    #   Pops up a little dialog that shows the data source providers that have
    #   been registered.  Also shows the status of those providers.
    #
    method _listDataProviders {} {
        set sources [$dataSources sources]
        ProviderListDialog .providers -sources $sources
	.providers modal
        grid rowconfigure . 0 -weight 1
        catch {destroy .providers};   # In case they used X not Ok.
    }
    ##
    # _deleteProvider
    #   Prompt for a provider to remove from the current list of providers.
    #   *   Illegal when the state is not in {NotReady, Halted}
    #   *   Will force the system into the NotReady state and all data sources
    #       will need to be restarted.
    #
    method _deleteProvider {} {
        if {[$stateMachine getState] ni [list NotReady Halted]} {
            Diagnostics::Info "You can only remove data sources when no run is ongoing"
            return
        }

        set sources [$dataSources sources]
        ProviderSelectDialog .providers -sources $sources
        if {[.providers modal] eq "Ok"} {
            set sid [.providers getSelected]
            
            if {$sid ne ""} {
                $dataSources stopAll
                $stateMachine transition NotReady
                $dataSources removeSource $sid
            }
        }
        catch {destroy  .providers}
	$self _setPausability

    }
    
    method _initProviders {} {
      if {[$stateMachine getState] ne "Halted"} {
        Diagnostics::Info "You can only init providers when in the Halted state."
        return
      }

      set dm [DataSourcemanagerSingleton %AUTO%]
      set srcs [$dm sources]
      set names [list all] 
      foreach src $srcs {
        set name [dict get $src provider]
        set id   [dict get $src sourceid]
        lappend names "$name:$id"
      }

      InitProviderDialog .providers -targets $names
      set action [.providers modal]
      if {$action eq "Ok"} {
        set sel [.providers getSelection] 
        if {$sel eq "all"} {
          $dm initall
        } else {

          set match [regexp -inline {^(\w+):(\d+)$} $sel]

          if {[llength $match]==3} {
            $dm init [lindex $match 2]
          } else {
            Diagnostics::Error "Unable to parse source id to initialize from selection ($sel)."
            return
          }
        }
      }

      $dm destroy
      catch {destroy  .providers}

    }

    ##
    # _checkFilesystem
    #    *  Ensure there's a stagearea.
    #    *  Ensure the stagearea has the minimal components of
    #       current, complete, experiment... if not
    #    * Create the hierarchy.
    #
    method _checkFilesystem {} {
        set stagearea [ExpFileSystem::getStageArea]
        if {![isDirOrLink $stagearea]} {
            Diagnostics::Error \
                "The stagearea '$stagearea' is not a directory or symbolic link\n Event recording will fail"
            return
        }
        #
        #  If the stage area is a link follow that linke to its target directory
        #
        if {[isLink $stagearea]} {
            set stagearea [file link $stagearea]
        }
        set stagearea [file normalize $stagearea]
        #
        #  Check for the required subdirectories:
        #
        set haveCurrent    [file isdirectory [file join $stagearea current]]
        set haveComplete   [file isdirectory [file join $stagearea complete]]
        set haveExperiment [file isdirectory [file join $stagearea experiment]]
        
        if {$haveCurrent && $haveComplete && $haveExperiment} {
            return ;                        # Hierarchy already there.
        }
        ExpFileSystem::CreateHierarchy
        
    }
    #--------------------------------------------------------------------------
    # Settings menu handler:
    #
    
    ##
    # _eventLoggerSettings
    #   Prompt for event logger settings and make it so they apply to the
    #   next start of the event logger (next begin run with recording enabled).
    #
    method _eventLoggerSettings {} {
        ::EventLog::promptParameters
    }
    ##
    # _logWindowSettings
    #
    #  Prompt for log window settings and apply immediately.
    #
    method _logWindowSettings {} {
        ::Output::promptSettings
    }
    ##
    # _setPausability
    # 
    #  Check the capabilities to determine if we are pausable.  Set the
    #  user interface accordingly.
    #
    method _setPausability {} {
	set caps [$dataSources systemCapabilities]
	set pauseable [dict get $caps canPause]
	set pauseable [expr {$pauseable ? 1 : 0}];           #normalize.
	[::RunControlSingleton::getInstance] configure -pauseable $pauseable
    }
    #--------------------------------------------------------------------------
    #  Procs
    
    proc convertPromptedValues parameters {
        set result [dict create]
        dict for {key value} $parameters {
            set realValue [lindex $value 2]
            dict set result $key $realValue
        }
        return $result
    }
    ##
    # isLink
    #   @param path - a path to check.
    #   @return bool - True if the path is a link false otherwise.
    # 
    proc isLink {path} {
        set status [catch {file link $path}]
        return [expr {!$status}]
    }
    ##
    # isDirOrLink
    #
    #  @param path - the path to check.
    #  @return bool - True if the path is a directory or a link false otherwise.
    #
    proc isDirOrLink {path} {
        if {[file isdirectory $path]} {
            return 1
        }
        return [isLink $path]
    }
}
namespace eval ::ReadoutGUIStateManagement {
    namespace export enter leave attach
}
##
# ::ReadoutGUIStateManagement::attach
#
#  No op but required bundle proc.
#
proc ::ReadoutGUIStateManagement::attach {state} {
    
}

proc ::ReadoutGUIStateManagement::precheckTransitionForErrors {from to} {
	return [list]
}
##
# ::ReadoutGUIStateManagement::leave
#   No up but required bundle proc.
#
proc ::ReadoutGUIStateManagement::leave {from to} {}
##
#  ::ReadoutGUIStateManagement::enter
#
#  Enter a new state.  If the state is in {Active, Halted}
#  The StateManagerSingleton is asked to save the state.
#
# @param from - state being left (ignored)
# @param to   - state being entered (see above)
#
proc ::ReadoutGUIStateManagement::enter {from to} {
    if {$to in [list Active Halted NotReady]} {
        set manager [StateManagerSingleton %AUTO%]
        $manager save
        $manager destroy
    }
}

