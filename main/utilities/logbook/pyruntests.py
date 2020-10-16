##
# @file
# @brief
#
#
import unittest
import os
from LogBook import LogBook

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
        
        self.shift1 = self.logbook.create_shift("Everybody", self.logbook.list_shifts())
        self.shift1.set_current()

    
    def tearDown(self) :
        del self.shift1
        
        del self.ron
        del self.giordano
        del self.sean
        del self.alex
        del self.ava
        
        del self.logbook
        os.unlink('logbook.log')
        
    def test_current1(self) :
        # initially there is no run:
        
        self.assertIsNone(self.logbook.current_run())
      
    def test_begin1(self, ):
        # Can begin a run with no exceptions and then there's a current run:
        
        run = self.logbook.begin_run(2, title='test title', comment='This is a comment')
        current = self.logbook.current_run()
        self.assertEqual(run.id, current.id)
   
    def test_begin2(self, ):
        # Can ommit the comment:
        
        run = self.logbook.begin_run(2, title='test title')
        
    def test_begin_begin3(self, ):
        # If I start a run with one already started I get a logbook exception
        
        run = self.logbook.begin_run(2, "Test title")
        self.assertRaises(LogBook.error, self.logbook.begin_run, 3, 'Another test title')
            
    def test_end1(self, ):
        # Can end a begin run and then there's no current run
        run = self.logbook.begin_run(2, 'test title')
        run.end("I'm ending the run now")
        self.assertIsNone(self.logbook.current_run())
    def test_end2(self, ):
        # Not allowed to double end:
        run = self.logbook.begin_run(2, 'test title')
        run.end("I'm ending the run now")
        self.assertRaises(LogBook.error, run.end, 'Double end is verboten')
    
    def test_pause1(self, ):
        run = self.logbook.begin_run(2, 'test title')
        run.pause()
        self.assertEqual(run.id, self.logbook.current_run().id)
    def test_pause2(self, ):
        # Paused runs can't pause.
        run = self.logbook.begin_run(2, 'test title')
        run.pause('Pausing the run now')
        self.assertRaises(LogBook.error, run.pause)
    
    def test_pause4(self, ):
        run = self.logbook.begin_run(2, 'test title')
        run.pause()
        # allowed to end the run:
        run.end()
    
    def test_pause5(self, ):
        # cant pause an ended run:
        run = self.logbook.begin_run(2, 'test title')
        run.end()
        self.assertRaises(LogBook.error, run.pause)
    
    
    def test_resume1(self, ):
        run = self.logbook.begin_run(2, 'test title')
        # Active runs can't be resumed:
        self.assertRaises(LogBook.error, run.resume, 'some comment')
    
    def test_resume2(self, ):
        # can resume a paused run:
        
        run = self.logbook.begin_run(12, 'test title')
        run.pause('Some comment for pause')
        run.resume(' Ive just resumed')
        self.assertEqual(run.id, self.logbook.current_run().id) #Still current.
    
    def test_resume3(self, ):
        #Can't resume an ended run:
        
        run = self.logbook.begin_run(2, 'test title')
        run.end('run just ended')
        self.assertRaises(LogBook.error, run.resume, 'some comment')



    def test_emend1(self, ):
        #Emergency end no longer current:
        
        run = self.logbook.begin_run(2, 'test title')
        run.emergency_end('something bad happened')
        self.assertIsNone(self.logbook.current_run())
      
    def test_emend2(self, ):
        # Can do this to paused runs:
        run = self.logbook.begin_run(2, 'test title')
        run.pause('the run is now paused')
        run.emergency_end('something bad!')
        self.assertIsNone(self.logbook.current_run())
    def test_emend3(self, ):
        run = self.logbook.begin_run(2, 'test title')
        run.pause()
        run.resume('run has resumed again')
        run.emergency_end('bad shit')
        self.assertIsNone(self.logbook.current_run())
  
    def test_isactive1(self, ):
        # Running runs are active
        
        run = self.logbook.begin_run(2, 'test title')
        self.assertTrue(run.is_active())
    def test_isactive2(self, ):
        #Paused runs are active.
        
        run = self.logbook.begin_run(2, 'test title')
        run.pause()
        self.assertTrue(run.is_active())
            
    def test_asactive3(self, ):
        run = self.logbook.begin_run(2, 'test title')
        run.pause()
        run.resume()
        #Resumed runs are active:
        
        self.assertTrue(run.is_active())
    def test_isactive4(self, ):
        #ended runs are not active:
        run = self.logbook.begin_run(2, 'test title')
        run.end()
        self.assertFalse(run.is_active())
    def test_isactive4(self, ):
        #Emergency ended runs are not active.
        run = self.logbook.begin_run(2, 'test title')
        run.emergency_end()
        self.assertFalse(run.is_active())
    def test_lasttrans1(self, ):
        run = self.logbook.begin_run(2, 'test title')
        #last transtion is BEGIN:
        
        self.assertEqual('BEGIN', run.last_transition())
        
    def test_lasttrans2(self, ):
        run = self.logbook.begin_run(2, 'test title')
        run.pause()
        self.assertEqual('PAUSE', run.last_transition())
    
    def test_lasttrans3(self, ):
        run = self.logbook.begin_run(2, 'test title')
        run.pause()
        run.resume()
        self.assertEqual('RESUME', run.last_transition())
    
    def test_lasttrans4(self, ):
        run = self.logbook.begin_run(2, 'test title')
        run.end()
        self.assertEqual('END', run.last_transition())
    def test_lasttrans5(self, ):
        run = self.logbook.begin_run(2, 'test title')
        run.emergency_end()
        self.assertEqual('EMERGENCY_END', run.last_transition())
    
    # The lasttransif tests require knowledge of the database guts
    
    def test_lasttransid1(self, ):
        run = self.logbook.begin_run(2, 'test title')
        self.assertEqual(1, run.last_transitionid())
    
    def test_lasttransid2(self, ):
        run = self.logbook.begin_run(2, 'test title')
        run.pause()
        self.assertEqual(3, run.last_transitionid())
    
    def test_lasttransid3(self, ):
        run = self.logbook.begin_run(2, 'test title')
        run.pause()
        run.resume()
        self.assertEqual(4, run.last_transitionid())
    
    def test_lasttransid4(self, ):
        run = self.logbook.begin_run(2, 'test title')
        run.end()
        self.assertEqual(2, run.last_transitionid())
    def test_lasttransid5(self, ):
        run = self.logbook.begin_run(2, 'test title')
        run.emergency_end()
        self.assertEqual(5, run.last_transitionid())
    
    
    
    
    
 
    
    

if __name__ == '__main__' :
    unittest.main()
    