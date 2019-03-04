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
# @file   projectdisplay.py
# @brief  Contains the code needed to render displays of a project on the UI
# @author <fox@nscl.msu.edu>

from PyQt4 import QtGui, QtCore
from nscldaq.expconfiguration import state, project, ringspec, programspec
from nscldaq.expconfiguration import RecorderDialog



##
# @class
#   HostTable
#     This class is derived from a QTableWidget and is used to
#     *  Display information about the set of hosts known by the project and
#     *  Interact with that information in a way that allows you to
#        maintain the host table.
# @note
#    For the editing functions to operate, the user must have connected to a
#    project.
#
class HostTable(QtGui.QTableWidget):
    ##
    # __init__
    #    Construct us as a derivation of a QTableWidget.
    #
    # @param parent - the parent widget.
    #
    def __init__(self, parent):
        super(HostTable, self).__init__(parent)
        self.setSortingEnabled(True)
        self.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
        self.customContextMenuRequested.connect(self._contextMenu)
        self.cellChanged.connect(self._changed)
    
    ##
    # _okToDelete
    #   Determines whether or not a host can safely be deleted.  Hosts can't
    #   be deleted if other tables have references into this host.
    #   If the host can't be deleted, an error message is displayed.
    #
    # @param id        - id of the host to delete.
    # @return Boolean - True if the host can be deleted, False otherwise.
    #
    def _okToDelete(self, id):
        rings = project.Rings(state.State.project)
        references = rings._query((id,), '', 'WHERE ring_id=?')
        
        if len(references):
            QtGui.QMessageBox.critical(
                self, 'Host Referenced',
                'This host is still referenced by: rings, and cannot be deleted'
            )
            return False
        else:
            return True
    
    ##
    # _contextMenu
    #
    #  Brings up the context menu and operates on the selected object based
    #  on the context menu selection.  Note that at this point, the context
    #  menu only has a Delete... operation so it's fine to do everything here.
    #  If the menu expands, this should be a dispatcher rather than a do-all.
    #
    # @param point - Widget point within the view at which the menu should
    #                pop up.
    #
    def _contextMenu(self, point):
        
        # TODO: Fix case when project has not yet been defined.
        
        row = self.currentRow()
        idItem = self.item(row,0)
        id = int(idItem.text())
        # Can't delete the one that has no id:
        
        if str(idItem.text()) != '':   
            globalPosition = self.mapToGlobal(point)
            contextMenu = QtGui.QMenu()
            contextMenu.addAction('Delete...')
            action = contextMenu.exec_(globalPosition)
            if action and (action.text() == 'Delete...'):
                if self._okToDelete(id):
                    confirm = QtGui.QMessageBox.question(
                        contextMenu, 'Delete? ',
                        'Are you sure you want to delete this host?',
                        QtGui.QMessageBox.Ok, QtGui.QMessageBox.Cancel
                    )
                    if confirm == QtGui.QMessageBox.Ok:
                        
                        hosts = project.Hosts(state.State.project)
                        hosts.delete(id)
                        self.populate(hosts.list())
    
            
    ##
    # _changed
    #   Called when a cell contents change
    #   * If the host is empty the assumption is you didn't want to do anything.
    #   * If this is the row with no id, a new item is added to the host table.
    #     (however if the host already exists, that's an error).
    #   * If this is a row with an id that row will be modified (again it's an
    #     error to duplicate a host).
    #
    def _changed(self, row, col):
        idItem   = self.item(row, 0)
        hostItem = self.item(row, 1)
        
        hosts   = project.Hosts(state.State.project)
        
        id   = str(idItem.text())
        host = str(hostItem.text())
        
        if host != '':     
            if id == '':
                if hosts.exists(host):
                    
                    QtGui.QMessageBox.critical(
                        self, 'Duplicate Host',
                        '%s is already defined as a host.' % host
                    )
                else:
                    hosts.add(host)
            else:
                if hosts.exists(host):
                    QtGui.QMessageBox.critical(
                       self, 'Duplicate Host',
                       '%s is already defined as a host.' % host 
                    )
                else:
                    hosts.modify(int(id), host)                   
            self.populate(hosts.list())
        
        
    #-------------------------------------------------------------------------
    #  Public methods
    
    ##
    # populate
    #   Clear the data in the current table and populate it with new data
    #
    # @param hosts - This is a tuple/list of dicts each dict contains the keys:
    #                -   id - The id of a host.
    #                -   host_name - the name of that host.
    # @note The hosts parameter is suitable for the outpt from the
    #       project.Hosts.list method.
    #
    def populate(self, hosts):
        # population will fire change signals so we first want to disconnect that:
        
        self.cellChanged.disconnect(self._changed)
        
        # start out with an empty table and set the header titles:
        
        
        self.clear()        
       

        self.setColumnCount(2)
        self.setRowCount(1 + len(hosts))    # 1 extra for new items.
        
        self.setHorizontalHeaderLabels(['id', 'Host Name'])
        
        #  Now populate the rows and columns from the list dict.
        #  Seems like the headers count as rows:
        
        row = 0
        for host in hosts:
            id = QtGui.QTableWidgetItem(str(host['id']))
            id.setFlags(id.flags() & (~(QtCore.Qt.ItemIsEditable)))
            self.setItem(row, 0, id)
            self.setItem(row, 1, QtGui.QTableWidgetItem(host['host_name']))
            row = row+1
        emptyId = QtGui.QTableWidgetItem('')
        emptyId.setFlags(emptyId.flags() & (~(QtCore.Qt.ItemIsEditable)))
        self.setItem(row, 0, emptyId)
            
        # From now on we want to know when cells get changed:
        
        self.cellChanged.connect(self._changed)
        

