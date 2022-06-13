/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2013.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


static const char* Copyright = "(C) Copyright Ron Fox 2002, All rights reserved";
/*! 
           Executes the module command.  The module command has the following form:
           module newname type {configuration data}
           module -list ?pattern
           module -delete name
           module -types
           
           The module command relies on the recognizer pattern.
           m_Creators is a list of module type creators.  The creational
           form of the module command iterates through the set of
           creators looking for one that matches the type keword.
           When one is found it is used to create the actual module.
           
           
*/

////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CModuleCommand.h"  
#include "CModuleCreator.h"
#include "CReadableObject.h"
#include "CReadOrder.h"
#include <tcl.h>
#include <algorithm>
#include <assert.h>
#include <TCLInterpreter.h>
#include <TCLResult.h>
using namespace std;



/*!  Constructor.  This is essentially a no-op since STL
  containers are capable of default initialization.
*/
CModuleCommand::CModuleCommand (CTCLInterpreter* pInterp,
				CDigitizerDictionary* pDictionary,
				const string&         rCommand) :
  CTCLProcessor(rCommand, pInterp),
  m_pModules(pDictionary)
{
   Register();
			

} 
/*!
   Destroy the module command.  Assumptions:
   - We are not resonsible for destroying the recognizers
     or the modules.
   - STL containers are capable of self destruction.

*/
 CModuleCommand::~CModuleCommand ( )
{
 
}

// Functions for class CModuleCommand

/*! 

Processes the module command. This level just 
determines the type of module command and dispatches
to the associated member function accordingly.
We pass control to protected member functions that process the 
cases:
- Create new module.
- List the set of modules matching some glob pattern.
- delete a specified module.
- Return help on the types of modules supported.

\param rInterp CTCLInterpreter& [in] TCL interpreter that is 
      running this command.
\param rResult CTCLResult& [in] TCL result string that will
      contain the results of the operation.
\param nArgs int [in] Number of command line parameters. Note
   that the command name is the first of the parameters.
\param pArgs char** [in] Set of parameter strings.

\return int one of:
   - TCL_OK    - If everything worked.
   - TCL_ERROR - If there was a problem.

*/
int 
CModuleCommand::operator()(CTCLInterpreter& rInterp, 
			   CTCLResult& rResult, 
			   int nArgs, char** pArgs)  
{ 
   int status;

   // Skip over the 'module' string.
   nArgs--;
   pArgs++;
   if(nArgs <= 0) {
     status = TCL_ERROR;
     rResult = Usage();
   }
   else {
   
     string operation(*pArgs);        // First parameter.
     // operation is either one of the command switches or
     // it is the name of a module:
     
     if( operation == string("-list")) {  // list modules:
       nArgs--;
       pArgs++;    // point to next param.
       status = List(rInterp, rResult, nArgs, pArgs);
     }
     else if( operation == string("-delete")) { // Delete module
       nArgs--; 
       pArgs++;
       status = Delete(rInterp, rResult, nArgs, pArgs);
     }
     else if( operation == string("-types")) { // List types.
       nArgs--;
       pArgs++;
       status = ListTypes(rInterp, rResult, nArgs, pArgs);
     }
     else {                        // New module.
       status = Create(rInterp, rResult, nArgs, pArgs);
     }
     
   }
   return status;
}
/*!  Function: 	

  Executes the command to  create a new digitization module.
  The form of this command is:
\verbatim
    module name type {configuration}
\endverbatim

  - name is the name of the module.
  - type is the type of module to create.
  - {configuration} a parameter describing the type specific 
    configuration.

 m_Creators is iterated through.  At each iteration, the 
 entry's Match function is called to determine if the 
 module creator is suitable for the module type.
 If so, the Creator's create member is called to create and
 configure a new module.

\param rInterp CTCLInterpreter& [in] TCL interpreter running
    the command that called us.
\param rResult CTCLResult& [out] TCL Result string that will 
   contain the text descrigibing what happened.
\param nArgs int [in] Number of parameters on the line
   (must be at least 2, module name and type.
\param pArgs char** [in] Pointers to parameter strings.

\note  Extra parameters are considered to be configuration
      parameters and are passed to the appropriate creator
      without interpretation.
      
\return int One of:
   - TCL_OK    - If everything worked.
   - TCL_ERROR - If there was a problem that prevented
	       the module from being created.
	       
\note if the function succeds, it will enter the new module
   into the module dictionary (m_pModules).
   
*/
int 
CModuleCommand::Create(CTCLInterpreter& rInterp, 
		       CTCLResult& rResult, 
		       int  nArgs, char** pArgs)  
{ 
   int status = TCL_OK;
 // Require the module name and type be present at least:
   
   if(nArgs < 2) {
      rResult = Usage();
      status  = TCL_ERROR;
   }
   else {
      string ModuleName(pArgs[0]);
      string Moduletype(pArgs[1]);
      
      // Determine if this is a duplicate module:
      
      if(DigitizerFind(ModuleName) != DigitizerEnd()) {
	rResult  = "Duplicate module name: ";
	rResult += ModuleName;
	status = TCL_ERROR;
      } else {
	std::pair<const char*, CTCLInterpreter*> info(ModuleName.c_str(), &rInterp);	
	CReadableObject* pModule = m_factory.create(Moduletype, &info);
	if(!pModule) {
	  rResult  = "Unable create module type: ";
	  rResult += Moduletype;
	  rResult += " does not have an associated creator";
	  status = TCL_ERROR;
	}	else {
	  // Meet the expectations of Configure:
	    
	    pArgs[1] = pArgs[0];
	    pArgs++;
	    nArgs--;
	    int status = pModule->Configure(rInterp, rResult, nArgs, pArgs);
	    m_pModules->DigitizerAdd(pModule);	    
	}
      }
   }
   return status;   
}  

