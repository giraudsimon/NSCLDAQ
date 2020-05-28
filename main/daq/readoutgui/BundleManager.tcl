#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file  BundleManager.tcl
# @brief Manage callback bundles.
# @author Ron Fox <fox@nscl.msu.edu>
#

#  This responsibility is being separated from the RunStateMachine as part of
#  daqdev/NSCLDAQ#711 - adding more bundle calls for master/slave state
#  changes which require bundle method invocations from outside of the
#  run state machine.

##
#  A bit about a bundle.  A bundle is a namespace that has several
#  mandatory, and now optional exports. The mandatory export are:
#
#   *  attach - called when a bundle is added to the list of bundles.
#   *  enter  - called when a new state is being entered.
#   *  leave  - Called when an old state is being left (prior to enter.)
#
# The optional exports are
#   * precheckTransitionForErrors - called to precheck a state transition.
#                     takes from and to states.
#   * remotecontrol - called when a remote control state change happened.
#                     this is new as of daqdev/NSCLDAQ#711
#
#   Each of these procs must be defined with the required number of parameters.
#
#   *  attach - one parameter, the current state.
#   *  enter  - Two parameters, prior state and state being entered.
#   *  leave  - Two parameters,  current state and state about to be entered.
#   *  remotecontrol - Two parameters, what happened and a node name.
#
#

package provide Bundles 1.0
package require snit


