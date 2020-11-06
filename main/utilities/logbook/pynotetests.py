##
# @file
# @brief
#
#
import unittest
from test import support
import os
from LogBook import LogBook
from datetime import date
from datetime import datetime


class aTest(unittest.TestCase) :
    def setUp(self) :
        if os.path.isfile('logbook.log') :
            os.unlink('logbook.log')
        LogBook.create('logbook.log', '0400x', 'Ron Fox', 'Python tests')
        self.logbook = LogBook.LogBook('logbook.log')
        
        # Make some people:
        
        self.ron = self.logbook.add_person('Fox', 'Ron', 'Mr.')
        self.giordano = self.logbook.add_person('Cerizza', 'Giodano', 'Dr.')
        self.sean = self.logbook.add_person('Liddick', 'Sean', 'Prof.')
        self.alex = self.logbook.add_person('Gade', 'Alexandra', 'Prof.')
        self.ava = self.logbook.add_person('Hill', 'Ava', 'Ms.')
        
        self.shift1 = self.logbook.create_shift("Everybody", self.logbook.list_people())
        self.shift1.set_current()
        
        # Make some runs:
        
        self.run1 = self.logbook.begin_run(1, "Test run1")
        self.run1.end()
        self.run2 = self.logbook.begin_run(2, "Test run2")   # Current run.
    
    def tearDown(self) :
        del self.shift1
        
        del self.ron
        del self.giordano
        del self.sean
        del self.alex
        del self.ava
        del self.run1
        del self.run2
        
        del self.logbook
        os.unlink('logbook.log')
        
    def test_sometest(self) :
        pass

if __name__ == '__main__' :
    unittest.main()
    