##
# @class RingDisplay
#
#   This class:
#    * Is a QTreeView
#    * Contains a QStandardItemModel model that provides the set of rings that
#      are defined grouped by the nodes in which they live.
#
#  For example if I have rings ring1, ring2 in host1 and ring3, ring4 in host2
#  The tree view might display:
# @verbatim
#    +host1
#    |- ring1
#    |- ring2
#    +host2
#    |- ring3
#    |- ring4
# @endverbatim
#
class RingDisplay(QtGui.QTreeView):
    ##
    # custom signals:
    #    newItem - Fired when a user selects New... from the context menu.
    #    editItem- Fired when a user selects Edit... from the context menu.
    #              parameter is the QtCore.QModelIndex of the selected item.
    #    deleteItem - Fired when the user selects Delete... from the context menu.
    #              parameter is the QtCore.QModelIndex of the selected item.
    #
    
    newItem    = QtCore.pyqtSignal()
    editItem   = QtCore.pyqtSignal(QtCore.QModelIndex)
    deleteItem = QtCore.pyqtSignal(QtCore.QModelIndex) 
    
    
    ##
    # __init__
    #   Construction - Create the tree view and enable the context menu
    #   to fire the _contextMenu method
    #
    # @param parent - parent of the tree view.
    #
    def __init__(self, parent):
        super(RingDisplay, self).__init__(parent)
        self.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
        self.customContextMenuRequested.connect(self._contextMenu)
        
        # Connect context menu signals:
        
        self.deleteItem.connect(self._delete)
        self.newItem.connect(self._new)
        self.editItem.connect(self._edit)

    ##
    # _stockHosts
    #   Stocks a RingForm with the set of hosts we know about now
    #
    # @param form - The RingForm object.
    #
    def _stockHosts(self, form):
        hostModel = project.Hosts(state.State.project)
        hosts     = hostModel.list()
        
        hostlist  = list()
        for host in hosts:
            hostlist.append(host['host_name'])
        form.setHosts(hostlist)
    
    ##
    # _requiredFields
    #    A ring must specify the name and the host name (the hostname is
    #    guaranteed by the fact that it's coming froma combobox, but we'll
    #    check anyway).  If either is not specified we'll emit a critical
    #    dialog
    #
    # @param  form    - The ring specification form object.
    # @return Boolean - True if all required fields were present, False if not.
    #
    def _requiredFields(self, form):
        missingFields = list()
        if form.name() == '':
            missingFields.append('Ring name')
        if form.host() == '':
            missingFields.append('Host name')
        
        if len(missingFields) == 0:
            return True
        else:
            fields = ', '.join(missingFields)
            QtGui.QMessageBox.critical(
                self, 'Missing Required Fields',
                'The following required field(s) were not filled in: %s' % fields
            )
            return False
    ##
    # _notDuplicate
    #    A new ring specification cannot duplicate an existing ring.
    #    if it does that's an error.  We'll check that and if necessary,
    #    throw up a critical dialog to indicate the duplication.
    #
    # @param rings - project.Rings object.
    # @param name  - Ring name.
    # @param host  - Host name.
    # @return Boolean - True if this is a new, unique ring/host combination.
    #                   False if not.
    #
    def _notDuplicate(self, rings, name, host):
        if rings.exists(name, host):
            QtGui.QMessageBox.critical(
                self, 'Ring exists',
                'The ring %s@%s already exists' % (name, host)
            )
            return False
        else:
            return True
    
    ##
    # _unused
    #   Ensure a ring is not referenced as an FK in other tables.
    #
    # @param id  id of the ring being checked.
    #
    # @return boolean - True if the ring is not used.
    #
    def _unused(self, id):
        
        # Could be referenced by event loggers:
        
        loggers = project.EventLoggers(state.State.project)
        
        return len(loggers.find(ring_id=id)) == 0
        
    ##
    # _edit
    #   Called when Edit... is selected from the context menu:
    #
    #  - Bring up a ring dialog stocked with what the selected ring looks like.
    #  - Let the user fill it in.
    #  - Make sure all the required items are still filled in.
    #  - If they change the host/ringname pair ensure this is not a duplicate.
    #  - Use Rings.modify to modify the things that changed.
    #
    # @param index - the index of the item selected by Edit...
    #
    def _edit(self, index):
        #
        # First we need to know everything about this ring:
        
        row = index.row()
        idIndex   = index.sibling(row, 0)
        nameIndex = index.sibling(row, 1)
        srcIdIndex= index.sibling(row, 2)
        
        idText    = idIndex.data().toString()
        name      = nameIndex.data().toString()
        srcid    = srcIdIndex.data().toString()
        if srcid == '':
            srcid = -1
        else:
            srcid = int(srcid)
        
        nodeIndex    = index.parent()
        nodeRow      = nodeIndex.row()
        nodeNameIndex= nodeIndex.sibling(nodeRow, 0)
        nodeName     = nodeNameIndex.data().toString()
        
        
        # Create and stock the dialog:
        
        dialog = ringspec.RingDialog(self)
        form   = dialog.form()
        self._stockHosts(form)
        
        form.setName(name)
        form.setHost(nodeName)
        form.setSourceId(srcid)
        
        if dialog.exec_() == QtGui.QDialog.Accepted:
            ringModel = project.Rings(state.State.project)
            newName = str(form.name())
            newHost = str(form.host())
            newSrcid = None if form.sourceId() == -1 else int(form.sourceId())
            
            if self._requiredFields(form):
                # IF either the ring name or its host have changed check for dups:
                
                if (newName != name or newHost != nodeName):
                    if not self._notDuplicate(ringModel, newName, newHost):
                        return
                # create a map of the changed items suitable for feeding into
                # Rings.modify
                
                modifications = dict()
                if newName != name:
                    modifications['name'] = newName
                if newHost != nodeName:
                    modifications['host_name'] = newHost
                if newSrcid != srcid:
                    modifications['sourceid'] = newSrcid
                
                
                if len(modifications) > 0:
                    ringModel.modify(int(idText), **modifications)
                    self.populate(ringModel.list())
            
    ##
    # _new
    #   Called when New... is selected from the context menu:
    #   - Bring up a RingDialog
    #   - Stock it's hosts with the known hosts.
    #   - Let the user fill it in.
    #   - Check that all required items are present.
    #   - Check that the ring does not yet exist.
    #   - If everything is a go, then use Rings.add_withHostname to
    #     add the ring to the system, and repopulate the tree.
    #
    def _new(self):
        dialog = ringspec.RingDialog(self)
        form   = dialog.form()
        self._stockHosts(form)
        if dialog.exec_() == QtGui.QDialog.Accepted:
            ringModel = project.Rings(state.State.project)
            ringName = str(form.name())
            hostName = str(form.host())
 
            if self._requiredFields(form) and self._notDuplicate(ringModel, ringName, hostName):
                sid      = int(form.sourceId())
                sid      = None if sid == -1 else int(sid)
                ringModel.add_withHostname(ringName, hostName, False, sid)
                self.populate(ringModel.list())
                

    ##
    # _delete
    #   Called when delete is selected from the context menu.
    #
    # @param index - Item that is selected.
    #
    def _delete(self, index):
        #
        # First we need to know everything about this ring:
        
        row = index.row()
        idIndex   = index.sibling(row, 0)
        nameIndex = index.sibling(row, 1)
        idText    = idIndex.data().toString()
        
        name    = nameIndex.data().toString()
        
        nodeIndex    = index.parent()
        nodeRow      = nodeIndex.row()
        nodeNameIndex= nodeIndex.sibling(nodeRow, 0)
        nodeName     = nodeNameIndex.data().toString()
        
        
        
        #  Ensure the user really wants to do this:
        
        if self._unused(int(idText)):
        
            confirm = QtGui.QMessageBox.question(
                 self, 'Delete? ',
                 'Are you sure you want to delete the ring %s@%s' % (name, nodeName),
                 QtGui.QMessageBox.Ok, QtGui.QMessageBox.Cancel
             )
            if confirm == QtGui.QMessageBox.Ok:
                rings = project.Rings(state.State.project)
                rings.delete(int(idText))
                self.populate(rings.list())
        else:
            QtGui.QMessageBox.warning(
                self, 'Ring in use',
                "The ring %s@%s is still being referenced " % (name, nodeName),
                QtGui.QMessageBox.Ok
            )
            
    ##
    # _contextMenu
    #    Displays the left mouse context menu and dispatches to the appropriate
    #    handler.  We'll use signals to manage the dispatch rather than direct
    #    calls.  This allows others to intercept the signals as well.
    #
    # @param point - A point describing over which veiw coordinates the
    #                menu should pop up.
    #
    def _contextMenu(self, point):
        
        # TODO: Fix case when project has not been defined yet.
        
        globalPosition = self.mapToGlobal(point)
        contextMenu    = QtGui.QMenu(self)
        
        # If this is a root element, only New... is allowed:
            
        contextMenu.addAction('New...')
        index = self.selectionModel().currentIndex()
        if index.parent() != QtCore.QModelIndex():
            contextMenu.addAction('Edit...')
            contextMenu.addAction('Delete...')
        
        selection = contextMenu.exec_(globalPosition)
        if selection:
            seltext = selection.text()
            if seltext == 'New...':
                self.newItem.emit()
            elif seltext == 'Edit...':
                self.editItem.emit(index)
            elif seltext == 'Delete...':
                self.deleteItem.emit(index)
            
        
    #--------------------------------------------------------------------------
    # Public methods
    
    def clear(self):
        pass
    
    def populate(self, rings):
        
        # Create a QStandard item model with columns names 'host', 'ring' 'sourceid'
        
        self._model = QtGui.QStandardItemModel()
        self._model.setHorizontalHeaderLabels(['host/ring id', 'ring', 'source id'])
        
        # root is the top level item into which nodes will be inserted:
        
        root = self._model.invisibleRootItem()
        
        # We're going to do this on the fly by maintaining a dict keyed by
        # host_name whose contents are the item into which that hosts rings are
        # inserted:
        
        hostItems = dict()
        for ring in rings:
            host = ring['host_name']
            id   = ring['ring_id']
            name = ring['ring_name']
            sid  = ring['sourceid']
            sid  = '' if sid == None else sid
            
            # If needed add a new host item otherwise, just get the one we have:
            
            if host not in hostItems.keys():
                h = QtGui.QStandardItem(host)
                h.setEditable(False)
                hostItem = [h]
                root.appendRow(hostItem)
                hostItems[host] = hostItem
            else:
                hostItem = hostItems[host]
            
            idItem = QtGui.QStandardItem(str(id))
            idItem.setEditable(False)
            
            nameItem = QtGui.QStandardItem(name)
            nameItem.setEditable(False)
            
            sidItem  = QtGui.QStandardItem(str(sid))
            sidItem.setEditable(False)
            
            item = [idItem, nameItem, sidItem]
            hostItem[0].appendRow(item)     
        
        # Set the model.
        
        self.setModel(self._model)
        self.expandAll()

