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
# @file   project.py
# @brief  Mediates access to the project database.
# @author <fox@nscl.msu.edu>
import sqlite3


#-----------------------------------------------------------------------------
##
# @class Project
#
#   This class is responsible for schema creation/test.
#
class Project:
    def __init__(self, dbPath):
        self.connection = sqlite3.connect(dbPath)
        self.connection.isolation_level = None
        self.connection.execute('PRAGMA foreign_keys = on')
    ##
    # open
    #   Open a database that's already been initialized/created.
    #   This will check the existence of the appropriate set of
    #   tables.
    #
    def open(self):
        curs = self.connection.cursor()
        curs.execute(
            '''
            SELECT COUNT(*) FROM sqlite_master WHERE type='table' and name='hosts'
            '''
        )
        if (1,) != curs.fetchone():
            raise RuntimeError('Project.open - missing hosts table')
    
    ##
    # create
    #   Create a new project in the dbPath
    #
    def create(self):
        self.connection.execute(
            '''CREATE TABLE hosts (
                id           INTEGER PRIMARY KEY,
                host_name    VARCHAR(64) NOT NULL
            )'''
        )
        self.connection.execute(
            '''CREATE TABLE rings (
                id        INTEGER PRIMARY KEY,
                name      VARCHAR(128) NOT NULL,
                host_id   INTEGER NOT NULL,
                sourceid  INTEGER DEFAULT NULL,
                FOREIGN KEY (host_id) REFERENCES hosts(id)
            )
            '''
        )
        self.connection.execute(
            '''CREATE TABLE programs (
                id           INTEGER PRIMARY KEY,
                path         VARCHAR(128) NOT NULL,
                working_dir  VARCHAR(128) NOT NULL,
                host_id      INTEGER NOT NULL,
                FOREIGN KEY (host_id) REFERENCES hosts(id)
            )
            '''
        )
        self.connection.execute(
            '''CREATE TABLE program_args (
                 id        INTEGER PRIMARY KEY,
                 argument  VARCHAR(128) NOT NULL,
                 program_id INTEGER NOT NULL,
                 FOREIGN KEY (program_id) REFERENCES programs(id)
            )
            '''
            )
        self.connection.execute(
            '''
            CREATE TABLE recorded_rings (
                id             INTEGER PRIMARY KEY,
                ring_id        INTEGER NOT NULL,
                directory_path VARCHAR(256) NOT NULL,
                FOREIGN KEY (ring_id) REFERENCES rings(id)
            )
            '''
        )

        
        
