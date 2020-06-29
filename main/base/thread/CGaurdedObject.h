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

#ifndef CGAURDEDOBJECT_H
#define CGAURDEDOBJECT_H
using namespace std;		// Needed for spectrodaq includes.

#include <pthread.h>




/*!
  CGaurded object implements monitor like functionality.  This is intended
  to be used as a based class.  To implement a monitor, derive from this class
  and have the critical functions call Enter and Leave as needed to thread-safe
  their critical regions.
*/
class CGaurdedObject
{
private:
  pthread_mutex_t* m_mutex;

public:

  // Canonicals.

  CGaurdedObject();
  virtual ~CGaurdedObject();

  // Forbid copying:

private:
  CGaurdedObject(const CGaurdedObject&);
  CGaurdedObject& operator=(const CGaurdedObject&);
  
public:

  void Enter();			//!< Enter a critical region.
  void Leave();			//!< Leave a critical region.
  pthread_mutex_t& mutex();	//!< Return the underlying mutex object.
};


#endif
