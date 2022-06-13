#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#

##
# @file DataSourceManager
# @brief Manage (model) data sources for the Readout GUI.
# @author Ron Fox
#

package provide DataSourceManager 1.0
package require snit
package require ReadoutGUIPanel


##
# @class
#   DataSourceManager
#
#  This class manages data source providers and provides mechanisms to
#  run them all together...that is it provides type methods that make it look
#  like a data source provider itself with the intersection of the capabilities
#  of all of the sources it is managing.
#
# TYPEMETHODS:
#   enumerateProviders - list the set of data sources.  These are the names of packages
#               that end in _Provider with the _Provider hacked off.
#
# METHODS:
#   load      - Load a data source provider into an instance of a data source
#               manager.
#   parameters - Get the parameterization of a data source provider.
#   capabilities - Get the capabilities of a data source provider.
#   systemCapabilities - Get the capabilities of the system.  This is the
#                        intersection of all capabilities of starte data sources.
#   addSource     - Start a data source.
#   check         - Check the liveness of all data sources.
#   stop          - Stop a single data source.
#   stopAll       - Stop all data sources.
#   begin         - Start a run in all data sources.
#   end           - end the run in all data sources.
#   pause         - Pause the run in all data sources.
#   resume        - Resume the run in all data sources.         
snit::type DataSourceManager {
    
    #---------------------------------------------------------------------------
    # Object data:
    
    variable loadedProviders [list];           # List of loaded providers.
    variable dataSources -array [list];        # Array of source info indexed by id.
    variable nextSourceId    0;                # Used to allocate unique source ids.
    variable state           inactive;         # Run state inactive, paused active
    
    #---------------------------------------------------------------------------
    # Type methods:
    #
    
    ##
    # enumerateProviders
    #    List the data source providers we know about.  The assumption is that
    #    package requiring us probably made all the packages known and therefore
    #    we can just use package names to find them.
    #
    typemethod enumerateProviders {} {
        # Ensure all known packages are known:
        
        catch {package require this-package-does-not-exist}
        
        # Get the package names for the *_Provider packages:
        
        set names [package names]
        set providerPackages [lsearch -all -inline -glob $names *_Provider]

        # Cut off the _Provider part to get the provider name:
        
        set endLength [string length _Provider]
        set providers [list]
        foreach pkgName $providerPackages {
            lappend providers [string range $pkgName 0 end-$endLength]
        }
        
        return [lsort -ascii -increasing $providers]
        
    }
    #---------------------------------------------------------------------------
    # Object methods:
    
    ##
    # load
    #
    #   Attempts to load the provider requested.
    #
    # @param name - Name of the provider (_Provider will be appended to determine
    #               the package).
    #
    # @throw - Error if the package can't be found.
    # @throw - Error if the package exists but does not define the namespace
    #          it should.
    #
    method load name {
        if {[$self _isLoaded $name]} {
            error "The $name provider is already loaded."
        }
        set package ${name}_Provider
        
        set status [catch {package require $package} msg]
        if {$status} {
            puts stderr "Loading ${name} failed: $msg\n $::errorInfo"
            error "No provider named $name could be found check the package load path : $msg : $::errorInfo"
        }
        if {![namespace exists ::$name]} {
            error "The ${name}_Provider package does not provide the ::${name}:: namespace so it cannot be a provider."
        }

        # Successful load remember the provider:
        
        lappend loadedProviders $name
        
    }

    ##
    # unload
    #
    #   The opposite of the load method. All pieces of a loaded package are to
    #   be removed. The package is forgotten and the loadedProviders are
    #   destroyed. If the named package doesn't exist, this is a no-op; it is
    #   not considered a failure.
    #
    # @param name   name of package to unload
    #
    method unload name {
      if {[$self _isLoaded $name]} {
        set package ${name}_Provider
        package forget $package

        set index [lsearch -exact $loadedProviders $name]
        set loadedProviders [lreplace $loadedProviders $index $index]
      }
    }

    ##
    # parameters
    #   Ask about the parameterization of a data source provider.
    #
    # @param name - Name of the provider (which must have been loaded).
    #
    # @return dict as from the provider's 'parameters' proc.
    #
    method parameters name {
        $self _requireLoaded $name
        return [::${name}::parameters]
    }
    ##
    # capabilities
    #   Return the capabilities of a single provider
    #
    # @param name - provider name.
    #
    # @return dict - of the provider capabilties.
    #
    method capabilities name {
        $self _requireLoaded $name
        return [::${name}::capabilities]

    }
    
    ##
    # systemCapabilities
    #
    #   Returns the system capabilities.  this is defined as the set of capabilities
    #   all running data sources have.  Thus for each element in the dict, if any element is
    #   false, the value of the system capability is false.  If a provider does
    #   not mention a capability it is assumed to not be supported as well.
    #
    # @return capabilities dict.
    #
    method systemCapabilities { } {
        #
        #  First merge the dicts of the providers:
        #  - If a capability is not yet in merged it is set as is.
        #  - If the capability is in merged it is set to the 'worst' of what's
        #    there and the provider's
        
        set merged [dict create]
        foreach sourceId [lsort -integer -increasing [array names dataSources]] {
            set provider [dict get $dataSources($sourceId) provider]
            set providerCap [::${provider}::capabilities]
            dict for {cap value} $providerCap {
                if {[dict exists $merged $cap]} {
                    set existingValue [dict get $merged $cap]
                    if {$existingValue} {
                        dict set merged $cap $value
                    }
                } else {
                    dict set merged $cap $value;    # capability not yet seen.
                }
            }
        }
        
        
        # Apply defaults of not supported.  This is done in this order
        # so that new capabilities can be added without needing to modify
        # providers that can't support it.
        
        foreach cap [list canPause runsHaveTitles runsHaveNumbers] {
            if {![dict exists $merged $cap]} {
                dict set merged $cap false
            }
        }
        return $merged
    }
    ##
    # addSource
    #   Creates an instance of a data source.
    #   - The provider must exist.
    #   - The provider's start proc is called.
    #   - Errors in the provider's start are reported unaltered back to the
    #     caller.
    #
    # @param providerName - Name of the data source provider.
    # @param params - Data source start parameters.  These are passed to
    #                 the provider without interpretation however the soruceid
    #                 part of the dict will be overwritten with a source id
    #                 allocated by the manager.
    #
    # @return integer - Source id allocated by the manager.
    #
    #
    method addSource {providerName params} {
        $self _requireLoaded $providerName
        set sourceId [$self _allocateSourceId]
        dict set params sourceid $sourceId
        
        set dataSources($sourceId)  [dict create provider $providerName {*}$params]
        return $sourceId
    }
    ##
    # removeSource
    #   Remove a data source
    #
    # @param id - id of the source to remove.
    #
    method removeSource id {
        $self stop $id ;     #checks for existence too
        array unset dataSources $id
    }
    ##
    # check
    #
    #   Return a dict with information about all of the data sources.
    #
    # @return dict - A dict whose keys are data source ids and whos values
    #                are the results of check on those data sources.
    #
    method check {} {
        
        set aggregateStatus [dict create]
        foreach sourceId [lsort -integer -increasing [array names dataSources]] {
            set provider [dict get $dataSources($sourceId) provider]
            set status [::${provider}::check $sourceId]
            dict set aggregateStatus $sourceId $status
        }
        
        return $aggregateStatus
    }
    ##
    # sources
    #
    # Return a list of data sources and their parameterizations.
    # For testing purposes the dicts returned are sorted by keys.
    #
    # @return a list of dicts of sources that are known.
    #        each dict contains the parameterization items and
    #        - sourceid - the id of the source.
    #        - provider - The provider name.
    #
    method sources {} {
        set result [list]
        foreach sourceId [lsort -integer -increasing [array names dataSources]] {
            lappend result [_SortDict $dataSources($sourceId)]
        }
        return $result
    }
    ##
    # stop
    #   Stop a data source.  Once stopped, all resources associated with that
    #   source are released as well.
    #
    # @param id - The source id.
    #
    # @throw  - It is an error to stop a source that does not exist.
    #
    method stop id {
        set sm [RunstateMachineSingleton %AUTO%]
        set rstate [$sm getState]
        $sm destroy
        
        if {$rstate eq "NotReady"} {
            return
        }
        if {[array names dataSources $id] eq ""} {
            error "There is no data source with the id $id"
        }
        
        # Stop the data source:
        
        set providerType [dict get $dataSources($id) provider]
        ::${providerType}::stop $id
        

    }
    ##
    # stopAll
    #    Stop all data sources in the order in which they were started.
    #
    method stopAll {} {
        if {![catch {$self _listOrderedSources ignore} sources]} {
            foreach id $sources {
                catch {$self stop $id}; # In case there are stopped sources.
            }
        }
        set state "inactive"
    }
    ##
    # startAll
    #   Start all data sources:
    #
    method startAll {} {
        set failures 0
        if {![catch {$self _listOrderedSources ignore} sources]} {
            foreach id $sources {
                set paramDict $dataSources($id)
                set providerType [dict get $paramDict provider]
                dict set paramDict sourceid $id
                if {[catch {::${providerType}::start $paramDict} msg]} {
                    set msg [list $msg : $::errorInfo]
                    puts stderr "Error $providerType start $msg"
                    incr failures
                    set msg [string map [list \" \\\"] $msg]
                    lappend errors [join $msg " "]
                }
            }
        }
        if {$failures > 0} {
            # standard stop does nothing if we are in Not ready so we need
            # to stop on our own:
            
            foreach id [array names dataSources] {
                set provider [dict get $dataSources($id) provider]
                catch {::${provider}::stop $id}
            }
            
            error "At least one source could not start: $errors"
        }
    }
    ##
    # Begin runs in all of the active data sources
    #
    # @param runNumber - Number of the run.
    # @param title     - Run Title.
    #
    # @throw - If there are no data sources this throiws an error.
    # @throw - It's an error to begin a run if we are not inactive.
    # @throw - Any errors from the data source begin methods are
    #          passed on without any interpretation. These result
    #          in ending the run in any started sources.
    #
    method begin {runNumber title} {
        
        # The lsort ensures that actions are in order of start.
        # since the source ids are assigned strictly increasing.
        
        set sources [$self _listOrderedSources \
            "No data sources are running so a run cannot be started."]

        # Check for the run already active:
        
        if {$state ne "inactive"} {
            error "Run is already active"
        }
        
        # In this section attempt to begin the runs in all sources.
        # If a source fails, all the sources that have been successfully begin
        # will be ended.
        #
        set startedSourceIds [list]
        foreach id $sources {
            set provider [dict get $dataSources($id) provider]
            if {[catch {::${provider}::begin $id $runNumber $title} msg]} {
                foreach sid $startedSourceIds {
                    set provider [dict get $dataSources($sid) provider]
                    ::${provider}::end $sid
                }
                error "Failed when starting $provider data source $id: $msg $::errorInfo"
            }
            lappend startedSourceIds $id
        }
        # All sources started:
        
        set state active
    }
    ##
    # end
    #   Ends the run.
    #
    # @throw - If there are no data sources.
    # @throw - If the run is not endable (state == 'inactive').
    # @throw - Any errors from the data sources.  These are accumulated into
    #          a list of pairs.  The first item in each pair is a two element
    #          list of provider name and source id.  The second item is
    #          the error message from that provider:sourceid
    #
    method end {} {
        
        set sources [$self _listOrderedSources \
            "No data sources are running so a run cannot be ended"]
        
        if {$state eq "inactive"} {
            error "Run is already halted"
        }
        
        # End all the data sources but accumulate error information
        # along the way into messages.
        
        set messages [list]
        foreach id $sources {
            set provider [dict get $dataSources($id) provider]
            if {[::${provider}::check $id]} {  ; # Don't end known dead srcs.
                if {[catch {::${provider}::end $id} msg]} {
                    lappend messages [list [list $provider $id] $msg $::errorInfo]
                }
            }
        }
        set state inactive;      # TODO: Is this really right if errors?
        #  If there were error messagse we throw them.
        # Otherwise this was a success:
        
        
        if {[llength $messages] > 0} {
            error $messages
        }
    }
    ##
    # pause
    #    Pause a run.
    #
    # @throw - No data sources.
    # @throw - State is not active.
    # @throw - not all sources are capable of a pause.
    # @throw - Errors from the provider durig the pause
    #
    method pause {} {
        set sources [$self _listOrderedSources \
            "No data sources are running so a run cannot be paused"]
       
       
        # Enumerate the invalid states:
        
        if {$state eq "inactive"} {
         error "Run is inactive and cannot be paused"
        }
        if {$state eq "paused"} {
            error "Run is already paused and cannot be paused again"
        }
        
        # Make sure all sources can pause:
        
        set systemCaps [$self systemCapabilities]
        if {![dict get $systemCaps canPause]} {
            error "Not all sources support paused runs"
        }
       
       # Now send the pause out to all the sources and collect error msgs.
       
        set messages [list]
        foreach id $sources {
            set provider [dict get $dataSources($id) provider]
            if {[catch {::${provider}::pause $id} msg]} {
                lappend messages [list [list $provider $id] $msg $::errorInfo]
            }
        }
        set state paused
        
        if {[llength $messages] > 0} {
            error $messages
        }
    }
    ##
    # resume
    #
    #  Resume a paused run.
    #
    # @throw   - If there are no data sources
    # @throw   - IF the run is not paused (this also weeds out the case that
    #             pause is not supported_/
    # @throw   - Errors from pausing the data sources are collected in a manner
    #            identical to resume and reported to the caller.
    #
    method resume {} {
       set sources [$self _listOrderedSources \
            "No data sources are running so a run cannot be paused"]
       if {$state ne "paused"} {
        error "The run is not paused and therefore cannot be resumed"
       }
   
        set messages [list]
        foreach id $sources {
            set provider [dict get $dataSources($id) provider]
            if {[catch {::${provider}::resume $id} msg]} {
                lappend messages [list [list $provider $id] $msg $::errorInfo]
            }
        }
        set state active
        
        if {[llength $messages] > 0} {
            error $messages
        }      
    }


    method init {id} {
      set sm [RunstateMachineSingleton %AUTO%]
      set runstate [$sm getState]
      $sm destroy
      if {$runstate ne "Halted"} {
        set msg "DataSourceManager::init Cannot initialize data providers "
        append msg "unless in Halted state."
        error $msg 
      }

      if {$state eq "active"} {
        set msg "DataSourceManager::init Cannot initialize while in active "
        append msg "state."
        error $msg 
      }

      if {$state eq "paused"} {
        set msg "DataSourceManager::init Cannot initialize from paused "
        append msg "state."
        error $msg 
      }

      set ids [array names dataSources]
      if {$id ni $ids} {
        error "DataSourceManager::init Source $id does not exist to initialize."
      }

      set provider [dict get $dataSources($id) provider]
      if {[catch {::${provider}::init $id} msg]} {
        error [list [list $provider $id] "$msg : $::errorInfo"]
      }

    }

    method initall {} {
      set sm [RunstateMachineSingleton %AUTO%]
      set runstate [$sm getState]
      $sm destroy
      if {$runstate ne "Halted"} {
        set msg "DataSourceManager::initall Cannot initialize data providers "
        append msg "unless in Halted state."
        error $msg 
      }

      set sources [$self _listOrderedSources \
        "DataSourceManager::initall No data sources exist."]

      if {$state eq "active"} {
        set msg "DataSourceManager::initall Cannot initialize while in active "
        append msg "state."
        error $msg 
      }

      if {$state eq "paused"} {
        set msg "DataSourceManager::initall Cannot initialize from paused "
        append msg "state."
        error $msg 
      }

      # Initialize all the data sources but accumulate error information
      # along the way into messages.

      set messages [list]
      foreach id $sources {
        set provider [dict get $dataSources($id) provider]
        if {[catch {::${provider}::init $id} msg]} {
          lappend messages [list [list $provider $id] "$msg $::errorInfo"]
        }
      }

      if {[llength $messages] > 0} {
        error $messages
      }

    }

    #--------------------------------------------------------------------------
    # Private utilities.
    
    ##
    # _isLoaded
    #
    # @param name - Name of the provider to test.
    #
    # @return bool - True if the provider is already loaded.
    #
    method _isLoaded name {
        return [expr {[lsearch -exact $loadedProviders $name] != -1}]
    }
    ##
    # _requireLoaded
    #   Complain if a provider is not loaded.
    #
    # @param name - name of the provider.
    #
    method _requireLoaded name {
       if {![$self _isLoaded $name]} {
            error "The $name provider is not loaded."
        }
         
    }
    ##
    # _allocateSourceId
    #    Allocate a per manager unique source id.
    #
    # @return integer - new source id.
    #
    method _allocateSourceId {} {
        set id $nextSourceId
        incr nextSourceId
        return $id
    }
    
    ##
    # _listOrderedSources
    #
    #   List the sources in increasing order.   This has the side effect of
    #   throwing an error if there are no sources.  It is normally used by
    #   the state transition  methods.
    #
    # @param msg   - The message to throw on error,.
    # @return list - List of source ids in order of startup.
    # @throw       - If there are no sources $msg is thrown.
    #
    method _listOrderedSources msg {
        set sources [lsort -integer -increasing [array names dataSources]]
        if {[llength $sources] == 0} {
            error $msg
        }
        return $sources
    }

    #--------------------------------------------------------------------------
    # Utility procs
    #
    
    ##
    # _SortDict
    #
    #   Returns a dict sorted by keys
    #
    # @param indict - The input dictionary.
    #
    # @return dict -same values/keys as input dict but sorted alphabetically.
    #
    proc _SortDict indict {
        set outdict [dict create]
        foreach key [lsort -ascii -increasing [dict keys $indict]] {
            dict set outdict $key [dict get $indict $key]
        }
        return $outdict
    }
}

#------------------------------------------------------------------------------
#
# Hooks to get this stuff integrated into the rest of the system.
#  - Singleton Data source manager object.
#  - Callback bundle for the RunstateMachine.
#

##
# @class DataSourceManagerSingleton
#
#   This class encapsulates a single DataSourceManager so that no matter
#  how many instantiations of the DataSourceManagerSingleton there's only one
#  underlying object.   Effectively this is a snit implementation of the
#  GOF singleton pattern.
#
snit::type DataSourcemanagerSingleton {
    component singleton
    
    delegate method * to singleton
    delegate option * to singleton
    
    typevariable actualObject ""
    
    constructor args {
        if {$actualObject eq ""} {
            set actualObject [DataSourceManager %AUTO%]
        }
        
        install singleton using set actualObject
    }
}

##
#  Bundle declaration:
#
namespace eval ::DataSourceMgr {
    namespace export leave enter attach
}

##
# ::DataSourceMgr::attach  
#
#   Called when the data source manager is attached to the run state machine.
#   does nothing for now.
#
# @param state - the current state machine state.
#
proc ::DataSourceMgr::attach {state} {}

##
# ::DataSourceMgr::enter
#
#   Called when a state is entered.  The following are done:
#   Halted -> Active : start data sources.
#   Paused -> Active : Resume data source.
#
# @param from - The state that was left.
# @param to   - The state being entered.
#
proc ::DataSourceMgr::enter {from to} {
    set mgr [DataSourcemanagerSingleton %AUTO%]
    
    if {($from eq "Halted") && ($to eq "Active")} {
        $mgr begin [::ReadoutGUIPanel::getRun] [::ReadoutGUIPanel::getTitle]
    }
    if {($from eq "Paused") && ($to eq "Active")} {
        $mgr resume
    }
    $mgr destroy
}
##
# ::DataSourceMgr::leave
#
#  Called when a state is left.  The following are done:
#
#  {Active, Paused} -> Halted : Stop the run.
#
# @param from - the state being left.
# @param to   - The state that will be entered.
# 
proc ::DataSourceMgr::leave {from to} {
    set mgr [DataSourcemanagerSingleton %AUTO%]
    
    #  If leaving NotReady for Starting...start the data sources and,
    #  if successful, schedule a transition from Starting to Halted.
    #
    if {($from eq "NotReady") && ($to eq "Starting")} {
        $mgr startAll
        
    }
    
    if {($from in [list Active Paused]) && ($to eq "Halted")} {
        $mgr end
    }
    if {($from eq "Active") && ($to eq "Paused")} {
        $mgr pause
    }
    if {($from in [list Active Paused Halted]) && ($to eq "NotReady")} {
        $mgr stopAll
    }
    
    $mgr destroy
}
##
# ::DataSourceMgr::register
#    Register our bundle with the state machine singleton.
#
proc ::DataSourceMgr::register {} {
    set mgr [RunstateMachineSingleton %AUTO%]
    $mgr addCalloutBundle DataSourceMgr
    $mgr destroy
}
##
# ::DataSourceMgr::unregister
#   remove ourselves as a callout.
#
proc ::DataSourceMgr::unregister {} {
    set mgr [RunstateMachineSingleton %AUTO%]
    $mgr removeCalloutBundle DataSourceMgr
    $mgr destroy    
}