##
# @class Hosts
#
#   This class is responsible for maintaining the hosts known to the project.
#   The hosts are used to stock various drop down lists that select the host
#   on which objects (ring, programs) will be instantiated.
#
#  See the Project class.
class Hosts:
    def __init__(self, project):
        self._project = project
        
    ##
    # _exists
    #  Returns true if a host is already in the table.
    #
    # @param host - name of the host.
    # @param column - The column to match with
    #
    def _exists(self, host, column='host_name'):
        conn = self._project.connection
        curs = conn.cursor()
        query = '''
            SELECT COUNT(*) FROM hosts WHERE %s = ?
            ''' % (column,)
        curs.execute(
            query, (host,)
        )
        return (0,) != curs.fetchone()
        
        

        
    # Client accessible methods
    
    ##
    # add
    #   Add a new host to the system.
    #   It is a RuntimeError to add a host that's already known.
    #   note that caller should probably take all hosts and convert them inot
    #   FQDN's so that charlie  == charlie.nscl.msu.edu
    #
    # @param host - host to add.
    # @return id (primary key of the host added)
    #
    def add(self,host):
        conn = self._project.connection
        curs = conn.cursor()
        if self._exists(host):
            raise RuntimeError("Project.Hosts.add - duplicate host")
        curs.execute(
            '''
            INSERT INTO hosts (host_name) VALUES (?)
            ''', (host,)
        )
        return curs.lastrowid
        
    ##
    #  list
    #    produces a list of the hosts in the table.  The hosts are
    #    sorted by host name.
    #
    # @return list of dicts.  Each dict  has the keys
    #          *  id        - Primary key of the record.
    #          *  host_name -  Name of the host.
    #
    def list(self):
        result = list()
        
        # Setup so that the query will use the row factory to give us key/value
        # pairs on field names.
        
        conn = self._project.connection
        oldFactory = conn.row_factory
        conn.row_factory = sqlite3.Row
        curs = conn.cursor()
         
        curs.execute('''
            SELECT * FROM hosts ORDER BY host_name ASC
                     ''')
        
        # Fetch the records out in to the result.
        
        row = curs.fetchone()
        while row is not None:
            record = {'id': row['id'], 'host_name': row['host_name']}
            result.append(record)
            row = curs.fetchone()
            
        # Don't forget to restore the original row factory.
        
        conn.row_factory = oldFactory
        return result
    
    ##
    # exists
    #   Determins if a specific host is in the hosts table
    #
    # @param host - the host we are checking for.
    #
    # @return boolean - True if the host is in the table, false otherwise.
    #
    def exists(self,host):
        conn = self._project.connection
        curs = conn.cursor()
        curs.execute('''
            SELECT COUNT(*) FROM hosts WHERE host_name = ?
                    ''', (host,))
        return (curs.fetchone() != (0,))
    
    ##
    # id
    #   Returns the id of a host
    #
    # @param host - name of the host to lookup.
    # @return mixed
    # @retval integer - id of the host if it exists.
    # @retval None    - If the host does not exist.
    #
    def id(self, host):
        conn = self._project.connection
        curs = conn.cursor()
        curs.execute('''
            SELECT id FROM hosts WHERE host_name = ?
                     ''', (host,))
        row = curs.fetchone()
        if row is None:
            return None
        else:
            return row[0]
    
    ##
    # delete
    #   Deletes a row from the hosts table.
    #
    # @param id - The id (primary key) of the host to delete.
    #
    # @throw - It is RuntimeError to attempt to delete a nonexistent row.
    #
    def delete(self, id):
        conn = self._project.connection
        if self._exists(id, 'id'):
            conn.execute('''
                DELETE FROM hosts WHERE id=?                     
                         ''', (id,))
        else:
            raise RuntimeError('project.Hosts.delete - no such host to delete')
    ##
    # modify
    #   Change the name of an existing host.
    #
    # @param id      - id of the host.
    # @param newName - new name for the host.
    #
    def modify(self, id, newName):
        conn = self._project.connection
        if self._exists(id, 'id'):
            conn.execute('''
                UPDATE hosts set host_name=? WHERE id= ?
                         ''', (newName, id))
        else:
            raise RuntimeError('project.Hosts.modify - no such host id')
            
#------------------------------------------------------------------------------