/*! 

Lists the set of modules that have been created.
The form of this command is 
\verbatim
    module -list ?pattern?
\endverbatim
 If the optional ?pattern? is supplied it is a glob matching pattern.
Only modules with names matching the pattern are listed.
The ?pattern? parameter defaults to * (match all modules).


   \param rInterp  CTCLInterpreter& [in] Refers to the 
	    Interpreter that's running this command.
   \param rResult CTCLResult& [out] Refers to the result string
	    That will hold any textual status information.
	    On success, this will include information
	    about the modules that have been instantiated.
	    On error, this will be an error message.  Note
	    that the success information will be in the form
	    of a Tcl list.  Each element of the list will be a
	    two element sublist with entries as follows:
	    - name - The name of a module.
	    - type - The type of a module.
   \param nArgs int [in] - Number of parameters remaining on 
	 the input line.
   \param pArgs char** [in] The parameters themselves.
   
   \return int See also the rResult parameter.  The value will
   be one of the following:
   - TCL_OK if everything was successful.
   - TCL_ERROR if there was a problem of some sort.
   
*/
int 
CModuleCommand::List(CTCLInterpreter& rInterp, 
		     CTCLResult& rResult, 
		     int nArgs, char** pArgs)  
{ 
   int nStatus = TCL_OK;
   // Figure out the match
   // pattern.  If none is supplied, it's *.
   
   const char* pPattern = "*";
   if(nArgs > 1) {
      rResult =  "Too many parameters: \n";
      rResult += Usage();
      nStatus = TCL_ERROR;
   }
   else {		       
      if(nArgs) {	      // This is really exactly one parameter : pattern.
								pPattern = *pArgs;    // Update the pattern.
      }
	 
      // Next iterate through the modules in the map
      // adding appropriate modules to the list>
      // Note that an empty list is an acceptable result.
      
      ListGatherer gather(rResult, pPattern);
      for_each(DigitizerBegin(), DigitizerEnd(),
	       gather);
   }
   return nStatus;
}  

/*!

Deletes a module from the module list.  If the module is part 
of the readout it is removed from the readout.
The form of the command is:
\verbatim
    module delete name
\endverbatim
  where name is the name of the module to delete.

\note There is some ugly coupling here: We need to remove the
module from the Readout list.  This can only be done if we
can ask the Readout list object to do that.  As a design 
restriction, we assume the readout list is a singleton that
can give us it's instance via the static member getInstance()

\param rInterp CTCLInterpreter& [in] The TCL interpreter that's
    running this command.
\param rResult CTCLResult& [out] Result string returned.  Empty
      on success, and contains an error message on failure.
\param nArgs int [in] Number of parameters remaining in 
	    the command.
\param pArgs char** [in]  The parameters themselves.

\return int  Return status code see also rResult above. This
      will be one of:
      - TCL_OK - if the command succeeded.
      - TCL_ERROR - if the command failed.

*/
int 
CModuleCommand::Delete(CTCLInterpreter& rInterp, 
		       CTCLResult& rResult, 
		       int nArgs, char** pArgs)  
{
   int nStatus = TCL_OK;
   
   // first validate the syntax.  There should only be
   // one parameter and that will be a module name:
   
  if(nArgs != 1)  {
      rResult  = "Extra parameters following module -delete \n";
      rResult += Usage();
      nStatus  = TCL_ERROR;
   }
   else {
      string sName(*pArgs);            // Name to delete.
      CDigitizerDictionary::ModuleIterator p = DigitizerFind(sName);
      if(p == DigitizerEnd()) {
								nStatus  = TCL_ERROR;
								rResult  = sName;
								rResult += ": Module does not exist";
      } else {

									// Remove the module from the map and...
							
									CReadableObject* pModule = p->second;
									assert(pModule);	// Better be non-null!!
									m_pModules->Remove(p);
									pModule->OnDelete();	// This unlinks it from any reader.
									delete pModule;	// Destroy it.
      }
   }
   return nStatus;
}

