/* Null trigger */

#include <config.h>
#include <config_pixie16api.h>
#include <iostream>
#include "CMyTrigger.h"
#include <stdlib.h>


using namespace std;

#define TRIGGER_TIMEOUT_SECS 5        // If no triggers in 5 seconds, auto-trigger.

CMyTrigger::CMyTrigger(): m_retrigger(false) , m_fifoThreshold(EXTFIFO_READ_THRESH*10)
{
  //  If FIFO_THRESHOLD is defined and is a positive integer, it replaces
  //  the default value of m_fifoThreshold - the number of words that must be
  //  in the fifo for the trigger to fire.
  //
  const char* pThreshold = getenv("FIFO_THRESHOLD");
  if (pThreshold) {
    char* pEnd;
    unsigned long newThreshold = strtoul(pThreshold, &pEnd, 0);
    if (newThreshold && (pEnd != pThreshold)) {
      m_fifoThreshold = newThreshold;
    }
  }
}

CMyTrigger::~CMyTrigger()
{
}

void CMyTrigger::setup()
{
    // Start the trigger timeout now:
    
    m_lastTriggerTime = time(nullptr);
}

void CMyTrigger::teardown()
{
  // called as data taking ends.  
  // DDAS does not need any further signal as data taking ends
  // since this function is also called on a pause of data taking
  // don't even think about desyncing modules here
}


// control for determing if trigger should poll modules or pass control back
// to CEventSegment for processing the previous block of data
void CMyTrigger::Reset()
{
  m_retrigger = false;
}

/* Receive the number of modules in the pixie16 setup from the event 
   segment class */
void CMyTrigger::Initialize(int nummod)
{
  NumberOfModules = nummod;
  m_retrigger = false;
}


bool CMyTrigger::operator()() 
{
  try{

    /* If pixie16 is in the middle of processing a data buffer in the event 
       segment, continue processing the data buffer.  Return a true trigger 
       to pass control back the event segment. */
    if(m_retrigger){
      m_lastTriggerTime = time(nullptr);   // Reset the trigger timeout.
      return true;
    } else {
      /* If there are no buffers currently being processed in the event segment 
         look at the pixie16 hardware to see if data currently needs to be 
         read out. */

      int retval = -1;

      for (unsigned short i=0; i<NumberOfModules; i++) {
        // Check how many words are stored in Pixie16's readout FIFO
        retval = Pixie16CheckExternalFIFOStatus(&nFIFOWords,i);

        /* Trigger a read if the number of words in the external FIFO of 
           any pixie16 module in a crate exceeds a threshold defined in 
           pixie16app_defs.h */

        if(nFIFOWords > m_fifoThreshold ) {
#ifdef PRINTQUEINFO
          std::cout << "CMyTrigger:: trig satisfied...mod=" << i << " nwords=" << nFIFOWords << std::endl;
#endif
          m_retrigger = true;
          m_lastTriggerTime = time(nullptr);
          return true;
        }

      } // end module loop
      if(m_retrigger) {
         m_lastTriggerTime = time(nullptr);
        return true;
      }
    }

  }
  catch(...) {
    cout << "exception in trigger " << endl;
  }

  time_t now = time(nullptr);
  if ((now - m_lastTriggerTime)  > TRIGGER_TIMEOUT_SECS) {
    m_lastTriggerTime = now;
    return true;
  }  
  return false;
}


