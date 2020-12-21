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
# @file textprompter.tcl
# @brief Provides a megawidget and dialog to prompt for text strings

# @author Ron Fox <fox@nscl.msu.edu>
#
package provide textprompter 1.0
package require Tk
package require snit
package require dialogwrapper

##
# @class textprompt
#    This is a labeled text widget.
# OPTIONS
#   Everything entry supports and:
# -text - contents of the text label that is associated with the prompter.
# -image  - Image in the label prompting for the text.
# -compound - says how to resolve the presence of both an image an text.
# -promptjustify - how the prompt label is justified.
# METHODS:
#    All methods supported by an entry widget.
#
snit::widgetadaptor textprompt {
    component entry;                # ttk::entry
    component label;                # ttk::label
    
    delegate option -text to label
    delegate option -image to label
    delegate option -compound to label
    delegate option -promptjustfy to label as -justify
    
    delegate option * to entry
    delegate method * to entry
    
    constructor args {
        installhull using ttk::frame
        
        install entry using ttk::entry $win.entry -takefocus 0
        install label using ttk::label $win.label
        
        grid $label $entry -sticky nsew
        grid rowconfigure $win 0 -weight 1
        
        $self configurelist $args
        
    }

}
##
# textprompt_dialog
#    Wraps textprompt up in a dialog and returns the value of the text
#    string if Ok was clicked.
#
# @param  topname - name of the top level widget that will be created.
# @param title    - title of the toplevel widget containing the dialog.
# @param options - passed as options to the textprompt dialog
# @return string - The value of the entry if Ok clicked.
# @retval ""     - if Ok was not clicked to dismiss the dialog.
#
proc textprompt_dialog {topname title args} {
    toplevel $topname
    wm title $topname $title
    set w [DialogWrapper $topname.wrapper]
    set container [$w controlarea]
    set form [textprompt $container.form {*}$args]
    $w configure -form $form
    pack $w -fill both -expand 1
    
    set choice [$w modal]
    if {$choice eq "Ok"} {
        set result [$form get]
    } else {
        set result ""
    }
    destroy $topname
    
    return $result
}
    
