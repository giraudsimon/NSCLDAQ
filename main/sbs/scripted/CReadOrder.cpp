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

static const char* Copyright = "(C) Copyright Michigan State University 1977, All rights reserved";

////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CReadOrder.h"    				

using namespace std;



#include <string>
#include <algorithm>
#include <TCLInterpreter.h>
#include <TCLResult.h>
#include <vector>
#include "CModuleCommand.h"
#include "CReadableObject.h"
#include "CDigitizerDictionary.h"
#include "CReadException.h"
#include "CConfigurationParameter.h"
#include "CIntConfigParam.h"
#include "CBoolConfigParam.h"
#include "PacketUtils.h"

/*!
	Constructor: Creates an instance of a CReadOrder class.
	The constructor is a no-op as we are not descended from a base class
	and STL takes care of intiailizing oure single attribute.
*/
CReadOrder::CReadOrder (CTCLInterpreter* pInterp,
			CDigitizerDictionary* pDict,
			const string& rCommand)  :
	CReadableObject(rCommand, *pInterp),
	m_pModules(pDict),
	m_nPacketId(-1),
	m_fPacketize(false),
	m_pPacketIdParam(0),
	m_pPacketizeParam(0)
{  
  SConfigurableObject::ParameterIterator p;
  p                 = AddBoolParam("packetize", false);
  m_pPacketizeParam = (CBoolConfigParam*)(*p);
  p                 = AddIntParam("id",-1); // Start with illegal id.
  m_pPacketIdParam  = (CIntConfigParam*)(*p);
  m_pPacketIdParam->setRange(0, 0xffff);    // The id  must fit in a word. 

  
} 
/*!
    If we are destroyed, then all the modules we are reading get unlinked   
    from us.

*/
CReadOrder::~CReadOrder ( )  //Destructor - Delete dynamic objects
{
  OnDelete();
}



// Functions for class CReadOrder

/*!
Adds a module to the end of the readout list.
\param pModule  CReadableObject* [in] The module to add.

*/
void 
CReadOrder::Add(CReadableObject* pModule)  
{ 
  if(pModule) {
    pModule->Link(this);	// Link the module to us...
    m_ReadoutList.push_back(pModule);
  }
  else {
    throw string("CReadOrderAdd:: pModule is a null pointer!");
  }
}  

/*!  
   Removes a module from the 
   readout list.  The module is selected by an iterator:
   
   \param p  ModuleIterator [in] 'points' to the module to delete.  If end(),
	    this function is a noop.

*/
void 
CReadOrder::Remove(ModuleIterator p)  
{ 
   if(p != readerend()) {
     (*p)->Unlink();
     m_ReadoutList.erase(p);
   }
   else {
     throw string("CReadOrder::Remove - ModuleIterator p == readerend()!");
   }
}  
void
CReadOrder::Remove(CReadableObject* pModule) 
{
  if(pModule) {
    ModuleIterator p = find(readerbegin(), readerend(), pModule);
    Remove(p);
  }
  else {
    throw string("CReadOrder::Remove - pModule is a null pointer!");
  }

}

/*!  Function: 	

- Update the configuration parameters from our configuration:
- Inititializes the modules in the reaout chain

*/
void 
CReadOrder::Initialize()  
{
  
  m_fPacketize = m_pPacketizeParam->getOptionValue();
  m_nPacketId  = m_pPacketIdParam->getOptionValue();
  if(m_fPacketize && (m_nPacketId < 0)) {
    throw string("CReadorder::Initialize- packetization enabled, but no id set");
  }
  ModuleInitialize init;
  for_each(readerbegin(), readerend(), init);
}  

/*!  Function: 	
  
Prepares the modules in the readout chain.

*/
void 
CReadOrder::Prepare()  
{
    ModulePrepare prepare;
    for_each(readerbegin(), readerend(), prepare);
}  


/*!
  Reads an event into an ordinary buffer.
  \return the number of words read.
*/
int
CReadOrder::Read(void* pBuf)
{
  ModuleIterator pM = readerbegin();
  uint16_t*         p((uint16_t*)pBuf);
  uint16_t*         pwords((uint16_t*)pBuf);
  int            nRead(0);
  CReadException except;
  bool           jumbo = true; 	//  for now ring buffers counts are all 32 bit.
	

  if(m_fPacketize) {
		if (jumbo) {
			p = PacketUtil::startPacket(p, m_nPacketId);
			nRead = 3;
		} else {
			p = PacketUtil::startPacket16(p, m_nPacketId);
			nRead = 2;			// Packet overhead is 2 words.
		}
  }

  try {
    while(pM != readerend()) {
      CReadableObject* pModule = *pM;
      int nThisRead            = pModule->Read(p);
      nRead                   += nThisRead;
      p                       += nThisRead;
      pM++;
    }
  }
  catch (CReadException e) {
    except.Add(e.GetString(), e.GetCount());
  }
  catch (string msg) {
    except.Add(msg, nRead);
  }
  catch (...) {
    except.Add(string("Unanticipated exception"),
		      nRead);
  }

  if(m_fPacketize) {
    if (jumbo) {
			nRead = PacketUtil::endPacket(pwords, p);
      
		} else {
			nRead = PacketUtil::endPacket16(pwords, p);
    }
  }
  if(except.GetCount()) {
    throw except;
  }
  return nRead;

}
/*!  
Clears the modules in the readout list.

*/
void 
CReadOrder::Clear()  
{ 
   ModuleClear clear;
   for_each(readerbegin(), readerend(), clear);
}  

