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
        
    def test_create1(self) :
        note = self.logbook.create_note("This is some text")
        self.assertEqual(1, note.id)
        self.assertEqual('This is some text', note.contents)
        self.assertIsNone(note.run)
        self.assertEqual(0, note.image_count())
        
    def test_create2(self, ):
        note = self.logbook.create_note("This is some text", run=self.run1)
        run  = note.run
        self.assertEqual(run.number, 1)
        self.assertEqual(run.title, 'Test run1')
        self.assertEqual(note.contents, 'This is some text')
        self.assertEqual( note.image_count(), 0)
        self.assertEqual(note.substitute_images(), 'This is some text')
    
    def test_create3(self, ):
        #  Create with a dummy image.
        note = self.logbook.create_note("![image text](/original/filename.img) some more text", \
            run=self.run1, images=["pynotetests.py",],  offsets=[0,])
        self.assertEqual(note.image_count(), 1)
        image = note.get_image(0)
        self.assertEqual(image.index, 0)
        self.assertEqual(image.offset, 0)
        self.assertEqual(image.original_file, 'pynotetests.py')
        final_contents = note.substitute_images()
        sbcontents = '![image text](' + image.exported_file + ') some more text'
        self.assertEqual(final_contents, sbcontents)
    

if __name__ == '__main__' :
    unittest.main()
    