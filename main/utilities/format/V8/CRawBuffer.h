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
#ifndef NSCLDAQ8_CRAWBUFFER_H
#define NSCLDAQ8_CRAWBUFFER_H

#include <V8/CV8Buffer.h>
#include <V8/DataFormatV8.h>
#include <ByteBuffer.h>

namespace DAQ
{
  namespace V8
  {
    /*! \brief Buffer for IO purposes
     *
     * The CRawBuffer is a class that is intended to provide a minimum set of common
     * functionality to a body of bulk, raw data. In this case, it stores the raw
     * data (unparsed), a parsed header, and also a flag determining whether the 
     * data in the unparsed buffer should be swapped. It's intent is to provide
     * the basic IO functionality required by buffers. There are overloads of the 
     * insertion(<<) and extraction (>>) operators that work with this form of 
     * the data.
     *
     * Every specialized buffer type (i.e. derived classes of CV8Buffer) can be
     * constructed from a CRawBuffer. They can also serialize their data into a 
     * CRawBuffer. 
     */
    class CRawBuffer : public CV8Buffer
    {
    private:
      bheader              m_parsedHeader;
      Buffer::ByteBuffer   m_unparsedBuffer;
      bool                 m_bytesNeededSwap;

    public:
      /*! Construct 
       *
       * This automatically reserves space according to the size parameter. In this
       * way, we reduce the number of times that new memory will have to be allocated.
       *
       * \param size  number of bytes in buffer for IO operations
       */
      CRawBuffer(std::size_t size = gBufferSize);

      /*! \brief Access the parsed header */
      bheader  getHeader() const;

      /*! \brief Retrieve the type */
      BufferTypes type() const { return GENERIC; }

      /*! \brief Serialize to another a raw buffer */
      void toRawBuffer(CRawBuffer &buffer) const;

      /*!  \brief Access the raw buffer in read-only mode */
      const Buffer::ByteBuffer& getBuffer() const;

      /*! \brief Set the raw buffer to a byte array
       *
       * This method is what really sets this object up. It copies the state
       * of the buffer passed in to its unparsed buffer, it then parses out the 
       * header, and then resizes itself to be gBufferSize. In that latter process, the
       * padding data is zero initialized.
       *
       * \param buffer    the data to copy
       */
      void setBuffer(const Buffer::ByteBuffer& buffer);

      /*! \brief Is the unparsed buffer in native byte order?
       *
       */
      bool bufferNeedsSwap() const { return m_bytesNeededSwap; }

    private:

      /*! \brief Parse the first 16-bit data words of the buffer into a bheader object
       *
       * \param buffer  the data to parse
       * \param swap    whether the byte order needs to be swapped
       */
      void parseHeader(const Buffer::ByteBuffer& buffer, bool swap);
    };

  } // end of V8
} // end of DAQ

#endif // CRAWBUFFER_H