##
# @class Rings
#
#   Manages the rings table ensuring that the host_id is a valid host
#
class Rings:
    def __init__(self, project):
        self._project = project
    
    
    def _exists(self, ringname, hostId):
        conn = self._project.connection
        curs = conn.cursor()
        curs.execute('''
            SELECT COUNT(*) FROM rings WHERE name = ? AND host_id = ?
                     ''', (ringname, hostId)
        )
        return ((0,) != curs.fetchone())
    ##
    # _existsId
    #   Return true if the specified ring id exists.
    #
    # @param id - the id of the ring to test.
    #
    def _existsId(self, id):
        conn = self._project.connection
        curs = conn.cursor()
        curs.execute('''
            SELECT COUNT(*) FROM rings WHERE id = ?
            ''', (id,))
        return ((0,) != curs.fetchone())
    ##
    # _query
    #   Common code to query everything about ring(s)
    # @param params - Parameterization of the query (stuff to fill in ?)
    # @param order - If supplied is an ORDER BY clause for the query.
    # @param where - If specivfied is a WHERE clause for the query.
    #
    def _query(self, params, order = '', where = ''):
        conn   = self._project.connection
        oldFact= conn.row_factory
        conn.row_factory = sqlite3.Row
        curs   = conn.cursor()
            
        query = '''
            SELECT r.id AS ring_id, r.name AS ring_name, r.sourceid,
                   h.id AS host_id, h.host_name AS host_name
                   FROM rings r
                   INNER JOIN hosts h ON h.id = r.host_id
                   %s
                   %s ''' % (where, order)
        curs.execute(query, params)
    
        result = list()
        for r in curs:
            result.append(r)
        
        conn.row_factory = oldFact
        return result
    #-------------------------------------------------------------------------
    # Public interface
    
    ##
    # add_withHostId
    #   Add a ring to the system given the host by id:
    #
    # @param name       - Name of the ring
    # @param hostid     - Id of the host the ring is created in
    # @param sourceid=None - If supplied, the source id associated with the
    #                     ring producer.
    # @return int - The id of the element added to the Rings table.
    # @throw RuntimeError - If the host id does not exist.
    # @throw RuntimeError - The ring already exists on that host.
    #
    def add_withHostId(self, name, hostid, sourceid=None):
        
        # Throw a runtime error if the hostid is not valid:
        
        hosts = Hosts(self._project)
        if not hosts._exists(hostid, 'id'):
            raise RuntimeError('project.Rings.add_withHostId - No such host')
            
        # Throw a runtime error if the ring/hostid combination already exist.
        
        if self._exists(name, hostid):
            raise RuntimeError('project.Rings.add_withHostid - This ring already is defined on that host')
        
        conn   = self._project.connection
        curs   = conn.cursor()
        
        curs.execute('''
            INSERT INTO rings (name, host_id, sourceid) VALUES (?,?,?)
                     ''', (name, hostid, sourceid)
        )
        return curs.lastrowid
    
    ##
    # add_withHostname
    #
    #  Add a ring to the system given the host by hostname.
    #
    # @param name          - Name of the ring.
    # @param hostname      - Name of the host the ring is created in.
    # @param sourceid=None - If provided the id source id associated with
    #                        the ring producer.
    # @param create=False  - If true and the hostname is not in the
    #                        hosts table it is created.
    # @return int   - The id of the element added to the rings table.
    # @throw RuntimeError - if create is False and the hostname is not
    #                       in the hosts table.
    # @throw RuntimeError - if the ring already has been defined on that host.
    #
    def add_withHostname(self, name, hostname, create=False, sourceid=None):
        hosts = Hosts(self._project)
        id    = hosts.id(hostname)
        if (id is None) and create:
            id = hosts.add(hostname)
        return self.add_withHostId(name, id, sourceid)
    
    ##
    # list
    #   Provides a list of the rings known to the system.
    #
    # @return list of dicts  Each dict has the following keys:
    #         * ring_id   - Primary key of the ring in rings table.
    #         * ring_name - Name of the ring.
    #         * sourceid  - Data source id (might be empty).
    #         * host_id   - Id of the host the ring is in.
    #         * host_name - Name of the host the ring is in.
    # The list is ordered by ring name, subordered by host name.
    #
    #
    def list(self):        
        return self._query((), 'ORDER BY ring_name, host_name ASC')    
    
    ##
    # find
    #   Searches for a specific ring and returns its information
    #   Note multiple matches are possible unless the hostname is
    #   provided.
    #
    #  @param ringName  - Name of the ring to find.
    #  @param hostName (None) If supplied the search is qualified by the hostName.
    #
    # @return list of dicts - see list above for the keys in the dict.
    #
    def find(self, ringName, hostName=None):
        if hostName is None:
            whereClause = 'WHERE ring_name=?'
            bindings    = (ringName,)
        else:
            whereClause = 'WHERE ring_name=? AND host_name = ?'
            bindings    = (ringName, hostName)
        
        return self._query(
            bindings, 'ORDER BY ring_name, host_name ASC',
            whereClause
        )

    ##
    # exists
    #   tests for existence of a ring
    #
    # @param ringName - Name of the ring to check for.
    # @param hostName - Name of the host the ring lives in.
    #
    # @return boolean
    #
    def exists(self, ringName, hostName):
        list = self.find(ringName, hostName)
        return len(list) != 0
    ##
    # delete
    #   Deletes a specific ring
    #
    #  @param id - Id of the ring to delete.
    #  @throw RuntimeError if the ring does not exist.
    #
    def delete(self, id):
        
        conn = self._project.connection
        curs = conn.cursor()
        
        curs.execute('''
            SELECT COUNT(*) FROM rings WHERE id = ?
                     ''', (id,))
        if curs.fetchone() == (0,):
            raise RuntimeError('project.Rings.delete - no such ring.')
        
        conn.execute('''
            DELETE FROM rings WHERE id = ?
                     ''', (id,))
    ##
    # delete_byname
    #
    #   Deletes a ring by name
    #
    # @param name - ring name.
    # @param host - host the ring lives in.
    #
    # @throw RuntimeError if the ring does not exist.
    #
    def delete_byname(self, name, host):
        items = self.find(name, host)
        if len(items) == 0:
            raise RuntimeError('project.Rings.delete_byname - no such ring')
        id = items[0]['ring_id']
        self.delete(id)
        
    ##
    # modify
    #   Modify a ring's properties.
    #
    #    @param id - The id of the ring to modify.
    #    @param **attributes  - Key/values of the things that change which can
    #                           be any of:
    #                      *   name     - change the name of the ring.
    #                      *  host_name - Change the name of the host (must exist).
    #                      *  sourceid  - Change the value of the sourceid.
    #   @throw RuntimeError - id is not an existing ring.
    #   @throw RuntimeError - host_name is provided but is not an existing host.
    #   @throw RuntimeError - After host_name has been applied if needed, the
    #                         resulting ring/host would duplicate another ring/host.
    #   @throw RuntimeError - A key is provided that is not legal.
    #
    def modify(self, id, **attributes):
        host = Hosts(self._project)
        
        # raise error if the ring does not exist.
        
        old = self._query((id,), '', 'WHERE ring_id=?')
        if len(old) == 0:
            raise RuntimeError('project.Rings.modify - no such ring')
        old = old[0]
        
        # raise error if there are bad keys:
        
        validKeys = set(['name', 'host_name', 'sourceid'])
        if len(set(attributes.keys()) - validKeys) > 0:
            raise RuntimeError('project.Rings.modify - invalid attribute key(s)')
            
        fields = list()
        values = list()
        for field in attributes.keys():
            #
            #  host_name -> host_id:
            if field == 'host_name':
                host = Hosts(self._project)
                hostid = host.id(attributes[field])
                # Raise an error if the host id is None:
                
                if hostid is None:
                    raise RuntimeError('project.Rings.modify - no such new host')
                
                fields.append('='.join(('host_id', '?')))
                values.append(hostid)
            else :
                # All others are name-> name.
                values.append(attributes[field])
                fields.append('='.join((field, '?')))
        
        values.append(id)
            
        # Check for and throw if the final host/ring name are duplicates:
        
        changing = False
        newHost = old['host_name']
        newRing = old['ring_name']
        if 'host_name' in attributes.keys():
            newHost = attributes['host_name']
            changing = True
        if 'name' in attributes.keys():
            newRing = attributes['name']
            changing = True
        
        # If the host/ring don't change we're ok otherwise need to check.
        
         
        if changing and self.exists(newRing, newHost):
            raise RuntimeError('project.Rings.modify - would make a duplicate ring')
        
        #  Combine fields into a set clause and construct the query:
        
        setFields = ', '.join(fields)
        query = '''
            UPDATE rings SET %s WHERE id = ?
        ''' % setFields
        
        self._project.connection.execute(query, values)
        
    ##
    # modify_byname
    #   Modify a ring's properties given its name
    #
    #  @param name  - Name of the ring to modify.
    #  @param host  - Host the ring is on.
    #  @param attributes - see modify() above.
    #
    #  @note this method figures out the id for name, host and then
    #        invokes modify..
    #
    def modify_byname(self, name, host, **attributes):
        ring = self.find(name, host)
        if len(ring) == 0:
            raise RuntimeError('project.Rings.modify -- no such ring')
        else:
            modify(ring[0]['ring_id'], **attributes)
    
   #---------------------------------------------------------------------------
   #
   
   
