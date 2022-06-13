/* Null trigger */

#include <config.h>
#include <config_pixie16api.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include "CMyEndCommand.h"
#include "CMyEventSegment.h"
#include "TCLInterpreter.h"
#include <TCLObject.h>
#include <RunState.h>
#include <CVMEInterface.h>

#include <config.h>

#include <stdlib.h>

#include <functional>
#include <tuple>
#include <chrono>
#include <thread>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

CMyEndCommand::CMyEndCommand(CTCLInterpreter& rInterp,
			     CMyEventSegment *myevseg,
			     CExperiment* pexp) : CEndCommand(rInterp) 
{
    myeventsegment = myevseg;
    m_pExp = pexp;
    NumModules = myeventsegment->GetNumberOfModules();
}

CMyEndCommand::~CMyEndCommand()
{
}

int CMyEndCommand::transitionToInactive() 
{

  cout << "Transitioning Pixies to Inactive" << endl;

    /* Stop run in the Director module (module #0) - a SYNC interrupt 
       should be generated to stop run in all modules simultaneously
       when running synchronously...

       We are not running synchronously when in INFINITY_CLOCK mode!
       If INFINITY_CLOCK mode is not set, then we need to end the run
       in every module!
    */
  if (::getenv("INFINITY_CLOCK") == nullptr) {
      int retval = Pixie16EndRun(0);
      if (retval<0) {
          std::cout << "Failed to communicate end run operation to the director module." << std::endl;
          return TCL_ERROR;
      }
  } else {

      for (int k=0; k<NumModules; ++k) {
          int retval = Pixie16EndRun(k);
          if (retval<0) {
              std::cout << "Failed to communicate end run operation to module #"
                        << k << std::endl;
              return TCL_ERROR;
          }
      }
  }

  for (int k=0; k<NumModules; ++k) {
      bool runEnded = false;
      int nRetries = 0;
      const int nMaxRetries = 10;
      while (! runEnded && nRetries < nMaxRetries) {
          int retval = Pixie16CheckRunStatus(k);
          runEnded = (retval == 0);
          nRetries ++;
          this_thread::sleep_for(chrono::milliseconds(100));
      }
      if (nRetries == nMaxRetries) {
          std::cout << "Failed to end run in module " << k << std::endl;
      }
  }

  return 0;
}

int CMyEndCommand::readOutRemainingData() 
{

    // we will poll trying to lock the mutex so that we have a better chance
    // of acquiring it.
    const int nAllowedAttempts = 10;
    int nAttemptsMade = 0;
    while (!CVMEInterface::TryLock(1) && (nAttemptsMade < nAllowedAttempts)) { 
      nAttemptsMade++; 
    }

    if (nAttemptsMade == nAllowedAttempts) {
      // failed to lock the interface, add an end event back onto the 
      // tail of the event stack. We will try again. This is to prevent
      // deadlocks between the CVariableBuffers thread sync and the end run
      // synchronization.
      rescheduleEndRead();
      return TCL_ERROR;
    }

    //int NumModules = 1;
    int retval = 0;
    int count = 0;
    char filnam[80];
    unsigned long nFIFOWords[24];
    unsigned long *lmdata;
    unsigned int mod_numwordsread;
    unsigned short EndOfRunRead;

    if((lmdata = (unsigned long *)malloc(sizeof(unsigned long)*131072)) == NULL){
        cout << "Failed to allocate memory block lmdata" << endl << flush;
    }

    /* Clear counters to 0 (counters keep track of how many words each 
       module has read) */
    for (int k=0; k<NumModules; k++) {
        nFIFOWords[k] = 0;
    } 


    usleep(100);
    // Make sure all modules indeed finish their run successfully.
    for(int k=0; k<NumModules; k++) {

        // For each module, check to see if a run is still in progress in the
        // kth module. If it is still running, wait a little bit and check again.
        // If after 10 attempts, we stop trying. Run ending failed.
        count = 0;
        do{
            retval = Pixie16CheckRunStatus(k);
            
            // If retval==1 then run is still in progress...
            // If retval==-1 then k is not a valid module number
            if (retval != 0) { 
                EndOfRunRead = 1;
		/* TODO:  figure this out for API 3.0 and larger */
#if XIAAPI_VERSION < 3
                sprintf(filnam, "lmdata_mod%d.bin", k);
                retval = Pixie16SaveExternalFIFODataToFile(filnam, &mod_numwordsread, 
                        k, EndOfRunRead);
                if(retval<0) {
		  cout << "*ERROR* Pixie16SaveExternalFIFODataToFile failed in module " << k
		       << "  retval = " << retval << endl << flush;
                    // Pixie_Print_MSG (ErrMSG);
                    //return -5;
                } // end error block

                nFIFOWords[k] += mod_numwordsread;
#endif		
            } else {
                // No run is in progress
                break;
            } // end retval conditional block 

            count ++;
            usleep(100);
        } while(count < 10);

        if(count == 10){
            cout << "End run in module " << k << " failed" << endl << flush;
        }
    }

    /* All modules have their run stopped successfully. Now read out the 
       possible last words from the external FIFO, and get statistics */
    ofstream outputfile;
    outputfile.open ("EndofRunScalers.txt", ios::app);

    for(int k=0; k<NumModules; k++) {
        EndOfRunRead = 1;
	/* TODO: Figure this out for 3.0 and later. */
#if XIAAPI_VERSION < 3	
        sprintf(filnam, "lmdata_mod%d.bin", k);
        retval = Pixie16SaveExternalFIFODataToFile(filnam, &mod_numwordsread, k, EndOfRunRead);
        if(retval<0) {

	  cout << "*ERROR* Pixie16SaveExternalFIFODataToFile failed in module "
	       << k << " retval = " << retval << endl << flush;

        }
        nFIFOWords[k] += mod_numwordsread;
#endif
        /* Get final statistics information */
        int retval;
#if XIAAPI_VERSION >= 3
	std::vector<unsigned int> statistics(Pixie16GetStatisticsSize(),0);
#else
	std::vector<unsigned int> statistics(448,0); // see any v11.3
#endif	

        retval = Pixie16ReadStatisticsFromModule(statistics.data(), k);
        if (retval < 0) {
            cout << "Error accessing scaler statistics from module " 
                << k << endl;
        } else {
            // cout << "Accessing statistics for module " 
            //      << k << endl;
        }

        double ChanEvents = 0; 
        double FastPeaks = 0;

        outputfile << "Module " << k << endl; 

        for (int i=0; i<16; i++) {

            ChanEvents = 0;
            FastPeaks = 0;

            // These are lines are taken straight from the pixie16app.c file.

            // Pixie16ComputeOutputCountRate code
            // first get upper 32 bits of the number and push them up 32 bits, then
            // append on the lower 32 bits. Not the greatest but should work.
	    ChanEvents = Pixie16ComputeOutputCountRate(statistics.data(), k, i);

            // Pixie16ComputeInputCountRate code

	    FastPeaks = Pixie16ComputeOutputCountRate(statistics.data(), k, i);


            /* Print out final statistics to file */
            // "Channel #" "output events" "input events"
            outputfile << "   Channel " << i << ": " << ChanEvents << " " 
                << FastPeaks << endl;

        }

    }

    outputfile.close();  
    free(lmdata);

    CVMEInterface::Unlock();
    return 0;	
}


