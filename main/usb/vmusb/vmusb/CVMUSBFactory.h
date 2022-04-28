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

#ifndef __CVMUSBFACTORY_H
#define __CVMUSBFACTORY_H

/**
 *  @file CVMUSBFactory.h
 *  @brief Define factory class for CVMUSB class hierarchy elements.
 */

/**
 * Forward definitions.
 */
class CVMUSB;			

/**
 * @class CVMUSBFactory
 *
 *  This class creates an appropriate CVMUSB object given directions to its factory method.
 */
class CVMUSBFactory
{
public:
  typedef enum _ControllerType {
    local, remote
  } ControllerType;

public:
  // Creational methods:

  static CVMUSB* createUSBController(ControllerType type, const char* specifier = 0);
  static CVMUSB* createLocalController(const char* serialNumber = 0);
  static CVMUSB* createRemoteController(const char* host = 0);
};

#endif
