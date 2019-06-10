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


/*!
  \file CApplication.cpp

  This file is an implementation of the CApplication class.  
See the documentation about that class for the exported
member inventory and documentation.
The application object encapsulates the top level logic
of the application. This logic can be expressed in the
pseudo code:
\verbatim
Process command arguments
Read channel file, Populating channel list.
While true {
   Connect to Tcl Server
   Connect channels in channel list.
   while connected {
      Update channels to server.
      Wait for interval seconds.
   }
}

\endverbatim


 */


#include <config.h>
#include "CApplication.h"    				
#include "CSocket.h"
#include "CUnitChannel.h"
#include "CChannelList.h"
#include "cmdline.h"
#include "CBuildChannelData.h"
#include "CLookupVisitor.h"
#include "CStrings.h"

#include <Exception.h>


#include <utility>
#include <fstream>  
#include <iostream> 
#include <sstream>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>

using namespace std;

static const int    YEARBASE(1900);           // Base year of struct tm.tm_year

static const int    DEFAULTPORT(2701);        // Default tcp/ip port.
static const string DEFAULTHOST("localhost"); // Default host to connect with.
static const int    INTERVAL(10);             // Default update interval.
static const float  TIMEOUT(1.0);             // Default EPICS timeout we'll use.
static const float  PLCTIMEOUT(10.0); // Timeout for PLC Channels.

static const int    CONNECTRETRYINTERVAL(10); // Seconds between retry attempts.
static const int    CONNECTRETRIES(1000);     // Number of retries allowed. 
static const int    MININTERVAL(5);     // Minimum update interval

static const string DataArrayName("EPICS_DATA"); // Name of data tcl array.
static const string UnitsArrayName("EPICS_UNITS"); // Name of units tcl array.
static const string UpdateArrayName("EPICS_UPDATED"); // when last updated.

/*!
    Construct the application.  We only fill in the default
    values for the setup file, port, host and interval. Much
    of the real work of getting this class ready to roll
    is done by the operator() entry point.
*/
CApplication::CApplication ()
  : m_sSetupFile(""),		// That's a required parameter.
    m_nPort(DEFAULTPORT),
    m_sHost(DEFAULTHOST),
    m_nInterval(INTERVAL),
    m_fMustAuthorize(false)
{ 
  
} 

/*!
   This is a no-op.
*/
 CApplication::~CApplication ( ) 
{

}


// Functions for class CApplication

/*! 

Entry point for the program.
- Parse the parameters
- Read the channel file to build the channel list.
- Connect to the serverand as long as the connection
  is alive, update the channels in the server.
  If the server dies, we try to reconnect.

  \param Parameters (getgetopt_args_info& [in]):
     The massaged program parameters from cmdline_parser.  Note that
     all validation is done either by the caller or the command line
     parser before we are even called.

  \return int
     -1  (since really we should not be exiting if we functioned correctly.

*/
int 
CApplication::operator()(gengetopt_args_info& Parameters)  
{ 
  // We'll just throw an exception if there's an error.
  // that prevents us from dealing with nasty nested ifs.

  try {
    // Process the command line parameters:
    
    // --port=tcp-ip-port
    
    if(Parameters.port_given) {		// Override the default port.
      ValidatePort(Parameters.port_arg); 
      m_nPort = Parameters.port_arg;
    }
    // --interval=seconds
    
    if(Parameters.interval_given) {
      ValidateInterval(Parameters.interval_arg);
      m_nInterval = Parameters.interval_arg;
    }
    // --host=name.
    
    if(Parameters.node_given) {
      m_sHost = string(Parameters.node_arg);
    }

    m_fMustAuthorize = Parameters.authorize_given;

    // Setup file:
    
    m_sSetupFile = string(Parameters.inputs[0]); // Required parameter.


    // Create the channel list.
    
    if(m_sSetupFile == string("-")) { //  Setup file is stdin:
      ReadChannelFile(cin);
      
    } else {			// Setup file is from a file:

      ifstream  ChannelFile(m_sSetupFile.c_str());
      if(ChannelFile) {
	ReadChannelFile(ChannelFile); // Read in the channels.
      } else {			// File open failed...
	ostringstream error;
	error << "Failed to open file: " << m_sSetupFile 
	      << " reason: " << strerror(errno);
	throw string(error.str());	
      }
    }
    // Note if there are no channels in the channel list we can just
    // give up now rather than wasting time and resources:

    if(m_Channels.size() == 0) {
      throw 
	string("There are no non-comment channel lines in the channel file");

    }

    // Main loop.

    while(1) {
      ConnectToServer(CONNECTRETRYINTERVAL,
		      CONNECTRETRIES);
      cerr << "Connected to TCL server looking up channels\n";
      CLookupVisitor ChannelInitializer;
      m_Channels.foreach(ChannelInitializer);	// Lookup channels and revive.
      cerr << "Channel lookup complete\n";
      while(m_Socket.getState() == CSocket::Connected) {
	Update();
	Delay(m_nInterval);
      }
    }
    throw "Unable to form a connection to the server";
    
    // Exit implies an error:
    
  }
  catch(string msg) {		// Exit with error message.
    cerr << msg << endl;
 }
  catch (...) {
    cerr << "An unanticipated exception was caught in CApplication::operator() "
	 << " -- exiting\n";
  }
  return -1;

}  

