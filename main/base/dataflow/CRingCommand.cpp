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
#include "CRingCommand.h"
#include "CRingMaster.h"
#include <CRingBuffer.h>
#include <Exception.h>
#include <ErrnoException.h>
#include <errno.h>

#include "TCLInterpreter.h"
#include "TCLObject.h"



using namespace std;

//////////////////////////////////////////////////////////////////////////////////
// Constructors and implemented canonicals:


/*!
  Construct the command processor
  \param interp  - The interpreter the command will be registered on.
  \param command - The command keyword, defaults to "ringbuffer"
*/
CRingCommand::CRingCommand(CTCLInterpreter& interp, 
			   string command) :
  CTCLObjectProcessor(interp, command)
{}

/*!
  Destructor just chains to the base class to ensure we get unregistered.
*/
CRingCommand::~CRingCommand()
{}


////////////////////////////////////////////////////////////////////////////////
//  Command processor:

/*!
  Gets control to process the command
  We are just going to validate the existence of and validity of the subcommand and
  dispacth to a subcommand processor.

  \param interp - The interpreter that is processing the command.
  \param objv   - Vector of objects that represent the command words as parsed/substituted
                  by interp.

  \return int
  \retval TCL_OK - the command succeeded.
  \retval TCL_ERROR - The command failed in some way.

\note The interpreter result will contain some error message if TCL_ERROR was returned;
See the comments for the various command processors for information about what else might
be returned at the script level as the command's result.
*/

int
CRingCommand::operator()(CTCLInterpreter&    interp,
			 vector<CTCLObject>& objv)
{
  //  Bind all of the objv's to the interpreter to save the processors the embarrassment:

  for (int i = 0; i < objv.size(); i++) {
    objv[i].Bind(interp);
  }
  // Ensure we at least have a command keyword and extract it:

  if (objv.size() < 2) {
    string result;
    result   += "Not enough command parameters:\n";
    result   += CommandUsage();
    interp.setResult(result);
    return TCL_ERROR;
  }
  string subCommand = objv[1];

  // Dispatch depending on the subcommannd:

  if (subCommand == string("create")) {
    return create(interp, objv);
  }
  else if (subCommand == string("format")) {
    return format(interp, objv);
  }
  else if (subCommand == string("disconnect")) {
    return disconnect(interp, objv);
  }
  else if (subCommand == string("usage")) {
    return usage(interp, objv);
  }
  else if (subCommand == string("remove")) {
    return remove(interp,objv);
  } else if (subCommand == string("list")) {
    return list(interp, objv);
  } else {
    string result;
    result += "Invalid subCommand keyword: ";
    result += subCommand;
    result += "\n";
    result += CommandUsage();
    interp.setResult(result);
    return TCL_ERROR;
  }


}

////////////////////////////////////////////////////////////////////////////////////////////////
//  Sub command processors.


/*************************************************************************/
/* create a new ring buffer:                                             */
/*  ringbuffer create  name ?size ?maxconsumers??                        */
/*  name - the name of the ring buffer.                                  */
/*  size - the optional size specification                               */
/*  maxconsumers - the optional maximum consumer count.                  */
/*                                                                       */
/* Result:                                                               */
/*   An error message if an error occurs.                                */
/*   The name of the ring buffer created if a new one is created.        */
/*************************************************************************/

