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

#ifndef FRAGIO_H
#define FRAGIO_H

/**
 * @brief fragment I/O library.
 * @author Ron Fox
 */



namespace EVB {
  typedef struct _Fragment Fragment;

}

/**
 * This class really provides a namespace on which to
 * hang static members that do fragment I/O.
 */

class CFragIO {
public:
  static EVB::Fragment* readFragment(int fd);
  static void writeFragment(int fd, EVB::Fragment* pFrag);
};
#endif
