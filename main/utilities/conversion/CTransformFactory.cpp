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

using namespace std;

namespace DAQ {
  namespace Transform {

    //
    CTransformFactory::CTransformFactory()
      : m_creators()
    {
    }

    //
    void
    CTransformFactory::setCreator(int vsnFrom, int vsnTo, std::unique_ptr<CTransformCreator> pCreator)
    {
      pair<int,int> transformId(vsnFrom, vsnTo);

      m_creators[transformId] = move(pCreator);
    }


    //
    unique_ptr<CBaseMediator>
    CTransformFactory::create(int vsnFrom, int vsnTo)
    {
      const pair<int, int> transformId(vsnFrom, vsnTo);

      auto& pCreator = m_creators.at(transformId);

      return (*pCreator)();

    }

  } // end of Transform
} // end of DAQ
