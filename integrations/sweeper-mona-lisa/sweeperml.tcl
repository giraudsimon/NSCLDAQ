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

EVBC::useEventBuilder

#  Default value for glom -dt  The user can override this by setting it
#  after doing the package require:

namespace eval EventBuilderParameters {
    variable glomdt      100
    variable glomid      0x5910
    variable destring    built
    
    variable sweeperhost spdaq34.nscl.msu.edu
    variable sweeperring sweeperbuilt
    variable sweepersid  0x5900

    variable monahost    spdaq40.nscl.msu.edu
    variable monarsid    0x5a00
    variable monaring    $::tcl_platform(user)

    variable lisahost    spdaq42.nscl.msu.edu
    variable lisasid     0x5b00
    variable lisaring    $::tcl_platform(user)
}

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

    # Initialize the event builder.
    
    EVBC::initialize -restart false -glombuild true           \
	-glomdt   $::EventBuilderParameters::glomdt           \
	-destring $::EventBuilderParameters::destring         \
	-glomid   $::EventBuilderParameters::glomid

    #  Register the three sources in order for the sweeper, mona and
    #  lisa DAQs.
    set sweeperRingUri [_ringUri                              \
       $::EventBuilderParameters::sweeperhost                 \
       $::EventBuilderParameters::sweeperring                 \
    ]
			    
    EVBC::registerRingSource                                   \
	$sweeperRingUri "" $EventBuilderParameters::sweepersid \
	"Sweeper magnet data" 1                                

    set monaRingUri [_ringUri                                  \
       $::EventBuilderParameters::monahost                     \
       $::EventBuilderParameters::monaring                     \
    ]
    EVBC::registerRingSource                                   \
	$monaRingUri "" $EventBuilderParameters::monasid       \
	"MoNA data" 1

    set lisaRingUri [_ringUri                                  \
	$::EventBuilderParameters::lisahost                    \
	$::EventBuilderParameters::lisaring                    \
			]
    EVBC::registerRingSource                                   \
	$lisaRingUri "" $EventBuilderParameters::lisasid       \
	"LISA Data" 1
	
}