int
CRingCommand::create(CTCLInterpreter& interp, 
		     vector<CTCLObject>& objv)
{
  // Validate the command count:

  if ((objv.size() < 3) || (objv.size() > 5)) {
    string result;
    result += "Incorrect number of parameters for ringbuffer create\n";
    result += CommandUsage();
    return TCL_ERROR;
  }
  
  // Pull out the name and default the numeric parameters.

  string name      = objv[2];
  size_t size      = CRingBuffer::getDefaultRingSize();
  size_t consumers = CRingBuffer::getDefaultMaxConsumers();

  // If present, update the size from the objv:

  if (objv.size() > 3) {
    try {
      size = (int)(objv[3]);
    }
    catch (...) {
      string result;
      result += "Optional size parameter must be numeric\n";
      result += CommandUsage();
      interp.setResult(result);
      return TCL_ERROR;
    }
  }
  // If present, update consumers from the objv:

  if (objv.size() == 5) {
    try {
      consumers = (int)(objv[4]);
    }
    catch(...) {
      string result;
      result += "Optional max consumer parameter must be numeric\n";
      result += CommandUsage();
      interp.setResult(result);
      return TCL_ERROR;
       
    }
  }
  // Create the ring buffer:

  try {
    CRingBuffer::create(name, size, consumers);
  }
  catch (CException& reason) {
    string result;
    result += "Failed to create ring buffer:\n";
    result += string(reason.ReasonText());
    interp.setResult(result);
    return TCL_ERROR;
  }
  catch (string msg) {
    string result;
    result += "Failed to create ring buff:\n";
    result += msg;
    result += '\n';
    interp.setResult(result);
    return TCL_ERROR;
  }
  catch (const char* pmsg) {
    string result;
    result += "Failed to create ring buffer\n";
    result += pmsg;
    result += '\n';
    interp.setResult(result);
    return TCL_ERROR;
  }
  catch (...) {
    string result;
    result += "Faile to create ring buffer\n";
    interp.setResult(result);
    return TCL_ERROR;
  }


  interp.setResult(name);
  return TCL_OK;
}
/**********************************************************************/
/* format a ring buffer that already exists.                          */
/* ringbuffer format name ?maxconsumers?                              */
/*                                                                    */
/* Result;                                                            */
/*  On TCL_OK - the name of the ring buffer is returned.              */
/*  on TCL_ERROR - some descriptive error message is returned.        */
/**********************************************************************/

int 
CRingCommand::format(CTCLInterpreter& interp, 
		     vector<CTCLObject>& objv)
{

  // We need 3 or 4 parameters:

  if ((objv.size() < 3) || (objv.size() > 4)) {
    string result;
    result += "Incorrect number of command parameters\n";
    result += CommandUsage();
    interp.setResult(result);
    return TCL_ERROR;
  }

  // Extract the name of the buffer to format and default value for max consumers:

  string name      = objv[2];
  size_t consumers = CRingBuffer::getDefaultMaxConsumers();

  // If we have a maxconsumers attempt to decode it:

  if (objv.size() == 4) {
    try {
      consumers = (int)(objv[3]);
    }
    catch (...) {
      string result;
      result += "Invalid maxconsumers value, must be an integer, was: ";
      result += (string)(objv[3]);
      result += '\n';
      interp.setResult(result);
      return TCL_ERROR;
    }
  }
  //  Attempt the format:

  try {
    CRingBuffer::format(name, consumers);
  }
  catch (CException& reason) {
    string result;
    result += "Failed to format ring buffer:\n";
    result += string(reason.ReasonText());
    interp.setResult(result);
    return TCL_ERROR;
  }
  catch (string msg) {
    string result;
    result += "Failed to format ring buff:\n";
    result += msg;
    result += '\n';
    interp.setResult(result);
    return TCL_ERROR;
  }
  catch (const char* pmsg) {
    string result;
    result += "Failed to format ring buffer\n";
    result += pmsg;
    result += '\n';
    interp.setResult(result);
    return TCL_ERROR;
  }
  catch (...) {
    string result;
    result += "Faile to format ring buffer\n";
    interp.setResult(result);
    return TCL_ERROR;
  }

  interp.setResult(name);
  return TCL_OK;
}
/**********************************************************************/
/* ringbuffer disconnect producer name                                */
/* ringbuffer disconnect consumer name index                          */
/*                                                                    */
/* Result:                                                            */
/*   TCL_OK - empty string.                                           */
/*   TCL_ERROR - reason for the error.                                */
/**********************************************************************/

