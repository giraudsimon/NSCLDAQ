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
# @file   cpanelWidget.py
# @brief  Run control panel widget.
# @author <fox@nscl.msu.edu>

from PyQt4 import QtGui, QtCore
import sys

#
#  This file contains a control panel widget that can be encapsulated
#  into a larger application.
#  This is because it:
#   *  Has an entry point to inform it of the current state.
#   *  Has an observer that can be informed of state change requests.
#
#  A typical use is for the encapsulating app to be attached to the state
#  manager.  Its state transition/establishment callaback would inform the
#  widget of the new (initial) state.  It would establish an observer as well
#  that would take transition requests and pass them on to the state manager.
#

##
# @class TitleWidget
#
#    megawidget that includes a label (Title) and a QLineEdit
#    field for an 80 character title.
#
# Signals:
#   *  titleChanged - when the title changed value.
#
# Properties:
#  *  title - The current title.
#
#
class TitleWidget(QtGui.QWidget):
    titleChanged = QtCore.pyqtSignal(QtCore.QString)
    
    ##
    # construtor
    #   * Create the pair of subwidgets.
    #   * Create and set the widget layout to QForm layout
    #   * Add the label and the input field to the form.
    #   * Link the title fields entry changed to the title changed
    #     signal.
    #
    #   @param parent - the parent of our widget
    #
    def __init__(self, parent = None):
        super(TitleWidget, self).__init__(parent)
        self._label = QtGui.QLabel('Title: ', self)
        self._title = QtGui.QLineEdit(self)
        
        self._layout = QtGui.QFormLayout(self)
        self.setLayout(self._layout)
        self._layout.addRow(self._label, self._title)
        
        self._title.editingFinished.connect(self._emitChanged)
        
    ##
    # _emitChanged
    #   Connects the editingFinished signal to the titleChanged signal
    #
    def _emitChanged(self):
        self.titleChanged.emit(self._title.text())
        
    #--------------------------------------------------------------------------
    # properties
    #
    
    # title:
    def setTitle(self, title):
        self._title.setText(title)
        
    def getTitle(self):
        return self._title.text()
        
##
# @class RunWidget
#  megawidget that include a label (Run Number) and a QSpinBox
#  for the run number.  The QSpinbox runs from 0 to 10000 initially, but
#  whenever the spinbox value gets 'close' to the upper limit, the upper limit
#  is doubled effectively making the sping box good for all positive
#  numbers.
#
# Signals:
#    *  runChanged - the value of the spin box changed.
#
# Properties:
#    * run - The current value of the run number.
#
class RunWidget(QtGui.QWidget):
    runChanged = QtCore.pyqtSignal(int)
    
    ##
    # constructor
    #   *  Create the widgets.
    #   *  Create a form layout, and set it as our layout.
    #   *  Add the widgets to the form.
    #   *  Connect the valuechanged signal to our runChanged signal
    #
    # @para parent - parent widget.
    
    def __init__(self, parent = None):
        super(RunWidget, self).__init__(parent)
        
        self._label = QtGui.QLabel('Run Number: ', self)
        self._run   = QtGui.QSpinBox(self)
        self._run.setRange(0, 10000)
        
        self._layout = QtGui.QFormLayout(self)
        self.setLayout(self._layout)
        self._layout.addRow(self._label, self._run)
        
        self._run.valueChanged[int].connect(self._emitChanged)
    
    ##
    # _emitChanged
    #
    # @param run - the new run number.
    #
    def _emitChanged(self, run):
        self.runChanged.emit(run)
        
    #------------------------------------------------------------------------
    # Properties
    #
    
    #  Run number:
    
    def getRun(self):
        self._run.value()
    def setRun(self, value):
        self._run.setValue(value)


