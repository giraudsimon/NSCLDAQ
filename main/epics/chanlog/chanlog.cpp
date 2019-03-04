/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


#include <config.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "getinfo.h"


#include <stdlib.h>
#include <string.h>

using namespace std;


static  char* pProgram;



/*!
  Print the program usage to stderr.

  \param pName (const char* [in]):
     Name of the program thats' running.

*/
void Usage(const char* pName)
{
  cerr << "Usage: \n";
  cerr << "   " << pName << " chanfile outfile\n";
  cerr << "Where: \n";
  cerr << "    chanfile - is the name of a file containing a list of\n";
  cerr << "               EPICS channel names.  '-' means stdin is used\n";
  cerr << "    outfile  - is the name of the file to which output \n";
  cerr << "               is written \n";

}

/*!  
   Open an input file. If the name argument is "-" then we return a reference
   to a dflt file.  Otherwise, open the input file named after the argument.
   If the input file cannot be opened, issue a message, usage and exit.
   \param fname (const char* [in]):  Name of the file to open.
   \param dflt (istream& [in]):
      Dflt file in case - is the name of the file.
   \return istream&
      Reference to the file that was opened (new'd).
*/
istream&
openinput(const char* fname, istream& dflt)
{

  if(strcmp(fname, "-") != 0) {
    ifstream* infile = new ifstream(fname);
    if(!(*infile)) {
      cerr << "Could not open input file: " << fname << endl;
      Usage(pProgram);
      exit(2);
    } else {
      return *infile;
    }
  } else {
    return  dflt;
  }
}
/*!
   Open an output file.  If the name argument is "-" then we return
   a reference to a dflt file.  Otherwise open the file and return
   a reference to the opened file.

   \param fname (const char* [in]): Name of file to open.
   \param dflt (ostream& dflt [in]): Dflt file stream.

   \return ostream&  
      reference to the output file to use.
*/
ostream&
openoutput(const char* fname, ostream& dflt)
{
  if(strcmp(fname, "-") != 0) {
    ofstream* pFile = new ofstream(fname);
    if(!(*pFile)) {
      cerr << "Open failed for: " << pFile << endl;
      Usage(pProgram);
      exit(2);
    } else {
      return *pFile;
    }
  }
  else {
    return dflt;
  }
}


/*!
    This program creates a log of the current set of channel values and
    units for a list of units in a file or from stdin.
    \verbatim
    Usage:
        chanlog chanfile outfile
        chanfile - the name of a file containing the list of channels to 
	           monitor if '-' the channel list is taken from stdin.
        outfile  - the name of a file to which the log is written.
	           If '-' then this is stdout.
    \endverbatim
    \param argc (int [in]):  
       Total number of command line words.
    \param argv (char** [in]):
       Pointers to the command line words.

    \return int 0 unix successful return.
*/
int
main(int argc, char** argv) 
{

  // There must be 2 parameters, after the name of the command:

  pProgram = argv[0];

  argc--;
  argv++;
  if(argc != 2) {
    Usage(pProgram);
    exit(-1);
  }

  // Open the input and output file.
  //
  istream& channelfile(openinput(argv[0], cin));
  ostream& outputfile(openoutput(argv[1], cout));
  
  // Build a vector of channel names:

  vector<string> channels;
  while (channelfile) {
    string channel;
    channelfile >> channel;
    channels.push_back(channel);
  }

  EpicsInit(argc,argv);


  for(int i = 0; i < channels.size(); i++) {
    if(channels[i] != "") {
      char data[1000];
      GetChannel(data, channels[i].c_str());
      outputfile << data << endl;
    }
  }


  EpicsShutdown();

  exit(0);

}
/*!
   \mainpage   Program usage

    \verbatim
        export LD_LIBRARY_PATH=/soft/intel/epics/lib/linux-x86
        chanlog    chanfile outfile

    \endverbatim
    - \em chanfile is the name of a file that contains the set of epics channels
      to list.  Each channel and its associated units will be written to the
      ouptut file.  If the channel file is a dash ('-'). The list of channels
      will be taken from stdin.  Channel lists are the names of channels, one 
      name per line.  No commenting or blank lines are allowed in the 
      channel list in this implementation.
    - \em outfile is the name of a file that will be open for write to which
      the channel information will be written.  If the outfile name is a dash
      ('-'), stdout will be used.  Each line of the output file will contain
      the channel name followed by a colon followed by the value and units.

*/
