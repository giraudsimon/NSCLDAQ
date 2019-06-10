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

#ifndef NSCLDAQ8_CSCALERBUFFER_H
#define NSCLDAQ8_CSCALERBUFFER_H

#include <V8/CV8Buffer.h>
#include <V8/bheader.h>

#include <vector>
#include <cstdint>

namespace DAQ
{
  namespace V8
  {

    // fwd declaration
    class CRawBuffer;

    /*! \brief Scaler buffer type
     *
     * There are two scaler buffer types, SCALERBF and SNAPSCLBF. As far as 
     * I know, the latter is not actually in use. Besides the data for the header,
     * there are 3 other pieces of information that are contained in the scaler
     * buffer: start and end of time of counting interval and the scaler data. The
     * start and end time (m_offsetBegin and m_offsetEnd respectively) are in units
     * of seconds and reference the start of the run.
     */
    class CScalerBuffer : public CV8Buffer
    {
    private:
      bheader                    m_header;
      std::uint32_t              m_offsetBegin;
      std::uint32_t              m_offsetEnd;
      std::vector<std::uint32_t> m_scalers;

    public:
      /*! \brief Default constructor
       *
       * Like the other CV8Buffer derived classes, this is not actually intended to
       * produce an object that is usable. The object it creates should be made
       * usable through assignment or via the setter methods.
       *
       */
      CScalerBuffer();

      /*! \brief Construct from raw buffer
       *
       * This is for turning a buffer of data produced with an IO operation into a 
       * usable entity.
       *
       * The begin and end time of the counting interval are extracted and all of the
       * scaler values. Note that in doing so, all values are converted to native byte
       * ordering.
       *
       * \param rawBuffer   the data buffer
       */
      CScalerBuffer(const CRawBuffer& rawBuffer);

      /*! \brief Construct from data
       *
       * \param header      the bheader
       * \param offsetBegin start of counting interval (seconds)
       * \param offsetEnd   end of counting interval (seconds)
       * \param scalers     scaler data
       *
       */
      CScalerBuffer(const bheader& header, std::uint32_t offsetBegin,
                    std::uint32_t offsetEnd,
                    const std::vector<std::uint32_t>& scalers);

      /*! \returns Header */
      bheader getHeader() const;

      BufferTypes type() const { return SCALERBF; }

      /*! \brief Serialize the data for IO purposes
       *
       * This method adds the data of this object into a byte array according
       * to the format of the version 8 scaler buffers.
       *
       * Format:
       * header         16 shorts
       * offset end     2  shorts
       * unused         3  shorts
       * offset begin   2 shorts
       * unused         3 shorts
       * scaler data    >=0 shorts
       *
       * \param buffer    the buffer to copy into
       */
      void toRawBuffer(CRawBuffer &buffer) const;

      //// Setters and getters //////
      void setOffsetBegin(std::uint32_t val) { m_offsetBegin = val; }
      std::uint32_t getOffsetBegin() const;

      void setOffsetEnd(std::uint32_t val) { m_offsetEnd = val; }
      std::uint32_t getOffsetEnd() const;

      std::vector<std::uint32_t> getScalers() const;
      std::vector<std::uint32_t>& getScalers() { return m_scalers; }

      void setScalers(const std::vector<std::uint32_t>& values) 
      { m_scalers = values; }

      /*! \brief Figure out the entire size of this scaler buffer in 16-bit units */
      std::size_t computeNWords() const;
    };

  } // end of V8
} // end of DAQ

#endif // CSCALERBUFFER_H
