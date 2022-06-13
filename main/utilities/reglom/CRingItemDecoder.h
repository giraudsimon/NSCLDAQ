/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CRingItemDecoder.h
 *  @brief: Abstract base class for event processing code.
 */
#ifndef    CRINGITEMDECODER_H
#define    CRINGITEMDECODER_H
class CRingItem;

class CRingItemDecoder {

public:
  virtual ~CRingItemDecoder() {}
    virtual void operator()(CRingItem* pItem) = 0;
    virtual void onEndFile() =  0;

};



#endif
