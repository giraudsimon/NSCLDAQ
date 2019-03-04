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


static const char* Copyright = "(C) Copyright Michigan State University 2014, All rights reserved";

#include "CDataSinkFactory.h"
#include "CFileDataSink.h"
#include "CRingDataSink.h"
#include <URL.h>
#include <string>
#include <iostream>
#include <errno.h>


/**! Factory method
* Parses the string as a URL and then call the appropriate 
* private utility method based on the protocol provided.
* Supported protocols are tcp:// and file://. The stdout can
* be obtained by providing file:///stdout or -
*
* \param uri a string of the form protocol://host/path:port
* \return a data sink on success, 0 on failure
*/
CDataSink* CDataSinkFactory::makeSink(std::string uri)
{
  CDataSink* sink = 0;

  // Treat the special case of -
  if (uri=="-") {
    sink = makeFileSink(uri);
  } else {

    // parse the uri
    URL url(uri);
    std::string host = url.getHostName();
    if ((host != "localhost") && (host != "")) {
      errno = EREMOTE;
      throw CErrnoException("CDataSinkFactory::makeSink");
    }

    // 
    if (url.getProto()=="file") {

      sink = makeFileSink(url.getPath());

    } else if (url.getProto()=="ring" || url.getProto()=="tcp") {

      sink = makeRingSink(url.getPath());

    } 

  }
  return sink;

}

/**! Handle the construction of a file data sink from a path
*
* On successful construction of a CFileDataSource, a pointer to
* the dynamically allocated object will be passed to the caller.
* The caller will own the object at this point.
*
* This may throw as a result of the constructor objects. 
* 
* \return pointer to a sink on success, 0 on failure 
*/
CDataSink* CDataSinkFactory::makeFileSink(std::string fname) 
{

  CDataSink* sink=0;

  try {

    if (fname=="-") {

      sink = new CFileDataSink(STDOUT_FILENO);

    } else {

      sink = new CFileDataSink(fname);

    }
  } catch (CErrnoException& err) {
    // If either of the above constructor throw,
    // then sink may not equal 0 but the memory will
    // have been freed. Ensure that this send out a
    // result that indicates failure
    sink = 0;
    throw CErrnoException(err);
  }

  return sink;
}

/**! Handle the construction of a ring data sink from a path
*
* On successful construction of a CRingDataSink, a pointer to
* the dynamically allocated object will be passed to the caller.
* The caller will own the object at this point.
*
* This may throw as a result of the constructor. 
* 
* \return pointer to a sink on success, 0 on failure 
*/
CDataSink* CDataSinkFactory::makeRingSink(std::string fname) 
{

  CDataSink* sink=0;

  try {

      sink = new CRingDataSink(fname);

  } catch (CErrnoException& err) {
    // If either of the above constructor throw,
    // then sink may not equal 0 but the memory will
    // have been freed. Ensure that this send out a
    // result that indicates failure
    sink = 0;
    throw CErrnoException(err);
  }

  return sink;
}
