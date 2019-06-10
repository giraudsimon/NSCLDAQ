/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/




#ifndef CTRANSPARENTFILTER_H
#define CTRANSPARENTFILTER_H

#include <CFilter.h>

/**! \class CTransparentFilter
  This class has handlers that do nothing more
  than return the pointer passed as an argument. It uses all of the base class 
  handler implementations and only provides the clone method. For the exact details
  of the handler methods, see CFilter.h
*/
class CTransparentFilter : public CFilter
{
  public:
    // Virtual constructor
    virtual CTransparentFilter* clone() const { return new CTransparentFilter(*this);} 
  
};

#endif
