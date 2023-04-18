#include "Main.h"
#include <iostream>
#include <stdexcept>
#include <TSysEvtHandler.h>

#include <TGMsgBox.h>

class MyExceptionHandler : public TStdExceptionHandler
{
public:
    EStatus Handle(std::exception& exc) {
        std::cout << "Caught an exception : " << exc.what() << std::endl;
        return kSEAbort;
    }
};

using namespace std;



int
main (int argc, char **argv)
{
    
       
    TApplication theApp ("App", &argc, argv);
    Main mainWindow (gClient->GetRoot ());
    
    
    gSystem->AddStdExceptionHandler(new MyExceptionHandler); // catch exceptions
    theApp.Run ();
    return 0;
}