##
# @class ProgramDisplay
#
#   Provides a tree view that displays programs.  The tree may be 2 levels deep
#   The top level are nodes, while the next level in are programs.  If the
#   program has parameter specifications those will be another level in.
#
#   Headings will therefore be:
#      Host/program id, Path, working directory, Program Arguments
#
class ProgramDisplay(QtGui.QTreeView):
    ##
    # custom signals:
    #    newItem - Fired when a user selects New... from the context menu.
    #    editItem- Fired when a user selects Edit... from the context menu.
    #              parameter is the QtCore.QModelIndex of the selected item.
    #    deleteItem - Fired when the user selects Delete... from the context menu.
    #              parameter is the QtCore.QModelIndex of the selected item.
    #
    
    newItem    = QtCore.pyqtSignal()
    editItem   = QtCore.pyqtSignal(QtCore.QModelIndex)
    deleteItem = QtCore.pyqtSignal(QtCore.QModelIndex)
    
    ##
    # __init__
    #   Construction - create the tree view base class and:
    #   * Enable context menu handling.
    #   * Patch our signals to slots that handle context menu selections.
    #
    def __init__(self, parent):
        super(ProgramDisplay,self).__init__(parent)
        
        # Context menu setup:
        
        self.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
               
        # Context menu signal connection:
        
        self.customContextMenuRequested.connect(self._contextMenu)
        
        self.deleteItem.connect(self._delete)
        self.newItem.connect(self._new)
        self.editItem.connect(self._edit)
    
    ##
    # _requiredItems
    #   Determines if the required items in a program form are all present.
    #
    # @param **kargs - Keyword/value pairs where:
    #                  * path - Value is the path  to the program.
    #                  * wd   - Value is the path to the working directory.
    #                  * host - Value is the host.
    #
    # @note - all keys must be present and have non empty string values.
    #
    # @return Boolean   - True : Required items are present, False not so.
    #
    def _requiredItems(self, **kargs):
        argsPresent = set(kargs.keys())
        argsRequired = set(['path', 'wd', 'host'])
        
        missingArgs = argsRequired - argsPresent
        if len(missingArgs) > 0:
            return False
        
        for key in kargs.keys():
            if kargs[key] == '':
                return False
            
        return True
     
    ##
    # _edit
    #    Edit an existing item.  Create the program form, and fill it in
    #    from the information about the selected item.
    #
    # @param index - Index of the selected item - used to get the item id
    #                which then allows the remainder of the info about the
    #                program to be fetched from the database.
    #
    def _edit(self, index):
        row = index.row()
        id  = int((index.sibling(row, 0)).data().toString())
        programs = project.Programs(state.State.project)
        hosts    = project.Hosts(state.State.project)
        
        programInfo = programs.find(program_id=id)[0]
        hostList    = hosts.list()
        hostnames   = map(lambda item: item['host_name'], hostList)
        
        dialog  = programspec.ProgramDialog(self)
        f       = dialog.form()
        f.setHosts(hostnames)
        
        # Now load up the rest of the dialog with the program information
        
        f.setPath(programInfo['path'])
        f.setWd(programInfo['working_dir'])
        f.setHost(programInfo['host_name'])
        f.setArgs(programInfo['args'])
        
        if dialog.exec_() == QtGui.QDialog.Accepted:
            path = str(f.path())
            wd   = str(f.wd())
            host = str(f.host())
            
            if not self._requiredItems(path=path, wd=wd, host=host):
                QtGui.QMessageBox.critical(
                    self, 'Missing params',
                    'The path, working directory and host parameters are all required',
                )
            else:
                args = f.args()
                programs.modify(
                    id, path=path, working_dir=wd, host_name=host, args=args
                )
                self.populate(programs.list())
        
        
        
        
    ##
    # _new
    #    Create a new item by prompting for it with the program form
    #    if accepted and all the mandatory stuff get filled in, save the item
    #    as a new program.  Note that duplicates are allowed.
    #
    def _new(self):
        # We need the hosts to fill in the form's combolist:
        
        hosts     = project.Hosts(state.State.project)
        hostList  = hosts.list()
        hostnames = map(lambda item: item['host_name'], hostList)
        
        dialog = programspec.ProgramDialog(self)
        dialog.form().setHosts(hostnames)
        if dialog.exec_() == QtGui.QDialog.Accepted:
            # Ensure we have non empty program paths, working directory,
            # and host.
            path = str(dialog.form().path())
            wd   = str(dialog.form().wd())
            host = str(dialog.form().host())
            
            if not self._requiredItems(path=path, wd=wd, host=host):
                QtGui.QMessageBox.critical(
                    self, 'Missing params',
                    'The path, working directory and host parameters are all required',
                )
            else:
                args = dialog.form().args()
                programs = project.Programs(state.State.project)
                programs.add_byHostname(path, wd, host, args)
                self.populate(programs.list())
                      
        
        
    ##
    # _delete
    #    Delete the indicated item from the programs.
    #    TODO:  We need to check at some point if the program is referenced
    #           elsewhere.  For now we just delete it in place.
    #
    # @param index - the index of the item to delete.
    #
    def _delete(self, index):
        #
        #  Get the info we need to
        #  *  Delete the item (the id).
        #  *  Specify the item to be deleted in the question dialog:
        
        row     = index.row()
        idText  = (index.sibling(row, 0)).data().toString()
        programs= project.Programs(state.State.project)
        info    = programs.find(program_id = int(idText))
        
        host    = info[0]['host_name']
        path    = info[0]['path']
        wd      = info[0]['working_dir']
        arglist = map(lambda item: "'" + item + "'", info[0]['args'])
        args    = ' '.join(arglist)
       
        message = 'Are you sure you want to delete:  %s:%s  %s (wd: %s)'  % \
                    (host, path, args, wd)
        confirm = QtGui.QMessageBox.question(
            self, 'Delete?', message, QtGui.QMessageBox.Ok, QtGui.QMessageBox.Cancel
        )
        if confirm == QtGui.QMessageBox.Ok:
            programs.delete(int(idText))
            self.populate(programs.list())
        
    ##
    # _contextMenu
    #    Called when a context menu has been requested (MB3).
    #    Pops up one in the vicinity of the pointer containing
    #    the entries:
    #    *   New... - Add a new element to the tree.
    #    *   Edit.. - Edit the selected element.
    #    *   Delete - Delete the selected element.
    #
    # @param pt - Location of the pointer.
    #
    def _contextMenu(self, pt):
        globalPosition = self.mapToGlobal(pt)
        contextMenu    = QtGui.QMenu(self)
        
        contextMenu.addAction('New...')
        # Other elements are only allowed if the item is a program line
        # (parent is a node).
        
        index = self.selectionModel().currentIndex()
        if index.parent() != QtCore.QModelIndex():
            contextMenu.addAction('Edit...')
            contextMenu.addAction('Delete...')
            
            # This loop ensures that if the click is in one of the args, we'll
            # return the index of the program not the arg.
            
            while index.parent().parent() != QtCore.QModelIndex():
                index = index.parent()
            
        # Pop up the menu and dispatch the results:
        
        selection = contextMenu.exec_(globalPosition)
        if selection:
            seltext = selection.text()         # Text on the menu.
            if seltext == 'New...':
                self.newItem.emit()
            if seltext == 'Edit...':
                self.editItem.emit(index)
            if seltext == 'Delete...':
                self.deleteItem.emit(index)
            
            
    ##
    #  Produce standard items that describe a program (this may include
    #  sub items that represent the arguments)
    #
    # @param program - description of a program.
    # @return list of QStandardItems, containing id, path, working directory.
    #         if the program has argumnts, we'll provide the children too
    #
    def _describeProgram(self,program):
        id    = program['program_id']
        path  = program['path']
        wd    = program['working_dir']
        args  = program['args']
        
        # Top level stuff:
        
        idItem    = QtGui.QStandardItem(str(id))
        idItem.setEditable(False)
        
        pathItem  = QtGui.QStandardItem(path)
        pathItem.setEditable(False)
        
        wdItem    = QtGui.QStandardItem(wd)
        wdItem.setEditable(False)
        
        
        # Program arguments:
        
        for arg in args:
            emptyItem1  = QtGui.QStandardItem()
            emptyItem1.setEditable(False)
            emptyItem2  = QtGui.QStandardItem()
            emptyItem2.setEditable(False)
            emptyItem3  = QtGui.QStandardItem()
            emptyItem3.setEditable(False)
            argItem = QtGui.QStandardItem(arg)
            argItem.setEditable(False)
            idItem.appendRow([emptyItem1, emptyItem2, emptyItem3, argItem])
                
        
        #  Result:
        
        return [idItem, pathItem, wdItem]
        
        
        
    #----------------------------------------------------------------------
    # Public methods
    
    ##
    # clear
    #    Clear the tree view...since we don't seem to use clear in isolation
    #    for now we can pass on it.
    
    def clear(self):
        pass
    
    ##
    # populate
    #   Populate the tree with the nodes, the program in each node and
    #   the program parameters.
    #
    # @param programs - The outputo of Programs.list()
    #
    def populate(self, programs):
        #  Create a QStandardItemModel with the appropriate columns:
        
        self._model = QtGui.QStandardItemModel()
        self._model.setHorizontalHeaderLabels(['Host/program id', 'Path', 'Working Directory', 'Program Arguments'])
        
        
      
        # Get the top level (invisible) item of which the hosts are children:
        
        root = self._model.invisibleRootItem()
        
        # Reorganize the data to reflect the hierarchy we want:
        
        hostItems = dict()
        for program in programs:
            host = program['host_name']
            
            #  If necessary create a new host item:
            
            if host not in hostItems.keys():
                h = QtGui.QStandardItem(host)
                h.setEditable(False)
                hostItem = [h]        # Row with only the host name.
                root.appendRow(hostItem)
                hostItems[host] = hostItem
            else:
                hostItem = hostItems[host]
                
            # Add this program as a child of the host.
            
            parent = hostItem[0]
            parent.appendRow(self._describeProgram(program))
            
            
        # Set the model and expand all nodes for now:
        
        self.setModel(self._model)
        self.expandAll()
 