/*!  

Returns the number of modules in the readout chain.


*/
int 
CReadOrder::readersize()  
{ 
   return m_ReadoutList.size();
}  

/*!  

Returns a begin iterator for the module list.


*/
CReadOrder::ModuleIterator 
CReadOrder::readerbegin()  
{ 
   return m_ReadoutList.begin();
}  

/*!  
Returns an end of iteration iterator for the
digitizers in the digitizer list.


*/
CReadOrder::ModuleIterator 
CReadOrder::readerend()  
{ 
   return m_ReadoutList.end();
}
/*!
    Locates a module by name.
    \param rName  const string& [in] Name of the module to locate.
    \return An iterator to the module else end() if the module does not exist.
*/
CReadOrder::ModuleIterator
CReadOrder::readerfind(const string& rName)
{
   CompareName cname(rName);
   return find_if(readerbegin(), readerend(), cname);
}

/*!
  Executes the readout command. This entry point just
  figures out if the subcommand is:
  - add - Dispatches to AddCommand
  - remove - Dispatches to RemoveCommand.
  - list - Dispatches to ListCommand.
  
  \param rInterp CTCLInterpreter& [in]  The interpreter that
	is running this command.
  \param rResult CTCLResult [ou] The result string that is
	filled in by this command.
   \param nArgc int [in] Number of command parameters,
	 note that the 0'th is the command itsef.
   \param pArgv char** [in] the parameters passed to the
	    command. Note that pArgv[0] == "readout".
   
   \return int one of:
   - TCL_OK - if everything worked ok.
   - TCL_ERROR - If there was an error.  More than likely,
     rResult will have error information.
*/
int
CReadOrder::operator()(CTCLInterpreter& rInterp,
		       CTCLResult&        rResult,
		       int nargc, char** pargv)
{
  int     nArgc(nargc);		// Use copies so the originals can be
  char**  pArgv(pargv);		// passed unmodified to base class operator()
  int nStatus = TCL_OK;
   
   // Skip the command keyword:
   
   nArgc--;
   pArgv++;
   
   // All commands require at least one additional parameter:
   
   if(nArgc < 1) {
      nStatus = TCL_ERROR;
      rResult = Usage();
   }
   else {
     // Dispatch to appropriate function:
     
     string SubCommand(*pArgv);
     if(SubCommand == string("add")) {
       nStatus = AddCommand(rInterp, rResult, nArgc, pArgv);
     }
     else if (SubCommand == string("list")) {
       nStatus = ListCommand(rInterp, rResult, nArgc, pArgv);
     }
     else if (SubCommand == string("remove")) {
       nStatus = RemoveCommand(rInterp, rResult, nArgc, pArgv);
     }
     else {
       // Could be processed by base classes:
       
       nStatus  = CReadableObject::operator()(rInterp, rResult,
					      nargc, pargv);
       if(nStatus != TCL_OK) {
	 rResult += Usage();	// Append our usage.
       }
     }
   }
   return nStatus;
}
   
