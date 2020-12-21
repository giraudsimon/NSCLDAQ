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
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file evbparameters.tcl
# @brief Prompt for event builder parameters.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide evbparameters 1.0


package require Tk
package require snit
package require evbui
package require textprompter



##
# @class GetEvbParameters
#    Megawidget for the event builder parameters.  This consist
#    of frames containing an EVBC::glomparams for the event building
#    parameter, an EVBC::destring, to configure the destination ring and
#    an entry into which the event source id can be configured.
#
#  These are laid out horizontally with an ok button to accept the
#  results. All options available to the components are exported along with
#  -sourcid which retrieves the value of the sourceid entry.
#
# OPTIONS:
#   -policy   - Timestamp assignment policy.
#   -build    - Build checkbox.
#   -dt       - Glomming interval.
#   -ring     - Name of output ring  buffer.
#   -sourceid - The source id.
#   -command  - Command associated with the Ok button.
#
snit::widgetadaptor GetEvbParameters {
    component glom
    component ring
    component sid
    component ok
    
    delegate option -policy to glom
    delegate option -build  to glom
    delegate option -dt     to glom
    
    delegate option -ring   to ring
    delegate option -record to ring
    
    delegate option -command to ok
    
    option -sourceid 
    
    ##
    # constructor
    #  Put together the user interface.
    #
    # @param args -configuration params.
    #
    
    constructor args {
        installhull using ttk::frame
        install glom using ::EVBC::glomparams $win.glom
        install ring using ::EVBC::destring   $win.oring
        textprompt $win.id -text "Output Source Id: " \
            -textvariable [myvar options(-sourceid)]
        #ttk::label $win.idlabel -text "Output Source Id: "
        #ttk::entry $win.id      -textvariable [myvar options(-sourceid)]
        
        install ok using ttk::button $win.ok -text Ok
        
        grid $glom $ring  $win.id
        grid $win.ok
        
        $self configurelist $args
        
        $self configure -build 1
        $self configure -record 1
        $self configure -policy latest
    }
}
