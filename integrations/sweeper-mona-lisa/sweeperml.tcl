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
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

#
# @file  sweeperml.tcl
# @brief Provide integration between the sweeper and MoNA LISA arrays.


package provide sweeper-mona-lisa 1.0
package require evbcallouts
package require Tk
package require snit
package require DataSourceUI;      # For dialogwrapper.

EVBC::useEventBuilder

#----------------------------------------------------------------------------
#  Widget that contains a form to let the user determine the configuration
#  run.  We allow:
#    *   mona+lisa+sweeper
#    *   mona+sweeper
#    *   lisa+sweeper
#    *   MoNA + LISA
#
# The assumption is that single system configurations are done 'differently.
#
snit::widgetadaptor SystemSelectionForm {
    option -systems [list  mona lisa sweeper]
    
    constructor args {
        installhull using ttk::frame
        
        ttk::radiobutton $win.monalisasweeper -variable [myvar options(-systems)]  \
            -value [list mona lisa sweeper] -text "MoNA + LISA + Sweeper"
        ttk::radiobutton $win.monasweeper  -variable [myvar options(-systems)]     \
            -value [list mona sweeper] -text "MoNA + Sweeper"
        ttk::radiobutton $win.lisasweeper -variable [myvar options(-systems)]      \
            -value [list lisa sweeper] -text "LISA + Sweeper"
        ttk::radiobutton $win.monalisa -variable [myvar options(-systems)]     \
            -value [list mona lisa] -text "MoNA + LISA"
        
        foreach child [list monalisasweeper monasweeper lisasweeper monalisa] {
            grid $win.$child -sticky w
        }
    }
}
namespace eval Integration {}
##
# getSystemConfiguration
#    Pop up a dialog that requests the desired system configuration.
#    Returns that configuration.   The selected configuration is used later to
#    - Determine the set of systems that will be enslaved.
#    - 
proc ::Integration::getSystemConfiguration {} {
    toplevel .sysconfig
    set d [DialogWrapper .sysconfig.dialog]
    set container [$d controlarea]
    set form [SystemSelectionForm $container.form]
    $d configure -form $form -showcancel 0
    pack $d -fill both -expand 1
    
    after 500 raise .sysconfig .
    set result [$d modal]
    
    if {$result eq "Ok"} {
        set selection [$form cget -systems]
        destroy .sysconfig
        return $selection
    } else {
        # They destroyed it -- that's an error.
        
        tk_messageBox -title "Select one" -type ok -icon error           \
            -message "You must choose a configuration and click 'Ok'"
        exit -1
    }
}
##
# makeSourceDict
#    Create the remote control data source definition gui for:
#
# @param system - Name of the system.
# @param id     - source id.
#
proc ::Integration::makeSourceDict {system id} {
    set host [set ::EventBuilderParameters::${system}host]
    set user [set ::EventBuilderParameters::${system}user]
    
    set result [dict create                                                  \
        host $host user $user sourceid $id provider RemoteGUI                \
    ]
}
##
# clearExistingSources
#   remove all the data sources from the data source manager singleton.
#
proc ::Integration::clearExistingDataSources {} {
    set dsources [DataSourcemanagerSingleton %AUTO%]
    set sources [$dsources sources]
    foreach source $sources {
	set id [dict get $source sourceid]
	$dsources removeSource $id
    }
    $dsources destroy
}
##
# setDataSources
#   Replaces the data sources read in from .settings with those we want.
#   Note that in order to do this we need to know that the 'r' command exists
#   as that's the ReadoutGui object we'll manipulate.  If it's not yet there,
#   we'll reschedule ourselves in a bit.
#
proc ::Integration::setDataSources {} {
    if {[info commands r] ne "r"} {
        
        after 100 ::Integration::setDataSources
        return
    }
    #  The GUI has been built.  Construct the list of required dicts.
    
    ::Integration::clearExistingDataSources
    
    set sources [list]
    set sid -1
    foreach system $::EventBuilderParameters::systems {
        lappend sources [::Integration::makeSourceDict $system [incr sid]]
    }
    r _setSources dataSources $sources
}

#------------------------------------------------------------------------------
#  Data associated with integration

#  Default value for glom -dt  The user can override this by setting it
#  after doing the package require:

namespace eval EventBuilderParameters {
    variable glomdt      100
    variable glomid      0x5910
    variable destring    built
    
    variable sweeperhost spdaq34.nscl.msu.edu
    variable sweeperuser sweeper
    variable sweeperring sweeper
    variable sweepersid  [list 1 2]

    variable monahost    spdaq40.nscl.msu.edu
    variable monasid    40
    variable monaring   mona
    variable monauser   $::tcl_platform(user)

    variable lisahost    spdaq42.nscl.msu.edu
    variable lisasid     42
    variable lisaring    lisa
    variable lisauser    $::tcl_platform(user)
    
    variable systems    [list]
}

wm withdraw .
set EventBuilderParameters::systems  [::Integration::getSystemConfiguration]
wm deiconify .

::Integration::setDataSources

#
# The procs live in the Integration namespace.  If this is the only thing
# ReadoutCallouts needs to do, you can just
#
#  package require sweeper-mona-lisa
#  namespace import Integration::*
#
#  and be done with it.

namespace eval Integration {
    namespace export OnStart
}
#-----------------------------------------------------------------------------
#  Executable code for the integration proper.

##
# _ringUri
#   Given a host and ring name creates the ring URI:
#
# @param host  - name of the host.
# @param ring  - name of the ring in the host.
#
proc _ringUri {host ring} {
    return tcp://$host/$ring
}

##
# OnStart
#   Should be called from OnStart to setup the event builder.
#
proc Integration::OnStart {} {
    puts "Starting combined event builder (norestart)"
    # Initialize the event builder.

    puts "Using: $EventBuilderParameters::systems"
    
    EVBC::initialize -restart false -glombuild true           \
	-glomdt   $::EventBuilderParameters::glomdt           \
	-destring $::EventBuilderParameters::destring         \
	-glomid   $::EventBuilderParameters::glomid           \
	-teering ""

    
    if {"sweeper" in $::EventBuilderParameters::systems} {
        set sweeperRingUri [_ringUri                              \
           $::EventBuilderParameters::sweeperhost                 \
           $::EventBuilderParameters::sweeperring                 \
        ]
                    
        EVBC::registerRingSource                                   \
        $sweeperRingUri "" $EventBuilderParameters::sweepersid \
        "Sweeper magnet data" 1                                
    }
    if {"mona" in $::EventBuilderParameters::systems} {
        set monaRingUri [_ringUri                                  \
           $::EventBuilderParameters::monahost                     \
           $::EventBuilderParameters::monaring                     \
        ]
        EVBC::registerRingSource                                   \
        $monaRingUri "" $EventBuilderParameters::monasid       \
        "MoNA data" 1
    }
    if {"lisa" in $::EventBuilderParameters::systems} {
        set lisaRingUri [_ringUri                                  \
        $::EventBuilderParameters::lisahost                    \
        $::EventBuilderParameters::lisaring                    \
                ]
        EVBC::registerRingSource                                   \
        $lisaRingUri "" $EventBuilderParameters::lisasid       \
        "LISA Data" 1
    }
	
}

