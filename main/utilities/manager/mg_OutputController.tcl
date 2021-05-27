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
# @file   mg_OutputController.tcl
# @brief  Controller like object for ManagerOutputModel and OutputWindow.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide ManagerOutputController 1.0
package require snit

##
# @class ManagerOutputController
#    Provides an output controller that mediates between a ManagerOutputModel
#    and a OutputWindow widget:
# OPTIONS:
#   -model   - Model object - note that we connect to all of the callback options.
#   -view    - OutputWindow or compatible object.
#
#
#  @note we only manage the model's ouptut option. Any additional callback options
#        must be handled by the client.
#
snit::type ManagerOutputController {
    option -model -configuremethod _cfgModel
    option -view
    
    constructor args {
        $self configurelist $args
    }
    
    #--------------------------------------------------------------------------
    # Configuration management
    
    ##
    # _cfgModel
    #    Called when a new model is configured.
    #    - Remove callbacks from any existing model.
    #    - Add our callbacks to the new model.
    # @param  optname - option name (-model).
    # @param value    - new proposed value.
    #
    method _cfgModel {optname value} {
        
        #  This is done in a way that the old model remains unchanged
        #  if there's an error configuring the new one (which only can happen
        #  if the model is not a ManagerOutputModel or compatible)
        #
        $value configure -output [mymethod _output]
        
        #  Now unconfigure the prior model and replace it:
        
        if {$options($optname) ne ""} {
            $options($optname) configure -outupt  [list]
        }
        set options($optname) $value
    }
    
    #--------------------------------------------------------------------------
    #  Private methods
    
    ##
    # _output
    #   Called when input is passed to us by the model;
    # @param model - model object.
    # @param txt   - Message text.
    #
    # @note we use log so that the text we output is given a timestamp.
    #
    #
    method _output {model txt} {
        $options(-view) log output $txt
    }
}
    


