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
# @file zoomPrompt.tcl
# @brief Form/dialog/convenience proc to prompt for custom zoom.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide zoomPrompt 1.0
package require Tk
package require snit
package require dialogwrapper
package require textprompter

##
# @class ZoomForm
#
#   Form that prompts for a custom zoom value.
#
# OPTIONS
#   -value   - value in the entry.
#
snit::widgetadaptor ZoomForm {
    component entry
    
    option -value -default 1.0 -configuremethod _configValue
    
    ##
    # constructor
    #   *  Make a label, install an entry
    #   *  lay them out.
    #   *  The entry has a validation to ensure we get numbers that are >0.
    #   *  Configure the megawidget.
    #
    constructor args {
        installhull using ttk::frame
        #ttk::label  $win.l -text "Zoom Factor: "
        #install entry using ttk::entry $win.e -width 6 -validate focusout \
        #    -validatecommand [mymethod _validate %s]
        install entry using textprompt $win.e -text "Zoom Factor: " \
            -width 6 -validate focusout -validatecommand [mymethod _validate %s]
        
        grid $entry
        
        # Return and keypad enter are also going to force the validation
        
        bind $entry <Return> [list $entry validate]
        bind $entry <KP_Enter> [list $entry validate]
        
        $self configurelist $args
        $entry insert end $options(-value)
    }
    
    ##
    # _configValue
    #    Called when -value is configured.
    #    - Put the new value into the entry.
    #    - validate the entry (that does all the other work).
    #
    # @param optname - name of the option being configured.
    # @param value   - new proposed value.
    #
    method _configValue {optname value} {
        $entry delete 0 end
        $entry insert end $value
        $entry validate
    }
    ##
    # _validate
    #   Validates the contents of the entry.
    #   *   If the contents are not a  number > 0.0, the entry is restored
    #       from options(-value)
    #   *   If the contents are valid, options(-value) is made to be the
    #       contents of the entry.
    #
    # @param v - value of the entry.
    #
    method _validate v {
        if {![string is double -strict $v] || ($v <= 0.0)} {
            $entry delete 0 end
            $entry insert end $options(-value)
            tk_messageBox -parent $win -title "Bad zoom" -icon error -type ok \
                -message "Value must be a number > 0.0 was $v"
            return 0
        } else {
            set options(-value) $v
            return 1
        }
    }
}

##
# @class ZoomDialog
#
#   Dialog wrapper with a ZoomForm in its control area.  modal is delegated
#   to the wrapper and -value to the ZoomForm so you instantiate,
#   invoke modal and then fetch the -value on ok.
#
#   See getNewZoom for a proc that bundles this all together neatly.
#
snit::widgetadaptor ZoomDialog {
    component dialog
    component form
    
    delegate option -value to form
    delegate method modal  to dialog
    
    ##
    # constructor
    #   *   Hull is a toplevel.
    #   *   The hull just contains a dialog wrapper.
    #   *   Dialogwrapper wraps a ZoomForm.
    #   *   configure ouselves.
    #
    constructor args {
        installhull using toplevel
        wm title $win {Zoom factor}
        
        install dialog using DialogWrapper $win.d
        set top [$dialog controlarea]
        install form using ZoomForm $top.f
        $dialog configure -form $form
        
        pack $dialog -fill both -expand 1
    }
}

##
# getNewZoom
#   Convenienc proc to return a new zoom factor.
#   - Create a new ZoomDialog
#   - make it modal.
#   - Return the result to the user.
#
# @return double - New zoom factor.
# @retval  ""    - no new zoom factor was chosen.
#
proc getNewZoom {} {
    set result ""
    if {![winfo exists .zoomprompt]} {
        ZoomDialog .zoomprompt
        set reply [.zoomprompt modal]
        
        if {$reply eq "Ok"} {
            set result [.zoomprompt cget -value]
        }
        destroy .zoomprompt
    }
    return $result
}
