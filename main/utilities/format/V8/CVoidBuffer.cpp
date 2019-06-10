/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/
#include "V8/CVoidBuffer.h"

namespace DAQ {
  namespace V8 {
    
    CVoidBuffer::CVoidBuffer()
      : m_header()
    {
    }

    void CVoidBuffer::toRawBuffer(CRawBuffer &rawBuf) const
    {
      // do nothing.
    }
    
  } // namespace V8
} // namespace DAQ
