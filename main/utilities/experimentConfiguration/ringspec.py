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
# @file   ringspec.py
# @brief  Form and dialog that describe rings.
# @author <fox@nscl.msu.edu>
import sys
from PyQt4 import QtGui, QtCore
from nscldaq.expconfiguration import formdialog

##
# @class SpinBox
#
#   Derivation of a spin box which allows for an empty selection.
#   The empty selection will be shown when the spinbox has it's minimum value
#   This code is shamelessly stolen from:
#   http://qt-project.org/faq/answer/how_can_i_set_an_empty_default_value_in_qspinbox
#
#  So the normal use is to instantiate the spinbox and give it a minimum value
#  that is one less than the actual usable minimum.  E.g. if you want to allow
#  numbers in the range (0, 1000] Set the range to be [0, 1000] the spinbox
#  will be blank when the value is 0.
#
#
class SpinBox(QtGui.QSpinBox):
    def __init__(self, parent):
        super(SpinBox, self).__init__(parent)
        
    ##
    #  Override valueFromText so that it sets the minimum value if '' is used:
    #
    def valueFromText(self, text):
        if text.isEmpty():
            return self.minimum()
        else:
            return super(SpinBox, self).valueFromText(text)

    ##
    #  Obverride textFromValue so that '' is returned when the value
    #  is minimum.
    #
    def textFromValue(self, v):
        if v == self.minimum():
            return QtCore.QString()
        else:
            return super(SpinBox, self).textFromValue(v)
            
##
# @class RingForm
#
#   This class provides an editor that can define/edit a ring definition.
#
# LAYOUT:
#   +----------------------------------------+
#   |  Name:  [                     ]        |
#   |  Host:  [combobox-of-hosts    ]        |
#   |  Sourceid:   [number spinbox? ]        |
#   +----------------------------------------+
#
# Key methods:
#    setName    - Sets the name of the ring - until set it's empty.
#    name       - Gets the name of the ring.
#    setHosts   - Sets the list of hosts that are known (populates the combobox)
#    setHost    - Sets the currently selected host (until then it's empty too)
#    host       - Get currently selected host.
#    setSourceId- Set the current source id.
#    sourceId   - Get the current source id.
#
class RingForm(QtGui.QFrame):
    def __init__(self,parent = None):
        super(RingForm, self).__init__(parent)
        self._setupUi()
        
    def _setupUi(self):
        #  Use a grid layout engine:
        
        layout = QtGui.QGridLayout()
        self.setLayout(layout)
        
        # Create and layout the widgets:
        
        layout.addWidget(QtGui.QLabel('* Name:', self), 0, 0)
        self._ringName = QtGui.QLineEdit(self)
        layout.addWidget(self._ringName, 0, 1)
        
        layout.addWidget(QtGui.QLabel('* Host:', self), 1, 0)
        self._hostName  = QtGui.QComboBox(self)
        layout.addWidget(self._hostName, 1, 1)
        self._hostModel = QtGui.QStandardItemModel(self._hostName)
        self._hostName.setModel(self._hostModel)
        
        layout.addWidget(QtGui.QLabel('Source Id', self), 2, 0)
        self._sourceId = SpinBox(self)
        self._sourceId.setRange(-1, 1000)
        self._sourceId.setValue(-1)
        layout.addWidget(self._sourceId, 2, 1)


    #--------------------------------------------------------------------------
    #  Public member functions:
    
    ##
    # setName
    #    Set a name in the ring name widget.
    #
    # @param ringName - the ring to use.
    #
    def setName(self, ringName):
        self._ringName.setText(ringName)
    ##
    # name
    #   Return the current value of the host name:
    #
    # @return QString - contents of the line edit widget.
    #
    def name(self):
        return self._ringName.text()
    ##
    # setHosts
    #   Sets the list of hosts one chan choose from in the host chooser.
    #
    # @param hostNameList - iterable entity that contains the host names
    #                       that are allowed.
    #
    def setHosts(self, hostNameList):
        self._hostModel.clear()
        
        #  Add the items:
        
        for host  in hostNameList:
            self._hostName.addItem(host)
            
    ##
    # setHost
    #   Selects the specified hostName as the default host.
    #
    # @param hostName - the desired host.
    # @throw RuntimeError if the host is not in the list of hosts.
    #
    def setHost(self, hostName):
        index = self._hostName.findText(hostName)
            
        if index < 0:
            raise RuntimeError('RingForm.setHost - host %s is not a known host' % hostName)
        else:
            self._hostName.setCurrentIndex(index)
    ##
    # host
    #   Returns the currently selected host -- this cannot be empty.
    #
    # @return QString
    #
    def host(self):
        return self._hostName.currentText()
    ##
    # setSourceId
    #   Set the value of the source id.
    # @param id - the new source id value.
    # @throw RuntimeError - if the range of the value is not in the min/max range
    #                       of the widget.
    #
    # @note - a value of -1 indicates there is no source id.
    #
    def setSourceId(self, id):
        if (id < self._sourceId.minimum()) or (id > self._sourceId.maximum()):
            raise RuntimeError('RingForm.setSourceId %s is not a valid source id' % id)
        else:
           self._sourceId.setValue(id)
    ##
    # sourceId
    #    @return int - the current vale of the spinbox.  -1 means no source id.
    #
    def sourceId(self):
        return self._sourceId.value()

##
# @class RingDialog
#
#   Dialog to prompt users to enter information that describes a ring.
#   The ring dialog is a composite that contains a RingForm.  The key methods
#   other than the dialog methods is form() which obtains the form
#   associated with the dialog
#
class RingDialog(formdialog.FormDialog):
    ##
    # __init__
    #   Construction
    #   *  Create/Layout the widgets.
    #   *  Connect button box signals to dialog slots
    def __init__(self, parent = None):
        form = RingForm()
        super(RingDialog, self).__init__(form, parent)
        
    

if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    ex = RingDialog()
    form = ex.form()
    form.setName('ARing')
    form.setHosts(['spdaq20', 'spdaq21', 'charlie'])
    form.setHost('spdaq20')
    form.setSourceId(-1)
    
    
    if ex.exec_() == QtGui.QDialog.Accepted:
        print("ok")
        print('Ring Name: ' + str(form.name()))
        print('Host: ' + str(form.host()))
        print('source Id: %d'  % (form.sourceId()))
    else:
        print('cancel')
