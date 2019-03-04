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

#ifndef NSCLDAQ10_RINGIO_H
#define NSCLDAQ10_RINGIO_H

#include <iosfwd>

class CDataSource;
class CDataSink;

namespace DAQ
{
  namespace V10 {
    class CRingItem;
  }
}

/*!
 * \brief Insert V10 CRingItem into a std::ostream
 *
 * This simply sends the entire item managed by the CRingItem to the
 * stream.
 *
 * \param stream  to send data to
 * \param item    any V10 ring item
 * \return the stream
 */
extern std::ostream& operator<<(std::ostream& stream,
                                const DAQ::V10::CRingItem& item);

/*!
 * \brief Insert V10 CRingItem into a CDataSink
 *
 * \param stream  the stream to send data to
 * \param item    any V10 ring item
 *
 * \return  the stream
 */
extern CDataSink& operator<<(CDataSink& stream,
                             const DAQ::V10::CRingItem& item);


/*!
 * \brief Extract a V10 CRingItem from the std::istream
 *
 * \param stream  the stream
 * \param item    a V10 item
 * \return the stream
 */
extern std::istream& operator>>(std::istream& stream,
                                DAQ::V10::CRingItem& item);

/*!
 * \brief Extract a V10 CRingItem from a CDataSource
 *
 * \param stream  the stream
 * \param item    a V10 item
 * \return the stream
 */
extern CDataSource& operator>>(CDataSource& stream,
                               DAQ::V10::CRingItem& item);

#endif // RINGIO_H
