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

#ifndef DATAFORMATV8_H
#define DATAFORMATV8_H

#include <cstdint>

namespace DAQ
{
  namespace V8
  {
    extern std::size_t gBufferSize;

    enum BufferVersion : std::uint16_t { StandardVsn=5, JumboVsn=6 };

    enum BufferTypes {
      VOID         = 0,
      DATABF       = 1,
      SCALERBF     = 2,
      SNAPSCBF     = 3,
      STATEVARBF   = 4,
      RUNVARBF     = 5,
      PKTDOCBF     = 6,
      BEGRUNBF     = 11,
      ENDRUNBF     = 12,
      PAUSEBF      = 13,
      RESUMEBF     = 14,
      PARAMDESCRIP = 30,
      GENERIC      = 40
    };

    enum ByteOrderMark {
      BOM16 = 0x0102,
      BOM32 = 0x01020304
    };

    enum PhysicsEventSizePolicy {
      Inclusive16BitWords,
      Inclusive32BitWords,
      Inclusive32BitBytes,
      Exclusive16BitWords
    };

  } // end of V8
} // end of DAQ

#endif // DATAFORMATV8_H
