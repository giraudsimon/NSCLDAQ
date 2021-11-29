/* Null trigger */

#include <config.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include "CMyEndCommand.h"
#include "CMyEventSegment.h"
#include "TCLInterpreter.h"
#include <TCLObject.h>
#include <RunState.h>
#include <stdlib.h>
#include <CVMEInterface.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

CMyEndCommand::CMyEndCommand(CTCLInterpreter& rInterp, CMyEventSegment *myevseg) : CEndCommand(rInterp) 
{
    myeventsegment = myevseg;
    NumModules = myeventsegment->GetNumberOfModules();
}

CMyEndCommand::~CMyEndCommand()
{
}

int CMyEndCommand::ExecutePreFunction() 
{
  CVMEInterface::Lock();    

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

    /* Stop run in the Director module (module #0) - a SYNC interrupt
       should be generated to stop run in all modules simultaneously
       when running synchronously...

       We are not running synchronously when in INFINITY_CLOCK mode!
       If INFINITY_CLOCK mode is not set, then we need to end the run
       in every module!
    */
    if (::getenv("INFINITY_CLOCK") == nullptr) {
        retval = Pixie16EndRun(0);
        if (retval<0) {
            std::cout << "Failed to communicate end run operation to the director module." << std::endl;
            return TCL_ERROR;
        }
    } else {

        for (int k=0; k<NumModules; ++k) {
            retval = Pixie16EndRun(k);
            if (retval<0) {
                std::cout << "Failed to communicate end run operation to module #"
                          << k << std::endl;
                return TCL_ERROR;
            }
        }
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
#if XIAAPI_VERSION < 3

                EndOfRunRead = 1;
                sprintf(filnam, "lmdata_mod%d.bin", k);
                retval = Pixie16SaveExternalFIFODataToFile(filnam, &mod_numwordsread, 
                        k, EndOfRunRead);
                if(retval<0) {
                    cout << "*ERROR* Pixie16SaveExternalFIFODataToFile failed in module %d, retval = " << k << endl << flush;
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
#if XIAAPI_VERSION < 3
        EndOfRunRead = 1;
        sprintf(filnam, "lmdata_mod%d.bin", k);
        retval = Pixie16SaveExternalFIFODataToFile(filnam, &mod_numwordsread, k, EndOfRunRead);
        if(retval<0) {
            cout << "*ERROR* Pixie16SaveExternalFIFODataToFile failed in module %d, retval = " << k << endl << flush;

        }
        nFIFOWords[k] += mod_numwordsread;
#endif
        /* Get final statistics information */
        int retval;
        unsigned int statistics[448] = {0};

        retval = Pixie16ReadStatisticsFromModule(statistics, k);
        
        if (retval < 0) {
            cout << "Error accessing scaler statistics from module " 
                << k << endl;
        } else {
            // cout << "Accessing statistics for module " 
            //      << k << endl;
        }
        double RealTime = Pixie16ComputeRealTime(statistics, k);
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
            ChanEvents = Pixie16ComputeInputCountRate(statistics, k, i) * RealTime;;
            FastPeaks  = Pixie16ComputeOutputCountRate(statistics, k, i) * RealTime;
            
            /* Print out final statistics to file */
            // "Channel #" "output events" "input events"
            outputfile << "   Channel " << i << ": " << ChanEvents << " " 
                << FastPeaks << endl;

        }

    }

    outputfile.close();  
    free(lmdata);


    /***** Save DSP parameters to file *****/
    //  retval = Pixie16SaveDSPParametersToFile ("DSPpar.set");
    //  if (retval < 0)
    //    printf ("saving DSP parameters to file failed, retval=%d\n", retval);
    //  else
    //    cout<<"Saving DSP parameters to file OK "<<retval<<endl<<flush;  
    //  for(int i=0;i<NumModules;i++){   
    //    double ChanParData;
    //    unsigned short mod,chan;
    //    mod=i;
    //    chan = 0;
    //    //retval = Pixie16ReadSglChanPar("CHANNEL_CSRA",&ChanParData,mod,chan);
    //    cout << " channel csras " << i << " " << ChanParData << " " << retval << endl << flush;
    //   }  


    // Read histograms from each module and then save them to files
    //  for(int i=0;i<NumModules;i++){   
    //    sprintf(filnam, "histogram_mod%d.bin", i);
    //    retval = Pixie16SaveHistogramToFile(filnam, i);
    //    cout << " saving histogram mod " << i << " " << retval << endl << flush;
    //  }

    //  cout<<"Pixie16 ended \n"<<flush;


    // retval = Pixie16ExitSystem(20);
    if(retval != 0) {
        cout << "Exit pixie16 " << retval << endl << flush;
    }


  CVMEInterface::Unlock();    
    return 0;	
}

int CMyEndCommand::ExecutePostFunction() 
{

    return 0;	
}


// Override the end command's function call operator to be sure our stuff
// gets called at the right time.

    int
CMyEndCommand::operator()(CTCLInterpreter& interp,
        std::vector<CTCLObject>& objv)
{
    RunState* pState = RunState::getInstance();

    // To end a run we must have no mor command parameters
    // and the state must be either active or paused:

    bool okToEnd = 
        ((pState->m_state == RunState::active)   ||
         (pState->m_state == RunState::paused))   &&
        (objv.size() ==1);

    if(okToEnd) ExecutePreFunction();
    CEndCommand::operator()(interp,objv);
    if(okToEnd) ExecutePostFunction();

  return TCL_OK;

}

