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
#ifndef CEVBCLIENTAPP_H
#define CEVBCLIENTAPP_H
/**
 * This file defines the abstract base class that must be extended to
 * produce a specific event builder data source client based on the
 * event builder client application framework.
 */
class CEVBClientApp {
public:
  CEVBClientApp();
  virtual ~CEVBClientApp();

  // The interface:

public:
  virtual void initialize();
  virtual bool dataReady(int ms) = 0;
  virtual void getEvents() = 0;
  virtual void shutdown();
};


#endif
