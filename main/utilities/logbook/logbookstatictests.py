##
# @file logbookstatictests.py
# @brief Test logbook module's static methods and attributes.
#
#
import unittest
import os
import LogBook.LogBook as LogBook

from test import support

class LogBookStatics(unittest.TestCase) :
    def setUp(self) :
        if os.path.isfile('logbook.log') :
           os.unlink('logbook.log') 
    
    def tearDown(self) :
        if os.path.isfile('logbook.log') :
           os.unlink('logbook.log')
        
    def test_create(self) :
        LogBook.create('logbook.log', '0400x', 'Ron Fox', 'Test python bindings')
        self.assertTrue(os.path.isfile('logbook.log'))
    def test_construct1(self):
        LogBook.create('logbook.log', '0400x', 'Ron Fox', 'Test python binding')
        a = LogBook.LogBook("logbook.log")
        self.assertIsInstance(a, LogBook.LogBook)
    def test_construct2(self):
        self.assertRaises(LogBook.error, LogBook.LogBook, 'logbook.log')
    def test_construct_3(self):
        LogBook.create('logbook.log', '0400x', 'Ron Fox', 'Test python binding')
        a = LogBook.LogBook(filename='logbook.log')
        self.assertIsInstance(a, LogBook.LogBook)
    
    
        
    

if __name__ == '__main__' :
    unittest.main()
