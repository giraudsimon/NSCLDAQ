
////////////////////////// FILE_NAME.cpp ///////////////////////////

#include <config.h>
#include "CFileSinkCreator.h"    				
#include "CFileSink.h"


using namespace std;


// Static attribute storage and initialization for CFileSinkCreator

/*!
    Create an object of type CFileSinkCreator
*/
CFileSinkCreator::CFileSinkCreator () 
{ 
} 

/*!
    Called to destroy an instance of CFileSinkCreator
*/
 CFileSinkCreator::~CFileSinkCreator ( )
{
}

// Functions for class CFileSinkCreator

/*! 

Description:

Returns an indication of the legality of a
filename path.  Note that essentially 
all filename paths are legal (although maybe
not usable).

Parameters:

\param Name (string [in]):
   The path name to check

\return bool
\retval true - indicating that all path strings are legal.

*/
bool 
CFileSinkCreator::isNameLegal(std::string Name)  
{
  return true;
}  

/*! 

Description:

Attempts to create a file sink that is
open on the name passed in.  In order
to ensure that we catch illegalities,
we open the file ourselves, and construct
the filesink via its file descriptor.  The file
is opened for write append.




Parameters:

\param Name (string [in])
   Name of the file to open.

\return CSink*
\return CSink*
\retval NULL - The file, or sink could
    not be created for whatever reason.
   Probably the error reason will be in errno
\retval !NULL - The sink created (in reality
   a CFileSink).

*/
CSink* 
CFileSinkCreator::Create(string sCommand, string sName)  
{
  CSink* pSink;
  try {
    pSink = new CFileSink(sCommand, sName);
    return pSink;
  }
  catch(...) {
    return (CSink*)NULL;
  }
}
/**
 * describe
 * @return std::string - description of the thing we're creating.
 */
std::string
CFileSinkCreator::describe() const
{
 return "File sink";
}