##
# @class Programs
#   This class manages the programs and program_args tables which together
#   define a program and its command line parameters.

class Programs:
    def __init__(self, project):
           self._project = project
           self._hosts   = Hosts(self._project)

    ##
    # _getArgs
    #   Get the arguments associated with a program.
    #
    # @param id - The id of the program.
    # @return list (posibly empty)
    #
    def _getArgs(self, id):
        conn = self._project.connection
        curs = conn.cursor()
        
        curs.execute('''
            SELECT argument FROM program_args WHERE program_id = ?
                     ''', (id,))
        result = list()
        for r in curs:
            result.append(r[0])
            
        return result

    ##
    # _query
    #   Perform a query that joins to the args table and marshalls stuff
    #   into the sort of dict returned by list or find.
    #
    # @param where  - If present a WHERE clause for the query.
    # @param qargs   -  query arguments (plug into where:)
    # @return dict - see list
    #
    def _query(self, where, qargs):
        conn       = self._project.connection
        oldFactory = conn.row_factory
        conn.row_factory = sqlite3.Row
        cursor     = conn.cursor()
        
        query = '''
            SELECT p.id AS program_id, p.path, p.working_dir,
                    h.id AS host_id, h.host_name
                FROM programs p
                INNER JOIN hosts h ON h.id = p.host_id
                       ''' 
        query += where
        cursor.execute(query, qargs)
        result = list()
        for row in cursor:
            args = self._getArgs(row['program_id'])
            r = dict(row)
            r['args'] = args
            result.append(r)
        
        conn.row_factory = oldFactory
        return result

    ##
    # _replaceArgs
    #
    #   Replace the arguments for a program (the caller is responsible
    #   for managing the atomicity of this)
    #
    #  @param conn - Connection
    #  @param id   - Id of the program.
    #  @param newargs - List of new paramters.
    # 
    def _replaceArgs(self, conn, id, newargs):
        #
        #  Kill off the old args:
        #
        conn.execute('''
            DELETE FROM program_args WHERE program_id = ?
                     ''', (id,))
        #
        # Crezte the new ones:
        for arg in newargs:
            conn.execute('''
                INSERT INTO program_args (program_id, argument) VALUES (?,?)
                        ''', (id, arg))
            

    #--------------------------------------------------------------------------
    #  Public interface
    
    ##
    # add
    #   Add a new program.
    #
    # @param path        - path to the program executable.
    # @param workingdir  - Directory the program will run in.
    # @param host_id     - Id  of host on which the program runs (hosts table).
    # @param args        - List of additional command line parameters.
    #
    # @return integer    - The id of the program (primary key in programs table).
    # @throw RuntimeError - If host_id does not exist.
    #
    def add(self, path, workingdir, host_id, args=list()):
        
        # Ensure the host exists:
        
        if not self._hosts._exists(host_id, 'id'):
            raise RuntimeError('projects.Programs.add - no such host.')
        
        conn = self._project.connection
        curs = conn.cursor()
        
        # Do the inserts as a single transaction
        
        with conn:
            curs.execute('''
                INSERT INTO programs (path, working_dir, host_id)
                    VALUES(?,?,?)
                         ''', (path, workingdir, host_id))
            id = curs.lastrowid
            for arg in args:
                curs.execute('''
                    INSERT INTO program_args (argument, program_id)
                        VALUES (?,?)
                             ''', (arg, id))
        return id
        
    ##
    # add_byHostname
    #    For the most part this just
    #    looks up the host and calls add
    #
    # @param path         - Path to the program.
    # @param workingdir   - Working directory for the program.
    # @param host_name    - Name of the host on which the program run.s
    # @param args         - Additional program parameters.
    # @param create       - If truem, create the host if it does not exist.
    #
    # @return integer - id of the newly created program record.
    # @throw RuntimeError - the host_name is not a defined host and create =False
    def add_byHostname(self, path, workingdir, host_name, args=list(), create=False):
        hostid = self._hosts.id(host_name)
        if (hostid is None) and create:
            hostid = self._hosts.add(host_name)
        return self.add(path, workingdir, hostid, args)
        
    ##
    # list
    #   Lists all programs and their associated arguments.
    #
    # @return list of dicts   each dict has the following keys:
    #      -  program_id  - Id of the program (primary key)
    #      -  path        - Program path
    #      -  working_dir - Working directory for the program.
    #      -  host_id     - Id of the target host.
    #      -  host_name   - Name of the target host.
    #      -  args        - possibly empty argument list.
    #
    #  Order is by the program_id which is  the order in which the
    #  records where added
    #
    def list(self):
        return self._query("", list())
    ##
    # find
    #   A generic find for programs.
    #
    # @param terms - The values of this parameter represent strings that
    #                will turn into WHERE xxx LIKE value
    #                that will all be anded together to filter the
    #                query.  Possible keys are
    #      -  program_id  - Id of the program (primary key)
    #      -  path        - Program path
    #      -  working_dir - Working directory for the program.
    #      -  host_id     - Id of the target host.
    #      -  host_name   - Name of the target host.
    #
    # For example
    #   {'path': /user/fox/programs/%', host_name: 'spdaq22.nscl.msu.edu'}
    # as the value for terms limits the result set to those programs that
    # are in the /user/fox/programs/ directory that will be started on
    # spdaq22.nscl.msu.edu
    #
    # @return - see list
    # @throw  - If one of the keywords is not a legitimate field selector.
    # @note   - Invalid host names or ids, or program ids are not errors, they
    #           just will return a null result set.
    # @throw RuntimeError - if there's a bad keyword.
    #
    def find(self, **terms):
        # Ensure all the keywords are good, otherwise throw a RuntimeError
        
        validKeys = set(['program_id', 'path', 'working_dir', 'host_id', 'host_name'])
        usedKeys  = set(terms.keys())
        badKeys = usedKeys - validKeys
        if len(badKeys) != 0:
            raise RuntimeError(
                'project.Programs.find - One or more invalid find keys wer provided %r' % badKeys
        )
        # Build and perform the query.
        where = ""
        wheres = list()
        values = list()
        for key in terms.keys():
            value = terms[key]
            wheres.append('%s LIKE ?' % key)
            values.append(value)
        if len(wheres) > 0:
            where = 'WHERE '
            where += 'AND '.join(wheres)
        return self._query(where, values)
    
    
    ##
    # ids
    #   Returns the program ids for a set of programs that match some
    #   criteria.
    # @param **terms - See find above for the meaning of those terms.
    #
    # @return list - possibly empty, of ids (primary keys) of programs that match
    # @throw see find.
    def ids(self, **terms):
        listing = self.find(**terms)
        result = list()
        for row in listing:
            result.append(row['program_id'])
        return result
    
    ##
    # modify
    #   Modify a program's attributes.
    #
    # @param id - the program id to modify.
    # @param **attributes - key/newvalue for the following keys:
    #      -  path        - Program path
    #      -  working_dir - Working directory for the program.
    #      -  host_id     - Id of the target host.
    #      -  host_name   - Name of the target host.
    #      -  args        - Parameters of the program.
    #
    # @throw - if host_id and host_name are both supplied (even if consistent).
    # @throw - if host_id, or host_name does not match an existing host.
    # @throw - if an invalid key is provided in the attributes.
    # @throw - If an invalid id is specified.
    #
    def modify(self, id, **attributes):
        baseAttributes   = set(['path', 'working_dir'])
        hostAttributes = set(['host_name', 'host_id'])
        otherAttributes = set(['args'])
        
        #  Ensure the attributes are all legal:
        
        allowedAttributes = baseAttributes | hostAttributes | otherAttributes
        if len(set(attributes.keys()) - allowedAttributes) > 0:
            raise RuntimeError(
                'project.Programs.modify -- Invalid attributes specified'
            )
            
        # Not allowed to have host_id and host_name:
        
        
        if len(hostAttributes & set(attributes.keys())) > 1 :
            raise RuntimeError(
                'project.Programs.modify - Cannot specify both host_id and host_name'
            )
        
        # The id must exist:
        
        if len(self.ids(program_id=id)) == 0:
            raise RuntimeError(
                'project.Programs.modify - No such program id'
            )
        
        # This is a transaction so it's all or nothing, no partial changes.
        
        conn = self._project.connection
        with conn:
                
            baseAttributes   = set(['path', 'working_dir'])
            
            #
            # Base attributes:
            #
            fields = list()
            values = list()
            for key in baseAttributes:
                if key in attributes.keys():
                    fields.append('%s=?' % key)
                    values.append(attributes[key])
                    
            # If there fields we need to do the update, generate/execute
            # the update statement:
            
            if len(fields) > 0:
                values.append(id)         # For the WHERE field.     
                sql = 'UPDATE programs SET %s WHERE id = ?' % ','.join(fields)
                conn.execute(sql, values)
            
            # host_id requires a bit of checking to be sure it's legal.
            # and to be sure host_name isn't present too.
            
            host_id = None
            if 'host_id' in attributes.keys():
                host_id = attributes['host_id']
                if not self._hosts._exists(host_id, 'id'):
                    raise RuntimeError(
                        'project.Programs.modify - host_id does not exist'
                    )
                    
            # host_name requires a lookup and then modify the host id:
           
            if 'host_name' in attributes.keys():
                host_id = self._hosts.id(attributes['host_name'])
                if host_id is None:
                    raise RuntimeError(
                        'project.Programs.modify - host_name does not exist'
                    )                
                
            if host_id is not None:
                sql = 'UPDATE programs SET host_id = ? WHERE id = ?'
                conn.execute(sql, (host_id, id))
                
            # Args:
            
            if 'args' in attributes.keys():
                newargs = attributes['args']
                self._replaceArgs(conn, id, newargs)
    
    ##
    # Delete a program given its id.
    #
    # @param id - the id of the program.
    #
    # @throw  if id is not a valid program id.
    #
    def delete(self, id):
        
        # Ensure the program exists:
        
        if len(self.ids(program_id=id)) == 0:
            raise RuntimeError(
                'project.Programs.delete - no such program id'
            )
        
        # Do this and the args in a transaction:
        
        conn = self._project.connection
        with conn:
            # Replace the parameters with an empty list to delete them:
            
            self._replaceArgs(conn, id, list())
            conn.execute('''
                DELETE FROM programs WHERE id = ?
                         ''', (id,)
            )
