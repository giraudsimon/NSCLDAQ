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
# @file   filemenu.py
# @brief  Responsible for creating the file menu and its actions.
# @author <fox@nscl.msu.edu>

from PyQt4 import QtGui
import os
import project
import state


##
# @class FileMenu
#
#   Populates the filemenu and connects its actions to controller methods.
#
class FileMenu:
    ##
    # construct.
    #   Get the menu bar.
    #   Add the file menu.
    #   Create the actions and use them to populate the file menu.
    #   Connect the file menu to controller actions.
    #
    def __init__(self, mainWindow):
        self._mainWindow = mainWindow
        
        menubar = mainWindow.menuBar()
        filemenu = menubar.addMenu('&File')
        
        # File->New...
        
        newProjectAction = QtGui.QAction('New...', mainWindow)
        newProjectAction.setShortcut('Ctrl+N')
        newProjectAction.setStatusTip('Create a new project file')
        newProjectAction.triggered.connect(self._new)
        filemenu.addAction(newProjectAction)
        
        # File->Open...
        
        openProjectAction = QtGui.QAction('Open...', mainWindow)
        openProjectAction.setShortcut('Ctrl+O')
        openProjectAction.setStatusTip('Open an existing project file')
        openProjectAction.triggered.connect(self._open)
        filemenu.addAction(openProjectAction)
        
        
        # Separator and exit:
        
        filemenu.addSeparator()
        exitProjectAction = QtGui.QAction('Exit', mainWindow)
        exitProjectAction.setStatusTip('Exit program (note all changes are saved')
        exitProjectAction.triggered.connect(mainWindow.close)
        filemenu.addAction(exitProjectAction)
        
        
    ##
    # _new
    #   Prompt for and create a new project file.
    #   *  The project file is created and initialized.
    #   *  The project file path is set in the statusbar of the main window.
    #
    def _new(self):
        fname = QtGui.QFileDialog.getSaveFileName(
            self._mainWindow, 'Save File', os.getcwd(),'*.experiment'
        )
        #  Cancel will give a size zero string:
        
        if fname.size() > 0:
            fname = str(fname)                   # QString -> String
            #
            # If the file does not have a type we append .experiment to it.

            fileInfo = os.path.splitext(fname)
            if (fileInfo[1] == "") :
                fname = fname + '.experiment'
                

            # If the file already exists, we need to get rid of it, the user
            # has already approved an overwrite.
            # This is needed becayuse the sqlite schema creation will fail if
            # the file is either not an sqlite database file or is an existing
            # project:
            
            if os.path.isfile(fname):
                os.remove(fname)
            myProject  = project.Project(fname)
            myProject.create()
            state.State.project = myProject
            
            # Update the UI:
            
            self._mainWindow.centralWidget().populate()
            self._mainWindow.setWindowTitle(fname)
            
    ##
    # _open
    #   Open an existing file as a project.
    #
    def _open(self):
        fname = QtGui.QFileDialog.getOpenFileName(
            self._mainWindow, 'Open File', os.getcwd(), '*.experiment'
        )
        # Cancel will give us a 0 length string so:
        
        if fname.size() > 0:
            fname = str(fname)
            myProject = project.Project(fname)
            myProject.open()
            state.State.project = myProject
            
            self._mainWindow.centralWidget().populate()
            self._mainWindow.setWindowTitle(fname)
