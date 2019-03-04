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
# @file   stateTest.py
# @brief  tests for the state class.
# @author <fox@nscl.msu.edu>

import unittest
import state

class StateTests(unittest.TestCase):
    
    
    ##
    # test_project_exists
    #   Ensure State has a project variable.
    #
    def test_project_exists(self):
        thrown = False
        try:
            project = state.State.project
            self.assertEquals(None, project)
        except:
            thrown = True
            
        self.assertFalse(thrown)