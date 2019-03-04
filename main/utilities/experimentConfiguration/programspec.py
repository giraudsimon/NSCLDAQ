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
# @file   programspec.py
# @brief  Forms and dialogs to specify various types of programs.
# @author <fox@nscl.msu.edu>

from PyQt4 import QtGui, QtCore
from nscldaq.expconfiguration import formdialog
import sys
import os

##
# @class ProgramSpec
#     Contains a prompter that defines a program.
#     A program consists of the following elements:
#     *   A file system path to the program (file chooser).
#     *   A working directory  in which the program runs (directory chooser).
#     *   The id of the host in which the program runs (combobox with host names).
#     *   A list of program parameters.
#     *   A line edit for adding program parameters to the list.
#
#
# LAYOUT:
#    +-------------------------------------------------+
#    | Program Path: [                 ] (Browse...)   |
#    | Working dir:  [                 ] (Browse...)   |
#    | Host:         [Host combobox    ]               |
#    |Program Args:                                    |
#    | +--------------------+                          |
#    | |                    |                          |
#    | ...                  ...                        |
#    | +--------------------+                          |
#    | [                  ]                            |
#    +-------------------------------------------------+
#
# METHODS:
#   *   setPath   - Set the value of the path.
#   *   path      - Get the path value.
#   *   setWd     - Set the value of the working directory.
#   *   wd        - Get the value of the working directory.
#   *   setHosts  - Set the legal hosts.
#   *   setHost   - Set the value of the host box.
#   *   host      - Get the host value.
#   *   setArgs   - Set the list of program arguments.
#   *   args      - Returns the list of program arguments.
#
#
class ProgramSpec(QtGui.QFrame):
    
        def __init__(self, parent = None):
            super(ProgramSpec, self).__init__(parent)
            
            #  Use a vertical box layout for the form:
            
            layout = QtGui.QGridLayout()
            self.setLayout(layout)
            
            #  Instantiate and layout the widgets
            
            layout.addWidget(QtGui.QLabel('* Program Path:', self), 0, 0)
            self._path = QtGui.QLineEdit(self)
            self._path.setFixedWidth(300)
            layout.addWidget(self._path, 0, 1)
            self._pathBrowse = QtGui.QPushButton('Browse...')
            layout.addWidget(self._pathBrowse, 0, 2)
            
            layout.addWidget(QtGui.QLabel('* Working dir:', self), 1, 0)
            self._wd = QtGui.QLineEdit(self)
            self._wd.setFixedWidth(300)
            layout.addWidget(self._wd, 1, 1)
            self._wdBrowse = QtGui.QPushButton('Browse...')
            layout.addWidget(self._wdBrowse, 1, 2)
            
            layout.addWidget(QtGui.QLabel('* Host: ', self), 2, 0)
            self._host = QtGui.QComboBox(self)
            layout.addWidget(self._host, 2,1)
            
            layout.addWidget(QtGui.QLabel('Program Args:'), 3, 0)
            self._args  = QtGui.QListWidget(self)
            layout.addWidget(self._args, 4, 0, 1, 3)
            self._arg   = QtGui.QLineEdit(parent)
            layout.addWidget(self._arg, 5, 0, 1, 3)
            
            #  Connect widget signals we want to react to (browse buttons).
            
            self._pathBrowse.clicked.connect(self._browseProgramPath)
            self._wdBrowse.clicked.connect(self._browseWd)
            self._arg.editingFinished.connect(self._addItem)
            self._args.itemDoubleClicked.connect(self._deleteItem)
            
            self._arg.installEventFilter(self)
        
        
        ##
        #  eventFilter
        #     This event filter allows Return/Enters in the argument line edit
        #     to
        #      *   Add the element to the args list.
        #      *   Not invoke the default (OK) button.
        #
        # @param object - The object emitting the event.
        # @param event  - The Event being emitted.
        # 
        def eventFilter(self, object, event):
            if (event.type() == QtCore.QEvent.KeyPress):
                if event.key() == QtCore.Qt.Key_Return or event.key() == QtCore.Qt.Key_Enter:
                    self._arg.editingFinished.emit()
                    self._arg.clear()
                    return True
            return False
        ##
        # _deleteItem
        #   Connected to the list item double clicked signal, removes the item
        #   that was double clicked.
        #
        # @param item - The item object that was double clicked
        #
        def _deleteItem(self, item):
            self._args.takeItem(self._args.row(item))
        
        ##
        #  _addItem
        #     Add the contents of the text widget to the list:
        #
        def _addItem(self):
            self._args.addItem(self._arg.text())
        ##
        # _browseProgramPath
        #   Respond to a click of the path's browse button by getting a filename
        #   fromt he user via a filesystem browser and setting that filename in the
        #   path widget.'
        #
        def _browseProgramPath(self, checked):
            fname = QtGui.QFileDialog.getOpenFileName(
                self, 'OpenFile', os.getcwd()
            )
            if fname.size() > 0:
                self._path.setText(fname)
                
        def _browseWd(self, checked):
            dirname = QtGui.QFileDialog.getExistingDirectory(
                self, 'Choose Wd', os.getcwd()
            )
            if dirname.size() > 0:
                self._wd.setText(dirname)

            
        #----------------------------------------------------------------------
        # Public methods:
        
        ##
        # setPath
        #    Set the value of the path name QLineEdit
        #
        # @param path - new path value.
        #
        def setPath(self, path):
            self._path.setText(path)
        ##
        # path
        #    Return the current path value.
        # 
        def path(self):
            return self._path.text()
            
        ##
        # setWd
        #   Set the value of the working directory line editor.
        #
        # @param path  the new path.
        def setWd(self, path):
            self._wd.setText(path)
        ##
        # wd
        # @return the current working directory of the form.
        #
        def wd(self):
            return self._wd.text()
        ##
        # setHosts
        #   Provides the list of hosts in the host dropdown selector.
        # @param hostList  list of strings.
        #
        def setHosts(self, hostList):
            self._host.clear()
            self._host.addItems(hostList)
        ##
        # setHost
        #   Sets the current value of the host.
        # @param host - the name of the host to select must be in the combobox list
        #
        def setHost(self, host):
            index = self._host.findText(host)
            if index == -1:
                raise RuntimeError('No such host')
            else:
                self._host.setCurrentIndex(index)
        ##
        # host
        #  @return string - current host name.
        def host(self):
            return self._host.currentText()
        ##
        # setAgs
        #   Provides the current list  of program arguments.
        #
        # @param args - list of strings.
        #
        def setArgs(self, args):
            self._args.clear()
            self._args.addItems(args)
        ##
        # args
        #   @return list of strings that are the current set of arguments.
        #
        def args(self):
             rows = self._args.count()
             result = list()
             for i in range(0,(rows-1)):
                item = self._args.item(i)
                result.append(str(item.text()))
             return result
 
##
#  @class ProgramDialog
#
#   Dialog to prompt for a program.
#
class ProgramDialog(formdialog.FormDialog):
    def __init__(self, parent = None):
        form = ProgramSpec()
        super(ProgramDialog, self).__init__(form, parent)

##
# main is provided for testing:
#

if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    
    d = ProgramDialog()
    d.form().setArgs(['a', 'b', 'c'])
    if d.exec_() == QtGui.QDialog.Accepted:
        w = d.form()
        print('accepted')    
        print('path: ', w.path())
        print('wd  : ', w.wd())
        print('host: ', w.host())
        print('args: ', w.args())
    else:
        print('cancelled')
        