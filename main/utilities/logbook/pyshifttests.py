##
# @file  pyshifttests.py
# @brief Test the python bindings for shifts
    

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
        LogBook.create('logbook.log', '0400x', 'Ron Fox', 'Python tests')
        self.logbook = LogBook.LogBook('logbook.log')
        
        # Make some people:
        
        self.ron = self.logbook.add_person('Fox', 'Ron', 'Mr.')
        self.giordano = self.logbook.add_person('Cerizza', 'Giodano', 'Dr.')
        self.sean = self.logbook.add_person('Liddick', 'Sean', 'Prof.')
        self.alex = self.logbook.add_person('Gade', 'Alexandra', 'Prof.')
        self.ava = self.logbook.add_person('Hill', 'Ava', 'Ms.')

    
    def tearDown(self) :    
        os.unlink('logbook.log')
    
    # Test shift creation - also tests attribute getters:
    
    def test_create1(self) :
        # empty shift.
        
        shift = self.logbook.create_shift('empty_shift')
        self.assertEqual('empty_shift', shift.name)
        self.assertEqual(1, shift.id)
        self.assertEqual(0, len(shift.members))
    
    def test_create_2(self, ):
        # One member; me:
        
        shift = self.logbook.create_shift('ron-shift', members=(self.ron,))
        self.assertEqual(1, len(shift.members))
        self.assertEqual('Fox', shift.members[0].lastname)
    
    def test_create3(self, ):
        # Shift of professors:
        
        shift = self.logbook.create_shift('professors', self.logbook.find_people('salutation = "Prof."'))
        self.assertEqual(2, len(shift.members))
        self.assertEqual('Liddick', shift.members[0].lastname)
        self.assertEqual('Gade', shift.members[1].lastname)
    
    def test_getshift1(self ):
        #  no such shift raises LogBook.error
        
        self.assertRaises(LogBook.error, self.logbook.get_shift, 1)
    
    def test_getshift2(self, ):
        # Can find a shift that exists:
        
        shift = self.logbook.create_shift('professors', self.logbook.find_people('salutation = "Prof."'))
        found = self.logbook.get_shift(shift.id)
        self.assertEqual(shift.name, found.name)
        
    def test_getshift3(self, ):
        # Finds the right one from among more than one:
        shift1 = self.logbook.create_shift('professors', self.logbook.find_people('salutation = "Prof."'))
        shift2 = self.logbook.create_shift('non-doctorates', (self.ron, self.ava))
        
        found = self.logbook.get_shift(shift1.id)
        self.assertEqual(shift1.id, found.id)
        
    def test_list1(self, ):
        # No shifts:
        
        self.assertEqual(0, len(self.logbook.list_shifts()))
    
    def test_list2(self, ):
        # Lists the one shift defined.
        
        shift2 = self.logbook.create_shift('non-doctorates', (self.ron, self.ava))
        listing = self.logbook.list_shifts()
        self.assertEqual(1, len(listing))
        self.assertEqual(shift2.id, listing[0].id)
        
    def test_list3(self, ):
        # Multiple shifts:
        
        shift1 = self.logbook.create_shift('professors', self.logbook.find_people('salutation = "Prof."'))
        shift2 = self.logbook.create_shift('non-doctorates', (self.ron, self.ava))
        
        listing = self.logbook.list_shifts();
        self.assertEqual(2, len(listing))
        self.assertEqual(shift1.id, listing[0].id)
        self.assertEqual(shift2.id, listing[1].id)
        
    def test_find1(self, ):
        # Find shift by name: Nosuch gives None
        
        self.assertRaises(LogBook.error, self.logbook.find_shift, 'no such')
       
        
    def test_find2(self, ):
        shift1 = self.logbook.create_shift('professors', self.logbook.find_people('salutation = "Prof."'))
        found = self.logbook.find_shift('professors')
        self.assertEqual(shift1.id, found.id)
        
    def test_find3(self, ):
        shift1 = self.logbook.create_shift('professors', self.logbook.find_people('salutation = "Prof."'))
        shift2 = self.logbook.create_shift('non-doctorates', (self.ron, self.ava))
        
        found = self.logbook.find_shift('non-doctorates')
        self.assertEqual(shift2.id, found.id)
        self.assertRaises(LogBook.error, self.logbook.find_shift, "Professors")  # miscased.
    
    def test_add1(self, ):
        #  Add member success:
        
        shift = self.logbook.create_shift('Initially empty')
        shift.add_member(self.ron)
        members = shift.members
        self.assertEqual(1, len(members))
        self.assertEqual('Fox', members[0].lastname)
        
    def test_add2(self, ):
        shift = self.logbook.create_shift('professors', self.logbook.find_people('salutation = "Prof."'))
        shift.add_member(self.ron)    # Professors and ron.
        members = shift.members
        self.assertEqual(3, len(members))
        self.assertEqual('Fox', members[2].lastname)
    
    def test_remove1(self):
        # remove shift member
        shift = self.logbook.create_shift('professors', self.logbook.find_people('salutation = "Prof."'))
        shift.remove_member(self.alex)
        m = shift.members
        self.assertEqual(1, len(m))
        self.assertEqual('Liddick', m[0].lastname)
        
    def test_remove2(self, ):
        # Fails on member not in shift:
        
        shift = self.logbook.create_shift('professors', self.logbook.find_people('salutation = "Prof."'))
        self.assertRaises(LogBook.error, shift.remove_member, self.ron)
    
    def test_current1(self, ):
        self.assertIsNone(self.logbook.current_shift())
        
    def test_current2(self, ):
        shift = self.logbook.create_shift('professors', self.logbook.find_people('salutation = "Prof."'))
        self.assertIsNone(self.logbook.current_shift())
        
    def test_current2(self, ):
        shift1 = self.logbook.create_shift('professors', self.logbook.find_people('salutation = "Prof."'))
        shift2 = self.logbook.create_shift('non-doctorates', (self.ron, self.ava))
        
        shift1.set_current()
        self.assertEqual(shift1.id, self.logbook.current_shift().id)
        shift2.set_current()
        self.assertEqual(shift2.id, self.logbook.current_shift().id)
        
if __name__ == '__main__' :
    unittest.main()
    