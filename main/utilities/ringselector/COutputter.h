/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file COutputter.h
 * @brief Defines the output thread class for the ringselector app
 * @author Ron Fox <fox@nscl.msu.edu>
 */


#ifndef COUTPUTTER_H
#define COUTPUTTER_H

#include <Thread.h>

// Forward definitions:

class Queues;


/**
 * @class COutputter
 *
 *    This class is the output thread for the ringselector.  It fishes data from the
 *    inter thread ring item queues and writes it to stdout.
 *    
 */
class COutputter : public Thread
{
private:
  Queues&       m_RingQueues;
  bool         m_exitOnEnd;

public:
  COutputter(Queues& ringQueues, bool oneshot);
  ~COutputter();

  // Thread's can't be copied or compared.
private:
  COutputter(const COutputter& rhs);
  COutputter& operator=(const COutputter& rhs);
  int operator==(const COutputter& rhs) const;
  int operator!=(const COutputter& rhs) const;

  // thread interface:

public:

  virtual void run();		// Thread entry point.
};

#endif
