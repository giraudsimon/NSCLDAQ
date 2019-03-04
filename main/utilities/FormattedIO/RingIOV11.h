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

#ifndef NSCLDAQ11_RINGIO_H
#define NSCLDAQ11_RINGIO_H

#include <iosfwd>

class CDataSource;
class CDataSink;

namespace DAQ {
  namespace V11 {
    class CRingItem;
  }
}

/*!
 * \brief Insert V11 CRingItem into a std::ostream
 *
 * \param stream  the stream
 * \param item    any V11 item
 *
 * \return the stream
 */
extern std::ostream& operator<<(std::ostream& stream,
                                const DAQ::V11::CRingItem& item);

/*!
 * \brief Insert V11 CRingItem into CDataSink
 *
 * \param stream  the sink
 * \param item    any V11 item
 * \return the sink
 */
extern CDataSink& operator<<(CDataSink& stream,
                             const DAQ::V11::CRingItem& item);

/*!
 * \brief Extract a V11 CRingItem from a std::istream
 *
 * \param stream  the stream
 * \param item    any V11 item
 *
 * \return the stream
 */
extern std::istream& operator>>(std::istream& stream,
                                DAQ::V11::CRingItem& item);

/*!
 * \brief Extract a V11 CRingItem from a CDataSource
 *
 * \param stream  the source
 * \param item    any V11 item
 *
 * \return the source
 */
extern CDataSource& operator>>(CDataSource& stream,
                               DAQ::V11::CRingItem& item);

#endif // RINGIO_H
