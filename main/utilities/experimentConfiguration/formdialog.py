#!/usr/bin/env python


#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   formdialog.py
# @brief  Base class for dialogs that have some form and the Ok/Cancel buttons.
# @author <fox@nscl.msu.edu>

from PyQt4 import QtGui

##
# @class FormDialog
#
#   Dialog to prompt users to enter information that describes a ring.
#   The ring dialog is a composite that contains a RingForm.  The key methods
#   other than the dialog methods is form() which obtains the form
#   associated with the dialog
#
class FormDialog(QtGui.QDialog):
    ##
    # __init__
    #   Construction
    #   *  Create/Layout the widgets.
    #   *  Connect button box signals to dialog slots
    #
    # @param form - The form containing the widgets the user fills in for the
    #               dialog.
    # @param parent - Parent widget which determines where the dialog appears.
    #
    def __init__(self, form, parent = None):
        super(FormDialog, self).__init__(parent)
        
        # Create and layout the widgets.
        
        self._form    =  form
        self._buttons =  QtGui.QDialogButtonBox(
            QtGui.QDialogButtonBox.Ok | QtGui.QDialogButtonBox.Cancel
        )
        bs  = self._buttons.buttons()
        for b in bs:
            b.setDefault(False)
            b.setAutoDefault(False)
        self.setWindowTitle('Specify Ring')
        
        layout = QtGui.QVBoxLayout()
        self.setLayout(layout)
        layout.addWidget(self._form)
        layout.addWidget(self._buttons)
        
        # Connect the button box signals to the dialog slots:
        
        self._buttons.accepted.connect(self.accept)
        self._buttons.rejected.connect(self.reject)
        
        #
        self.setModal(True)
        
    #--------------------------------------------------------------------------
    # public methods

    ##
    # form
    #   Returns the dialog form so values can be set and fished out
    #
    # @return RingForm
    #
    def form(self):
        return self._form
