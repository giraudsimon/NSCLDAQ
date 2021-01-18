##
# @file pypersontests.py
# @brief Test the person wrappings.
#
#
import unittest
from test import support
from LogBook import LogBook
import os

class aTest(unittest.TestCase) :
    def setUp(self) :
        if os.path.isfile('logbook.log') :
           os.unlink('logbook.log') 
        LogBook.create('logbook.log', '0400x', 'Ron Fox', 'Python tests')
        self.logbook = LogBook.LogBook('logbook.log')
    
    def tearDown(self) :
        os.unlink('logbook.log')
        
    def test_nosuch1(self) :
        self.assertRaises(LogBook.error, LogBook.Person, 1, self.logbook)
    def test_nosuch2(self):
        self.assertRaises(LogBook.error, LogBook.Person, id=1, logbook=self.logbook)
    def test_notlogbook(self, ):
        self.assertRaises(LogBook.error, LogBook.Person, 1, 1)
    def test_new1(self):
        self.logbook.add_person('Fox', 'Ron', 'Mr.')   #should not throw.
    def test_new2(self):
        p = self.logbook.add_person('Fox', 'Ron', 'Mr.')
        self.assertIsInstance(p, LogBook.Person)
    def test_id1(self) :
        p1 = self.logbook.add_person('Fox', 'Ron', 'Mr.')
        p2 = self.logbook.add_person("Cerizza", 'Giordaon', salutation='Dr.')
        self.assertEqual(1, p1.id)
        self.assertEqual(2, p2.id)
    def test_lastname(self, ):
        p = self.logbook.add_person('Fox', 'Ron', 'Mr.')
        self.assertEqual('Fox', p.lastname)
    def test_firstname(self, ):
        p = self.logbook.add_person('Fox', 'Ron', 'Mr.')
        self.assertEqual('Ron', p.firstname)
    def test_saultation(self, ):
        p = self.logbook.add_person('Fox', 'Ron', 'Mr.')
        self.assertEqual('Mr.', p.salutation)
    def test_find1(self):
        p = self.logbook.find_people('1=1')
        self.assertEqual(0, len(p))
    def test_find2(self, ):
        p = self.logbook.add_person('Fox', 'Ron', 'Mr.')
        self.logbook.add_person('Cerizza', 'Giordaon', 'Dr.')
        r = self.logbook.find_people('salutation =  "Mr."')
        self.assertEqual(1, len(r))
        self.assertEqual('Fox', r[0].lastname)
    def test_find3(self, ):
        p = self.logbook.add_person('Fox', 'Ron', 'Mr.')
        self.logbook.add_person('Cerizza', 'Giordano', 'Dr.')
        r = self.logbook.find_people('1=1')
        self.assertEqual(2, len(r))
        self.assertEqual('Fox', r[0].lastname)
        self.assertEqual('Giordano', r[1].firstname)
    def test_find4(self, ):
        p1 = self.logbook.add_person('Fox', 'Ron', 'Mr.')
        p2 = self.logbook.add_person('Cerizza', 'Giordano', 'Dr.')
        r = self.logbook.find_people()
        self.assertEqual(2, len(r))
        self.assertEqual('Fox', r[0].lastname)
        self.assertEqual('Giordano', r[1].firstname)
        
    def test_list1(self, ):
        r = self.logbook.list_people()
        self.assertEqual(0, len(r))
    def test_list2(self, ):
        p1 = self.logbook.add_person('Fox', 'Ron', 'Mr.')
        p2 = self.logbook.add_person('Cerizza', 'Giordano', 'Dr.')
        r = self.logbook.list_people()
        self.assertEqual(2, len(r))
        self.assertEqual('Fox', r[0].lastname)
        self.assertEqual('Giordano', r[1].firstname)
    
    def test_get1(self, ):
        self.assertRaises(LogBook.error, self.logbook.get_person, 1)
    def test_get2(self, ):
        p1 = self.logbook.add_person('Fox', 'Ron', 'Mr.')
        p2 = self.logbook.add_person('Cerizza', 'Giordano', 'Dr.')
        p = self.logbook.get_person(p1.id)
        self.assertEqual('Fox', p.lastname)
    
       
    
    
    
    
    
    
    
    

if __name__ == '__main__' :
    unittest.main(verbosity=1)
    