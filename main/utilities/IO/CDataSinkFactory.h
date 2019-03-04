/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/



#ifndef CDATASINKFACTORY_H
#define CDATASINKFACTORY_H

#include <string>

class CDataSink;


/*! Factory class for constructing CDataSink objects
*
* When provided a universal resource identifier (URI), this 
* will return the appropriate type of data sink.
*
* Supported sinks at the present are:
*   CFileDataSink   - specified by the file:// protocol
*                     (stdout can be specified as file:///stdout or - )
*
*   CRingDataSink   - specified by the tcp:// or ring:// protocol.
*
*/
class CDataSinkFactory
{
public:
    /**! 

      Parse the argument and return the proper type of sink
    */
    virtual CDataSink* makeSink(std::string uri);

private:
    /**!
       Create a file data sink for the specified file    
    */
    CDataSink* makeFileSink(std::string fname); 

    /**!
       Create a ring data sink with the specified name
    */
    CDataSink* makeRingSink(std::string ringname); 
};

#endif
