/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

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
  \class CMmapError
  \file MmapError.h

  This file defines the CMmapError class.

  Author:
     Jason Venema
     NSCL
     Michigan State University
     East Lansing, MI 48824-1321
     mailto: venemaja@msu.edu
*/

#ifndef CMMAPERROR_H
#define CMMAPERROR_H

#include <Exception.h>
#include <string>

class CMmapError : public CException
{
  std::string m_ReasonText;     // Reason text

 public:
  // Default constructor
  CMmapError(const char* pDoing) :
    CException(pDoing)
    { UpdateReason(); }

  CMmapError(const std::string& rDoing) :
    CException(rDoing)
    { UpdateReason(); }
  
  // Copy constructor
  CMmapError(const CMmapError& aCMmapError) :
    CException(aCMmapError)
    { UpdateReason(); }
  
  // Destructor
  virtual ~CMmapError() { }
  
  // Operator= Assignment operator
  CMmapError operator= (const CMmapError& aCMmapError)
    {
      if(this != &aCMmapError) {
	CException::operator= (aCMmapError);
	UpdateReason();
      }
      return *this;
    }

  // Operator== Equality operator
  int operator== (const CMmapError& aCMmapError)
    {
      return (CException::operator== (aCMmapError));
    }

  // Interfaces implemented from the CException class.
  //
 public:
  virtual const char* ReasonText() const;
  //virtual Int_t ReasonCode() const

 protected:
  void UpdateReason();
};

#endif