/*!
    processes subcommand to add a set of modules to the
    readout list.  The modules to add are specified by name
    on the command line. Adding the module list is an all or 
    nothing proposotion.  Either all are aded or none.
    \param rInterp CTCLInterpreter& [in] TCL interpreter that
	  excecuted this command.
    \param rResult CTCLResult& [out] The result string showing
	  the final status of the command.
    \param nArgc int [in] The number of parameters remaining
	  on the command line.  The main command has been
	 stripped off, but the subcommand is still in the
	 list.
    \param pArgv char** [in] The command parameters including the
       subcommand keyword.
   \return int status of the command:
      - TCL_OK - The command worked fine.
      - TCL_ERROR - There was a problem executing the command
	    the reason for the error will be returned in 
	    rResult.
*/
int 
CReadOrder::AddCommand(CTCLInterpreter& rInterp,
			   CTCLResult&       rResult,
			   int nArgc, char** pArgv)
{
   int nStatus = TCL_OK;
   
   // Skip command keyword:
   
   nArgc--;
   pArgv++;
   
   // Build vectors of good and bad modules (those that
   // exist and those that don't.
   //
   vector<CReadableObject*> good;
   vector<string> bad;
   
   while (nArgc) {
      CDigitizerDictionary::ModuleIterator p = m_pModules->DigitizerFind(string(*pArgv));
      if(p == m_pModules->DigitizerEnd()) {
	 nStatus = TCL_ERROR;
	 bad.push_back(string(*pArgv));
      }
      else {
	 good.push_back((p->second));
      }
      nArgc--;
      pArgv++;
   }
   if(nStatus == TCL_OK) {	// All modules are ok.
      for(int i =0; i < good.size(); i++) {
	 Add(good[i]);
      }
   }
   else {                      // Some modules don't exist.
      rResult = "The following modules do not exist: ";
      for(int i =0; i < bad.size(); i++) {
	 rResult += bad[i];
	 rResult += " ";
      }
      rResult += "\nNo modules added to readout";
   }

   return nStatus;
}
/*!
   Removes a module from the readout list.
   The module still exists and can be added back to the
   list at any time in the future.
    \param rInterp CTCLInterpreter& [in] TCL interpreter that
	  excecuted this command.
    \param rResult CTCLResult& [out] The result string showing
	  the final status of the command.
    \param nArgc int [in] The number of parameters remaining
	  on the command line.  The main command has been
	 stripped off, but the subcommand is still in the
	 list.
   \return int status of the command:
      - TCL_OK - The command worked fine.
      - TCL_ERROR - There was a problem executing the command
	    the reason for the error will be returned in 
	    rResult.

*/
int 
CReadOrder::RemoveCommand(CTCLInterpreter& rInterp,
			     CTCLResult&       rResult,
			     int nArgc, char** pArgv)
{
   int nStatus = TCL_OK;
   
   // Skip past the subcommand parameter:
   
   nArgc--;
   pArgv++;
   
   // The next parameter is a single module that will be deleted:
   
   if(nArgc != 1) {
      nStatus = TCL_ERROR;
      rResult = "Too many parameters after module remove\n";
      rResult += Usage();
   }
   else {
	string Module(*pArgv);
	ModuleIterator p = readerfind(Module);
	if(p != readerend()) {
	   Remove(p);
	}
	else {
	   nStatus = TCL_ERROR;
	   rResult = "Module ";
	   rResult += Module;
	   rResult += " not in the readout list";
	}
   }
   return nStatus;
}
/*!
   List the modules that are in the readout list that match an optional pattern.
   If the pattern is omitted, then it is assumed to be *.
   \param rInterp CTCLInterpreter& [in] TCL interpreter that
	  excecuted this command.
    \param rResult CTCLResult& [out] The result string showing
	  the final status of the command.
    \param nArgc int [in] The number of parameters remaining
	  on the command line.  The main command has been
	 stripped off, but the subcommand is still in the
	 list.
   \param pArgv char** [in] the command parameters including the 
      subcommand.  There should be at most two:
	 - The subcommand
	 - The optional match pattern.
   \return int status of the command:
      - TCL_OK - The command worked fine.
      - TCL_ERROR - There was a problem executing the command
	    the reason for the error will be returned in 
	    rResult.

   \note We use the for_each generic algorithm which a function object
   that matches the pattern and conditionally adds the listing to a reference to
   the result string.  Each entry in the string is a sublist containing:
      module_name module_type

*/
int 
CReadOrder::ListCommand(CTCLInterpreter& rInterp,
			   CTCLResult&       rResult,
			   int nArgc, char** pArgv)
{
   int nStatus = TCL_OK;
   
   // Skip the subcommand parameter:
   
   nArgc--;
   pArgv++;
   
   // Validate the command parameter count and figure out 
   // the match pattern.
   
   if(nArgc > 1) {
      nStatus = TCL_ERROR;
      rResult   = "Extra parameters following 'readout list' command\n";
      rResult += Usage();
   }
   else {
      string pattern("*");
      if(nArgc) {
	 pattern = *pArgv;
      }
      Lister visitor(rResult, pattern);
      for_each(readerbegin(), readerend(), visitor);
     
   }
   return nStatus;
}
/*!
    Return a string that provides helpful usage information.
    
*/
string
CReadOrder::Usage()
{

  string   result;
  string  cmd = getCommandName();
  result      = CReadableObject::Usage(); // Base class usage too!!
  result     +="\t";
  result     +=  cmd;
  result     += " add module1 ?module2...?\n";
  result     +="\t";
  result     += cmd;
  result     += " list ?glob-pattern?\n";
  result     +="\t";
  result     += cmd;
  result     += " remove module\n";   

   return result;
}
/*!
   Called prior to delete: unlink everyone from us.  It's harmless to call
   this multiple times.
*/
void
CReadOrder::OnDelete()
{
  ModuleIterator p = readerbegin();
  for(; p != readerend(); p++) {
    (*p)->Unlink();		// They must be linked or they would not be in
  }                             // our module list!
 
  // Let the list destructor delete the list itself.

}
/*!
  Return the  type of the module this is (packetizer).

*/
string
CReadOrder::getType() const
{
  return string("packet-container");
}