##
# @class BundleManager
#
#   Manage bundles  and call methods in them.
#
snit::type BundleManager {
    # Class level data:
    
    # Singleton instance:
    
    typevariable instance ""
    
    # Array indexed by required callback name contents are the number of parameters the
    # callback must accept.
    
    typevariable requiredExports -array [list \
        enter 2 leave 2 attach 1]
    
    # Same for optional exports:
    
    typevariable optionalExports -array [list \
        precheckTransitionForErrors 2        \
        remotecontrol 2
    ]
    
    #  Object level data - in theory, there can be more than one bundle
    #  manager, in practice, there's only a singleton.
    
    #  List of currently registered bundle namespaces.
    
    variable callouts [list]
    
    #------------------------------------------------------------------------
    # class methods:
    
    ##
    # getInstance
    #   What makes this a singleton:
    typemethod getInstance {} {
        if {$instance eq ""} {
            set instance [BundleManager %AUTO%]
        }
        return $instance
    }
    
    
    #-----------------------------------------------------------------------
    #  Object methods.
    
    ##
    #  List the callout bundles:
    #
    # @return list of namespace names.
    #
    method listCalloutBundles {} {
        return $callouts
    }
    ##
    # addCalloutBundle
    #
    #   Adds a new callout bundle to the state machine.  To be added (preconditions);
    #   *   The stated namespace must exist.
    #   *   The stated namespace must not already be registered.
    #   *   The stated namespace must export the attach, enter,leave procs.
    #   *   The stated namespace's attach, enter, leave procs must have the
    #       correct number of parameters.
    #
    # @param name - Name of the bundle to add.  This is assumed to be relative to
    #               the global namespace e.g. junk -> ::junk
    # @param state - current run state
    # @param where - If supplied, the bundle is added prior to the specified existing
    #                bundle, if not it's appended.
    # @throws if duplicate bundle.
    # @throws if bundle doesn't have 'good' methods.
    # @note When successful (postconditions):
    #   *  The bundle name is added to the callouts variable.
    #   *  A namespace ensemble is generated for that bundle.
    #
    method addCalloutBundle {name state {where ""}} {
        if {![namespace exists ::$name]} {
            error "No such bundle $name"
        }
        if {[lsearch -exact $callouts $name] != -1} {
            error "$name is already registered as a callback bundle"
        }
        $self _checkRequiredExports $name
        $self _checkRequiredParams $name
        $self _checkOptionalExports $name
        

        #  Now we can make a namespace ensemble from the bundle and
        #  add it to the callout list.
        
        namespace eval ::${name} {
            namespace ensemble create
        }
        
        if {$where eq "" } {
            lappend callouts $name
        } else {
            set idx [lsearch -exact $callouts $where]
            if {$idx == -1} {
                error "Attempt to register callout bundle $name before $where which does not exist"
            }
            set callouts [linsert $callouts $idx $name]
        }
        
        # Finally invoke the attach method:
        
        ::$name attach $state
    }
    ##
    # removeCalloutBundle
    #
    #   Remove a callout bundle:
    #   * Ensure the bundle is registered.
    #   * Remove it from the callouts list.
    #   * Undefine its namespace ensemble.
    #
    # @param name - Name of namespace that holds the bundle methods.
    #
    method removeCalloutBundle name {
        set index [lsearch -exact $callouts $name]
        if {$index == -1} {
            error "$name has not been registered"
        }
        set callouts [lreplace $callouts $index $index]
        rename ::${name} {}
    }
    ##
    #  invoke
    #    Perform a callback in the list of callbacks.
    #
    # @param method - Name of the namespace callback method (e.g. enter)
    # @param args     - Arguments to hand to the callback method
    #
    #  daqdev/NSCLDAQ#659 - we keep track of the successful callbacks.
    #  If there is a callback failure:
    #   - Call the failure method on each of the bundles passing the same
    #     args we got (normally this will be from/to)
    #   - re-signal the failure along with the original traceback.
    #   - any failures of the failure method are just absorbed and dropped.
    #
    method invoke {method args} {
        set succeeded [list]
        foreach cb $callouts {
            if {[catch {_callBundleMethodIfExists $cb $method {*}$args} msg]} {
                set traceback $::errorInfo
                foreach s [lreverse $succeeded] {
                    catch {_callBundleMethodIfExists $s failure {*}$args} 
                }
                error "$msg from $traceback"
            }
            lappend succeeded $cb
        }
    }
    ##
    # precheck
    #   Precheck is a bit special so a separate method.  Specifically we need
    #   to glom together the results of the precheck call
    # @param from - current state
    # @param to - next state
    method precheck {from to}  {
      set errors [list]
      foreach cb $callouts {
        set error [_callBundleMethodIfExists $cb precheckTransitionForErrors  $from $to]
        if {$error ne {}} {
            lappend errors [list $cb $error]
        }
      }
      return $errors
    }

    #---------------------------------------------------------------------------
    #  Private:
    
    # _checkRequiredExports
    #
    #   Checks that a namespace has the required exports for a callback bundle.
    #   Throws an error if not.
    #
    # @param name - Name of the namespace to check
    # @throw If one or more exports is missing.
    #
    method _checkRequiredExports name {
        
        # Make a list of missing exported procs.  The only way to figure out which
        # procs are exported from a namespace is to do a namespace export in the
        # context of the namespace itself:
        
        set exports [namespace eval ::${name} { namespace export}]
        set missingProcs [list]
        
        #
        #  The sort below is done to make the output repeatable/predictble
        #  and hence testable.
        #
        foreach proc [lsort -ascii -increasing [array names requiredExports]] {
            if {[lsearch -exact $exports $proc] == -1} {
                lappend missingProcs $proc
            }
        }
        if {[llength $missingProcs] > 0} {
            set missingList [join $missingProcs ", "]
            error "$name is missing the following exported procs: $missingList"
        }
    }
    ##
    # _checkOptionalExports
    #    Checks that the optional exports of a namespace, if they exist, have the
    #    right parameter counts.
    #
    #  @param name - name of the namespace that contains the bundle.
    #  @throws error if there exists an optional method that has the wrong parameterization.
    #
    method _checkOptionalExports {name} {
        set exports [namespace eval ::${name} { namespace export}] ; #only way I know to list the exports.
        foreach proc [lsort -ascii -increasing [array names optionalExports]]  {
            if {$proc in $exports} {
                set req [$self _checkParamCount ::${name}::${proc} $optionalExports($proc)]
                if {$req ne ""} {
                    error "::${name}::{$proc} requires $req parameters, should require $optionalExports($proc)"
                }
            }
        }
        # Survived so it's ok.
    }
    
    ##
    # _checkRequiredParams
    #
    #  Checks that a namespace that is being proposed as a callback bundle
    #  has the right number of parameters for each of the required exported procs.
    #
    # @param name - Path to the namespace relative to ::
    #
    # @throw If one or more required exports has the wrong argument signature.
    #
    method _checkRequiredParams name {
        
        set badProcs [list]
        set actualArgs [list]
        set requiredArgs [list]
        foreach proc [lsort -ascii -increasing [array names requiredExports]] {
            set params [$self _checkParamCount ::${name}::${proc} $requiredExports($proc)]
            if {$params ne ""} {
                lappend badProcs     $proc
                lappend actualArgs   $params
                lappend requiredArgs $requiredExports($proc)
            }
        }
        if {[llength $badProcs] > 0} {
            foreach proc $badProcs required $requiredArgs actual $actualArgs {
                lappend badList       ${proc}(${actual})
                lappend requiredList  ${proc}(${required})
            }
            set badList [join $badList ", "]
            set requiredList [join $requiredList ", "]
            error "$name has interface procs with the wrong number of params: $badList"
        }
    }
    ##
    #  _checkParamCount
    #    For a given fully qualified proc that must have a specific numger
    #    of parameters, determine if it has the required number of parameters:
    #
    # @param procname - name of the proc.
    # @param nargs    - required argument count.
    # @return mixed:
    # @retval ""   - match.
    # @retval integer - actual number of args if not ok
    #
    method _checkParamCount {procname nargs} {
        set params [llength [info args $procname]]
        if {$params != $nargs} {
            return $nargs
        }
        return ""
    }
        
    
    ##
    # _callBundleMethodIfExists
    #    If the specified method of the specified bundle exists it is called.
    #    This is used to allow for optional methods see e.g.
    #    issue: daqdev/NSCLDAQ#659
    #
    # @param bundle     - bundle namespace name.
    # @param methodname - Name of the bundle method
    # @param args       - args to pass.
    # @return - whatever the callback returns if it exists, "" if not.
    #
    #   @note if ::$bundle::$methodname exists its' called:
    #            ::$bundle::$methodname {*}$args
    #
    proc _callBundleMethodIfExists {bundle methodname args} {
        set qualifiedProc ::${bundle}::${methodname}
        if {[info procs $qualifiedProc] ne ""}  {
            return [$qualifiedProc {*}$args]
        } else {
            return ""
        }
    }
    
}
    
