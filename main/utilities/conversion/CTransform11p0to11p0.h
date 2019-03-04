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

#ifndef CTransform11p0to11p0_H
#define CTransform11p0to11p0_H

#include <V11/CRingItem.h>
#include <memory>

class CFilter;

namespace DAQ {
  namespace Transform {

    class CTransform11p0to11p0
    {
    public:
      using InitialType = V11::CRingItem;
      using FinalType =   V11::CRingItem;

    private:
      std::unique_ptr<CFilter> m_pFilter;

    public:
      CTransform11p0to11p0(std::unique_ptr<CFilter> pFilt);

      FinalType operator()(InitialType& item);

      FinalType dispatch(InitialType* item);
    };

  } // end Transform
} // end DAQ

#endif