##
# @class ControlPanel
#
#  Top level widget for the control panel.  At present, this
#  contains the following layout:
#
#   +---------------------------------------------+
#   |  +----------------------+                   |
#   |  | State dependent      |  Current State    |
#   |  | UI                   |                   |
#   |  +----------------------+                   |
#   +---------------------------------------------+
#
#
class ControlPanel(QtGui.QWidget):
    buttonPush = QtCore.pyqtSignal(QtCore.QString)
    titleChanged = QtCore.pyqtSignal(QtCore.QString)
    runChanged   = QtCore.pyqtSignal(int)
    recordChanged= QtCore.pyqtSignal(bool)

    
    _titleWidget      = None
    _runWidget        = None
    _recordingWidget  = None
    _titleString      = ''
    _runNumber        = -1
    _recording        = False
    
    ##
    # __init__
    #   Construction initializes the base frame,
    #   creates an empty observer list and invokes _setup to
    #   create and layout the UI elements.
    #   
    def __init__(self, parent=None):
        super(ControlPanel, self).__init__(parent)
        self._observers = list()
        self._setup()

    ##
    # _setup
    #   Layout the widget and attach events.
    #
    def _setup(self):
        
        #  Use a grid layout for future expansion:
        
        self._layout = QtGui.QGridLayout()
        self.setLayout(self._layout)
        
        # For now just put a frame up for the buttons because we don't know
        # which buttons to display yet:
        
        self._buttonFrame  = QtGui.QFrame(self)
        self._buttonFrame.setLayout(QtGui.QGridLayout())
        self._buttonFrame.resize(400,400)
        self._buttons      = list()    # No child widgets.
        
        self._layout.addWidget(self._buttonFrame, 0, 0)
        
        # Add the state label - initialized to
        # unknown state.  It's going to cell 0, 1 of the grid
        
        
        self._stateLabel = QtGui.QLabel('>>Unknown<<', self)
        self._layout.addWidget(self._stateLabel, 0, 1)
        
    
    
        
    ##
    # _setStateButtons
    #    *  Destroys any old control buttons.
    #    *  Creates the control buttons that are appropriate to this
    #       state.
    #
    # @param state - Name of state we just entered.
    #
    # @note we're going to be a bit dynamic here.  We assume the
    #       existence of a method crate<State>Buttons where
    #      <State> is the value of the current state, and that that method
    #      actually knows how to create the buttons we need and attach
    #      them to events.
    #
    def _setStateButtons(self, state):
        for b in self._buttons:
            b.hide()
            b.destroy()
            del b
        self._buttons = list()             # should delete ad this is the only ref
        self._titleWidget     = None
        self._runWidget       = None
        self._recordingWidget = None
        creator = "self._create%sButtons()" % (state.upper())
        exec creator
            
 
    ##
    # _emitButtonPush
    #   Action handler for clicking a button.. the text of the button is
    #   retrieved, converted to upper case and used as the value of the
    #   buttonPush slot emit
    #
    def _emitButtonPush(self):
        b  = self.sender()
        transition = str(b.text())
        transition = transition.upper()
        self.buttonPush.emit(transition)
    
    ##
    # _runChanged
    #   Catch/resignal the run changed.
    #
    # @param newRun - new run number.
    #
    def _runChanged(self, newRun):
        self.runChanged.emit(newRun)
    
    ##
    # _titleChanged
    #   Caught the title change signal... get the title
    #   and, emit our titleChanged signal.
    #
    # @param newTitle - the new title string,.
    #
    def _titleChanged(self, newTitle):
        self.titleChanged.emit(newTitle)
    
    #--------------------------------------------------------------------------
    #  Button creation/layout for each state
    
    ##
    # _createActionButton
    #   * Creates a button with the specified text.
    #   * Connects it to the _emitButtonPush method
    #   * adds it to the _buttons list
    #
    # @param text - Label on the button.
    # @return button - The button widget.
    #
    def _createActionButton(self, text):
        b = QtGui.QPushButton(text, self)
        b.clicked.connect(self._emitButtonPush)
        self._buttons.append(b)
        return b
    
    ##
    # _createNotReadyButtons
    #   In this state, the only legal operation is to Boot the system:
    def _createNOTREADYButtons(self):
        boot = self._createActionButton('Boot')
        self._layout.addWidget(boot, 0, 0, 1, 1, QtCore.Qt.AlignLeft)
        
    ##
    # _createBootingButtons
    #   In this state the user can fail the transition if they suspect
    #   a problem.
    #
    def _createBOOTINGButtons(self):
        fail = self._createActionButton('Fail')
        self._layout.addWidget(fail, 0, 0, 1, 1, QtCore.Qt.AlignLeft)
        
    ##
    # _createReadyButtons
    #   Creates Begin/Fail buttons Top to bottom.
    #   Adds to this:
    #    * Title input string (QLineEdit)
    #    * Run number  (QSpinBox)
    #
    #  Title updates on editing done run number updates on valueChanged
    #
    def _createREADYButtons(self):
        title = TitleWidget(self)
        title.titleChanged.connect(self._titleChanged)
        self._titleWidget = title
        title.setTitle(self._titleString)
        
        run   = RunWidget(self)
        run.runChanged.connect(self._runChanged)
        self._runWidget   = run
        if self._runNumber != -1:
            run.setRun(self._runNumber)
            
        record = QtGui.QCheckBox("Record", self)
        record.stateChanged.connect(self.recordChanged.emit)
        record.setChecked(self._recording)
        self._recordingWidget = record
        
        
        begin = self._createActionButton('Begin')
        fail  = self._createActionButton('Fail')
        self._layout.addWidget(title, 0, 0)
        self._layout.addWidget(run,   1, 0, 1, 1, QtCore.Qt.AlignLeft)
        self._layout.addWidget(record, 1, 1, 1, 1, QtCore.Qt.AlignLeft)
        self._layout.addWidget(begin, 2, 0, 1, 1, QtCore.Qt.AlignLeft)
        self._layout.addWidget(fail,  3, 0, 1, 1, QtCore.Qt.AlignLeft)
        
        self._buttons = [title, run, record,  begin, fail]
        
    ##
    #  Active runs can be ended or failed if the user supsects a problem.
    # 
    def _createACTIVEButtons(self):
        end = self._createActionButton('End')
        fail= self._createActionButton('Fail')
        self._layout.addWidget(end, 0, 0, 1, 1, QtCore.Qt.AlignLeft)
        self._layout.addWidget(fail, 1, 0, 1, 1, QtCore.Qt.AlignLeft)
                               
        pass
        
        
        
    #--------------------------------------------------------------------------
    #   public methods:
    #
    #
    
    ##
    # setState
    #   Provides the widget with a new state value.
    #
    # @param state - the new state string
    #
    def setState(self, state):
        self._stateLabel.setText(state)
        self._setStateButtons(state)
        
    ##
    #  setTitle
    #   Set the title both in our property and, if the widget is defined in the
    #   title text widget.
    # @param title -the new title.
    #
    def setTitle(self, title):
        self._titleString = title;
        if self._titleWidget != None:
            self._titleWidget.setTitle(title)
    ##
    # setRun
    #   set the run number both in our property and, if the widget is defined, in
    #   its widget.
    #
    # @param run - the new run number.
    #
    def setRun(self, run):
        self._runNumber = run
        if self._runWidget != None:
            self._runWidget.setRun(run)
    
    ##
    # setRecord
    #   Set the recording state.
    # @param state - the boolean that turns recording on or off.
    #
    def setRecord(self, state):
        self._recording = state
        if self._recordingWidget != None:
            self._recordingWidget.setChecked(state)
    ##
    # addObserver
    #   Adds a callable which will be invoked when a button is clicked.
    #   The obsever gets passed the name of the transition that button would
    #   attempt to produce (e.g. BOOT, BEGIN, END).
    #
    # @param callable
    #
    # @note Observers are called in the order in which they get established.
    #
    def addObserver(self, callable):
        self._observers.append(callable)
    ##
    # removeObserver
    #  Removes an existing observer from the list.  This is a no-op if there
    #  is no observer like that in the list.
    #
    # @param callable - the observer to remove.
    #
    def removeObserver(self, callable):
        try:
            self._observers.remove(callable)
        except ValueError:    # Raised if callable not in list.
            pass
        


##
#  If this is run as a main then pop up the widget, give it an initial
#  state and run an observer that will simulate the statemachine.
#  This supports testing.
#

if __name__ == '__main__':
    def fakeStateMachine(transition):
        t = transition
        if t == 'BOOT':
            w.setState('Ready')   # Skip booting since not button gets us out.
        if t == 'BEGIN':
            w.setState('Active')
        if t == 'END':
            w.setState('Ready')
        if t == 'FAIL':
            w.setState('NotReady')
    
    app = QtGui.QApplication(sys.argv)
    w   =  ControlPanel()
    w.setWindowTitle('testing control panel')
    w.show()
    
    w.setState('NotReady')
    
    # Connect a fake state machine to the control panel:
    
    w.buttonPush.connect(fakeStateMachine)
    
    sys.exit(app.exec_())