##
# @class EventLoggerDisplay
#    Display/edit event loggers.   This is a table with the rows:
#    *  id      - The id of the logger (primary key)
#    *  ringid  - The id of the ring
#    *  ringname - The name of the ring.
#    *  hostname - The host in which the ring lives.
#    *  log path - The path to which data are logged.
#
class EventLoggerDisplay(QtGui.QTableWidget):
    ##
    # constructor
    #   Construct a derivation of the table wicdget. We'll use
    #   a context menu to delete, add or edit existing entries.
    #   actual entry of data into the table will be disabled.
    #
    # @param parent - the parent of the widget.
    #
    def __init__(self, parent =  0):
        super(EventLoggerDisplay, self).__init__(parent)
        self.setSortingEnabled(True)
        self.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
        self.customContextMenuRequested.connect(self._contextMenu)
    
    
    #--------------------------------------------------------------
    # Slots:
    #
    
    ##
    # _setDialogRings
    #    Given an eventlogger dialog, populate it with the
    #    set of rings.  The rings are listed as ringname@hostname
    #
    # @param d - the dialog.
    #
    def _setDialogRings(self, d):
        rings = project.Rings(state.State.project)
        ringData = rings.list()
        
        ringlist = list()
        for ring in ringData:
            ring = '@'.join([ring['ring_name'], ring['host_name']])
            ringlist.append(ring)
        
        d.form().setRings(ringlist)
    
    ##
    # _add
    #   Add a new event logger to the system.
    #   * Create a RecorderDialog
    #   * Populate the data source pulldown with the set of rings.
    #   * Let the dialog run.
    #   * If Ok was clicked get an EvntLoggers object and add the item
    #     specified by the dialog.
    #
    # @note the caller will repop the table.
    # @note We assume the ring name has no '@' symbols in it.  If it does
    #       we'd need to:
    #      *  take the last element of the split as the host.
    #      *  reassemble the ringname from all but the last elemetn of the
    #         split.
    #
    def _add(self):
        dialog = RecorderDialog.RecorderDialog()
        self._setDialogRings(dialog)
        if dialog.exec_() == QtGui.QDialog.Accepted:
            hostRing = str(dialog.form().ring()).split('@')
            ring = hostRing[0]
            host = hostRing[1]
            path = str(dialog.form().path())
            
            loggerDb = project.EventLoggers(state.State.project)
            loggerDb.add_byringname(host, ring, path)
                   
    ##
    # _edit
    #   Exactly like _add, but we first stock the dialog box with the
    #   contents of the ring the pointer was over when the context menu
    #   was popped up.
    #
    # @param row - Row of the table the pointer was over.
    #
    def _edit(self, row):
        id        = int(self.item(row, 0).text())
        ring_name = str(self.item(row, 2).text())
        ring_host = str(self.item(row, 3).text())
        path      = str(self.item(row, 4).text())
        
        dialog = RecorderDialog.RecorderDialog()
        self._setDialogRings(dialog)
        dialog.form().setRing('@'.join([ring_name, ring_host]))
        dialog.form().setPath(path)
        
        if dialog.exec_() == QtGui.QDialog.Accepted:
            hostRing = str(dialog.form().ring()).split('@')
            ring = hostRing[0]
            host = hostRing[1]
            path = str(dialog.form().path())
            
            loggerDb = project.EventLoggers(state.State.project)
            loggerDb.delete(id)
            loggerDb.add_byringname(host, ring, path)
    ##
    # _delete
    #   Delete the entry in the designated row.
    #   *   Get the id of the event logger.
    #   *   Get an eventlog data base handle.
    #   *   Delete the logger.
    #
    # @param row - The Row of the table that is under the pointer when
    #              the menu was popped up...that row is the one that will
    #              be deleted.  Column 0 has the id of the logger to
    #              delete.
    # @note the caller will repop the table.
    def _delete(self, row):
        
        #  Figure out the id of the item to delete
        
        item = self.item(row, 0)
        itemText = item.text()
        id = int(itemText)
        
        # Get the EventLoggers object for the current database and delete:
        
        loggerDb = project.EventLoggers(state.State.project)
        loggerDb.delete(id)
    ##
    # _contextMenu
    #   Post the context menu and get the value.
    #   The context menu always has Insert and has Edit.../Delete..
    #   if the pointer is over a row.
    #
    # @param point - where the mouse is.
    #
    def _contextMenu(self, point):
        item = self.itemAt(point)
        
        # Make an populate the menu:
        
        menu =  QtGui.QMenu()
        menu.addAction('New...')
        
        # If over an item include Edit.., Delete..
        
        if item :
            menu.addAction('Edit...')
            menu.addAction('Delete...')
            
        # Post it and wait for a user response:
        
        action = menu.exec_(self.mapToGlobal(point))
        
        # Dispatch action  note that None action means no selection
        
        if action:
            if action.text() == 'New...':
                self._add()
            elif action.text() == 'Edit...':
                self._edit(item.row())
            elif action.text() == 'Delete...':
                self._delete(item.row())
            else:
                raise RuntimeException('invalid action from context menu')
            loggers = project.EventLoggers(state.State.project)
            self.clear()
            self.populate(loggers.list())
        
    #--------------------------------------------------------------
    # utilities
    
    ##
    #  _addField
    #   add a field to the table widget:
    #
    # @param row - Row number.
    # @param col - Column number.
    # @param contents - What's displayed.
    #
    def _addField(self, row, col, contents):
        fW = QtGui.QTableWidgetItem(str(contents))
        fW.setFlags(fW.flags() & (~(QtCore.Qt.ItemIsEditable)))
        self.setItem(row, col, fW)
                    
    #------------------------------------------------------------
    #  Public methods
    #
    
    #
    # Note that clear() is implemented by the QTableWidget
    #
    
    
    ##
    # populate
    #   Clear the current table and populate the table with
    #   current values
    #
    # @param loggers - The result of the list() method call on the
    #                  event logging part of the project.
    #
    def populate(self, loggers):
        self.clear()
        self.setColumnCount(5)
        self.setRowCount(len(loggers))   # +1 since headers count.
        self.setHorizontalHeaderLabels(
            ['id', 'ring id', 'Ring Name', 'Ring Host name', 'Log Directory']
        )
        ##
        #  Populate the rows:
        #
        
        row = 0
        for logger in loggers:
            col = 0
            self._addField(row, col, logger['id'])
            col += 1
            
            for field in ( 'ring_id', 'ring_name', 'host_name'):
                self._addField(row, col, logger['ring'][field])
                col += 1
            
            self._addField(row, col, logger['path'])
            
            row += 1
                
            
            
            