/*! 

Lists the types of modules that can be created.
Iterates through the m_Creators and asks each 
of them for their help text.  The form of the help
text is expected to be:
\verbatim
      type   - description of modules of this type
\endverbatim
The information is returned as a command result with successful
completion.  The output at this time is not intended to be 
used by TCL commands as input... it's just normal help text
for humans such as what Usage() returns.

\param rInterp CTCLInterpreter& [in] The interpreter that is 
	 running this command.
\param rResult CTCLResult& [out] The result string that will
   be filled in with the help text.
\param nArgs int [in]  The number of parameters remaining on
      the command line.
\param pArgs char** [in] The parameters themselves.

\return int The status of the command, see also rResult above.
      This can be:
      - TCL_OK - Everything worked, and rResult is the 
	       help text.
      - TCL_ERROR - Some problem was encountered and rResult
	    is an error message.
*/
int 
CModuleCommand::ListTypes(CTCLInterpreter& rInterp, 
			  CTCLResult& rResult, 
			  int nArgs, char** pArgs)  
{ 
   int nStatus = TCL_OK;

   if(nArgs) {                 // No more params allowed.
      rResult  = "Extra parameters after module -types\n";
      rResult += Usage();
      nStatus = TCL_ERROR;
   }
   else {
				std::vector<std::string> ds = m_factory.getDescriptions();
				std::string r;
				for (int i =0; i < ds.size(); i++) {
					r += ds[i];
					r += "\n";
				}
				rResult = r;
			}
   return nStatus;
}  

/*!
Add a module creator to the list of module
creators checked during the module create
command.

   \param pCreator CModuleCreator* [in] Pointer to the new
	 creator to add.  This is assumed to be managed by
	 clients.  Note that modules, on the other hand are
	 managed by us.

*/
void 
CModuleCommand::AddCreator(const char* type, CModuleCreator* pCreator)  
{
  if(pCreator) {
				m_factory.addCreator(std::string(type), pCreator);
  }
}

/*!
  Returns an iterator to the front of the created modules 
Dictionary.
*/
CDigitizerDictionary::ModuleIterator 
CModuleCommand::DigitizerBegin()
{
   return m_pModules->DigitizerBegin();
}
/*!
  Returns an end of iteration iterator to the created modules
Dictionary..
*/
CDigitizerDictionary::ModuleIterator 
CModuleCommand::DigitizerEnd()
{
   return m_pModules->DigitizerEnd();
}
/*!
  Returns the number of modules that have been created:
*/
int               
CModuleCommand::DigitizerSize()
{
   return m_pModules->DigitizerSize();
}
/*!
   Returns command usage information.
*/
string 
CModuleCommand::Usage()
{
   string usage("Usage: \n");
   string cmd = getCommandName();
   usage += "\t\t Create a module: \n\n";
   usage += '\t';
   usage += cmd;
   usage += " name type ?config_params...? \n";
   usage += "\t\t List modules whose names match a pattern\n\n";
   usage += "\t";
   usage += cmd;
   usage += "  -list ?pattern?\n";
   usage += "\t\t Delete a module\n\n";
   usage += "\t";
   usage += cmd;
   usage += "  -delete name\n";
   usage += "\t\t List types of modules that can be created\n\n";
   usage += "\t";
   usage += cmd;
   usage += "  -types\n";
   
   return usage;
}

//  Implementation of local classes:


// ListGatherer:

void 
CModuleCommand::ListGatherer::operator()(pair<string,CReadableObject*>p)
{
  string name((p.first));
  CReadableObject* pModule = p.second;
  assert(pModule);
  string type(pModule->getType());
  if(Tcl_StringMatch(name.c_str(), m_pMatch)) {
    string element(name);
    element += " ";
    element += type;
    m_rResult.AppendElement(element.c_str());
  }
}
//   TypesGatherer:

