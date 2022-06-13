#ifndef COBJECT_H
#define COBJECT_H

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


/*!
  This is a pure abstract base class.  It defines the base of a tree
  of objects that may or may not be containers of other objects.
*/
class CObject
{
public:
  virtual bool isComposite() const = 0;
};
#endif
