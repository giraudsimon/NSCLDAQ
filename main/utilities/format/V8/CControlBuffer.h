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

#ifndef DAQ_V8_CCONTROLBUFFER_H
#define DAQ_V8_CCONTROLBUFFER_H

#include <V8/CV8Buffer.h>
#include <V8/bheader.h>

#include <string>
#include <cstdint>

namespace DAQ {
  namespace V8 {
    
    // Forward declarations
    class CRawBuffer;

    /*!
     * \brief The CControlBuffer class
     *
     * A version 8 control buffer representation. This represents the
     * data for BEGRUNBF, ENDRUNBF, PAUSEBF, and RESUMEBF. It keeps a
     * title, time of creation relative to the epoch and to the start of the run,
     * and the run number.
     */
    class CControlBuffer : public CV8Buffer
    {
    private:
      bheader       m_header;
      std::string   m_title;
      std::uint32_t m_offset;
      bftime        m_time;

    public:
      /*!
       * \brief Default constructor
       *
       * Initializes an object that is not really intended to be used. It has a
       * default constructed bheader (i.e. type=VOID), an empty string for the title,
       * a 0 offset and a null time structure.
       *
       * You should assign a proper value into this.
       */
      CControlBuffer();

      /*!
       * \brief Construct from data
       *
       * \param header - a buffer header that is already initialized
       * \param title - the title string
       * \param offset - number of seconds since run began
       * \param time - creation time
       */
      CControlBuffer(const bheader& header, const std::string& title,
                     std::uint32_t offset, const bftime& time);

      /*!
       * \brief Construct from a raw data buffer
       *
       * Usually when dealing with data, we have read it in from a byte
       * stream. The standard mechanism for doing that is with a V8::CRawBuffer.
       * This is used by V8::format_cast() to transform a V8::CRawBuffer into
       * something more user friendly like an instance of this class.
       *
       * This also swaps the data to native byte ordering if it needs to be done.
       *
       * \param rawBuf - a raw data buffer
       */
      CControlBuffer(const CRawBuffer& rawBuf);

      /*!
       * \brief Access the V8::bheader
       * \return copy of the header owned by this
       */
      bheader getHeader() const { return m_header; }

      /*!
       * \brief Retrieve the underlying type
       *
       * \return type value stored in bheader.
       */
      BufferTypes type() const { return BufferTypes(m_header.type); }

      /*!
       * \brief Serialize data into a generic buffer of bytes
       *
       * \param buffer - the buffer to copy into
       *
       * \throws std::runtime_error if size of buffer will not fit into the raw buffer
       */
      void toRawBuffer(CRawBuffer &buffer) const;

      /*!
       * \brief Replaces the current title
       *
       * The title that is passed in will be resized to have a length of 80.
       * The padding character to enable expansion is the space character ' '.
       *
       * In other words, "my title" becomes
       * "my title                                                                        "
       *
       * \param title - the new title
       */
      void setTitle(const std::string& title);

      /*!
       * \brief Retrieve the current title
       * \return copy of the title
       */
      std::string getTitle() const;

      // More getters and setters

      bftime getTimeStruct() const { return m_time; }     
      void setTimeStruct(const bftime& time) { m_time = time; }

      std::uint32_t getOffset() const { return m_offset; }
      void setOffset(std::uint32_t offset) { m_offset = offset; }

      /*!
       * \brief The number of words is fixed at 65 for control buffers.
       * \return 65
       */
      std::size_t computeNWords() const { return 65; }

    }; // end of CControlBuffer

  } // namespace V8
} // namespace DAQ

#endif // DAQ_V8_CCONTROLBUFFER_H
