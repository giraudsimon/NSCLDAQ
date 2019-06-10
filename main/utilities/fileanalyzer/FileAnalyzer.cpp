/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <CFatalException.h>
#include <CFilterMain.h>

#include "CSourceCounterFilter.h"

#include <limits>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
using namespace std;

struct CmdlineArgs {
  string s_outputFile;
  bool   s_built;
};

void printSpecialUsage() {
  cout << "  -O" << endl;
  cout << "  --output-file   The name of the file to write statistics to" << endl;

  cout << "\n  -u" << endl;
  cout << "  --unbuilt       If present, data is not treated as built data." << endl;
}


vector<string> 
cArgsToCppArgs(int argc, char* argv[]) 
{
  vector<string> args(argc);
  for (int i=0; i<argc; ++i) {
    args[i] = string(argv[i]);
  }
  return args;
}


pair<vector<string>, CmdlineArgs>
processAndRemoveSpecialArgs(const vector<string>& argv)
{
  CmdlineArgs cmdArgs = {"", true};

  vector<string> filteredArgv;
  for (size_t i=0; i<argv.size(); ++i) {
    string option(argv[i]);
    if (option.find("--output-file") == 0) {
      if (option.size() == 13) {
        cmdArgs.s_outputFile = argv[i+1]; 
        ++i;
      } else {
        cmdArgs.s_outputFile = option.substr(14); 
      }
    } else if (option.find("-O")== 0 ) {
      if (option.size() == 2) {
        cmdArgs.s_outputFile = argv[i+1]; 
        ++i;
      } else {
        cmdArgs.s_outputFile = option.substr(3); 
      }
    } else if (option == "--unbuilt" || option == "-u") {
      cmdArgs.s_built = false;
    } else if (option == "--help" || option == "-h") {
      atexit( printSpecialUsage );
    } else {
      filteredArgv.push_back(string(argv.at(i)));
    } 
  }
  return make_pair(filteredArgv, cmdArgs);
}

char** createNewCArgV(const vector<string>& argV)
{
  char** pArgV = new char*[argV.size()];
  for (int i=0; i<argV.size(); ++i) {
    pArgV[i] = new char[argV[i].size()+1];
    strcpy(pArgV[i], argV[i].data());
    pArgV[i][argV[i].size()] = 0;
  }

  return pArgV;
}

/// The main function
/**! main function
  Creates a CFilterMain object and 
  executes its operator()() method. 

  \return 0 for normal exit, 
          1 for known fatal error, 
          2 for unknown fatal error
*/
int main(int argc, char* argv[])
{
  int status = 0;

  try {

    auto argV         = cArgsToCppArgs(argc, argv);
    auto parserResult    = processAndRemoveSpecialArgs(argV);
    vector<string> newArgV  = parserResult.first;

    if (newArgV == argV) {
      cout << "User did not provide an output file. Specify --output-file or -O option" << endl;
      return 1;
    }

    argc = newArgV.size();
    argv = createNewCArgV(newArgV);

    // Create the main
    CFilterMain theApp(argc,argv);

    auto cmdLineOpts = parserResult.second;
    // Construct filter(s) here.
    CSourceCounterFilter srcCounter(numeric_limits<uint32_t>::max(), cmdLineOpts.s_outputFile);
    srcCounter.setBuiltData(cmdLineOpts.s_built);

    theApp.registerFilter(&srcCounter);

    // Run the main loop
    theApp();

  } catch (CFatalException exc) {
    status = 1;
  } catch (...) {
    cout << "Caught unknown fatal error...!" << endl;
    status = 2;
  }

  return status;
}

