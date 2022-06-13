/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

#include "CTransformFactory.h"
#include <stdexcept>

using namespace std;

namespace DAQ {
  namespace Transform {

    
    void
    CTransformFactory::setCreator(int vsnFrom, int vsnTo, CTransformCreator* pCreator)
    {
      uint64_t key = (static_cast<uint64_t>(vsnFrom) << 32) | vsnTo;
      addCreator(key, pCreator);
    }


    //
    CBaseMediator*
    CTransformFactory::create(int vsnFrom, int vsnTo)
    {
      uint64_t key = (static_cast<uint64_t>(vsnFrom) << 32) | vsnTo;
      auto result = TransformFactory::create(key);
      if (result == nullptr) {
        throw std::out_of_range("No such mediator creating from factory");
      }
      return result; 
      

    }

  } // end of Transform
} // end of DAQ
