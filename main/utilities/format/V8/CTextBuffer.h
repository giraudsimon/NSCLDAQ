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

#ifndef DAQ_V8_CTEXTBUFFER_H
#define DAQ_V8_CTEXTBUFFER_H


#include <V8/CV8Buffer.h>
#include <V8/bheader.h>
#include <V8/DataFormatV8.h>

#include <vector>
#include <string>

namespace DAQ {
  namespace V8 {
    
    // fwd declarations
    class CRawBuffer;

    /*!
     * \brief A V8 Buffer of Textual Data
     *
     * This class represents a buffer containing textual data, and is intended to
     * interact with the data of the buffer in an intuitive and simple way. It also provides
     * the logic to serialize and deserialize it for IO operations. When serialized, it resembles a physics
     * event buffer because the body layout is
     * not fully determined. It is a size entity followed by a series of null terminated
     * strings. Each null terminated string begins on an even byte boundary, which is enforced
     * by adding appropriate padding in between each string.
     *
     */
    class CTextBuffer : public CV8Buffer
    {
      bheader                  m_header;
      std::vector<std::string> m_strings;

    public:

      /*!
       * \brief CTextBuffer
       *
       * This creates an empty buffer.
       *
       * \param type  - type to set in bheader
       */
      CTextBuffer(BufferTypes type = RUNVARBF);

      /*!
       * \brief Construct fully specified buffer
       *
       * \param header
       * \param strings - the data
       */
      CTextBuffer(const bheader& header, const std::vector<std::string>& strings);

      /*!
       * \brief Deserialize data from a raw buffer
       *
       * This parses the data in a byte array filled by some IO operation
       * into a usable form that is more usable.
       *
       * \param rawBuf - a byte array
       */
      CTextBuffer(const CRawBuffer& rawBuf);

      /*!
       * \brief Copy construct
       *
       *  This performs a deep copy of the strings.
       *
       * \param rhs
       */
      CTextBuffer(const CTextBuffer& rhs);

      ~CTextBuffer() {}

      /*! \returns a copy of the header */
      bheader getHeader() const { return m_header; }

      /*!
       * \brief Access the header type
       * \returns the type
       */
      BufferTypes type() const { return BufferTypes(m_header.type); }

      /*!
       * \brief Serialize data into a byte array
       *
       *  This appends data to the raw buffer passed into that represents the
       *  format of this object for IO operations. The format is:
       *
       *  header
       *  16-bit inclusive body size (number of shorts)
       *  string_0
       *  string_1
       *  ...
       *
       * \param buffer  the byte array to fill
       */
      void toRawBuffer(CRawBuffer &buffer) const;

      /*!
       * \brief Full access to strings
       * \return reference to strings
       */
      std::vector<std::string>& getStrings() {return m_strings; }

      /*!
       * \brief Read only access to strings
       * \return a copy of the strings
       */
      std::vector<std::string> getStrings() const { return m_strings; }

      /*!
       * \brief Compute size of body in bytes
       * \return number of bytes
       */
      std::uint32_t totalBytes() const;

      /*!
       * \brief Compute size of body in 16-bit units
       * \return number of shorts
       */
      std::uint16_t totalShorts() const;

      /*!
       * \brief Add a string if it fits in the buffer
       *
       * The logic of this is similar to CPhysicsEventBuffer::appendEvent().
       * The size of the buffer after the string is appended is computed
       * and is then compared against the gBufferSize. If it is less than or
       * equal, then the string is appended. Otherwise, the string doesn't
       * fit and is not appended.
       *
       * \param str   the string to add
       * \return boolean
       * \retval true   string was successfully appended
       * \retval false  string did not fit
       */
      bool appendString(const std::string& str);

      /*! \brief Utility function to determine how much space is available
       *
       * Computes the number of bytes that can be appended before gBufferSize
       * is met.
       *
       *  \returns number of bytes
       */
      std::size_t getNBytesFree() const;

    private:
      ///////////////// UTILITIES

      void validateTypeToConstructFrom();
      void validateDeadEndMeaningful(Buffer::ByteBuffer::const_iterator deadEnd,
                                     Buffer::ByteBuffer::const_iterator bufferEnd);
      std::uint16_t extractTotalShorts(const Buffer::ByteBuffer& buffer, bool needsSwap) const;
      void parseStringsFromBuffer(Buffer::ByteBuffer::const_iterator beg,
                                  Buffer::ByteBuffer::const_iterator end);
      Buffer::ByteBuffer::const_iterator skipNullCharPadding(Buffer::ByteBuffer::const_iterator beg,
                                                             Buffer::ByteBuffer::const_iterator end);
    };
    
  } // namespace V8
} // namespace DAQ

#endif // DAQ_V8_CTEXTBUFFER_H
