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
# @file   projectRingTest.py
# @brief  Tests for the project.Rings class.
# @author <fox@nscl.msu.edu>

import unittest
import project

class projectRingTest(unittest.TestCase):
    def setUp(self):
        self._project = project.Project(':memory:')
        self._project.create()
        self._hosts   = project.Hosts(self._project)
        self._rings   = project.Rings(self._project)
    def tearDown(self):
        pass
    
    
    ###
    #  Tests for add_withHostid
    
    ##
    # test_add_nosource
    #   Legally add a ring with no sourcid
    #
    def test_add_nosource(self):
        ringname = 'fox'
        hostid = self._hosts.add('test.nscl.msu.edu')
        ringid = self._rings.add_withHostId(
            ringname, hostid
            )
        
        conn = self._project.connection
        curs = conn.cursor()
        curs.execute('''
            SELECT * FROM rings WHERE name=?
                     ''', (ringname,))
        row = curs.fetchone()
        self.assertNotEquals(None, row)
        self.assertEquals(ringid, row[0])
        self.assertEqual(ringname, row[1])
        self.assertEqual(hostid, row[2])
        
    
    ##
    # test_add_source
    #    Legally add a ring with a sourceid.
    #
    def test_add_source(self):
        ringname = 'fox'
        sourceid  = 1234
        hostid = self._hosts.add('test.nscl.msu.edu')
        ringid = self._rings.add_withHostId(
            ringname, hostid, sourceid
            )
        
        conn = self._project.connection
        curs = conn.cursor()
        curs.execute('''
            SELECT * FROM rings WHERE name=?
                     ''', (ringname,))
        row = curs.fetchone()
        self.assertNotEquals(None, row)
        self.assertEquals(ringid, row[0])
        self.assertEqual(ringname, row[1])
        self.assertEqual(hostid, row[2])
        self.assertEqual(sourceid, row[3])
    
    ##
    # test_add_nohost
    #    Adding a ring with an invalid host id should
    #    raise a RuntimeError
    #
    def test_add_nohost(self):
        thrown = False
        try:
            self._rings.add_withHostId(
                'testring', 1234
            )
        except RuntimeError:
            thrown = True
        except:
            pass
        
        self.assertTrue(thrown)
    
    ##
    # test_add_dup
    #
    def test_add_dup(self):
        ringname = 'fox'
        hostid   = self._hosts.add('test.nscl.msu.edu')
        self._rings.add_withHostId(ringname, hostid)
        
        thrown = False
        try:
            self._rings.add_withHostId(ringname, hostid)
        except RuntimeError:
            thrown = True
        except:
            pass
        
        self.assertTrue(thrown)
    ###
    # Tests for add_withHostname

    
    ##
    # test_addwhost_ok
    #   Test add_withHostname when everything is in order.
    #
    def test_addwhost_ok(self):
        hostname = 'test.nscl.msu.edu'
        ringname = 'fox'
        
        hostid = self._hosts.add(hostname)
        ringid = self._rings.add_withHostname(ringname, hostname)

        conn = self._project.connection
        curs = conn.cursor()
        curs.execute('''
            SELECT * FROM rings WHERE name=?
                     ''', (ringname,))
        row = curs.fetchone()
        self.assertNotEquals(None, row)
        self.assertEquals(ringid, row[0])
        self.assertEqual(ringname, row[1])
        self.assertEqual(hostid, row[2])
        
    ##
    # test_addwhost_srcid
    #
    def test_addwhost_srcid(self):
        hostname = 'test.nscl.msu.edu'
        ringname = 'fox'
        
        hostid = self._hosts.add(hostname)

        srcid  = 1234
        ringid = self._rings.add_withHostname(ringname, hostname, False, srcid)
        conn = self._project.connection
        curs = conn.cursor()
        curs.execute('''
            SELECT * FROM rings WHERE name=?
                     ''', (ringname,))
        
        
        row = curs.fetchone()
        self.assertNotEquals(None, row)
        self.assertEquals(ringid, row[0])
        self.assertEqual(ringname, row[1])
        self.assertEqual(hostid, row[2])
        self.assertEqual(srcid, row[3])
        
        
    ##
    # test_addwhost_nocreate
    #    Creating a ring with a new host but no setting create true
    #    should raise RunTime exception.
    #
    def test_addwhost_nocreate(self):
        thrown = False
        try:
            self._rings.add_withHostname('fox', 'test.nscl.msu.edu')
        except RuntimeError:
            thrown = True
        except:
            pass
        
        self.assertTrue(thrown)
    
    ##
    # test_addwhost_create
    #
    def test_addwhost_create(self):
        ringname = 'fox'
        hostname = 'test.nscl.msu.edu'
        
        id = self._rings.add_withHostname(ringname, hostname, True)
        conn = self._project.connection
        curs = conn.cursor()
        curs.execute('''
            SELECT name, host_name FROM rings
                INNER JOIN hosts ON hosts.id = rings.host_id
                WHERE rings.id = ?
                ''', (id,))
        row = curs.fetchone()
        self.assertNotEquals(None, row)
        self.assertEquals(ringname, row[0])
        self.assertEquals(hostname, row[1])
    
    ###
    # Tests for the list method
    
    ##
    # test_list_empty
    #    listing an empty table gives an empty list.
    #
    def test_list_empty(self):
        result = self._rings.list()
        self.assertEquals(list(), result)
    
    ## test_list_one'
    #    Listing a table with a single element gets it.
    #
    def test_list_one(self):
        ringname  = 'fox'
        hostname = 'test.nscl.msu.edu'
        sourceid = 1234
        hostid   = self._hosts.add(hostname)
        ringid   = self._rings.add_withHostname(ringname, hostname, False, sourceid)
        rings = self._rings.list()
        
        self.assertEquals(1, len(rings))
        ring = rings[0]
        self.assertEquals(ringname, ring['ring_name'])
        self.assertEquals(ringid,   ring['ring_id'])
        self.assertEquals(sourceid, ring['sourceid'])
        self.assertEquals(hostid,   ring['host_id'])
        self.assertEquals(hostname, ring['host_name'])
    
    ##
    # test_list_4
    #    Listing of multiple rings in multiple hosts
    #    gets them all appropriately connected to their hosts.
    #
    def test_list_4(self):
        self._rings.add_withHostname('fox1', 'test.nscl.msu.edu', True)
        self._rings.add_withHostname('fox1', 'aaa.nscl.msu.edu', True)
        self._rings.add_withHostname('foxy', 'test.nscl.msu.edu')
        self._rings.add_withHostname('foxy', 'aaa.nscl.msu.edu')
        
        rings = self._rings.list()
        
        ring = rings[0]
        self.assertEquals('fox1', ring['ring_name'])
        self.assertEquals('aaa.nscl.msu.edu', ring['host_name'])
        
        ring = rings[1]
        self.assertEquals('fox1', ring['ring_name'])
        self.assertEquals('test.nscl.msu.edu', ring['host_name'])
        
        ring = rings[2]
        self.assertEquals('foxy', ring['ring_name'])
        self.assertEquals('aaa.nscl.msu.edu', ring['host_name'])

        ring = rings[3]
        self.assertEquals('foxy', ring['ring_name'])
        self.assertEquals('test.nscl.msu.edu', ring['host_name'])
    ##
    #  test_find_0
    #     Find with no matches.
    
    def test_find_0(self):
        rings = self._rings.find('no such ring')
        self.assertEquals(list(), rings)
    
    ##
    # test_find_1
    #   Find with one match.
    def test_find_1(self):
        self._rings.add_withHostname('fox1', 'test.nscl.msu.edu', True, 1)
        self._rings.add_withHostname('foxy', 'test.nscl.msu.edu', True, 2)
        
        result = self._rings.find('fox1')
        self.assertTrue(1, len(result))
        ring = result[0]
        self.assertEquals('fox1', ring['ring_name'])
        self.assertEquals('test.nscl.msu.edu', ring['host_name'])
        self.assertEquals(1, ring['sourceid'])
        
    
    
    ##
    # test_find_multiple
    #      find with multiple matches.
    #
    def test_find_multiple(self):
        self._rings.add_withHostname('fox1', 'test.nscl.msu.edu', True, 1)
        self._rings.add_withHostname('fox1', 'aaa.nscl.msu.edu', True, 2)
        
        result = self._rings.find('fox1')
        self.assertEquals(2, len(result))
        ring = result[0]
        self.assertEquals('fox1', ring['ring_name'])
        self.assertEquals('aaa.nscl.msu.edu', ring['host_name'])
        self.assertEquals(2, ring['sourceid'])
         
        ring = result[1]
        self.assertEquals('fox1', ring['ring_name'])
        self.assertEquals('test.nscl.msu.edu', ring['host_name'])
        self.assertEquals(1, ring['sourceid'])
 
    ##
    # test_find_whost
    #      Find specifyhing host
    #
    def test_find_whost(self):
        self._rings.add_withHostname('fox1', 'test.nscl.msu.edu', True, 1)
        self._rings.add_withHostname('fox1', 'aaa.nscl.msu.edu', True, 2)
        
        result = self._rings.find('fox1', 'aaa.nscl.msu.edu')
        self.assertEquals(1, len(result))
        ring = result[0]
        self.assertEquals('fox1', ring['ring_name'])
        self.assertEquals('aaa.nscl.msu.edu', ring['host_name'])
        self.assertEquals(2, ring['sourceid'])
         
        
    ##
    # test_find_whost_0
    #    No matches because the host is specified.
    #
    def test_find_whost_0(self):
        self._rings.add_withHostname('fox1', 'test.nscl.msu.edu', True, 1)
        self._rings.add_withHostname('fox1', 'aaa.nscl.msu.edu', True, 2)
        
        result = self._rings.find('fox1', 'no.such.host')
        self.assertEquals(0, len(result))
    
    ##
    # test_exists_0
    #
    def test_exists_0(self):
        self.assertEquals(0, self._rings.exists('test ring', 'no.such.host'))
    ##
    # test_exists_1
    #
    def test_exists_1(self):
        self._rings.add_withHostname('fox1', 'test.nscl.msu.edu', True, 1)
        self.assertEquals(1, self._rings.exists('fox1', 'test.nscl.msu.edu'))
    ##
    # test_delete_ok
    
    def test_delete_ok(self):
        id = self._rings.add_withHostname('fox1', 'test.nscl.msu.edu', True, 1)
        self._rings.delete(id)
        self.assertFalse(self._rings.exists('fox1', 'test.nscl.msu.edu'))
     
    ##
    # test_delete_nosuch
   
    def test_delete_nosuch(self):
        throw = False
        try:
            self._rings.delete(1234)
        except RuntimeError:
            throw = True
        except:
            pass
        
        self.assertTrue(throw)
        
    ##
    # test_delete_byname_ok
    #    Delete works.
    def test_delete_byname_ok(self):
        host = 'test.nscl.msu.edu'
        ring = 'fox1'
        self._rings.add_withHostname(ring, host, True, 1)
        self._rings.delete_byname(ring, host)
        self.assertFalse(self._rings.exists(ring, host))
                     
    ##
    # test_delete_byname_noring
    
    def test_delete_byname_noring(self):
        host = 'test.nscl.msu.edu'
        ring = 'fox1'
        self._rings.add_withHostname(ring, host, True, 1)
        
        thrown = False
        try:
            self._rings.delete_byname('fox', host)
        except RuntimeError:
            thrown = True
        except:
            pass
        self.assertTrue(thrown)
        
    ## 
    # test_delet_byname_nohost
    #
    def test_delete_byname_nohost(self):
        host = 'test.nscl.msu.edu'
        ring = 'fox1'
        self._rings.add_withHostname(ring, host, True, 1)
        
        thrown = False
        try:
            self._rings.delete_byname(ring, 'no.such.host')
        except RuntimeError:
            thrown = True
        except:
            pass
        self.assertTrue(thrown)
    ##
    # test_modify_ok
    #    Change everthing about a ring and ok.
    #
    def test_modify_ok(self):
        id = self._rings.add_withHostname('fox', 'host.nscl.msu.edu', True, 1)
        self._hosts.add('test.nscl.msu.edu')
        self._rings.modify(
            id, name = 'fox1', host_name = 'test.nscl.msu.edu',
            sourceid = 1234
        )
        ring = self._rings.find('fox1', 'test.nscl.msu.edu')
        self.assertEquals(1, len(ring))
        ring = ring[0]
        self.assertEquals(1234, ring['sourceid'])
    
    ##
    # test_modify_nosuchring
    #    No such ring
    def test_modify_nosuchring(self):
        threw = False
        try:
            self._rings.modify(
                1, name = 'fox1', host_name = 'test.nscl.msu.edu',
                sourceid = 1234
            )
        except RuntimeError:
            threw = True
        self.assertTrue(threw)
        
    ##
    # test_modify_nosuchhost
    #    No such host.
    #
    def test_modify_nosuchhost(self):
        id = self._rings.add_withHostname('fox', 'host.nscl.msu.edu', True, 1)
        threw = False
        try:
            self._rings.modify(
                id, name = 'fox1', host_name = 'test.nscl.msu.edu',
                sourceid = 1234
            )
        except RuntimeError:
            threw = True
        except:
            pass
        self.assertTrue(threw)


    ##
    # test_modify_wouldduplicate
    #    resulting ring/host would be a duplicate
    
    def test_modify_wouldduplicate(self):
        id = self._rings.add_withHostname('fox', 'host.nscl.msu.edu', True, 1)
        self._rings.add_withHostname('fox', 'test.nscl.msu.edu', True, 2)
        
        threw = False
        try:
            self._rings.modify(id, host_name='test.nscl.msu.edu')
        except RuntimeError:
            threw = True
        except:
            print("some other exception")
            pass
        
        self.assertTrue(threw)
    
    ##
    # test_modify_badkey
    #   Bad modify key.
    def test_modify_badkey(self):
        id = self._rings.add_withHostname('fox', 'host.nscl.msu.edu', True, 1)
        threw = False
        try:
            self._rings.modify(id, whirlygig = 'junko')
        except RuntimeError:
            threw = True
        except:
            pass
        self.assertTrue(threw)
        
    ##
    # test__modifybyname_ok
    #
    def test_modifybyname_ok(self):
       id  = self._rings.add_withHostname('fox', 'host.nscl.msu.edu', True, 1)
       self._rings.modify(id, sourceid=1234)
       ring = self._rings.find('fox', 'host.nscl.msu.edu')
       ring = ring[0]
       self.assertEquals(1234, ring['sourceid'])
    
    ##
    # test_modifybyname_noring
    #
    def test_modifybyname_noring(self):
        threw =  False
        try:
            self._rings.modify(1, sourceid=1234)
        except RuntimeError:
            threw = True
        except:
            pass
        self.assertTrue(threw)
        
        
        
        

if __name__ == '__main__':
    unittest.main()