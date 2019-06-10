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



/*! \class CReadException   
           CLASS_PACKAGE
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

////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CReadException.h"    				
#include <TCLString.h>

using namespace std;

    
 
/*!
   Parameterized constructor.   This constructs a CReadException with
   optoinal non default initial values for the aggregate exception string and, optionally
   the number of words read.
   \param rInitialReason (const string& default ="")
      An initial value for the aggregate exception string.
   \param nWordsRead   (int default=0):
      An initial value for m_nWordsRead.

*/
CReadException::CReadException(const string& rInitialReason,
			       int           nWords) :
  CException("Reading an Event"),
  m_sAggregateException(rInitialReason),
  m_nWordsRead(nWords)
{
  
}
//! Destroy a read exception.  No action required.

CReadException::~CReadException ( )
{
}

/*!
   Construct a temp copy of a reference object.
   \param aCReadException  (const CReadException&)
      The reference object that we are copy constructing.

*/
CReadException::CReadException (const CReadException& aCReadException ) :
  CException(aCReadException),
  m_sAggregateException(aCReadException.m_sAggregateException),
  m_nWordsRead(aCReadException.m_nWordsRead)
{

  
} 
/*!
   Assigns *this to contain a copy of aCReadException.
   \param aCReadException (const CReadException&)
      The rhs of the assignment operator.
   \return CReadException& 
   *this  in order to support operator chaining.
*/
CReadException& CReadException::operator= (const CReadException& aCReadException)
{ 
  if(this != &aCReadException) { 
    CException::operator=(aCReadException);
    m_sAggregateException = aCReadException.m_sAggregateException;
    m_nWordsRead          = aCReadException.m_nWordsRead;
  }
  return *this;
}

/*!
  \return int
   Return nonzero if *this is the same as aCReadException.
   \param aCReadException (CreadException&)
      The object we compare *this to.
*/

int 
CReadException::operator== (const CReadException& aCReadException)
{
  return (CException::operator==(aCReadException)                          &&
	  (m_sAggregateException == aCReadException.m_sAggregateException) &&
	  (m_nWordsRead          == aCReadException.m_nWordsRead));
}
/*!
    \return int
    returns the inverse of operator==.
    \param aCReadException (CreadException&)
      The object we compare *this to.
*/
int
CReadException::operator!=(const CReadException& aCReadException) 
{
  return !operator==(aCReadException);
}

/*!  

\return m_nWordsRead


*/
int 
CReadException::GetCount() const  
{ 
  return m_nWordsRead;
}  

/*!  

  \return m_sAggregateException


*/
string 
CReadException::GetString() const  
{ 
  return m_sAggregateException;
}


/*! 

Adds an exception string to the current list in m_sAggregateException.
Exception strings are added in such a way that this member remains
a valid TCL list.
\param rMessage (const string&)
    The string to append as a list element to the aggregate message.
    

*/
void 
CReadException::AddItem(const string& rMessage)  
{
  CTCLString aggregate(m_sAggregateException);
  aggregate.AppendElement(rMessage);

  m_sAggregateException = (const char*)aggregate;
}  

/*!  

Adds the paramter to the m_nWordsRead field.
\param nWords (int)
   Number of words read to add to m_nWordsRead.
*/
void 
CReadException::AddCount(int nWords)  
{
  m_nWordsRead += nWords;
}  
/*!
   Adds an item and a count.
   \param rMessage (const string&)
      appended as a list element to the aggregate message
   \param nWords (int)
      added to m_nWordsRead.
*/
void
CReadException::Add(const string& rMessage,
		    int           nWords)
{
  AddItem(rMessage);
  AddCount(nWords);
}
/*!
  \return string
     Returns the aggregate reason message.
*/
const char* 
CReadException::ReasonText() const
{
  return m_sAggregateException.c_str();
}
/*!
  \return int
      Return the reason code, which in this case is the total number
      of words read.
*/
int
CReadException::ReasonCode() const
{
  return m_nWordsRead;
}

