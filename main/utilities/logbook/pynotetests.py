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
        note = self.logbook.create_note(self.ron, "This is some text")
        self.assertEqual(1, note.id)
        self.assertEqual('This is some text', note.contents)
        self.assertIsNone(note.run)
        self.assertEqual(0, note.image_count())
        self.assertEqual(note.author.id, self.ron.id)
        
    def test_create2(self, ):
        note = self.logbook.create_note(self.giordano, "This is some text", run=self.run1)
        run  = note.run
        self.assertEqual(run.number, 1)
        self.assertEqual(run.title, 'Test run1')
        self.assertEqual(note.contents, 'This is some text')
        self.assertEqual( note.image_count(), 0)
        self.assertEqual(note.substitute_images(), 'This is some text')
        self.assertEqual(note.author.id, self.giordano.id)
    
    def test_create3(self, ):
        #  Create with a dummy image.
        note = self.logbook.create_note(self.ron, "![image text](/original/filename.img) some more text", \
            run=self.run1, images=["unittests",],  offsets=[0,])
        self.assertEqual(note.image_count(), 1)
        image = note.get_image(0)
        self.assertEqual(image.index, 0)
        self.assertEqual(image.offset, 0)
        self.assertEqual(image.original_file, 'unittests')
        final_contents = note.substitute_images()
        sbcontents = '![image text](' + image.exported_file + ') some more text'
        self.assertEqual(final_contents, sbcontents)
    
    def test_get_note1(self, ):
        note = self.logbook.create_note(self.giordano, "This is some text", run=self.run1)
        id = note.id + 1            # no such note.
        self.assertRaises(LogBook.error, self.logbook.get_note, id)
    
    def test_get_note2(self, ):
        note = self.logbook.create_note(self.ron, "This is some text", run=self.run1)
        id = note.id
        same_note = self.logbook.get_note(id)
        self.assertEqual(same_note.contents, note.contents)
    
    def test_list_notes1(self, ):
        notes = self.logbook.list_all_notes()
        self.assertEqual(len(notes), 0)
    
    def test_list_notes2(self, ):
        note = self.logbook.create_note(self.giordano, "This is some text", run=self.run1)
        notes = self.logbook.list_all_notes()
        self.assertEqual(len(notes), 1)
        note1 = notes[0]
        self.assertEqual(note.id, note1.id)
    
    def test_list_notes3(self, ):
        note1 = self.logbook.create_note(self.ron, "This is some text", run=self.run1)
        note2 = self.logbook.create_note(self.giordano, "Second note test", run=self.run2)
        notes = self.logbook.list_all_notes()
        self.assertEqual(len(notes), 2)
        self.assertEqual(notes[0].id, note1.id)
        self.assertEqual(notes[1].id, note2.id)
    
    def test_list_notes_by_run_number1(self, ):
        note1 = self.logbook.create_note(self.ron, "This is some text")
        note2 = self.logbook.create_note(self.giordano, "this is another note")
        notes = self.logbook.list_notes_for_run_number(1)
        self.assertEqual(len(notes), 0)
        
    def test_list_notes_by_run_number2(self, ):
        note1 = self.logbook.create_note(self.ron, "This is some text", run=self.run1)
        note2 = self.logbook.create_note(self.giordano, "Second note test", run=self.run2)
        notes = self.logbook.list_notes_for_run_number(2)
        self.assertEqual(len(notes), 1)
        self.assertEqual(notes[0].id, note2.id)
        notes = self.logbook.list_notes_for_run_number(1)
        self.assertEqual(len(notes), 1)
        self.assertEqual(notes[0].id, note1.id)
        notes = self.loogbook.list_notes_for_run_number(3)
        self.assertEqual(len(notes), 0)
    
    def test_list_notes_by_run_number2(self) :
        note1 = self.logbook.create_note(self.ron, "This is some text", run=self.run2)
        note2 = self.logbook.create_note(self.giordano, "Second note test", run=self.run2)
        notes = self.logbook.list_notes_for_run_number(2)
        self.assertEqual(len(notes), 2)
        self.assertEqual(notes[0].id, note1.id)
        self.assertEqual(notes[1].id, note2.id)

    def test_list_notes_by_run_id1(self, ):
        note1 = self.logbook.create_note(self.ron, "This is some text")
        note2 = self.logbook.create_note(self.giordano, "this is another note")
        notes = self.logbook.list_notes_for_run_id(1)
        self.assertEqual(len(notes), 0)
        
    def test_list_notes_by_run_id2(self, ):
        note1 = self.logbook.create_note(self.ron, "This is some text", run=self.run1)
        note2 = self.logbook.create_note(self.giordano, "Second note test", run=self.run2)
        notes = self.logbook.list_notes_for_run_id(self.run1.id)
        self.assertEqual(len(notes), 1)
        self.assertEqual(notes[0].id, note1.id)
        notes = self.logbook.list_notes_for_run_id(self.run2.id)
        self.assertEqual(len(notes), 1)
        self.assertEqual(notes[0].id, note2.id)
    
    def test_list_notes_by_run_id3(self, ):
        note1 = self.logbook.create_note(self.ron, "This is some text", run=self.run1)
        note2 = self.logbook.create_note(self.giordano, "Second note test", run=self.run1)
        notes = self.logbook.list_notes_for_run_id(self.run1.id)
        self.assertEqual(len(notes), 2)
        self.assertEqual(notes[0].id, note1.id)
        self.assertEqual(notes[1].id, note2.id)
    
    def test_list_notes_for_run1(self, ):
        note1 = self.logbook.create_note(self.ron, "This is some text")
        note2 = self.logbook.create_note(self.giordano, "Second note test", run=self.run2)
        notes = self.logbook.list_notes_for_run(self.run1)
        self.assertEqual(len(notes), 0)
        
    def test_list_notes_for_run2(self, ):
        note1 = self.logbook.create_note(self.ron, "This is some text", run=self.run1)
        note2 = self.logbook.create_note(self.giordano, "Second note test", run=self.run2)
        notes = self.logbook.list_notes_for_run(self.run1)
        self.assertEqual(len(notes), 1)
        self.assertEqual(notes[0].id, note1.id)
        notes = self.logbook.list_notes_for_run(self.run2)
        self.assertEqual(len(notes), 1)
        self.assertEqual(notes[0].id, note2.id)
    
    def test_list_notes_for_run3(self) :
        note1 = self.logbook.create_note(self.ron, "This is some text", run=self.run1)
        note2 = self.logbook.create_note(self.giordano, "Second note test", run=self.run1)
        notes = self.logbook.list_notes_for_run(self.run1)
        self.assertEqual(len(notes), 2)
        self.assertEqual(notes[0].id, note1.id)
        self.assertEqual(notes[1].id, note2.id)
        self.assertEqual(len(self.logbook.list_notes_for_run(self.run2)), 0)
    
    def test_list_non_run_notes1(self, ):
        note1 = self.logbook.create_note(self.ron, "This is some text", run=self.run1)
        note2 = self.logbook.create_note(self.giordano, "Second note test", run=self.run1)
        notes = self.logbook.list_nonrun_notes()
        self.assertEqual(len(notes), 0)
    
    def test_list_non_run_notes2(self, ):
        note1 = self.logbook.create_note(self.ron, "This is some text")
        note2 = self.logbook.create_note(self.giordano, "Second note test", run=self.run1)
        notes = self.logbook.list_nonrun_notes()
        self.assertEqual(len(notes), 1)
        self.assertEqual(notes[0].id, note1.id)
    
    def test_list_non_run_notes3(self, ):
        note1 = self.logbook.create_note(self.ron, "This is some text")
        note2 = self.logbook.create_note(self.giordano, "Second note test")
        notes = self.logbook.list_nonrun_notes()
        self.assertEqual(len(notes), 2)
        self.assertEqual(notes[0].id, note1.id)
        self.assertEqual(notes[1].id, note2.id)
    
    def test_get_note_run1(self, ):
        note1 = self.logbook.create_note(self.ron, "This is some text")
        run =  self.logbook.get_note_run(note1)
        self.assertIsNone(run)
        
    def test_get_note_run2(self, ):
        note1 = self.logbook.create_note(self.giordano, "This is some text", run=self.run1)
        run   = self.logbook.get_note_run(note1)
    
     
    
    
if __name__ == '__main__' :
    unittest.main()
    
