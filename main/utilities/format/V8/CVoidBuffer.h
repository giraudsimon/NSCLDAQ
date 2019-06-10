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
#ifndef DAQ_V8_CVOIDBUFFER_H
#define DAQ_V8_CVOIDBUFFER_H

#include <V8/DataFormatV8.h>
#include <V8/CV8Buffer.h>

namespace DAQ {
  namespace V8 {
    
    /*! \brief A NO-OP IO class
     *  
     *  VOID buffers are intended to be NULL objects. They do not produce
     *  any data for output.
     *
     *  VOID buffers have :
     *  type = 0 
     *  nwds = 16
     *  nevt = 0
     *  ssignature = 0x0102
     *  lsignature = 0x01020304
     *  buffmt = 5
     *  Zero for everything else.
     *
     */
    class CVoidBuffer : public CV8Buffer
    {
      bheader m_header;

    public:
      CVoidBuffer();

      bheader getHeader() const { return m_header; }

      BufferTypes type() const { return VOID; }

      /*! Does nothing. */
      void toRawBuffer(CRawBuffer& rawBuf) const;

    };
    
  } // namespace V8
} // namespace DAQ

#endif // DAQ_V8_CVOIDBUFFER_H
