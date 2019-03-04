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
# @file   projectHostsTest.py
# @brief  Tests for Project.Hosts
# @author <fox@nscl.msu.edu>


import unittest
import project

class projectHostsTest(unittest.TestCase):
    
    def setUp(self):
        self._project = project.Project(':memory:')
        self._project.create()
        self._hosts   = project.Hosts(self._project)
        
    def tearDown(self):
        pass
    
    ##
    # test_addok
    #   Successful add of a new host:
    #
    def test_addok(self):
        host = 'test.nscl.msu.edu'
        self._hosts.add(host)
        conn = self._project.connection
        curs = conn.cursor()
        curs.execute(
            '''
            SELECT COUNT(*) FROM hosts WHERE host_name = ?
            ''', (host,)
        )
        self.assertEquals((1,), curs.fetchone())
    ##
    #  test_addDup
    #   adding a duplicate should cry foul via a RuntimeError:
    # 
    def test_addDup(self):
        host = 'test.nscl.msu.edu'
        self._hosts.add(host)               # should work.
        
        threw = False
        try:
            self._hosts.add(host)
        except RuntimeError:
            threw = True
        except:
            pass

        self.assertTrue(threw)
    ##
    # test_list_empty
    #    Listing and empty hosts table gives an empy dict.
    #
    def test_list_empty(self):
        result = self._hosts.list()
        self.assertEquals(0, len(result))
    
    ##
    # test_list_1
    #    List when table has one element:
    #
    def test_list_1(self):
        host  = 'test.nscl.msu.edu'
        self._hosts.add(host)
        result = self._hosts.list()
        self.assertEquals(1, len(result))
        self.assertEquals(1, result[0]['id'])
        self.assertEquals(host, result[0]['host_name'])
    ##
    # test_list_order
    #    Lists come out in alphabetical host order.
    #
    def test_list_order(self):
        host1 ='test.nscl.msu.edu'
        host2 = 'charlie.nscl.msu.edu'
        self._hosts.add(host1)
        self._hosts.add(host2)
        result = self._hosts.list()
        
        self.assertEquals(host2, result[0]['host_name'])
        self.assertEquals(host1, result[1]['host_name'])
    ##
    # tests_exists_no
    #   Tests existence for something that does not exist.
    #
    def test_exists_no(self):
        self.assertFalse(self._hosts.exists('test.nscl.msu.edu'))
    
    ##
    # test_exists_yes
    #    tests existence for something that does exist.
    #
    def test_exists_yes(self):
        host='test.nscl.msu.edu'
        self._hosts.add(host)
        self.assertTrue(self._hosts.exists(host))
    ##
    # test_delete_exists
    #
    def test_delete_exists(self):
        host = 'test.nscl.msu.edu'
        id = self._hosts.add(host)
        self._hosts.delete(id)
        self.assertFalse(self._hosts.exists(host))
    
    ##
    # test_delete_nonexist
    #
    def test_delete_nonexist(self):
        thrown = False
        try:
            self._hosts.delete(124)
        except RuntimeError:
            thrown = True
        except:
            pass
        self.assertTrue(thrown)
    ##
    # test_id_exists
    #    Tests the id function if the host exists.
    #
    def test_id_exists(self):
        host = 'test.nscl.msu.edu'
        id = self._hosts.add(host)
        self.assertEquals(id, self._hosts.id(host))
            
    ##
    # test_id_nonexist
    #   Tests the id functino if the host does not exist.
    #
    def test_id_nonexist(self):
        self.assertEquals(None, self._hosts.id('no.such.host'))
    
    ##
    # test_modify_ok
    #    Test changing the name of an existing host.
    #
    def test_modify_ok(self):
        host='test.nscl.msu.edu'
        mhost='spdaq.nscl.msu.edu'
        id  = self._hosts.add(host)
        self._hosts.modify(id, mhost)
        self.assertEquals(id, self._hosts.id(mhost))
        
    ##
    # test_modify_nosuch
    #   exception if modifying the name of a host that does not exist.
    #
    def test_modify_nosuch(self):
        thrown = False
        try:
            self._hosts.modify(1234, 'no.such.item')
        except RuntimeError:
            thrown = True
        except:
            pass
        self.assertTrue(thrown)
        
if __name__ == '__main__':
    unittest.main()