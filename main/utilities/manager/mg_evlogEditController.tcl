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
# @file   mg_evlogEditController.tcl
# @brief  mediate between an event log edit model and an event log edit view.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide evlogEditController 1.0
package require evlogeditmodel
package require evLogEditViews
package require snit

##
# @class evlogEditController
#    This class is a controller in the sense of an MVC pattern. It mediates
#    between a view and a model for event log definitions.
#    - On construction it loads the view with data from the model and
#    - It responds to save commands by replacing the database event log
#      definitions with those in the view.
#    - It responds to the cancel command by reloading the view from the database.
#
# OPTIONS:
#   -  -model - evlogEditModel or compatible object.  Supplies access to the data.
#   -  -view  - evlogEditorView or compatible object. Supplies presentation of
#               the data and UI controls to operate on it.
#
snit::type evlogEditController {
    option -model -default "" -configuremethod  _cfgModel
    option -view  -default "" -configuremethod _cfgView
    
    constructor {args} {
        $self configurelist $args
    }
    #---------------------------------------------------------------------------
    #  Configuration processing
    
    ##
    # _cfgModel
    #    If there is a view, it must be updated from the new model.
    #
    # @param optname - name of the option that's being configured.
    # @param optval  - option value.
    #
    method _cfgModel  {optname optval} {
        if {($optval ne "")  && ($options(-view) ne "")} {
            set v $options(-view)
            $v configure -containers [$optval listContainers] \
                -data [$optval listEventLogs]
        }
        
        set options($optname) $optval
    }
    ##
    # _cfgView
    #    If there's a prior view we need to unregister our callbacks because
    #    the client may not destroy it.
    #    For a non null view we must:
    #       -  Register our callbacks.
    #    If there's a new view and a non null model:
    #       -  Update the view with the model
    #
    # @param optname - name of the option being modified.
    # @param view    - new view.
    #
    method _cfgView {optname view} {
        #
        #   Kill off callbacks on the old view.
        #
        if {$options(-view) ne ""} {
            $options(-view) configure -savecommand [list] -cancelcommand [list]
        }
        
        if {$view ne ""} {
            $view configure                         \
                -savecommand [mymethod _onSave]     \
                -cancelcommand [mymethod _onCancel]
            if {$options(-model) ne ""} {
                set m $options(-model)
                $view configure \
                    -containers [$m listContainers] -data [$m listEventLogs]
            }
        }
        
        set options($optname) $view
    }
    #------------------------------------------------------------------------
    #  Event handling.
    #
    
    ##
    #  _onSave
    #    Called to save the current configuration.  If
    #    there's a non-empty model we save the definitions that are in the
    #    view to the model.
    #
    # @param currently defined data.
    #
    method _onSave {data} {
        if {$options(-model) ne ""} {
            $options(-model) updateEventLogs $data
        }
    }
    ##
    # _onCancel
    #    Called to cancel all edits in the view.  We reload the view from
    #    current data.
    #
    method _onCancel {} {
        if {$options(-model) ne ""} {
            $options(-view) configure                           \
                -containers [$options(-model) listContainers]   \
                -data       [$options(-model) listEventLogs]    
        }
    }
    
}

    
