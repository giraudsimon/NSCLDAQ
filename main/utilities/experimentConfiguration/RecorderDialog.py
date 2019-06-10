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
# @file   RecorderDialog.py
# @brief  Prompts for information about where to run event recorders
# @author <fox@nscl.msu.edu>

from PyQt4 import QtGui, QtCore
from nscldaq.expconfiguration import formdialog
import sys
import os

##
# @class RecorderSpec
#
#   Provides a form that prompts/displays information about an event recorder.
#
# LAYOUT:
#   +----------------------------------------------+
#   |  Data Source:     [ Ring selector ]          |
#   |  Directory Root:  [ path          ] [Browse] |
#   +----------------------------------------------+
#
# SIGNALS:
#      (none)
#
# SLOTS:
#
#   chosePath - Graphically choose the directory path.
#
# PROPERTIES (have setter/getters):
#
#   *  rings  - List of ring names
#   *  ring   - Ring currently selected.
#   *  path   - Directory path
#
# ENCAPSULATED BEHAVIOR:
#
#   The Browse button allows the user to graphically select a directory
#   via a QtGui.QFileDialog.getExistingDirectory dialog.
#
class RecorderSpec(QtGui.QFrame):
    
    ##
    # constructor
    #   *  Define the widgets,
    #   *  Lay them out on a grid (the brows button scotches the use of
    #      a form layout)
    #   *  Connect the Browse button to a method that choose the path.
    #
    # @param parent - widget's parent widget.
    def __init__(self, parent=None):
        super(RecorderSpec, self).__init__(parent)
        
        #  Create the widgets:
        
        dataSourceLabel          = QtGui.QLabel('Data Source:', self)
        self._dataSourceSelector = QtGui.QComboBox(self)
        
        pathLabel          = QtGui.QLabel('Directory Root:', self)
        self._pathEditor   = QtGui.QLineEdit(self)
        pathBrowseButton   = QtGui.QPushButton('Browse...', self)
        
        # Layout the widgets:
        
        self._layout       = QtGui.QGridLayout(self)
        self.setLayout(self._layout)
        
        self._layout.addWidget(dataSourceLabel, 0, 0)
        self._layout.addWidget(self._dataSourceSelector, 0, 1)
        
        self._layout.addWidget(pathLabel, 1, 0)
        self._layout.addWidget(self._pathEditor, 1, 1)
        self._layout.addWidget(pathBrowseButton, 1, 2)
        
        # TODO: Connect pathBrowseButton's click signal to
        #      choosePath
        
        pathBrowseButton.clicked.connect(self.choosePath)
    
    #-------------------------------------------------------------------
    #  Slots:
    
    ##
    # choosePath
    #    Pop up a directory chooser and, if the user selects
    #    a directroy set it as the contents of self._pathEditor.
    #
    def choosePath(self):
        dirname = QtGui.QFileDialog.getExistingDirectory(
            self, 'Choose where event data are recorded', os.getcwd()
        )
        if dirname.size() > 0:
            self._pathEditor.setText(dirname)
    
    #------------------------------------------------------------------------
    # Properties:
    #
    #   *  rings  - List of ring names
    #   *  ring   - Ring currently selected.   
    #   *  path   - Directory path

    ##
    # rings
    #  @return - list of defined ring names.
    #
    def rings(self):
        nRings = self._dataSourceSelector.count()
        ringList = list()
        
        for i in range(0,nRings):
            ringList.append(self._dataSourceSelector.itemText(i))
        
        return ringList
    ##
    # setRings
    #   Set the selectable items int he combobox.
    #
    # @param rings - Iterable containing the list of rings the combobox
    #                can select from.
    #
    # @ note - Rings are unique only within a host It's recommended you use
    #          either the ring URI or e.g. ring@hostname for the ring names.
    def setRings(self, rings):
        # First clear the selections:
        
        self._dataSourceSelector.clear()
        
        # Now populate it an item at a atime.
        
        for ring in rings:
            self._dataSourceSelector.addItem(ring)
        
    ##
    # ring
    #
    # @return - Text of the current ring in the combobox.
    #
    def ring(self):
        return self._dataSourceSelector.currentText()
    
    ##
    # setRing
    #    Force the selection of a specific ring.
    #
    # @param ring - The name of the ring to select; must be in the
    #               list returned by rings()
    #
    def setRing(self, ring):
        if not (ring in self.rings()):
            raise RuntimeError('Ring is not in combobox: %s' % (ring))
        
        self._dataSourceSelector.setEditText(ring)
    
    ##
    # path
    #   Return the current contents of the path line edit.
    #
    # @return text in current path.
    #
    def path(self):
        return self._pathEditor.text()
    ##
    # setPath
    #   set the value of the path editor:
    #
    # @param path - new path.
    #
    def setPath(self, path):
        self._pathEditor.setText(path)            
        
    
##
# @class RecorderDialog
#
# Full dialog that uses a RecorderSpec for its form.
#
class RecorderDialog(formdialog.FormDialog):
    def __init__(self, parent = None):
        form = RecorderSpec()
        super(RecorderDialog, self).__init__(form, parent)
    
    
        
if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    d   = RecorderDialog()
    d.form().setRings(['fox@charlie', '0400x@spdaq20', 'caesar@u6pc2'])
    if d.exec_() == QtGui.QDialog.Accepted:
        print('accepted')
        print('Ring: %s' % (d.form().ring()))
        print('Path: %s' % (d.form().path()))
    else:
        print('cancelled')