/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

#ifndef CFAKEDATASOURCE_H
#define CFAKEDATASOURCE_H

#include <CDataSource.h>

class CRingItem;

/*! \brief A fake source that continually produces begin run items
 *
 */
class CFakeDataSource : public CDataSource
{
  /**! \brief Generates an empty ring item of type 1 */
  CRingItem* getItem();
};

#endif