##
# @class ProjectDisplay
#
#   This is a megawidget that is meant to be set as the CentralWidget of the
#   main window of the editor.  This would be a tabbed notebook with pages
#   for each of the views into the project.
#


class ProjectDisplay(QtGui.QTabWidget):
    ##
    # __init__
    #
    # @param parent - parent widget which is assumed to be a MainWindow
    #
    def __init__(self, parent):
        super(ProjectDisplay, self).__init__(parent)
        parent.setCentralWidget(self)
        self._setupUi()
    
    ##
    #  _setupUi
    #    Setup our user interface widgets (the contents of the frame).
    #
    def _setupUi(self):
        
        #  Add the host table:
        
        self._hostTable = HostTable(self)
        self.addTab(self._hostTable, "&Hosts")
        
        # Rings table
        
        self._rings     = RingDisplay(self)
        self.addTab(self._rings, '&Rings')
        
        # Programs editor
        
        self._programs  = ProgramDisplay(self)
        self.addTab(self._programs, '&Programs')
        
        # Event logging editor:
        
        self._eventLoggers = EventLoggerDisplay(self)
        self.addTab(self._eventLoggers, '&Event Logging')
    
        
    
    #--------------------------------------------------------------------
    # Public methods
    ##
    # clear
    #    Clear the project (called after  a new project has been created)
    #
    def clear(self):
        self._hostTable.clear()
        self._rings.clear()
        self._programs.clear()
        self._eventLoggers.clear()
        pass
    
    ##
    # populate
    #   Populate the UI if a project has just been opened.
    #
    def populate(self):
        self.clear()
        
        # Populate the host table:
        
        hosts = project.Hosts(state.State.project)
        self._hostTable.populate(hosts.list())
        
        # Populate the rings treeview:
        
        rings = project.Rings(state.State.project)
        self._rings.populate(rings.list())
        
        #  Populate the programs tree view:
        
        programs = project.Programs(state.State.project)
        self._programs.populate(programs.list())
        
        loggers = project.EventLoggers(state.State.project)
        self._eventLoggers.populate(loggers.list())
        
