// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

/*!
  \class CFileSinkCreator
  \file  .h

  Creates file sinks  The connection string
  represents a filesystem path.  Since filesystem paths
  can be pretty much anything if relative paths are
  allowed, all name strings are considered legal, although
  you may regeret using some of them.

*/


#ifndef CFILESINKCREATOR_H  //Required for current class
#define CFILESINKCREATOR_H

//
// Include files:
//

                               //Required for base classes
#include "CSinkCreator.h"
 
class CFileSinkCreator  : public CSinkCreator        
{
public:
    CFileSinkCreator ();		//!< Constructor.
    virtual  ~ CFileSinkCreator ( ); //!< Destructor.

public:

    virtual   bool isNameLegal (std::string Name)   ; 
    virtual   CSink* Create (std::string sCommand, std::string sName)   ; 
    std::string describe() const;

};

#endif
