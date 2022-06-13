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

#ifndef CTRANSFORMFACTORY_H
#define CTRANSFORMFACTORY_H

#include <CBaseMediator.h>
#include <CTransformMediator.h>
#include <CExtensibleFactory.h>
#include <map>
#include <memory>
#include <utility>
#include <string>
#include <stdint.h>

/*
 * daqdev/NSCLDAQ#510 - rewritten to use CExtensible factory NOTE!!!!
 * the original implementation used as a lookup key std::pair<int,int>
 * where the pair was the from/to  pair.  We  transform that into a uint64_t
 * key composed by (from << 32) | to.
 * Really this whole implementation suffers from **2 scalability problems.
 * The 'correct' way to do this is a two step transform. First step transforms
 * to highest level of NSCLDAQ and second level transforms to the desired level.
 * That reduces the number of mediators from O(n**2) to O(n*2)
 * That's for a later day when we look at what's needed (if anything) to deal with
 * with the change in body header contents in 12.x
 *   Note on std::pointers -it's up to the caller to decide to or not to wrap
 * returned pointersin std::pointers because otherwise there are transfer issues.
 */


namespace DAQ {
  namespace Transform {

    


    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////


    /*! \brief Generic creator for CTransformMediator template classes
      *
      * Provided a transformer type in the template parameter, this will create
      * the appropriate type of CTransformMediator<> and return a pointer to a
      * new instance created with the default constructor
      */

    class CTransformCreator : public CCreator<CBaseMediator>
    {
    public:
      CBaseMediator* operator()(void* unused) = 0;
      std::string describe() const { return ""; }  // We're not using descriptions
    };
    
    template<class T>
    class CGenericCreator : public CTransformCreator {
      CBaseMediator* operator()(void* unused) {
          return new CTransformMediator<T>;
      }
    };
    
    //  This typedef is the factory we us
    

    using TransformFactory = CGenericExtensibleFactory<uint64_t, CBaseMediator>;

    // We wrap it to supply an adaptor to existing code.
  
    class  CTransformFactory : public TransformFactory {
    public:
      void setCreator(int vsnFrom, int vsnTo, CTransformCreator* pCreator);
      CBaseMediator* create(int vsnFrom, int vsnTo);
    };
  
    
  } // end of Transform
} // end of DAQ

#endif // CTRANSFORMFACTORY_H
