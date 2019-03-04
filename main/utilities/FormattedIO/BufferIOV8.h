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

#ifndef BUFFERIOV8_H
#define BUFFERIOV8_H

#include <iosfwd>

namespace DAQ {
  namespace V8 {
    class CRawBuffer;
  }
}

// Fwd declarations
class CDataSource;
class CDataSink;

/*!
 * \brief Extract V8 buffer from an std::istream
 *
 *  This reads in exactly gBufferSize bytes into the buffer. All prior information
 *  that was in the buffer is wiped out. Data is read into a new ByteBuffer and then
 *  CRawBuffer::setBuffer() is called with it as the argument.
 *
 * \param stream  to read from
 * \param buffer  buffer to fill
 * \return the stream
 */
extern std::istream& operator>>(std::istream& stream, DAQ::V8::CRawBuffer& buffer);

/*!
 * \brief Extract V8 buffer from a CDataSource
 *
 *  Same semantics as the istream version.
 *
 * \param stream  to read from
 * \param buffer  buffer to fill
 * \return the stream
 */
extern CDataSource& operator>>(CDataSource& stream, DAQ::V8::CRawBuffer& buffer);


/*!
 * \brief Insert V8 buffer into a std::ostream
 *
 *
 * \param stream  to insert data into
 * \param buffer  buffer containing data
 *
 * \return  the stream
 *
 * \throws std::runtime_error if the data size does not match gBfferSize
 */
extern std::ostream& operator<<(std::ostream& stream, const DAQ::V8::CRawBuffer& buffer);

/*! \brief Insert V8 buffer into a CDataSink
 *
 * Same semantics as the ostream version
 *
 * \param stream  to insert data into
 * \param buffer  buffer containing data
 *
 * \returns the stream

 * \throws std::runtime_error if the data size does not match gBfferSize
 */
extern CDataSink& operator<<(CDataSink& stream, const DAQ::V8::CRawBuffer& buffer);



#endif // BUFFERV8IO_H
