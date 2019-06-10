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




#ifndef CTEMPLATEFILTER_CPP
#define CTEMPLATEFILTER_CPP

#include <CFilter.h>
#include <iostream>
#include <CPhysicsEventItem.h>
#include <stdint.h>

/**! \class CTemplateFilter
    Here is a sample implementation of a filter to append a reversed copy of the
    data in physics event to its body. This is for illustration purposes.

    See the documentation for the CFilter base class for the virtually declared 
    methods available for dealing with non-physics events. The user has access 
    to all of the different ring item types. In fact, it is not necessary for 
    the user to return the same type of ring item from method as it received. 
*/
class CTemplateFilter : public CFilter
{
  public:
    /**! Virtual copy constructor
        DO NOT FORGET THIS!
    */
    virtual CTemplateFilter* clone() const { return new CTemplateFilter(*this);}


    /**! A sample filter for handling physics events 

        This filter will be called for every physics event item. It will produce
        a ring item double the size of the original item with the first half being
        the original data and the second half being the data in reversed order. 
        This filter is unlikely to have any real use but is defined to be 
        illustrative of how to manipulate the data of a ring item. 

        For some precautionary measures, it also demonstrates how to compute whether
        the new data is going to overflow the available space in the filtered item.
        Reaching the storage capacity is an unlikely occurence because by default a
        ring item has 8192 bytes of storage space. 

        @param pItem a pointer to the raw physics event item to process
        @return the resulting ring item from this filter. This can be the same item
                pointed to by pItem or a newly allocated one. Can be any derived 
                type of ring item.
    */
    virtual CRingItem* handlePhysicsEventItem(CPhysicsEventItem* pItem) 
    {
      // Create a copy of the original item to manipulate. This is unnecessary
      // but allows one to safely abort filtering and return the original ring
      // item.
      CPhysicsEventItem* pFiltItem = new CPhysicsEventItem(*pItem);

      // Get the amt of data currently in the body and the available storage 
      size_t bodySize = pFiltItem->getBodySize();
      size_t storageSize = pFiltItem->getStorageSize();

      uint16_t *pBodyBegin=0, *pBodyLast=0, *pCursor=0; 
      uint16_t *pStorageBegin=0, *pStorageEnd=0;

      // Get a pointer to the first piece of data in the body of the ring item
      // and compute the location of the last item in the body
      pBodyBegin = reinterpret_cast<uint16_t*>(pFiltItem->getBodyPointer());
      pBodyLast  = pBodyBegin + bodySize/sizeof(uint16_t) - 1; 

      // Get a pointer to the location where we are expected to put data next
      pCursor = reinterpret_cast<uint16_t*>(pFiltItem->getBodyCursor());

      // Compute the storage space limit is for the current filtered item
      pStorageBegin = reinterpret_cast<uint16_t*>(pFiltItem->getItemPointer());
      pStorageEnd   = pStorageBegin + storageSize/sizeof(uint16_t);

      // Copy the body data to the end in reverse order. 
      uint16_t* pIter = pBodyLast;
      while ( pIter>=pBodyBegin && pIter<pStorageEnd) {

          // Copy the data 
          *pCursor = *pIter;

          // Update our pointers 
          ++pCursor;
          --pIter;
      }

      if (pIter==pStorageEnd) {
        std::cout << "Ran out of space!" << std::endl;
      }

      // Tell the item where the next data item is 
      pFiltItem->setBodyCursor(pCursor);
      // Compute the new size of the ring item
      pFiltItem->updateSize();
      
      return pFiltItem;
    }


};

#endif
