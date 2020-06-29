/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

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
Aggregates the exceptions that can be thrown by 
readout objects contained wthin a ReadOrder.
In a read order object, each item read can do
any of the following:
- Return a count if succesful.
- Throw a string exception if a primitive.
- Throw a CReadException if another read order.
In the event exceptions are thrown, the read order
objects aggregate the exception strings into a list
of exception strings and keep track of the number of
words that were actually read.  
This aggregation is done within a CReadException which
is then thrown if any exceptions occured.


*/
//! \class: CReadException           
//! \file:  .h
// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

#ifndef CREADEXCEPTION_H  //Required for current class
#define CREADEXCEPTION_H

//
// Include files:
//

#include <Exception.h>
#include <string>        //Required for include files  


class CReadException : public CException     
{
private:
  
    std::string m_sAggregateException;  //!<  Aggregate of the exception strings.  
    int    m_nWordsRead;	   //!<  Number of words successfully read.  
   
    

    // Constructors etc. 
public:

  CReadException(const std::string& rInitialReason = std::string(""), 
		 int           nWordsRead = 0); //!< Parameterized construction
  virtual ~ CReadException ( ); //!< Destruction.

  CReadException (const CReadException& aCReadException ); //!< copy
  CReadException& operator= (const CReadException& aCReadException); //!< Assignment
  int operator== (const CReadException& aCReadException); //!< == comparison
  int operator!= (const CReadException& aCReadExceptoin); //!< != comparison.
  
  // Selectors:

public:
  int    GetCount () const   ;              //!< Selector for m_nWordsRead. 
  std::string GetString () const   ;             //!< Selectors for m_sAggregateException 


  // Class operations.
public:
  
  void AddItem (const std::string& rMessage)   ; //!< Append a std::string to the aggregate.
  void AddCount (int nWords)   ;            //!< Increment the count. 
  void Add      (const std::string& rMessage, 
		 int           nwords);	    //!< AddItem and AddCount. 

  // Overrides of the base class functionality:

public:
  virtual const char* ReasonText () const  ; //!< Really will return m_sAggregateException
  virtual       int ReasonCode () const  ; //!< Will return m_nWordsRead for now.
  
};

#endif
