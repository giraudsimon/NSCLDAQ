/*!
   Main program for daqstart.
   - Stock the sink factory with the known creators.
   - Create the application
   - Run it
   - catch all exceptions and exit states.
*/


// Headers:

#include <config.h>

#include <CSinkFactory.h>
#include <CFileSinkCreator.h>
#include <CDAQStart.h>

#include <DesignByContract.h>
#include <Exception.h>
#include <string>

#include <iostream>

using namespace std;




int
main(int argc, char** argv)
{
  // Stock the sink factory with all the legal sink types:

  CSinkFactory* pFact = CSinkFactory::getInstance();
  
  pFact->AddCreator(string("file"), new CFileSinkCreator); // File sinks.

  // Create and run the application within the giant try block:

  try {
    CDAQStart  Application;
    Application(argc, argv);	// Run the application
  }
  catch(CException& except) {	// NSCL exceptions.
    cerr << argv[0] << "NSCL Exception caught: "
         << except.ReasonText() << " while " << except.WasDoing() 
         << endl;
  }
  catch(DesignByContract::DesignByContractException& except) { // Contract violations.
    cerr << argv[0] << ": contract exception caught: "
         << except << endl;
  }
  catch (string& msg) {		// String throws.
    cerr << argv[0] << ": string exception caught: "
         << msg << endl;
  }
  catch(...) {			// Last chance exceptions...
  }
}
void* gpTCLApplication(0);