##
# @class EventLoggers
#
#   Manipulate the set of event loggers known to the system.
#   an event logger is a ring (data source) and a directory (data sink).
#
class EventLoggers:
    ##
    # construction
    #  @param project - the project containing the evetn loggers we care about:
    #
    def __init__(self, project):
        self._project = project
        self._rings   = Rings(project)
        
    #-------------------------------------------------------------------------
    # utilities:
    
    ##
    # _exists
    #   True if a specific logger is already defined.
    #
    def _exists(self, ringid, path):
        cursor = self._project.connection.cursor()
        cursor.execute(
            '''
            SELECT COUNT(*) FROM recorded_rings
                WHERE ring_id=? AND directory_path = ?
            ''', (ringid, path)
        )
        row = cursor.fetchone()
        return row[0] == 1
        
    #--------------------------------------------------------------------------
    #  CRUD members:
    
    # C is for CREATE
    
    ##
    # add
    #   adds a new ring given the ring id and a directory path:
    #
    # @param ringId - id of the ring.
    # @param path   - Recording path.
    #
    def add(self, ringId, path):
        
        if self._exists(ringId, path):
            raise RuntimeError('Logger already exists for this ringl')
        
        connection = self._project.connection
        connection.execute(
            '''
            INSERT INTO recorded_rings (ring_id, directory_path)
                VALUES (?,?)
            ''', (ringId, path)
        )
    ##
    # add_byringname
    #    Adds a new event logger given the name/host of the ring
    #
    # @param hostName - Name of the host the ring is on.
    # @param ringName - Name of the ring.
    # @param path     - Directory patht the event files.
    #
    def add_byringname(self, hostName, ringName, path):
        rings = self._rings.find(ringName, hostName)
        
        if len(rings) == 0:
            raise RuntimeError('No such ring as specified')
        
        ringId= rings[0]['ring_id']
        self.add(ringId, path)
        
    # R is for retrieve
    
    ##
    # list
    #   Lists the rings as a (possibly empty) array of dicts where each dict
    #   has the following key/values:
    #   * id       - Id of the entry (primary key).
    #   * ring     - dict containing everthing about the ring (see e.g. Ring.find)
    #   * path     - The path to the recording directory.
    #
    def list(self):
        return self.find()
    
    ##
    # find
    #   Finds a set of matching records from the database
    #
    # @param **match = Keywords match the keywords in the return dict, and
    #                  values are requested matches e.g. ring_name='fox', host_name='ahost'
    #
    # @return array of dicts as per list above
    #
    def find(self, **match):
        whereList = list()
        whereValues = list()
        
        # Ensure the keys are all valid
        
        validKeys = set((
            'ring_id', 'path', 'ring_name', 'sourceid', 'host_id', 'host_name'
        ))
        suppliedKeys = set(match.keys())
        invalidKeys = suppliedKeys - validKeys
        if len(invalidKeys) > 0:
            raise RuntimeError('Invalid Keys: %s' % (', '.join(invalidKeys)))
        
        
        for criterion in match.keys():
            whereList.append('%s=? ' % criterion)
            whereValues.append(match[criterion])
        
        # The base query...need to potentially add a WHERE clause to it:
        
        query = '''
            SELECT rr.id AS id, rr.ring_id AS ring_id, rr.directory_path AS path,
                r.name AS ring_name, r.host_id AS host_id,
                r.sourceid AS sourceid,
                h.host_name AS host_name
                FROM recorded_rings rr
                INNER JOIN rings r ON r.id = rr.ring_id
                INNER JOIN hosts h ON h.id = r.host_id
            '''
        if len(whereList) > 0:
            query = query + ' WHERE ' + 'AND '.join(whereList)
            
        # Query is built now execute it:
        
        oldRowFactory                     = self._project.connection.row_factory
        self._project.connection.row_factory = sqlite3.Row
        curs                                 = self._project.connection.cursor()
        
        curs.execute(query, whereValues)
        
        # Marshal the result:
        
        result = list()
        for r in curs:
            row = dict(id = r['id'], path=r['path'])
            row['ring'] = dict (
                ring_id=r['ring_id'], ring_name=r['ring_name'],
                sourceid=r['sourceid'], host_id=r['host_id'],
                host_name=r['host_name']
            )
            result.append(row)
        
        self._project.connection.row_factory = oldRowFactory
        return result

    #  D is for Delete.
    
    ##
    # delete
    #   Delete the record indicated by id.
    #
    # @param id - the primary key (id) of the record to delete.
    #
    def delete(self, id):
        cursor = self._project.connection.cursor()
        
        cursor.execute(
            '''
            DELETE FROM recorded_rings WHERE id=?
            ''', (id,)
        )
        if cursor.rowcount == 0:
            raise RuntimeError('delete - no such row')