/*! 


Reads the channel file, building up
the channel list.  As each channel is
created and added to the channel list
it is asked to look itself up in epics.
The following are ignored as comments:
- Blank lines
- Lines whose first nonblank character is a #
- Anything after the first `word' on the line.

Channels that fail their lookup are discarded noisily.
The noise comes out on stderr.

\param Input (istream& [in]):
   Input file.

*/
void 
CApplication::ReadChannelFile(istream& Input)  
{ 
  int nLineNumber = 0;
  string ChannelName; 
  while(!Input.eof()) {
    char line[1000];
    Input.getline(line, sizeof(line));
    if(Input.fail() && !Input.eof()) {
      throw string("Input line is longer than maximum allowed (1000 chars)");
    }
    if(Input.fail() && Input.eof()) {
      return;
    }
    nLineNumber++;
    if(strlen(line) == 0) {
      continue;			// Ignore empty lines.
    }
    int nlBlanks = strspn(line, " \t\r");	// count leading whitespace.
    if(nlBlanks == strlen(line)) {
      continue;			// Line all blanks.
    }
    char* WordStart = &(line[nlBlanks]); // Pointer to the start of the word...
    if(*WordStart == '#') {
      continue;			// First nonblank is a # 
    }
    int nInWord = strcspn(WordStart, "\t \r"); // Chars in the word...
    ChannelName = "";		// Init the name to empty.
    for(int i =0; i < nInWord; i++) {
      ChannelName += WordStart[i]; // Fill in the name.
    }
    // ChannelName  is now a channel name. create the channel.
    // If a channel cannot be created we yell and continue.
    //
    try {
      CUnitChannel* pChannel = new CUnitChannel(ChannelName);
      m_Channels.push_back(*pChannel);
    }
    catch(string error) {
      cerr << "Failed to connect to channel " <<  ChannelName  << " in: " 
	   << m_sSetupFile << ":" << nLineNumber << endl;
      cerr << "According to channel layer this is because: "
	   << error << endl;
    }
    catch(...) {
      cerr << "Unanticipated failure type in CApplication::ReadChannelFile"
	   << "Reading file: " << m_sSetupFile << ":" << nLineNumber 
	   << "Channel name (empty if not a channel) '" << ChannelName << "'"
	   << endl;
    }
					       
    
  }
}  

/*!  

- Invokes the Update member function of 
  m_Channels, to update the current control system
  values.
- Creates a CBuildChannelData visitor object and
  passes it to  m_Channels.foreach to collect the
  current data.
- Passes the CBuildChannelData object to 
  ChannelsToServer to update the TCLserver's
  idea of what the channel values etc. are.

*/
void 
CApplication::Update()  
{
  CBuildChannelData collector;
  m_Channels.foreach(collector);
  ChannelsToServer(collector);
  
}  