// Override the end command's function call operator to be sure our stuff
// gets called at the right time.

int
CMyEndCommand::operator()(CTCLInterpreter& interp,
        std::vector<CTCLObject>& objv)
{
  if (objv.size() != 1) {
    return TCL_ERROR;
  }
  int status = endRun();
  if (status == TCL_OK) {
    readOutRemainingData();
  }
  return TCL_OK;
}

int CMyEndCommand::endRun() 
{
    RunState* pState = RunState::getInstance();

    // To end a run we must have no mor command parameters
    // and the state must be either active or paused:

    bool okToEnd = 
        ((pState->m_state == RunState::active)   ||
         (pState->m_state == RunState::paused));

    // in case any of these end run stages fail, they will be rescheduled and picked
    // up again
    if(okToEnd) { 
      // we will poll trying to lock the mutex so that we have a better chance
      // of acquiring it.
      const int nAllowedAttempts = 10;
      int nAttemptsMade = 0;
      while (!CVMEInterface::TryLock(1) && (nAttemptsMade < nAllowedAttempts)) { 
        nAttemptsMade++; 
      }

      if (nAttemptsMade == nAllowedAttempts) {
        // failed to lock the interface, add an event to the 
        // tail of the event stack to try again later. This is to prevent
        // deadlocks between the CVariableBuffers thread sync and the end run
        // synchronization.
        rescheduleEndTransition();
      } else {
        // successfull acquired the lock
        int deviceEndStatus = transitionToInactive();
        CVMEInterface::Unlock();

        int triggerEndStatus;
        string result;
        tie(triggerEndStatus, result) = CEndCommand::end();
        if (deviceEndStatus != TCL_OK || triggerEndStatus != TCL_OK) return TCL_ERROR;
      }
    }

    return TCL_OK;
}

int CMyEndCommand::handleEndRun(Tcl_Event* pEvt, int flags)
{
  EndEvent* pEnd = reinterpret_cast<EndEvent*>(pEvt);
  pEnd->s_thisPtr->endRun();
  return 0;
}

int CMyEndCommand::handleReadOutRemainingData(Tcl_Event* pEvt, int flags)
{
  EndEvent* pEnd = reinterpret_cast<EndEvent*>(pEvt);

  pEnd->s_thisPtr->readOutRemainingData();
  return 0;
}

void CMyEndCommand::rescheduleEndTransition() {
  EndEvent* pEnd = reinterpret_cast<EndEvent*>(Tcl_Alloc(sizeof(EndEvent)));
  pEnd->s_rawEvent.proc = CMyEndCommand::handleEndRun;
  pEnd->s_thisPtr = this;
  Tcl_QueueEvent(reinterpret_cast<Tcl_Event*>(pEnd), TCL_QUEUE_TAIL);
}

void CMyEndCommand::rescheduleEndRead() {
  EndEvent* pEnd = reinterpret_cast<EndEvent*>(Tcl_Alloc(sizeof(EndEvent)));
  pEnd->s_rawEvent.proc = CMyEndCommand::handleReadOutRemainingData;
  pEnd->s_thisPtr = this;
  Tcl_QueueEvent(reinterpret_cast<Tcl_Event*>(pEnd), TCL_QUEUE_TAIL);
}
