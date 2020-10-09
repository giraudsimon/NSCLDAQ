##
# @file logbookstatictests.py
# @brief Test logbook module's static methods and attributes.
#
#
import unittest
import os
import LogBook.LogBook as LogBook

from test import support

class aTest(unittest.TestCase) :
    def setUp(self) :
        if os.path.isfile('logbook.log') :
           os.unlink('logbook.log') 
    
    def tearDown(self) :
        if os.path.isfile('logbook.log') :
           os.unlink('logbook.log')
        
    def test_sometest(self) :
        LogBook.create('logbook.log', '0400x', 'Ron Fox', 'Test python bindings')
        assert(os.path.isfile('logbook.log'))
        

if __name__ == '__main__' :
    unittest.main()