/*! 

Takes the channels parameter and iterates through
the pairs of channel name/value information.
For each channel name/value, a TCL command is
created and the value appropriately quoted/escaped.
The TCL Command is sent to the TCL server for execution.

\param chans (CBuildChannelData&)
   The visitor that has been building up channel data.

*/
void 
CApplication::ChannelsToServer(CBuildChannelData& chans)  
{ 
  CBuildChannelData::ChannelItemIterator i = chans.begin();
  while(i != chans.end()) {

    // Fetch the data out of the channel data block:

    string name     = i->first;
    string value    = i->second.m_sValue;
    string units    = i->second.m_sUnits;
    string updated  = FormatTime(i->second.m_Updated);

    // Format the commands and send them on to the server.
    // If we lose the connection then break from the loop.
    // the main loop will take care of any reconnection stuff.
    //
    try {
      m_Socket.Flush();
      string command;
      command = GenerateSet(DataArrayName, name, value);
      m_Socket.Write((void*)command.c_str(), command.size());
      
      if(units != string("")) {
	command = GenerateSet(UnitsArrayName, name, units);
	m_Socket.Write((void*)command.c_str(), command.size());
      }
      
      command = GenerateSet(UpdateArrayName, name, updated);
      m_Socket.Write((void*)command.c_str(), command.size());
    }
    catch (CException& problem) {
      cerr << ">>>WARNING<<< failed to write all channels to server: " 
	   << problem.ReasonText() << endl;
      cerr << "              Will attempt to retry server connection\n";
      break;
    }
    i++;
  }
}  

/*! 


Attempts to connect to the TCLserver specified by 
the member variables. If the connection fails, 
it will be retried periodically until a connection is
established, or until the retry count is exceeded.
\param nRetryInterval  (int) Seconds between retries.
\param nRetries        (int) Maximum number of retries.

\throw string
   Throws a string if the connection could not be established.


If m_fMustAuthorize is true our current username is sent to the
server for authentication purposes.


*/
void 
CApplication::ConnectToServer(int nRetryInterval, int nNumRetries)  
{
  ostringstream p;
  p << m_nPort;
  string port(p.str());

  // Validate the parameters.
  
  if(nRetryInterval <= 0) {
    throw string("CApplication::ConnectToServer - invalid nRetryInterval");
  }
  if(nNumRetries <= 0) {
    throw string("CApplication::ConnectToServer - invalid nNumRetries");
  }


  // Now loop on connection attempts until connected or the
  // retry count is exceeded.


  while(m_Socket.getState() != CSocket::Connected  && nNumRetries) {
    try {
      m_Socket.Connect(m_sHost, port);
      if(m_Socket.getState() == CSocket::Connected) {
	break;			// Success!
      }
    } 
    catch (CException& Except) {
      nNumRetries--;
      cerr << "Connection to tclserver failed..." 
	   << Except.ReasonText()
	   << " " << nNumRetries << "Retries remaining " << endl;
    }
    sleep(nRetryInterval);
  }
  if((nNumRetries == 0) && (m_Socket.getState() != CSocket::Connected)) {
    ostringstream error;
    error << "Unable to connect to " << m_sHost << ":" << m_nPort;
    throw string(error.str());
  }
  else {
    // Connected if necessary authorize:

    if (m_fMustAuthorize) {
      uid_t euid = geteuid();             // We'll use effective uid for username.
      struct passwd* pwdEntry = getpwuid(euid);

      if (pwdEntry) {
	string authString = pwdEntry->pw_name;
	authString += "\n";
	void *pData = const_cast<char*>(authString.c_str());
	m_Socket.Write(pData, authString.size());
      }

    }
  }
}
//// Utility functions:

/*!  
   Ensure a port number is valid.
   \param int - port number.
   \throw string 
      If the port is invalid a descriptive error message is thrown.
*/
void
CApplication::ValidatePort(int port)
{
  // Ports are positive.

  ostringstream error;
  int maxport = ((1 << (sizeof(short)*8)) -1);
  
  if(port < 0) {
    error << "Invalid port number: " << port 
          << " Port numbers must be positive integers.";
    throw string(error.str());
  }
  // Ports fit into shorts.

  if(port > maxport)  {
    error << "Invalid port number. "  << port
	  << " Port numbers must be <= " << maxport;
    throw string(error.str());
 
  }
  
}
/*!
   Validate the time interval over which we allow the channels to update.
   The interval is unbounded on the upper end, however it must be
   greater than MININTERVAL.  
   \param interval - the interval in seconds.
   \throw string 
      if the interval is not valid.
*/
void
CApplication::ValidateInterval(int interval)
{
  if(interval < MININTERVAL) {
    ostringstream error;
    error << "Invalid interval value: " << interval
	  << " Intervals must be >= " << MININTERVAL;
    throw string(error.str());
  }
}

/*!
   Construct a time from the time_t array in cannonical time form.
   We steal the time format from sql's concept of time.  A time is 
   formatted as:
   /verbatim
   yyyy-mm-dd hh:mm:ss
  /endverbatim
  - yyyy is a four digit year number.
  - mm   is the two digit month number.
  - dd   Is the two digit day number.
  - hh  is the two digit hour.
  - mm  is the two digit minutes.
  - ss  is the two digit second.

  This format supports date comparisons as simple lexical string compares.

  \param time_t t
      The time value to convert
  \return string
     The formatted time.
*/
string
CApplication::FormatTime(time_t t)
{
  if(t == 0) {
    return string("0000-00-00 00:00:00");
  } 
  else {
    char timestamp[100];
    struct tm* timestruct = localtime(&t);
    sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d",
	    timestruct->tm_year+YEARBASE,
	    timestruct->tm_mon+1,
	    timestruct->tm_mday,
	    timestruct->tm_hour,
	    timestruct->tm_min,
	    timestruct->tm_sec);
    return string(timestamp);
  }
}
/*!
   Creates the Set command for a variable, its index and 
   value.  Note that the value will be quoted and escaped so that
   the command is always legal.
   \param arrayname   (const string&)
     name of the array to do the set command for.
   \param index       (const string&)
     index string for the array.
   \param value       (const string&)
     value to set the array element to.

*/
string
CApplication::GenerateSet(const string& arrayname, const string& index, 
		     const string& value)
{
  if(arrayname == string("")) {
    // There must be a variable name!!!

    throw string("CApplication::GenerateSet called with a blank arrayname");
  }
  if(index == string("")) {
    // there must be an index...
    throw string("CApplication::GenerateSet called with a blank index");
  }

  // Construct the value string escaped and quoted so that 
  // all possible values will be legal...

  string escaped = CStrings::EscapeString(value.c_str(), 
					  "\"[$#;", "\\");
  string quotedvalue = "\"";
  quotedvalue += escaped;
  quotedvalue += "\"";

  // Now construct the command:

  string command("set ");
  command  += arrayname;
  command  += "(";
  command  += CStrings::EscapeString(index.c_str(), "\"[$#;", "\\");
  command  += ") ";
  command  += quotedvalue;
  command  += "\n"; 

  return command;
}


/*!
   Delay for the specified number of seconds.
   - Check that there are no pending IO's  This should be true since
     I/O according to EPICS are outstanding get/search requests and we have
     finished those.  An error is emitted if there are outstanding I/O's.
     This is informative, not fatal.
   - Call ca_pend_event for the indicated timeout. According to my reading
     of the book, this will block me but allow EPICS to continue to do whatever
     background processing it needs to do.
   \param seconds (int):
      Number of seconds to delay.

*/
void
CApplication::Delay(int seconds)
{
  CChannel::doEvents((float)seconds);

} 
