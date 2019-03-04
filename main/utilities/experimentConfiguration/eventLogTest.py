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
# @file   eventLogTest.py
# @brief  Tests of the Project.EventLoggers class
# @author <fox@nscl.msu.edu>

import project
import unittest

class TestEventLog(unittest.TestCase):
    def setUp(self):
        self._project = project.Project(':memory:')
        self._project.create()
        self._hosts   = project.Hosts(self._project)
        self._rings   = project.Rings(self._project)
        self._loggers = project.EventLoggers(self._project)

        # Create a host and a ring that all will uswe:
        
        self._host = 'charlie.nscl.msu.edu'
        self._ringName = 'fox'
        self._charlieId = self._hosts.add(self._host)
        self._foxId     = self._rings.add_withHostId(self._ringName, self._charlieId, 0)
        
    def tearDown(self):
        pass
    
    ##
    #  add - Test ability to add a logger using raw table info.
    #
    def test_add1(self):
        path = '/scratch/fox/test'
        self._loggers.add(self._foxId, path)
        
        # See if the table has the right stuff.
    
        connection = self._project.connection
        cursor     = connection.cursor()
        cursor.execute('''
            SELECT * FROM recorded_rings
                       '''
        )
        row = cursor.fetchone()
        self.assertIsNotNone(row)
        self.assertEqual((1, self._foxId, path), row)
        self.assertIsNone(cursor.fetchone())
        
    def test_add2(self):
        path1 = '/scratch/fox/test1'
        path2 = '/scratch/fox/test2'
        
        self._loggers.add(self._foxId, path1)
        self._loggers.add(self._foxId, path2)

        connection = self._project.connection
        cursor     = connection.cursor()
        cursor.execute('''
            SELECT * FROM recorded_rings
                       '''
        )
        row = cursor.fetchone()
        self.assertIsNotNone(row)
        self.assertEqual((1, self._foxId, path1), row)
        
        row = cursor.fetchone()
        self.assertIsNotNone(row)
        self.assertEqual((2, self._foxId, path2), row)
        
        self.assertIsNone(cursor.fetchone())
    
    def test_add_dup(self):
        path1 = '/scratch/fox/test'
        
        self._loggers.add(self._foxId, path1)
        threw      = False
        rightThrow = False
        
        # Not allowed to add a duplicate.
        
        try:
            self._loggers.add(self._foxId, path1)
        except RuntimeError:
            threw = True
            rightThrow = True
        except:
            threw = True
            
        self.assertTrue(threw)
        self.assertTrue(rightThrow)
    
    # add_byringname - look up the ring id rathern than
    #                  using the ring id directly.
    
    def test_add_byringname_ok(self):
        path = '/scratch/fox/test'
        self._loggers.add_byringname(self._host, self._ringName, path)
 
        # Make sure the tables have the right stuff.
    
        connection = self._project.connection
        cursor     = connection.cursor()
        cursor.execute('''
            SELECT * FROM recorded_rings
                       '''
        )
        row = cursor.fetchone()
        self.assertIsNotNone(row)
        self.assertEqual((1, self._foxId, path), row)
        self.assertIsNone(cursor.fetchone())
    
    def test_add_byringname_nosuchring(self):
        path = '/scratch/fox/test'
        
        threw       = False
        threwRight  = False
        try:
            self._loggers.add_byringname(self._host, 'george', path)
        except RuntimeError:
            threw = True
            threwRight = True
        except:
            threw = True
            
        self.assertTrue(threw)
        self.assertTrue(threwRight)
    
    # list - list the event loggers.
    
    def test_list_empty(self):
        loggers = self._loggers.list()
        self.assertEquals(0, len(loggers))
        
    def test_list_1(self):
        path = '/scratch/fox/test'
        self._loggers.add_byringname(self._host, self._ringName, path)
        loggers = self._loggers.list()
        
        self.assertEquals(1, len(loggers))
        self.assertEquals(dict(
            id   = 1,
            path = path,
            ring = dict(
                ring_id   = 1,
                ring_name = self._ringName,
                sourceid  = 0,
                host_id   = 1,
                host_name = self._host
            )
        ), loggers[0])
    #
    #  find
    
    def test_find_empty(self):
        path = '/scratch/fox/test'
        
        info = self._loggers.find()
        self.assertEquals(0, len(info))
    
    def test_find_match_nocond(self):
        path = '/scratch/fox/test'
        self._loggers.add_byringname(self._host, self._ringName, path)
        result = self._loggers.find()
        
        self.assertEquals(1, len(result))
        self.assertEquals(dict(
            id   = 1,
            path = path,
            ring = dict(
                ring_id   = 1,
                ring_name = self._ringName,
                sourceid  = 0,
                host_id   = 1,
                host_name = self._host
            )
        ), result[0])
        
    def test_find_nomatch_ring(self):
        path = '/scratch/fox/test'
        self._loggers.add_byringname(self._host, self._ringName, path)
        result = self._loggers.find(ring_name='george')
        
        self.assertEquals(0, len(result))
        
    def test_find_match_ring(self):
        path = '/scratch/fox/test'
        self._loggers.add_byringname(self._host, self._ringName, path)
        result = self._loggers.find(ring_name=self._ringName)
        
        self.assertEquals(1, len(result))
        self.assertEquals(dict(
            id   = 1,
            path = path,
            ring = dict(
                ring_id   = 1,
                ring_name = self._ringName,
                sourceid  = 0,
                host_id   = 1,
                host_name = self._host
            )
        ), result[0])
        
    def test_find_nomatch_directory_path(self):
        path = '/scratch/fox/test'
        self._loggers.add_byringname(self._host, self._ringName, path)
        result = self._loggers.find(path='/no/such/path')
        
        self.assertEqual(0, len(result))
    
    def test_find_match_directory_path(self):
        path = '/scratch/fox/test'
        self._loggers.add_byringname(self._host, self._ringName, path)
        result = self._loggers.find(path=path)
        
        self.assertEquals(1, len(result))
        self.assertEquals(dict(
            id   = 1,
            path = path,
            ring = dict(
                ring_id   = 1,
                ring_name = self._ringName,
                sourceid  = 0,
                host_id   = 1,
                host_name = self._host
            )
        ), result[0])
    
    def test_find_nomatch_ring_dir(self):
        path = '/scratch/fox/test'
        self._loggers.add_byringname(self._host, self._ringName, path)
        result = self._loggers.find(path=path, ring_name='george')
        
        self.assertEquals(0, len(result))
    
    def test_find_match_ring_dir(self):
        path = '/scratch/fox/test'
        self._loggers.add_byringname(self._host, self._ringName, path)
        result = self._loggers.find(path=path, ring_name=self._ringName)
        
        self.assertEquals(1, len(result))
        self.assertEquals(dict(
            id   = 1,
            path = path,
            ring = dict(
                ring_id   = 1,
                ring_name = self._ringName,
                sourceid  = 0,
                host_id   = 1,
                host_name = self._host
            )
        ), result[0])
    
    def test_find_badkey(self):
        threw      = False
        rightThrow = False
        
        try:
            result = self._loggers.find(george='harry')
        except RuntimeError:
            threw = True
            rightThrow = True
        except:
            threw = True
            
        self.assertTrue(threw)
        self.assertTrue(rightThrow)
  
  #  Delete
  
    def test_delete_deleted(self):
        path = '/scratch/fox/test'
        self._loggers.add_byringname(self._host, self._ringName, path)
        result = self._loggers.list()
    
        id = result[0]['id']
        self._loggers.delete(id)
        
        # After deleting the only item, we should have an empty table:
        
        self.assertEquals(0, len(self._loggers.list()))
   
    def test_delete_right_one(self):
        path1 = '/scratch/fox/test1'
        path2 = '/scratch/fox/test2'
        
        self._loggers.add(self._foxId, path1)
        self._loggers.add(self._foxId, path2)
        
        oneToDelete = self._loggers.find(path=path1)
        self._loggers.delete(oneToDelete[0]['id'])
        
        remaining = self._loggers.list()
        self.assertEquals(path2, remaining[0]['path'])
    
    def test_delete_fail(self):
        threw      = False
        threwRight = False
        
        try:
            self._loggers.delete(1234)
        except RuntimeError:
            threw = True
            threwRight = True
        except:
            threw = True
        self.assertTrue(threw)
        self.assertTrue(threwRight)
        
        
        
if __name__ == '__main__':
    unittest.main() 