int
CRingCommand::disconnect(CTCLInterpreter&    interp,
			 vector<CTCLObject>& objv)
{
  // We need to have the producer/consumer keyword at least:

  if (objv.size() < 3) {
    string result;
    result += "Not enough ringbuffer disconnect parameters\n";
    result += CommandUsage();
    interp.setResult(result);
    return TCL_ERROR;
  }

  // Extract the type of disconnection and branch accordingly:

  string which = objv[2];
  if (which == string("producer")) {
    
    // Disconnecting a producer.  We just need the ring name:

    if (objv.size() != 4) {
      string result;
      result += "Incorrect number of command parameters for ringbuffer disconnect producer\n";
      result += CommandUsage();
      interp.setResult(result);
      return TCL_ERROR;
    }

    string name = objv[3];

    try {
      CRingBuffer ring(name, CRingBuffer::manager);
      ring.forceProducerRelease();
    }
    catch (CException& reason) {
      string result;
      result += "Failed to disconnect producer:\n";
      result += string(reason.ReasonText());
      interp.setResult(result);
      return TCL_ERROR;
    }
    catch (string msg) {
      string result;
      result += "Failed to disconnect producer:\n";
      result += msg;
      result += '\n';
      interp.setResult(result);
      return TCL_ERROR;
    }
    catch (const char* pmsg) {
      string result;
      result += "Failed to disconnect producer\n";
      result += pmsg;
      result += '\n';
      interp.setResult(result);
      return TCL_ERROR;
    }
    catch (...) {
      string result;
      result += "Failed to disconnect producer\n";
      interp.setResult(result);
      return TCL_ERROR;
    }

  }
  else if (which == string("consumer")) {
    // Need the name of the ring buffer and a consumer index.

    if (objv.size() != 5) {
      string result;
      result += "Incorrect number of command parameters for ringbuffer disconnect consumer\n";
      result += CommandUsage();
      interp.setResult(result);
      return TCL_ERROR;
    }
    string name = objv[3];
    int    index;
    try {
      index = objv[4];
    }
    catch (...) {
      string result;
      result += "Consumer index must be an integer not: ";
      result += (string)(objv[4]);
      result += " for ringbuffer disconnect consumer\n";
      result += CommandUsage();
      interp.setResult(result);
      return TCL_ERROR;
    }

    try {
      CRingBuffer ring(name, CRingBuffer::manager);
      ring.forceConsumerRelease(index);
    }
    catch (CException& reason) {
      string result;
      result += "Failed to disconnect consumer:\n";
      result += string(reason.ReasonText());
      interp.setResult(result);
      return TCL_ERROR;
    }
    catch (string msg) {
      string result;
      result += "Failed to disconnect consumer:\n";
      result += msg;
      result += '\n';
      interp.setResult(result);
      return TCL_ERROR;
    }
    catch (const char* pmsg) {
      string result;
      result += "Failed to disconnect consumer\n";
      result += pmsg;
      result += '\n';
      interp.setResult(result);
      return TCL_ERROR;
    }
    catch (...) {
      string result;
      result += "Failed to disconnect consumer\n";
      interp.setResult(result);
      return TCL_ERROR;
    }

  }
  else {
    string result;
    result += "ringbuffer disconnect can only disconnect consumers or producers\n";
    result += CommandUsage();
    interp.setResult(result);
    return TCL_ERROR;
  }

  return TCL_OK;

}
/***************************************************************************/
/*   Determine and return the usage of a ring buffer.                      */
/*   ringbuffer usage name                                                 */
/*                                                                         */
/*   The information is returned as a Tcl list. where the list is composed */
/*   of an initial set of scalar entries:                                  */
/*   Size of the buffer space,                                             */
/*   Amount of available put space                                         */
/*   Maximum number of consumer connections supported                      */
/*   Pid of the producer (-1 if there isn't one)                           */
/*   Largest amount of get data from the list of consumers                 */
/*   Smallest amount of get data from the list of consumers                */
/*                                                                         */
/*  The final entry in the list is a list of the consumers attached to     */
/*  the ring.  This is a list of two element sublists where each sublist   */
/*  provides the pid and unread data for one of the consumers              */
/*                                                                         */
/* If there's an error, the result is a descriptive error message instead  */
/* of all this nice stuff.                                                 */
/***************************************************************************/


