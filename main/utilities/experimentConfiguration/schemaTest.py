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
# @file   schemaTest.py
# @brief  Test schema creation etc. for projects.
# @author <fox@nscl.msu.edu>

import project
import unittest

class TestProject(unittest.TestCase):
    # Utility code:
    
    ##
    # check_table
    #   assert the presence of a specific table:
    def check_table(self, tableName):
        conn = self._project.connection
        curs = conn.cursor()
        curs.execute(
            'SELECT COUNT(*) FROM sqlite_master WHERE type="table" AND name=?',
            (tableName,)
        )
        self.assertEquals((1,), curs.fetchone())        
    
    # Common test code:
    
    def setUp(self):
        self._project = project.Project(':memory:')
        self._project.create()
        
    def tearDown(self):
        pass
    
    # The tests:
    
    ##
    # test_create_nodes
    #   Creating a project should give us a nodes table:
    #
    def test_create_hosts(self):
        self.check_table('hosts')
    ##
    # test_create_rings
    #    Creating  a project should give us a rings table:
    #
    def test_create_rings(self):
        self.check_table('rings')
    ##
    # test_create_programs
    #
    def test_create_programs(self):
        self.check_table('programs')
    ##
    # test_create_program_args
    #
    def test_create_program_args(self):
        self.check_table('program_args')
        
    ##
    #  test_create_recorded_rings
    def test_create_recorded_rings(self):
        self.check_table('recorded_rings')
        
if __name__ == '__main__':
    unittest.main()