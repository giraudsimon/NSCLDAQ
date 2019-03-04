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
# @file latePopup.tcl
# @brief Provide non-modal popup display of late statistics.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide EVB::LatePopup 1.0
package require Tk
package require EVB::Late

 ##
 # @class LateWindow
 #    This is a widget adaptor for a toplevel widget which displays
 #    data late statistics.   Normally this will be managed by
 #    a latePopup which will create it as needed when late statistics change.
 #
 #   On the whole this is pretty simple, we are a top level that contains the
 #   lateFragments widget..all methods get delegated to that widget.
 #
 snit::widgetadaptor EVB::LateWindow {
    component contents
    
    delegate method * to contents
    delegate option * to contents
    
    ##
    # constructor
    #   Bind the hull to a toplevel and install the lateFragments widget
    #
    constructor args {
        installhull using toplevel
        install contents using EVB::lateFragments $win.contents
        
        pack $contents -fill both -expand 1;           # let it grow with the toplevel.
        
        $self configurelist $args
        
        set me $::tcl_platform(user)
        wm title $win "Late frags $me"
    }
    
 }
 
 ##
 # @class LatePopup
 #    A class that manages a late popup.  For the most part it intercepts
 #    the calls/configurations for the lateFragments widget in an EVB::LateWindow.
 #    if the window does not exist, it is created before the option/command is
 #    relayed.
 #
 snit::type EVB::LatePopup {
    option -count  -configuremethod _setOption
    option -worst  -configuremethod _setOption
    
    ##
    # source
    #
    #  Create the late window if needed and relay this to its
    #  source command
    #
    method source {id count late} {
        set name [$self _makeIfNeeded]
        $name source $id $count $late
    }
    ##
    # clear
    #   Create the late window if needed and relay this to its
    #   clear command.
    #
    method clear {} {
        set name [$self _makeIfNeeded]
        $name clear
    }
    ##
    # _setOption
    #    If necessary create the late window and relay the opton
    #    setting.  We cache the option locally so that cget doesn't need
    #    to bother/require the widget.
    #
    method _setOption {optname value} {
        set name [$self _makeIfNeeded]
        $name configure $optname $value
        set options($optname) $value
    }
    ##
    # _toplevel
    #
    # Compute the toplevel name from our object name
    #
    # @return widget name
    #
    method _toplevel {} {
        return [string map [list :: _] ".late$self"]    
    }
    ##
    # _makeIfNeeded
    #    If necessary crate a new lateWindow
    #
    # @return widget name
    #
    method _makeIfNeeded {} {
        set name [$self _toplevel]
        if {$name ni [winfo children .]} {
    
            EVB::LateWindow $name -count $options(-count) -worst $options(-worst)
        }
        return $name
    }
 }