int
CRingCommand::usage(CTCLInterpreter&    interp, 
		    vector<CTCLObject>& objv)
{
  // Need the name of the ring buffer:

  bindAll(interp, objv);
  std::string name = "<all rings>";
  try {
    requireAtMost(objv, 3, "Too many command line parameters for ringbuffer usage command");
    
    // If there's a ring name, objv[2] is it:
    
    
    if(objv.size() == 3) {
        name = std::string(objv[2]);
      
        // Now we attach to the ring buffer and get the usage from it.
        // Exceptions caused (e.g. by the ring buffer not existing) are reported as errors:
      
        CRingBuffer::Usage usageInfo;
       
          CRingBuffer ring(name, CRingBuffer::manager); // managers don't affect usage.
      
          usageInfo = ring.getUsage();
            // Build the result first the scalar elements:
          
            CTCLObject Result;
            CTCLObject consumerList;
            Result.Bind(interp);
            consumerList.Bind(interp);
          
            Result += (int)usageInfo.s_bufferSpace;
            Result += (int)usageInfo.s_putSpace;
            Result += (int)usageInfo.s_maxConsumers;
            Result += (int)usageInfo.s_producer;
            Result += (int)usageInfo.s_maxGetSpace;
            Result += (int)usageInfo.s_minGetSpace;
          
            for (int i = 0; i < usageInfo.s_consumers.size(); i++) {
              CTCLObject consumerEntry;
              consumerEntry.Bind(interp);
          
              consumerEntry += (int)usageInfo.s_consumers[i].first;
              consumerEntry += (int)usageInfo.s_consumers[i].second;
          
              consumerList += consumerEntry;
            }
            Result += consumerList;
          
            interp.setResult(Result);
    } else {
        // Want the data from all rings:
        
        CRingMaster master;
        std::string usage = master.requestUsage();
        interp.setResult(usage);
        
    }
  }
  catch (CException& reason) {
    string result;
    result += "Failed to get usage info:";
    result += name;
    result = " \n";
    result += string(reason.ReasonText());
    interp.setResult(result);
    return TCL_ERROR;
  }
  catch (string msg) {
    string result;
    result += "Failed to get usage info: ";
    result += name;
    result = " \n";
    result += msg;
    result += '\n';
    interp.setResult(result);
    return TCL_ERROR;
  }
  catch (const char* pmsg) {
    string result;
    result += "Failed to get usage info ";
    result += name;
    result = " \n";
    result += pmsg;
    result += '\n';
    interp.setResult(result);
    return TCL_ERROR;
  }
  catch (...) {
    string result;
    result += "Failed to to get usage info ";
    result += name;
    result = " \n";
 
    interp.setResult(result);
    return TCL_ERROR;
  }

  return TCL_OK;

}
/**
 * list
 *   Process the list command.  The list command simply produces a list of rings
 *   as the interpreter result.
 *
 * @param interp - Reference to the interpreter object running the command.
 * @param objv   -  Vector of Tcl_obj's that make up the command words.
 * @return int
 * @retval TCL_OK - Everthing worked.  The result is the list of rings.
 * @retval TCL_ERROR - Failure of some sort.  The result is an error message.
 */
int
CRingCommand::list(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireExactly(objv, 2, "ringbuffer list has no additional parameters");
        
        CRingMaster master;
        std::string stringResult = master.requestUsage();
        
        CTCLObject objResult;
        objResult.Bind(interp);
        objResult = stringResult;
        
        // Now we can analyze the usage list.  The first element of each
        // sublist is the ring name:
        
        CTCLObject interpResult;
        interpResult.Bind(interp);
        
        for (int i = 0; i < objResult.llength(); i++) {
            CTCLObject ringUsage = objResult.lindex(i);
            ringUsage.Bind(interp);
            
            CTCLObject ringName;
            ringName.Bind(interp);
            ringName = ringUsage.lindex(0);
            
            interpResult += ringName;
            
        }
        
        interp.setResult(interpResult);
    }
  catch (CException& reason) {
    string result;
    result += "Failed to get ring list: ";
    result += string(reason.ReasonText());
    interp.setResult(result);
    return TCL_ERROR;
  }
  catch (string msg) {
    string result;
    result += "Failed to get ring list: ";
    result += msg;
    result += '\n';
    interp.setResult(result);
    return TCL_ERROR;
  }
  catch (const char* pmsg) {
    string result;
    result += "Failed to get ring list: ";
    result += pmsg;
    result += '\n';
    interp.setResult(result);
    return TCL_ERROR;
  }
  catch (...) {
    std::string result;
    result += "Failed to get ring list: Unexpected exception type caught\n";
 
    interp.setResult(result);
    return TCL_ERROR;
  }

  return TCL_OK;
    
}
/**************************************************************************/
/*  Remove a ring buffer.  The usual stuff. we need a ring name           */
/*  ringbuffer remove name                                                */
/**************************************************************************/
int
CRingCommand::remove(CTCLInterpreter&    interp, 
		     vector<CTCLObject>& objv)
{
  if (objv.size() != 3) {
    string result;
    result += "Insufficent command parameters for remove\n";
    result += CommandUsage();
    interp.setResult(result);
    return TCL_ERROR;
  }
  string name = objv[2];

  try {
    CRingBuffer::remove(name);
  }
  catch(CErrnoException& except) {
    string result;
    if (except.ReasonCode() == ENOENT) { // We can get a good error message here:
      result += "Ring buffer ";
      result += name;
      result += " does not exist";
   
    } else if (except.ReasonCode() == EACCES) {
      result += "User lacks write permissions to remove ";
      result += "ring buffer " + name;
    }
    else {
      result = except.ReasonText();
    }
    interp.setResult(result);
    return TCL_ERROR;
  }
  catch (CException& except) {
    string result = except.ReasonText();
    interp.setResult(result);
    return TCL_ERROR;
  }
  catch (string msg) {
    string result;
    result += "Failed to delete ring ";
    result += name;
    result += msg;
    interp.setResult(result);
    return TCL_ERROR;
  }
  catch (const char* msg) {
    string result;
    result += "Failed to delete ring ";
    result += name;
    result += msg;
    interp.setResult(result);
    return TCL_ERROR;
  }
  catch (...) {
    string result;
    result += "Failed to delete ring ";
    result += name;
    result += " due to an unanticipated excpetion type";
    interp.setResult(result);
    return TCL_ERROR;
  }

  interp.setResult(name);
  return TCL_OK;

}

////////////////////////////////////////////////////////////////////////////
// Private utility functions.

/***************************************************************************/
/* Return a string that describes the command usage                        */
/***************************************************************************/

string
CRingCommand::CommandUsage()
{
  string usage;
  usage += "Usage:\n";
  usage += "  ringbuffer create name ?size ?maxconsumers??\n";
  usage += "  ringbuffer format name ?maxconsumers?\n";
  usage += "  ringbuffer disconnect producer name\n";
  usage += "  ringbuffer disconnect consumer name index\n";
  usage += "  ringbuffer usage ?name?\n";
  usage += "  ringbuffer list\n";
  usage += "  ringbuffer remove name\n";
  usage += "Where\n";
  usage += "  name         - Is the name of a ring buffer\n";
  usage += "  size         - Is the number of data bytes a ring buffer can have\n";
  usage += "  maxconsumers - Is the maximum number of conumser clients that can connect\n";
  usage += "  index        - Is the consumer index for a connected consumer\n";
  usage += "And anything bracketed with ?'s is an optional parameter.\n";


  return usage;
}
